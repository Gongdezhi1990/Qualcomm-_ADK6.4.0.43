############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2016 - 2019 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
"""
This module enables ACAT to recognise downloadable capabilities.
"""
import json
import logging
import os
import sys

from ACAT.Core import Arch
from ACAT.Core.exceptions import DebugInfoNoVariableError
from ACAT.Core.logger import method_logger

try:
    from future_builtins import hex
except ImportError:
    pass

logger = logging.getLogger(__name__)


class MappedTable(object):
    """Maps global addresses to downloadable capability's local ones.

    This class defines a table of mapped addresses used to convert global
    addresses of debuginfo and chipdata to local addresses of downloaded
    capabilities. Full protocol is described in CS-340329-SP.

    Args:
        chipdata
        debuginfo
    """

    def __init__(self, chipdata, debuginfo):
        self.chipdata = chipdata
        try:
            self.ptr_table_address = debuginfo.get_var_strict(
                '$___kymera_debug_map_addr'
            ).address
        except DebugInfoNoVariableError:
            self.ptr_table_address = None
        self.table_address = 0
        self.identify_field = 0
        self.num_of_entries = 0
        self.entries = []
        self.file_id_dict = {}
        self.checksum = 0

    def __str__(self):
        # update the table before printing.
        self._update_table()
        out_str = '\nIdentify:  {0}  Entries: {1}\n'.format(
            hex(self.identify_field), self.num_of_entries
        )
        out_str += '{0:<20}{1:<10}{2:<15}{3:<15}\n'.format(
            'Device address', 'length', 'file address', 'file elf_id'
        )
        for entry in self.entries:
            out_str += str(entry)
        out_str += 'Checksum: {0}\n'.format(hex(self.checksum))
        return out_str

    def _get_checksum(self):
        """Calculates the checksum.

        Returns:
            int: The checksum value.
        """
        checksum_addr = self.table_address + 2 * Arch.addr_per_word
        checksum_addr += self.num_of_entries * 4 * Arch.addr_per_word
        return self.chipdata.get_data(checksum_addr)

    def _update_table(self):
        """Internal method used for updatign the capability table."""
        if not self.ptr_table_address:
            return

        new_table_address = self.chipdata.get_data(self.ptr_table_address)
        if new_table_address != 0:
            self.table_address = new_table_address
            new_identify_field = self.chipdata.get_data(self.table_address)

            if new_identify_field != 0:
                new_num_of_entries = self.chipdata.get_data(
                    self.table_address + Arch.addr_per_word
                )
            else:
                new_num_of_entries = 0
        else:
            new_identify_field = self.identify_field
            new_num_of_entries = self.num_of_entries

        checksum = self._get_checksum()

        if (self.table_address != new_table_address or
                self.identify_field != new_identify_field or
                self.num_of_entries != new_num_of_entries or
                checksum != self.checksum):

            self.identify_field = new_identify_field
            self.num_of_entries = new_num_of_entries

            self.entries = []
            self.file_id_dict = {}
            curr_address = self.table_address + 2 * Arch.addr_per_word
            # Start of the entries to the table
            for _ in range(self.num_of_entries):
                data_vals = list(
                    self.chipdata.get_data(
                        curr_address, 4 * Arch.addr_per_word
                    )
                )  # Read 4 words
                # When pointing in the PM region, bit 31 is a flag that always
                # set. However after kerelfreader reads downloaded capability
                # data, this flag is removed, thus we mask it here
                # before adding new entry to a table
                try:
                    Arch.get_pm_region(data_vals[0])
                    data_vals[2] &= 0x7FFFFFFF
                except Arch.NotPmRegion:
                    # StandardError occurred thus it means that we are not in
                    # PM region and masking is not required
                    pass
                entry = MappedTableEntry(
                    data_vals[0], data_vals[1], data_vals[2], data_vals[3]
                )

                if entry.file_id not in list(self.file_id_dict.keys()):
                    self.file_id_dict[entry.file_id] = []
                    # Try to load the elf id by searching the path
                    # in the file "elf_id_mapped_to_elf_path.json"
                    self.load_bundle_from_internal_builds(entry.file_id)

                self.file_id_dict[entry.file_id].append(entry)
                self.entries.append(entry)
                curr_address += 4 * Arch.addr_per_word

            self.checksum = checksum

    @method_logger(logger)
    def convert_addr_to_build(self, addr_in_file, cap_elf_id):
        """
        Converts local downloaded capability's address to chipdata's.

        If conversion is not valid according to the table, None is
        returned.

        Args:
            addr_in_file: address we want to convert.
            cap_elf_id (int): id of capability we want to map.
        """
        self._update_table()
        entry = self.get_table_entry(addr_in_file, cap_elf_id)
        if entry is None:
            raise ValueError("Address " + hex(addr_in_file) +
                             " is not in any downloadable capability.")
        else:
            offset = addr_in_file - entry.file_address
            device_address = entry.device_address + offset
            return device_address

    @method_logger(logger)
    def convert_addr_to_download(self, addr_in_build, cap_elf_id):
        """Converts chipdata's address to downloaded capability's.

        If conversion is not valid according to the table, None is
        returned.

        Args:
            addr_in_file: address we want to convert.
            cap_elf_id (int): id of capability we want to map.
        """
        self._update_table()
        entry = self.get_table_entry(addr_in_build, cap_elf_id, False)
        if entry is None:
            raise ValueError(
                "Address " +
                hex(addr_in_build) +
                " is not in any downloadable capability.")
        else:
            offset = addr_in_build - entry.device_address
            file_address = entry.file_address + offset
            return file_address

    @method_logger(logger)
    def get_table_entry(self, addr, cap_elf_id, addr_in_file_given=True):
        """Returns the table entry for conversion.

        By default it assumes that local address of downloaded capability
        is provided.

        Args:
            addr
            cap_elf_id
            addr_in_file_given (bool, optional)
        """
        self._update_table()
        # Returns entry if address is valid and None if not
        if cap_elf_id not in list(self.file_id_dict.keys()):
            # no elf_id known, complain
            raise ValueError(
                'Given capability elf_id is not found in the table'
            )
        table_entries = self.file_id_dict[cap_elf_id]
        if addr_in_file_given:
            addr_in_file = addr
            for entry in table_entries:
                entry_start = entry.file_address
                entry_end = entry_start + entry.length
                if addr_in_file >= entry_start and addr_in_file < entry_end:
                    return entry
        else:
            addr_in_build = addr
            for entry in table_entries:
                entry_start = entry.device_address
                entry_end = entry_start + entry.length
                if addr_in_build >= entry_start and addr_in_build < entry_end:
                    return entry
        return None

    @method_logger(logger)
    def get_elf_id_from_address(self, addr):
        """Gets the elf_id from an address.

        Given the address it receives the capability elf file id, to which
        that address maps to  address provided must be a global address in
        chipdata.

        Args:
            addr
        """
        self._update_table()
        for identifier, entries in self.file_id_dict.items():
            for entry in entries:
                if (addr >= entry.device_address and
                        addr < entry.device_address + entry.length):
                    return identifier
        return None

    @method_logger(logger)
    def is_addr_in_table(self, addr):
        """Checks whether a global address mapped to the table or not.

        Args:
            addr: Address to check against.
        """
        self._update_table()
        return self.get_elf_id_from_address(addr) is not None

    @method_logger(logger)
    def load_bundle_from_internal_builds(self, cap_elf_id):
        """Loads a bundle from internal builds.

        Method used to load an elf file by search the path in the
        "elf_id_mapped_to_elf_path.json" file.

        Args:
            cap_elf_id
        """
        # Avoid loading a bundle multiple times
        if cap_elf_id in self.chipdata.debuginfo.debug_infos:
            return

        # import here to avoid circular dependency
        from ACAT.Core.MulticoreSupport import load_bundle

        # Load the elf by searching the path "elf_id_mapped_to_elf_path.json"
        if sys.platform == "win32":
            elf_paths = r"\\root.pri\fileroot\UnixHomes\home\svc-audio-dspsw"
        else:
            elf_paths = r"/home/svc-audio-dspsw"

        try:
            with open(
                os.path.join(elf_paths, "elf_id_mapped_to_elf_path.json"),
                "r"
            ) as handler:
                ids_to_paths = json.load(handler)
                if str(cap_elf_id) in ids_to_paths.keys():
                    # For each elf id we provide first the windows and then
                    # the unix path
                    if sys.platform == "win32":
                        self.chipdata.debuginfo.update_bundles(
                            load_bundle(ids_to_paths[str(cap_elf_id)][0])
                        )
                    else:
                        self.chipdata.debuginfo.update_bundles(
                            load_bundle(ids_to_paths[str(cap_elf_id)][1])
                        )
        except IOError:
            pass


class MappedTableEntry(object):
    """Encapsulates the information of a MappedTable entry.

    Args:
        device_address
        length
        file_address
        file_id
    """

    def __init__(self, device_address, length, file_address, file_id):
        self.device_address = device_address
        self.length = length
        self.file_address = file_address
        self.file_id = file_id

    def __str__(self):
        out_str = '{0:<20}{1:<10}{2:<15}{3:<15}\n'.format(
            hex(self.device_address), self.length,
            hex(self.file_address), hex(self.file_id)
        )
        return out_str
