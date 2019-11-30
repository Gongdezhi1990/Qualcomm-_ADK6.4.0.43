############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2012 - 2019 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
"""
Base module to connect to a live chip.
"""
import re
import sys
import traceback
import threading
from functools import wraps

from . import Arch
from . import ChipData
from ACAT.Core import CoreUtils as cu
from ACAT.Core.exceptions import OutOfRangeError, ChipNotPoweredError

try:
    from future_builtins import hex
except ImportError:
    pass
re_entrant_thread_lock = threading.RLock()


def re_entrant_lock_decorator(function):
    """Locks the threading before executing a function.

    Args:
        function: Function to decorate.
    """

    @wraps(function)
    def _new_method(self, *arg, **kws):
        """
        This will be the newly generated method.
        """
        try:
            re_entrant_thread_lock.acquire()
            # call the actual function.
            return function(self, *arg, **kws)
        finally:
            re_entrant_thread_lock.release()

    return _new_method


##################################################
class LiveChip(ChipData.ChipData):
    """Provides access to chip data on a live chip over SPI."""
    # data memory
    dm = []
    # pm memroy
    pm = []
    # kalimba access
    kal = None

    def __init__(self):
        ChipData.ChipData.__init__(self)

    def is_volatile(self):
        """This is a live device over SPI so the contents is volatile."""
        return True

    @re_entrant_lock_decorator
    def get_data(self, address, length=0, ignore_overflow=False):
        """Returns the contents of one or more addresses.

        This allows you to grab a chunk of memory, e.g. get_data(0x600,
        50). Addresses can be from any valid DM region (DM1, PM RAM,
        mapped NVMEM, memory-mapped registers, etc.).

        Note:
            The length supplied is measured in addressable units.

                get_data(addr) will return a single value;
                get_data(addr, 1) will return a list with a single member.
                get_data(addr, 10) will return a list with ten members or
                    a list with three members (when the memory is octet
                    addressed, 32bit words).

        Note that reading PM RAM via DM is not supported (since not all chips
        map PM into DM). Use `get_data_pm()` instead.

        Args:
            address
            length
            ignore_overflow (bool, optional): Ignore if the read goes out
                from the memory region and append zeros. This is useful if
                an union is at the end of the memory region.

        Raises:
            KeyError: If the address is out of range.
            OutOfRangeError: If the address is valid but the length is not
                (i.e. address+length is not a valid address).

        """
        # We helpfully accept address as either a hex string or an integer
        try:
            # Try to convert address from a hex string to an integer
            address = int(address, 16)
        except TypeError:
            # Probably means address was already an integer
            pass
        address = cu.get_correct_addr(address, Arch.addr_per_word)

        if length == 0:
            mem_region = Arch.get_dm_region(address, False)
        else:
            # kalaccess will wrap the memory if the read goes out of boundary.
            # So to ignore overflow just get the memory region from the
            # beginning.
            if ignore_overflow:
                mem_region = Arch.get_dm_region(address)
            else:
                mem_region = Arch.get_dm_region(
                    address + cu.convert_byte_len_word(length, Arch.addr_per_word) -
                    Arch.addr_per_word
                )
        try:
            if ('DM' in mem_region or 'SLT' in mem_region or
                    'MMR' in mem_region or 'NVMEM' in mem_region or
                    'PMRAM' in mem_region or 'MCU' in mem_region):
                if length == 0:
                    return self.dm[address:address + Arch.addr_per_word][0]

                return tuple(
                    [
                        int(x)
                        for x in self.
                        dm[address:address +
                           cu.convert_byte_len_word(length, Arch.addr_per_word)]
                    ]
                )
            else:
                if length == 0:
                    raise KeyError(
                        "Key " + hex(address + Arch.addr_per_word) +
                        " is not a valid DM address",
                        address + Arch.addr_per_word)

                raise OutOfRangeError(
                    "Key " + hex(
                        address + cu.convert_byte_len_word(
                            length, Arch.addr_per_word
                        ) - Arch.addr_per_word
                    ) + " is not a valid DM address",
                    address + cu.convert_byte_len_word(
                        length, Arch.addr_per_word
                    ) - Arch.addr_per_word)
        except Exception as exception:
            if "Transport failure (Unable to read)" in exception:
                sys.stderr.write(str(exception))
                raise ChipNotPoweredError

            # Will also spit exceptions to log file, if set.
            sys.stderr.write(traceback.format_exc())

            raise exception

    @re_entrant_lock_decorator
    def get_data_pm(self, address, length=0):
        """Get data from PM.

        This method works in the exactly the same way as get_data(self,
        address, length = 0) method, but it returns data from PM region
        instead of DM.

        Args:
            address
            length (int, optional)
        """
        # We helpfully accept address as either a hex string or an integer
        try:
            # Try to convert address from a hex string to an integer
            address = int(address, 16)
        except TypeError:
            # Probably means address was already an integer
            pass

        address = cu.get_correct_addr(address, Arch.addr_per_word)

        if length == 0:
            mem_region = Arch.get_pm_region(address)
        else:
            mem_region = Arch.get_pm_region(
                address + cu.convert_byte_len_word(length, Arch.addr_per_word) -
                Arch.addr_per_word
            )
        try:
            if ('PMRAM' in mem_region or 'PMCACHE' in mem_region or
                    'SLT' in mem_region):
                if length == 0:
                    return self.pm[address:address + Arch.addr_per_word][0]

                return tuple(
                    [
                        int(x)
                        for x in self.
                        pm[address:address +
                           cu.convert_byte_len_word(length, Arch.addr_per_word)]
                    ]
                )
            else:
                if length == 0:
                    raise KeyError(
                        "Key " + hex(address + Arch.addr_per_word) +
                        " is not a valid PM RAM address",
                        address + Arch.addr_per_word)

                raise OutOfRangeError(
                    "Key " + hex(
                        address + cu.convert_byte_len_word(
                            length, Arch.addr_per_word
                        ) - Arch.addr_per_word
                    ) + " is not a valid PM RAM address",
                    address + cu.convert_byte_len_word(
                        length, Arch.addr_per_word
                        ) - Arch.addr_per_word)
        except Exception as exception:
            if "Transport failure (Unable to read)" in exception:
                sys.stderr.write(str(exception))
                raise ChipNotPoweredError
            else:
                raise exception

    @re_entrant_lock_decorator
    def get_proc_reg(self, name):
        """Return the value of the processor register specified in 'name'.

        `name` is a string containing the name of the register in upper or
        lower case, with or without the prefix 'REGFILE_' e.g.
        "REGFILE_PC", "rMAC", "R10".

        Args:
            name

        Raises:
            KeyError
            AttributeError
        """
        # Processor registers are available via KalSpi.<reg name>,
        # always lower case and without 'REGFILE_'.
        match = re.match('regfile_(.*)', name, re.IGNORECASE)
        if match is not None:
            name = match.groups()[0]
        return self.kal.reg.read_register(self.kal.reg.get_register_id(name))

    @re_entrant_lock_decorator
    def get_all_proc_regs(self):
        """Returns a dictionary containing all processor registers.

        Dictionary maps name to value.  NB on a running chip there is the
        obvious risk of tearing.  Consider stopping the chip if this is an
        issue.
        """
        # Using kal.reg.read_register_set() hopefully slightly reduces the
        # chance of tearing.
        names = self.kal.reg.get_register_names()
        reg_ids = [self.kal.reg.get_register_id(i) for i in names]
        vals = self.kal.reg.read_register_set(reg_ids)
        regs = {}
        for i, value in enumerate(vals):
            regs["REGFILE_" + names[i]] = value
        return regs

    @re_entrant_lock_decorator
    def set_data(self, address, value):
        """Sets the contents of one or more addresses.

        This allows you to write a list of values to a chunk of memory.
        Addresses can only be from any valid DM RAM or memory mapped
        register region.  e.g. set_data(0x3e5d, [1 2 3])

        Note:
            set_data(address, [val]) writes address with a single value val.
            set_data(address, [val1 ... valn]) writes the list of values
                to memory starting from address.

        This function should only be implemented for live chips. And should not
        be called if the chip is not live.


        Args:
            address
            value

        Raises:
            KeyError: If the address is out of range.
            OutOfRangeError: If the address is valid but the length is not
                (i.e. address+length is not a valid address).
        """
        # We helpfully accept address as either a hex string or an integer
        try:
            # Try to convert address from a hex string to an integer
            address = int(address, 16)
        except TypeError:
            # Probably means address was already an integer
            pass

        address = cu.get_correct_addr(address, Arch.addr_per_word)

        length = len(value)
        if length == 1:
            mem_region = Arch.get_dm_region(address)
        else:
            mem_region = Arch.get_dm_region(
                address +
                cu.convert_byte_len_word(
                    length,
                    Arch.addr_per_word) -
                Arch.addr_per_word)

        try:
            if 'DM' in mem_region or 'MMR' in mem_region:
                self.dm[address] = value
            else:
                if length == 1:
                    raise KeyError(
                        "Key " + hex(Arch.addr_per_word + address) +
                        " is not in a DM or PM region",
                        address + Arch.addr_per_word)
                else:
                    raise OutOfRangeError(
                        "Key " + hex(
                            address + cu.convert_byte_len_word(
                                length, Arch.addr_per_word
                            ) - Arch.addr_per_word
                        ) + " is not in a DM or PM region",
                        address + cu.convert_byte_len_word(
                            length, Arch.addr_per_word
                        ) - Arch.addr_per_word)
        except Exception as exception:
            if "Transport failure (Unable to read)" in exception:
                sys.stderr.write(str(exception))
                raise ChipNotPoweredError
            else:
                raise exception

    def get_firmware_id(self):
        """Returns the firmware ID integer."""
        build_id_addr = self._get_slt_entry(1)
        build_id_int = self.get_data(build_id_addr)
        if build_id_addr % Arch.addr_per_word != 0:
            # in a 32 bit word, the 16 bit build ID int can be in either the 2
            # MS bytes or 2 LS bytes
            build_id_int >>= 16
        return build_id_int & 0xFFFF

    def get_firmware_id_string(self):
        """Returns the firmware ID string."""
        unpacked_string = False
        # Find the address of the string in memory
        id_string_addr = self._get_slt_entry(3)
        if id_string_addr is None:
            # maybe it's not packed in this slt
            id_string_addr = self._get_slt_entry(2)
            if id_string_addr is None:
                # Can't find it in the slt return None
                return None
            unpacked_string = True
        # parse the null terminated string
        last = build_string = ""
        # There is no reason for the build string to contain
        # any non ASCII character but do it like this to avoid
        # breaking support for Python 2.7
        try:
            char = unichr
        except NameError:
            char = chr
        while last != "\0":
            word = self.get_data(id_string_addr)
            if unpacked_string:
                if Arch.addr_per_word == 4:
                    # Two char per word
                    build_string += char(word & 0x00FFFF)
                    if build_string[-1] == "\0":
                        break
                    last = char((word & 0xFFFF0000) >> 16)
                    build_string += last
                else:
                    # Only one char per word
                    last = char(word)
                    build_string += last
            else:
                # Four chars per word
                if Arch.addr_per_word == 4:
                    string = cu.get_string_from_word(Arch.addr_per_word, word)
                    stop_decoding = False
                    for char in string:
                        if char != '\0':
                            build_string += char
                        else:
                            stop_decoding = True
                            break
                    last = string[3:]

                    if stop_decoding:
                        break
                else:
                    # Two chars per word
                    build_string += char((word & 0xFF00) >> 8)
                    if build_string[-1] == "\0":
                        break
                    last = char(word & 0x00FF)
                    build_string += last
            # Move to the next word in the string
            id_string_addr += Arch.addr_per_word

        # remove the \0 we don't want the terminator
        if build_string[-1] == "\0":
            build_string = build_string[:-1]

        return build_string.strip()

    def _get_slt_entry(self, entry):
        """Navigates the slt looking for an entry value.

        Args:
            entry
        """
        # Slt ptr is the second word in the SLT region (first is fingerprint)
        # Note how we deliberately don't use any debug information here (e.g.
        # finding the location of $_audio_slt_table).
        if Arch.addr_per_word == 4:
            sltptr_addr = Arch.pRegions['SLT'][0] + Arch.addr_per_word
            slt_entry_addr = self.get_data_pm(sltptr_addr)  # index 0 of slt
        else:
            sltptr_addr = Arch.dRegions['SLT'][0] + Arch.addr_per_word
            slt_entry_addr = self.get_data(sltptr_addr)  # index 0 of slt
        # Run through the slt looking for the entry we want, if we can't find
        # the one we're looking for maybe it's not in the list
        while entry > self.get_data(slt_entry_addr):
            slt_entry_addr += 2 * Arch.addr_per_word

        if entry == self.get_data(slt_entry_addr):
            return self.get_data(slt_entry_addr + Arch.addr_per_word)

        return None
