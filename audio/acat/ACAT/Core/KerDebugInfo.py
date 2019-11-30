############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2012 - 2019 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
"""
KerDebugInfo creates an interface to kalelfreader.
"""
import logging
import math
import mmap
import numbers
import os
import re
import sys
from contextlib import contextmanager
from tempfile import NamedTemporaryFile

from . import Arch
from ACAT.Core import CoreTypes as ct
from ACAT.Core import CoreUtils as cu
from ACAT.Core import DebugInfo as di
from ACAT.Core.compression import is_zip, extract
from ACAT.Core.exceptions import (
    DebugInfoNoVariableError, DebugInfoNoLabelError,
    InvalidPmAddressError, UnknownPmEncodingError,
    InvalidDebuginfoCallError, InvalidDebuginfoTypeError,
    InvalidDebuginfoEnumError, InvalidDmConstAddressError,
    InvalidDmConstLengthError
)
from ACAT.Core.logger import method_logger

try:
    from future_builtins import hex
except ImportError:
    pass

##################################################
# This class gets debug information from kalelfreader and appends it to
# our records

# private section flag
PRIV_SECTION_FLAG = 0x20000000
logger = logging.getLogger(__name__)


def section_to_mapped_dict(sections, strip_addr=False):
    """Converts sections to a mapped dictionary.

    Function that converts tuple describing a section to a dictionary
    which maps addresses to values. The tuple fields are (bit_width,
    byte_addressing, start_addr, num_bytes, data).

    Args:
        sections
        strip_addr (bool, optional)
    """
    return_dic = {}
    for (_, byte_addressing, start_addr, _, data) in sections:
        # We store each word as 4 bytes in the Elf file, regardless of
        # whether it is 32-bit, 24-bit or 16-bit
        if byte_addressing != 0:
            inc = 4
        else:
            inc = 1
        current_addr = start_addr
        for value in data:
            if strip_addr and Arch.addr_per_word != 4:
                current_addr = cu.strip_elf_addr(current_addr, Arch.kal_arch)
            return_dic[current_addr] = value
            current_addr += inc
    return return_dic


def variable_dict_search(dictionary, name):
    """Finds a variable name in a variable dictionary.

    Searches in a dictionary full with variables mapped by names for a
    specific name.

    Args:
        dictionary (dict): Dictionary full of variables mapped by names.
        name (name): Name to search for.

    Returns:
        list: The returning list can be explained as:
            * If the list has zero entries, the name could not be found in
              the keys.
            * If the list has a single entry, that's your key.
            * If the list has multiple entries, a number of potential
              matches were found.
    """
    results = []

    # First, does it match exactly?
    if name in dictionary:
        results.append(name)
        # Bingo
        return results

    # Remove any special characters from the beginning.
    # This is useful to get the root name (without $ or $_) of a variable.
    basematch = re.match(r"\$?_?(.+)", name)
    if basematch:
        base_name = basematch.group(1)
    else:
        # An error: bail out now.
        return results

    # No. So build up a list of common variations to try.
    # This helps to hone in on the correct name before we do a brute-force
    # search of every symbol name. For a name of 'doloop_start' there
    # won't be many spurious matches, but looking for 'pc' will find
    # hundreds of PCM registers which probably aren't what you want.  Keep
    # the variations list small, though, because it may mask out valid
    # matches. (example: 'lpc_status' will hit the variation
    # '$LPC_STATUS', and hence won't find
    # $LPC_STATUS_DEEP_SLEEP_ACTIVE_MASK).
    variations = ['$' + base_name, '$_' + base_name, "L_" + base_name]
    if base_name.upper() != base_name:
        variations += ['$' + base_name.upper(), '$_' + base_name.upper()]

    for variation in variations:
        if variation in dictionary:
            results.append(variation)

    if results:
        # Return now, so we don't pollute the existing matches with
        # others that are less-likely to be correct.
        return results

    # Still haven't found anything (or we are in dumb mode).
    # So tediously go through all names in the dictionary looking for a
    # match.use re.escape() so that a name with (e.g.) a '.' doesn't
    # match regexp '.'
    # The regex must be compiled and the "match" function should be
    # pre-loaded to avoid accessing it for each key.
    name_match = re.compile(".*" + re.escape(base_name) + ".*", re.I).match
    for k in dictionary.keys():
        if name_match(k):
            results.append(k)

    return results


class KerDebugInfo(di.DebugInfoInterface):
    """Gets debug information from kalelfreader.

    Args:
        build_output_path: Path to the elf file.
    """

    def __init__(self, build_output_path):
        super(KerDebugInfo, self).__init__()
        # Set default values to instance variables.
        self._elf_modified_time = None

        self.cap_data_type = None
        self.constants = None
        self.debug_strings = None
        self.debug_strings_data = None
        self.debuginfo_is_bundle = False
        self.dm_byte_width = None
        self.dm_const = None
        self.dm_word_width = None
        self.elf_id = None
        self.enums = None
        self.labels = None
        self.mmap_lst = None
        self.pm_instructions = None
        self.pm_rom = None
        self.pm_rom_by_name = None
        self.pm_section_hdrs = None
        self.section_hdrs = None
        self.types = None
        self.var_by_addr = None
        self.var_by_name = None

        # Remove the elf file extension if present.
        if build_output_path.endswith(('.elf', '.zip')):
            self.debuginfo_path, extension = os.path.splitext(build_output_path)
        else:
            extension = '.elf'
            self.debuginfo_path = build_output_path

        # Locate build output
        self.lst_path = os.path.normpath(self.debuginfo_path + '.lst')
        self.elf_path = os.path.normpath(self.debuginfo_path + extension)

    def get_constants_variables(self):
        """Return all the constants and variables in a single dictionary.

        Returns:
            dict
        """
        result = self.constants.copy()
        result.update(self.var_by_name)

        return result

    @method_logger(logger)
    def has_changed(self):
        """Check whether the external elf file has changed.

        Returns:
            bool: Indicates whether the target elf file is changed.
        """
        return os.path.getmtime(self.elf_path) != self._elf_modified_time

    def _read_debug_strings(self, kal_elf_reader):
        """Reads the debug strings.

        It reads them from the elf file using the Kalelfreader object.
        Helper method for read_debuginfo.

        Args:
            kal_elf_reader: Kalelfreader object.
        """
        try:
            # returns a tuple of the form
            # (bit_width, byte_addressing = 1, start_addr, num_bytes, data)
            self.debug_strings_data = \
                kal_elf_reader.get_named_non_allocable_section_data(
                    "debug_strings")
        except BaseException:
            # this should be KerErrorSectionDataNotFound but we don't import
            # all the kalelfreader just for this error.
            self.debug_strings_data = None
            logger.info("No debug strings in the build.")

    def _read_rom(self, kal_elf_reader):
        """Reads the pm rom statements from the elf file.

        Uses the Kalelfreader object. Helper method for read_debuginfo.

        Args:
            kal_elf_reader: Kalelfreader object.
        """
        # Now create our own pm_rom dictionary. This contains all valid
        # *code statements* (not instructions - those are in pm data and
        # will go into self.pm_instructions).  What we'd like to do is
        # initialise each SourceInfo object with its nearest label, but
        # that's quite slow.
        self.pm_rom = {}
        for addr, srcline in kal_elf_reader.get_statements().items():
            # (address, module_name, src_file, line_number):
            self.pm_rom[addr] = ct.SourceInfo(
                addr, srcline[0], srcline[1], srcline[2]
            )
        # Each line of the function is in a separate pm_rom module
        # so to remove duplication use a dictionary
        self.pm_rom_by_name = {}
        for label in self.pm_rom.values():
            self.pm_rom_by_name[label.module_name] = label.address

    def _read_variables(self, kal_elf_reader):
        """Reads variables from an elf file.

        Uses the Kalelfreader object from Python Tools library.

        Args:
            kal_elf_reader (:obj:`Kalelfreader`)
        """
        # Create two dictionaries to store variables.
        # Each variable is a Variable object, and we support searching by
        # both address and name.
        self.var_by_name = {}
        self.var_by_addr = {}

        for var_name, var in sorted(kal_elf_reader.get_variables().items()):
            # New Variable object.
            # If we somehow get a size of 0, interpret it as 1 instead. TODO
            newvar_size = var[0] if var[0] > 0 else 1
            #             name, address, size, value = None, type
            newvar = ct.Variable(
                name=var_name,
                address=var[1],
                size=newvar_size,
                value=None,
                var_type=var[2],
                debuginfo=self
            )

            if var[1] in self.var_by_addr:
                # There was already an entry for this address!
                # This can happen sometimes.
                if (re.search('.__Limit', var_name) is not None or
                        re.search('.__Base', var_name) is not None):
                    # Add to the 'by name' list but not the
                    # 'by var_name' list; see below.
                    self.var_by_name[var_name] = newvar
                    continue

                # If two symbols have the same address then
                # they are either simple aliases
                # (rare), or one is parent of the other.
                existing_entry = self.var_by_addr[var[1]]
                if newvar_size == existing_entry.size:
                    # Modify the existing Variable, and add a
                    # reference to it under its new alias.
                    self.var_by_addr[var[1]].name += (" or " + var_name)
                    self.var_by_name[var_name] = self.var_by_addr[var[1]]
                else:
                    # Create separate entries for the two variables,
                    # and only reference
                    # the largest one in the by_addr dictionary.
                    self.var_by_name[var_name] = newvar

                    if newvar_size > existing_entry.size:
                        self.var_by_addr[var[1]] = newvar
            else:
                # Create a new entry. Both dictionaries reference the same
                # object.
                self.var_by_name[var_name] = newvar

                # Never add metasymbols to the 'by var_name' list. They
                # will have an unhelpful size/type so we don't want to
                # consider them in the 'same entry at this address' code
                # above.
                if (re.search('.__Limit', var_name) is None and
                        re.search('.__Base', var_name) is None):
                    self.var_by_addr[var[1]] = newvar

    def _read_section_hdrs(self, kal_elf_reader):
        """Reads section headers from an elf file.

        Uses the Kalelfreader object. Helper method for read_debuginfo.

        Args:
            kal_elf_reader (:obj:`Kalelfreader`)
        """
        # .elf section headers (used to work out whether code is minim or
        # maxim) Dict of tuples (VMA start_addr, [LMA start addr, ]
        # num_octets, type) or a dict of KerElfSectionHeaderInfo objects
        # (after kalimbalab23). Key is section name.  VMA and LMA are
        # Virtual and Load Memory Addresses for the sections. Older
        # versions of kalelfreader only returned the VMA; newer versions
        # return both.
        # Type meanings are
        #   0:unknown,
        #   1:Maxim code,
        #   2:Minim code,
        #   3:data
        # We could use the section header info as a more accurate version
        # of Arch.get_[d|p]m_region, if we wanted.
        self.section_hdrs = kal_elf_reader.get_elf_section_headers()

        # Construct a list which excludes DM sections.
        self.pm_section_hdrs = {}

        for name, info in self.section_hdrs.items():
            self.pm_section_hdrs[name] = (
                cu.strip_elf_addr(info.address, Arch.kal_arch),
                info.num_bytes,
                info.type,
                info.flags
            )

    def _read_debuginfo(self):
        """Reads and stores the debug-information.

        Core method called by the public interface read_debuginfo.
        """
        with self._get_kal_elf_reader() as kal_elf_reader:
            # Elf Reader initialization was successful. Recording the
            # creation time of the elf file.
            self._elf_modified_time = os.path.getmtime(self.elf_path)

            try:
                logger.debug(
                    'ELF reader version: %s', kal_elf_reader.get_version()
                )

            except AttributeError:
                raise Exception("The Pythontools version is too old.")

            self._read_debug_strings(kal_elf_reader)
            # Convert constants into a dictionary, so they can be easily
            # looked-up by name
            self.constants = kal_elf_reader.get_constants()

            self._read_rom(kal_elf_reader)

            self._read_variables(kal_elf_reader)

            # Sort the list of labels. This makes get_nearest_label more
            # efficient
            self.labels = sorted(
                kal_elf_reader.get_labels(), key=lambda x: x[1]
            )

            # save the dm data constants
            self.dm_const = section_to_mapped_dict(
                kal_elf_reader.get_dm_data(), True
            )

            # Every instruction in PM (RAM or ROM)
            self.pm_instructions = section_to_mapped_dict(
                kal_elf_reader.get_pm_data(),
                False
            )

            # get_architecture returns (dsp_rev, addr_width, dm_data_width,
            # pm_data_width,  dm_byte_addressing, pm_byte_addressing)
            (_, _, dm_data_width, _, _, _) = kal_elf_reader.get_architecture()
            # How many bits in one word?
            self.dm_word_width = dm_data_width
            # KalimbaLab doesn't tell you this yet, but we need to know.
            # For now, we can guess based on dm_word_width (assume that 32-bit
            # Kalimbas - and only 32-bit Kalimbas - are octet-addressed).
            if self.dm_word_width == 32:
                self.dm_byte_width = 8
            elif self.dm_word_width == 24:
                self.dm_byte_width = 24
            else:
                raise NotImplementedError()

            # Save the type information.
            # This is explained in get_type_info().
            self.types = kal_elf_reader.get_types()

            # Save the enum values
            self.enums = kal_elf_reader.get_enums()

            self._read_section_hdrs(kal_elf_reader)
            # get the elf id from the debug information.
            try:
                elf_id_var = self.var_by_name["__devtools_image_checksum"]
                elf_id_address = elf_id_var.address
                self.elf_id = self.dm_const[elf_id_address]
            except KeyError:
                # dowloadable capability not installed
                self.elf_id = 0

            self._acquire_debug_strings()
            self._read_cap_extra_op_data()

    @method_logger(logger)
    def read_debuginfo(self):
        """Reads and stores the debug-information.

        Raises:
            NoFileLoadedError: If the elf file is unreadable.
        """
        # try to read the debug information and give more information on the
        # errors.
        try:
            self._read_debuginfo()

        except cu.global_options.ker.NoFileLoadedError:
            sys.stderr.write("Cannot read %s!\n\n" % self.elf_path)
            sys.stderr.flush()
            raise

    ##################################################
    # DebugInfoInterface
    ##################################################

    @method_logger(logger)
    def get_elf_id(self):
        """Returns the elf_id.

        Returns:
            int: The class' elf id.
            None: The class' elf id may not be initialized.
        """
        return self.elf_id

    @method_logger(logger)
    def get_cap_data_type(self, cap_name, elf_id=None):
        """Returns the data type name for a given capability.

        Note:
            The data type is used to hold all the information for a
            capability.

        Args:
            cap_name (str): Capability name.
            elf_id (int, optional)

        Returns:
            The name of the extra data type for the given capability.
            Returns None if no information found.
        """
        if elf_id is not None and self.elf_id != elf_id:
            raise InvalidDebuginfoCallError()

        try:
            return self.cap_data_type[cap_name]
        except KeyError:
            return None

    @method_logger(logger)
    def get_constant_strict(self, name, elf_id=None):
        """Return value is a ConstSym object (which may be None).

        Args:
            name (str): Name of the constant.
            elf_id (int, optional): Used to check if the right debuginfo
                is called.

        Raises:
            KeyError: If 'name' is not the exact name of a known constant.
        """
        if elf_id is not None and self.elf_id != elf_id:
            raise InvalidDebuginfoCallError()

        try:
            return ct.ConstSym(name, self.constants[name])
        except KeyError:
            raise DebugInfoNoVariableError()

    @method_logger(logger)
    def get_var_strict(self, identifier, elf_id=None):
        """Searchs the list of variables for the supplied identifier.

        Args:
            identifier: This will be interpreted as an address if it is an
                integer, and a name otherwise.
            elf_id (int) Used to check if the right debuginfo is called.

        Raises:
            InvalidDebuginfoCallError: When the elf_id is not specified
                and the its value is different from `self.elf_id`.
            DebugInfoNoVariableError:  When the variable can not be found.
            KeyError: If `identifier` is not the exact name or start address
                of a known variable.
        """
        if elf_id is not None and self.elf_id != elf_id:
            raise InvalidDebuginfoCallError()

        try:
            if isinstance(identifier, numbers.Integral):
                variable_name = hex(identifier)
                retval = self.var_by_addr[identifier]
            else:
                variable_name = identifier
                retval = self.var_by_name[identifier]
        except KeyError:
            raise DebugInfoNoVariableError(
                "Cannot find variables %s" % variable_name
            )
        return retval

    @method_logger(logger)
    def get_dm_const(self, address, length=0, elf_id=None):
        """Get DM constant.

        Args:
            address
            length (int)
            elf_id (int, optional): If provided, this will be used to
                check if the right debuginfo is called.

        Raises:
            InvalidDebuginfoCallError: When the elf_id is not specified
                and the its value is different from `self.elf_id`.
            KeyError: If the address is out of range.
            OutOfRangeError: If the address is valid but the length is not
                (i.e.  address+length is not a valid address).
        """
        if elf_id is not None and self.elf_id != elf_id:
            raise InvalidDebuginfoCallError()

        if Arch.addr_per_word != 4:
            address = cu.strip_elf_addr(address, Arch.kal_arch)
        address = cu.get_correct_addr(address, Arch.addr_per_word)
        # By default, return a single value.
        if length == 0:
            if address not in self.dm_const:
                raise InvalidDmConstAddressError(
                    "No DM constant at " + hex(address)
                )
            return self.dm_const[address]

        length = cu.convert_byte_len_word(length, Arch.addr_per_word)
        # Otherwise, return a list
        ret_val = []
        for i in range(0, length, Arch.addr_per_word):
            if address + i in self.dm_const:
                ret_val.append(self.dm_const[address + i])
            else:
                if i == 0:
                    raise InvalidDmConstAddressError(
                        "No DM constant at " + hex(address)
                    )
                raise InvalidDmConstLengthError(i, address)

        return ret_val

    @method_logger(logger)
    def get_source_info(self, address):
        """Gets information about a code-address module-name.

        Args:
            address: This can be either a code address (int) or a module
                name (str).

        Returns:
            Return value is a SourceInfo object.

        Raises:
            DebugInfoNoLabelError if the identifier is not valid.
        """
        # Crescendo has only even addresses for this part, so correct any
        # address that is odd.
        if Arch.addr_per_word == 4:
            div = address % 2
            if div != 0:
                address -= div

        nearest_label = self.get_nearest_label(address)

        if address in self.pm_rom:
            self.pm_rom[address].nearest_label = nearest_label
            return self.pm_rom[address]

        if nearest_label is None:
            # this is probably a private pm address
            raise DebugInfoNoLabelError()

        try:
            label_source_info = self.pm_rom[nearest_label.address]
            src_file = label_source_info.src_file
            module_name = label_source_info.module_name
            line_number = label_source_info.line_number
        except KeyError:
            src_file = "Unknown file (symbol may be censored)"
            module_name = nearest_label.name
            line_number = 0

        return_sym = ct.SourceInfo(
            address, module_name, src_file, line_number
        )
        return_sym.nearest_label = None
        return return_sym

    @method_logger(logger)
    def get_nearest_label(self, address):
        """Finds the nearest label to the supplied code address.

        Args:
            address

        Returns:
            value is a CodeLabel object.
        """
        # We go through the list of labels looking for the closest one.
        # NB this will only return a single result; it's possible (albeit
        # unlikely) that an address has two equivalent labels. For now ignore
        # that corner-case.
        return self._find_nearest_label(address)

    @method_logger(logger)
    def get_instruction(self, address):
        """Return the contents of Program Memory at the supplied address.

        The width of the return value depends on whether the instruction is
        encoded as Minim (16-bit) or Maxim (32-bit).

        Args:
            address

        Raises:
            KeyError: If the address is not in PM RAM/ROM, or the address
                is not the start of an instruction.
        """
        # self.pm_instructions (as derived from KalElfReader) assumes every
        # instruction is Maxim, i.e. 32 bits. For KAL_ARCH_3 chips, this is
        # true. For all other chips our PM is octet-addressed, so the keys in
        # self. pm_instructions are all divisible by four. That means
        # if we're actually looking for a Minim-encoded instruction at an
        # address like 0x2202, we'll need to get the entry for 0x2200 and
        # take the MS 16-bits.
        # Why the MS 16 bits? If you have two minim instructions packed
        # together, they are for some reason packed out-of-order; i.e. value
        # 0x12345678 contains the instructions 0x5678 and then 0x1234.
        # AND ANOTHER THING. self.pm_instructions is otherwise stored
        # little-endian, as it appears in PM. For some reason, our .lst files
        # give everything in big-endian*, so don't get confused.
        # For example:
        #     Maxim
        #   listing file says:          81401134:    00 00 20 f3     push r0;
        #   pm_instructions[0x401134]:  0xf3200000
        #   Actual instruction:         0xf3200000
        #
        #     Minim
        #   listing file says:          814082a4:    61 f0 2d ea     call (m)
        # $base_op.are_enough_terminals_connected;
        #   pm_instructions[0x4082a4]:  0xea2df061
        #   Actual instructions:        0xf061, 0xea2d [prefix then call instr]
        #
        #   listing file says:
        #                               814082fc:    14 0a      I4 = r0 + Null;
        #                               814082fe:    1e 0b      L4 = r1 + Null;
        #   pm_instructions[0x4082fc]:  0x0b1e0a14
        #   Actual instructions:        0x0a14, 0x0b1e [two instructions]
        #
        # * That's true for Amber, anyway. Gordon .lst files actually are
        #   little-endian.  But since Gordon is word-addressed, we don't
        #   care. Maybe by this stage neither do you.
        try:
            if not self.is_maxim_pm_encoding(address):
                # We're looking for a Minim instruction.
                if address % 4 == 0:
                    # We want the LS 16 bits of the PM value
                    instruction = self.pm_instructions[address]
                    instruction = instruction & 0x0000ffff
                else:
                    instruction = self.pm_instructions[address - 2]
                    instruction = (instruction & 0xffff0000) >> 16
            else:
                # Maxim instruction: easy!
                instruction = self.pm_instructions[address]
        except (KeyError, UnknownPmEncodingError):
            raise InvalidPmAddressError(
                "Cannot find 0x%08x in pm instructions" % (address)
            )

        return instruction

    @method_logger(logger)
    def get_enum(self, enum_name, member=None, elf_id=None):
        """Get Enum.

        If supplied just with a name, returns a dictionary mapping member
        name -> value. If also supplied with a member name/value, returns
        a tuple containing any matching values/names.

        e.g.
            get_enum('ACCMD_CON_TYPE')
            Returns a dictionary of all enum members.

            get_enum('ACCMD_CON_TYPE', 'ACCMD_CON_TEST')
            Returns value of ACCMD_CON_TEST member (1 in this case).

            get_enum('ACCMD_CON_TYPE', 1)
            Returns the name(s) of any entries with value 1
            ('ACCMD_CON_TEST' in this case).

        If either enum_name or member is not found, a KeyError exception
        will be raised.

        Args:
            enum_name (str)
            member (int)
            elf_id (int, optional): If supplied, will be used to check if
                the right debuginfo is called.
        """
        if elf_id is not None and self.elf_id != elf_id:
            raise InvalidDebuginfoCallError()

        if enum_name not in self.enums:
            raise InvalidDebuginfoEnumError(
                "Enum %s not in debug" % enum_name
                )

        if member is None:
            return self.enums[enum_name]

        enum = self.enums[enum_name]
        if isinstance(member, numbers.Integral):
            # Search by value. Could be multiple entries with the same value!
            names = []
            for name, val in enum.items():
                if val == member:
                    names.append(name)
            if names:
                return tuple(names)
            else:
                raise KeyError(
                    'Invalid enum value ' + str(member) +
                    ' for enum ' + enum_name
                )
        else:
            # enum is keyed by name.
            return enum[member]

    def type_has_union(self, type_id):
        """Checks if the typed with type_id has an union in it.

        Args:
            type_id
        """
        # Internal type
        itype = self.types[type_id]
        if itype.form == 2:
            return True
        for mem in itype.members:
            if self.type_has_union(mem.type_id):
                return True
        return False

    def get_type_info(self, type_name_or_id, elf_id=None):
        """Looks up information in the type database.

        Takes a type name (e.g. 'ENDPOINT' or 'audio_buf_handle_struc') or
        a valid typeId, and looks up information in the type database.
        Returns a tuple containing:

            * The (fully-qualified) name of the type.
            * The typeid (redundant if 'type' is already a typeid).
            * The pointed-to typeid (if the type is a pointer).
            * The array length (if the type or pointed-to type is an array).
            * The typeid which describes any members of this type (if it
              is a structure or union).
            * Type size in addressable units (if the type or pointed-to
              type is a structure or union).
            * The elf id of this debug information.

        Note:

            Unfortunately, a small number of types are defined as an
            array, but have a length of 0. That means to determine whether
            or not the type is an array you have to compare array length
            to None, not zero.

        Consider the cases:
         - pointer to an array (we set pointed_to_type, and also array_len
         - array of pointers (we set array_len, but not pointed_to_type)
         - array of pointers to structs (array length is set, members typeid
           defines pointer type, not struct type)

        Args:
                type_name_or_id
                elf_id (int, optional): If supplied, will be used to check
                    if the right debuginfo is called.
        """
        if elf_id is not None and self.elf_id != elf_id:
            raise InvalidDebuginfoCallError()

        if type_name_or_id is None:
            # A type of 0 is valid!
            raise InvalidDebuginfoCallError("Invalid type!")

        if type_name_or_id == 0xffffffff:
            # Unknown type id.
            raise InvalidDebuginfoTypeError(
                "Unknown typeId " + hex(type_name_or_id)
            )
        elif isinstance(type_name_or_id, numbers.Integral):
            # Assume type was already a typeid
            try:
                matched_types = [self.types[type_name_or_id]]
            except IndexError:
                raise InvalidDebuginfoTypeError(
                    "Unknown typeId " + hex(type_name_or_id)
                )
        else:
            # Could get multiple matches. Sometimes a typedef and a struct
            # both have the same name. References always work backwards, so
            # pick the one with the highest number
            matched_types = [
                i
                for i in self.types
                if i.name == type_name_or_id
            ]
        if not matched_types:
            # Couldn't find this type name
            raise InvalidDebuginfoTypeError(
                "Type " + str(type_name_or_id) + " not found!"
            )
        elif len(matched_types) > 1:
            # Multiple matches. Sometimes a typedef and a struct both have
            # the same name. References always have addressable unit set to 0.
            for match in matched_types:
                if match.size_in_addressable_units != 0:
                    first_tid = match.type_id
                    break
        else:
            first_tid = matched_types[0].type_id

        # elements of the type name
        # type_str and base_name are kept separate because we might have a
        # typedef containing (say) the struct/enum name, referencing the
        #  struct/enum type which has no name.
        qualifiers = ''  # e.g. const, volatile
        type_str = ''  # e.g. struct, union, enum
        base_name = ''  # e.g. 'utils_Set'
        pointer_str = ''  # e.g. '* '

        # other information we might glean while traversing the types table
        pointed_to_type = None
        array_len = None
        # tid which defines the list of (struct|union) members
        members_type = None
        tid = first_tid

        has_union = self.type_has_union(tid)
        size = 0
        failsafe = 0
        while failsafe < 20:
            failsafe += 1

            if tid == 0xffffffff:
                # Unknown type id. Stop looking.
                break

            # Internal type
            itype = self.types[tid]

            form = itype.form
            name = itype.name

            # we are not using pointer to types so don's set the size
            # we will go through a number of qualifiers like "pointer to"
            # "type defed" "structure" (separately).
            old_size = size
            try:
                # kalimbalab 23
                size = itype.num_words  # should be number if bytes
            except AttributeError:
                # post kalimbalab 23
                size = itype.size_in_addressable_units
            if old_size != 0 and size == 0:
                # always search for the last non zero qualifier
                size = old_size

            if tid == 0:
                # Normally means we've found an unknown typeid for
                # whatever reason.  If no other information has been
                # found, this usually signifies a function pointer.
                name = '[function]'
                if base_name == '':
                    base_name = name
                # Also, an enum type will often refer to type 0.
                # That case should be handled by the logic below, though.

            tid = itype.ref_type_id  # next tid

            # base
            if form == 0:
                # Ignore a base type, unless we don't have a base type
                # yet.  (Otherwise, we should already have the most useful
                # type name).
                if base_name == '' and type_str == '':
                    type_str = name
                break

            # struct or union (identical apart from the type string)
            elif form == 1 or form == 2:
                if form == 1:
                    type_str = 'struct '
                else:
                    type_str = 'union '
                # If array_len is set, members_type is already set to the array
                # type - which we don't want to override.
                if array_len is None:
                    members_type = itype.type_id  # The current typeId
                if base_name == '' and name != '':
                    base_name = name
                # Assume that once we reach a struct definition, we're done.
                break

            # array
            elif form == 3:
                # Consider the cases:
                # - pointer to an array (set pointed_to_type, also set
                #   array_len)
                # - array of pointers (set array_len, don't set
                #   pointed_to_type)
                # - array of pointers to structs (array len is set,
                #   members typeid defines pointer type, not struct type)
                array_len = itype.array_count
                tid = itype.array_type_id
                # So we know what this is an array of
                members_type = itype.array_type_id

            # typedef
            elif form == 4:
                # Ignore a typedef, unless base_name is empty.
                # (Otherwise, we should already have the most useful type
                # name).  We have no way of knowing at this point whether
                # the name describes a type (int, etc) or a base name
                # (e.g. struct name)
                if base_name == '' and name != '':
                    base_name = name

            # pointer
            elif form == 5:
                pointer_str = pointer_str + '* '
                # if we already have a pointed_to_type, don't over-write it:
                # if the user passed in (say) a char**, we want to return
                # char*, not char.
                if not pointed_to_type:
                    # Consider the cases:
                    # - pointer to an array (set pointed_to_type, also set
                    #   array_len)
                    # - array of pointers (set array_len, don't set
                    #   pointed_to_type)
                    if not array_len:
                        pointed_to_type = tid
                # Continue here to get the full name of the thing we point to

            # const
            elif form == 6:
                qualifiers = 'const ' + qualifiers

            # volatile
            elif form == 7:
                qualifiers = 'volatile ' + qualifiers

            # enum
            # This is similar to a typedef in that we want to ignore the name,
            # unless base_name is empty.
            elif form == 8:
                type_str = 'enum '
                if base_name == '' and name != '':
                    base_name = name
                # enums usually have a reference tid of 0; no point
                # looking at that though.
                break

        if type_str == '' and base_name == '':
            base_name = '[unknown type]'

        if array_len is not None and pointed_to_type is not None:
            # Pointer to an array. We want this to look like 'struct foo[4] *'.
            # (Arrays of pointers don't include the array size in the name)
            base_name += '[' + str(array_len) + '] '

        type_name = qualifiers + type_str + base_name + pointer_str
        # at least one of the qualifiers must have a size. If there are no
        # valid size it means that the type is unreferenced
        if size == 0:
            raise InvalidDebuginfoTypeError(type_name + hex(first_tid))
        # Return the size of last thing we look at; should be fine..
        return (
            type_name, first_tid, pointed_to_type, array_len, members_type,
            size, self.elf_id, has_union
        )

    @method_logger(logger)
    def read_const_string(self, address, elf_id=None):
        """Reads a constant string.

        Takes the address of a (filename) string in const, returns a
        string.

        Args:
            address: Can be the address of a filename in string as well.
            elf_id (int, optional): If supplied, will be used to check if
                the right debuginfo is called.
        """
        if elf_id is not None and self.elf_id != elf_id:
            raise InvalidDebuginfoCallError()

        ret_string = ""
        char = self.get_dm_const(address, 0)

        if Arch.addr_per_word == 4:
            # Sometimes, the string does not start at the beginning of the
            # word, therefore it does a check if the address supplied
            # starts at the beginning of the word and if not, it decodes
            # the first characters ignoring the octets before the
            # beginning of the relevant string.
            remainder = address % Arch.addr_per_word
            char >>= 8 * remainder
            ret_string += cu.get_string_from_word(
                Arch.addr_per_word - remainder, char
            )
            address += Arch.addr_per_word
            char = self.get_dm_const(address, 0)

            while 1:
                string = cu.get_string_from_word(Arch.addr_per_word, char)
                stop_decoding = False
                for letter in string:
                    if letter != '\0':
                        ret_string += letter
                    else:
                        stop_decoding = True
                        break
                if stop_decoding:
                    break

                address = address + Arch.addr_per_word
                char = self.get_dm_const(address, 0)
        else:
            while char:
                if char < 0x100:
                    ret_string = ret_string + chr(char)
                else:
                    ret_string = ret_string + "???"
                    break
                address = address + Arch.addr_per_word
                char = self.get_dm_const(address, 0)

        return ret_string

    def _inspect_array(self, var, members_typeid):
        # Do a forward-lookup to get the element size
        arraytype = self.types[members_typeid]

        try:
            # kalimbalab 23
            element_size = arraytype.num_words  # Actually bytes
        except AttributeError:
            # post kalimbalab23
            element_size = arraytype.size_in_addressable_units

        if var.array_len == 0:
            # A small number of types give us an array length of 0.
            # Try to infer it based on the variable size
            if element_size == 0:
                element_size = 1
            # surely this will yield an integer
            var.array_len = var.size // element_size

        if element_size == 2:
            var_values = cu.uint32_array_to_uint16(var.value)
            divisor = element_size
        elif element_size == 1:
            var_values = cu.uint32_array_to_uint8(var.value)
            divisor = element_size
        elif element_size >= Arch.addr_per_word:
            var_values = var.value
            divisor = Arch.addr_per_word
        else:
            var_values = var.value
            divisor = Arch.addr_per_word
            logger.info("@ ALERT: Array element size is %d octets!", element_size)

        # Iterate over every member of the array
        for count in range(var.array_len):
            member_name = var.name + '[' + str(count) + ']'
            member_addr = var.address + (count * element_size)

            start_index = (member_addr - var.address) // divisor
            end_index = int(
                math.ceil(
                    (member_addr - var.address + element_size) / float(divisor)
                )
            )
            # Get the member value. Note this will be in words.
            member_value = var_values[start_index:end_index]
            # Create the new variable for this member.
            new_var = ct.Variable(
                name=member_name,
                address=member_addr,
                size=element_size,
                value=member_value,
                var_type=members_typeid,
                debuginfo=self,
                members=None,
                parent=var
            )
            # Add the new variable as a member.
            var.members.append(new_var)
            # Recurse to fill in the member details.
            self.inspect_var(new_var)

    def _inspect_bitfield(self, var, mem):
        """Inspects bitfields enums or struct/union.

        Helper method for inspect_var.

        Args:
            var: Variable object.
            mem: Type id of the member.
        """
        # Ok I'm going to make my life easier and assume that all
        # bitfields are less than one word big. That means
        # 'bit offset' will always be 0 at the start of a new
        # wordin the struct.

        # Round up the member size
        member_size = int(
            math.ceil(float(mem.bit_size) / float(self.dm_byte_width))
        )
        # Create a mask of just the bits associated with this struct member.
        # DWARF information lists bitfield offsets relevant to the MSb of the
        # 'container object' (which is usually one byte, but could be more
        # if a field spans multiple bytes).
        # On 24-bit Kalimbas (big-endian), an offset of 0 means it starts
        # at bit 23.  On 32-bit Kalimbas (little-endian), an offset of 0
        # means it starts at bit 7 of the highest-addressed byte in the
        # field.
        # Because of the difference in endianness of the 32 and 24-bit
        # Kalimbas, there are 2 different algorithms for bitfields.
        member_val_mask = 0
        member_value = var.value[mem.offset // Arch.addr_per_word]
        remainder = mem.offset % Arch.addr_per_word
        if Arch.addr_per_word == 4:
            member_val_mask = int(math.pow(2, mem.bit_size) - 1)
            if remainder == 0:
                member_val_rshift = mem.bit_offset_from_offset
            else:
                member_val_rshift = (
                    mem.bit_offset_from_offset +
                    self.dm_byte_width * remainder
                )
            member_value >>= member_val_rshift
            # mask out irrelevant bits
            member_value &= member_val_mask
        else:
            for bit in range(mem.bit_size):
                # (       24        -1) - (      0      +  0 )
                member_val_mask |= (
                    1 << (self.dm_byte_width - 1) -
                    (mem.bit_offset_from_offset + bit)
                )
            # shift down so that value starts at first relevant bit
            member_val_rshift = (self.dm_byte_width - 1) - (
                mem.bit_offset_from_offset + mem.bit_size - 1
            )
            # mask out irrelevant bits
            member_value &= member_val_mask
            member_value = member_value >> member_val_rshift
        # Convert into a single-element list
        member_value = [member_value]

        return (member_size, member_value, member_val_mask, member_val_rshift)

    def _inspect_non_bitfield(self, var, mem):
        """Inspects non bitfields enums or struct/union.

        Helper method for inspect_var.

        Args:
            var: Variable object.
            members_typeid: Type id of the member.
        """
        # Sizes are in bits; convert to bytes
        member_size = mem.bit_size // self.dm_byte_width
        # if member_size is <4, then it only needs to take one word
        # (Arch4 specific)
        if member_size < Arch.addr_per_word and Arch.addr_per_word == 4:
            # This is probably a non bit field enum.
            # get the member offset in bytes knowing that the
            # values are in words.
            word_offset = (
                mem.offset + var.address % Arch.addr_per_word
            )
            # get the right word
            value = var.value[word_offset // Arch.addr_per_word]
            # convert the value to little endian format.
            value = cu.swap_endianness(value, Arch.addr_per_word)
            # get the member byte offset in the word.
            byte_offset = word_offset % Arch.addr_per_word
            # the values in the words are shifted by the number
            # of octets preceding the start of the variable
            # offset is measured in addressable units
            shif_amount = (
                8 * (Arch.addr_per_word - byte_offset - member_size)
            )
            value = value >> shif_amount
            # mask the value.
            value &= int(math.pow(2, mem.bit_size) - 1)
            # convert the value back to big-endian format.
            value = cu.swap_endianness(value, member_size)
            # Convert into a single-element list
            member_value = [value]
        else:
            start_index = mem.offset // Arch.addr_per_word
            end_index = member_size / float(Arch.addr_per_word)
            end_index = int(math.ceil(end_index))
            end_index = mem.offset // Arch.addr_per_word + end_index
            # Get the member value. Note this will be in words.
            member_value = var.value[start_index:end_index]
        return (member_size, member_value)

    def _inspect_union_or_struct(self, var, members_typeid):
        """Inspects uninons and structures.

        Helper method for inspect_var.

        Args:
            var: Variable object.
            members_typeid: Type id of the member.
        """
        membertype = self.types[members_typeid]
        members = membertype.members

        # Note that the offset of a structure member may not be equal to the
        # sum of lengths of all previous elements. On octet-addressed
        # systems, the compiler may decide to insert padding.
        for mem in members:
            member_name = var.name + '.' + mem.name
            # This works for unions because the offset is always 0. Handy!
            member_addr = var.address + mem.offset

            # Is the member part of a bitfield?
            if mem.bit_offset_from_offset != 0xFFFFFFFF:
                # In the case of a non-bitfield, the variable
                # bit_offset_from_offset is 0xFFFFFFFF, see B-214101
                (
                    member_size,
                    member_value,
                    member_val_mask,
                    member_val_rshift
                ) = self._inspect_bitfield(var, mem)
            elif mem.bit_size == 0:
                # A small number of types give us an element length of 0.
                # This for example happens when we have variable length
                # array defined at the the end of a structure. In these
                # cases we return the address of the variable
                member_size = 0
                member_value = member_addr
            else:
                member_size, member_value = self._inspect_non_bitfield(
                    var,
                    mem
                )

            # Create the new variable for this member.
            new_var = ct.Variable(
                name=member_name,
                address=member_addr,
                size=member_size,
                value=member_value,
                var_type=mem.type_id,
                debuginfo=self,
                members=None,
                parent=var
            )
            if mem.bit_offset_from_offset != 0xFFFFFFFF:
                # If the variable is part of a bitfield, make a note of how
                # big it really is.
                new_var.size_bits = mem.bit_size
                new_var.val_mask = member_val_mask
                new_var.val_rshift = member_val_rshift
            new_var.base_name = mem.name
            # Add the new variable as a member.
            var.members.append(new_var)
            # Recurse to fill in the member details.
            self.inspect_var(new_var)

    def inspect_var(self, var, elf_id=None):
        """Analyses a variable object contents.

        This isn't strictly 'private' to the DebugInfo class, but we don't
        want Analyses to call it. (If you want something similar, use
        get_type_info instead.)

        Args:
            var
            elf_id (int, optional): If supplied, will be used to check if
                the right debuginfo is called.
        """
        if elf_id is not None and self.elf_id != elf_id:
            raise InvalidDebuginfoCallError()

        # Get a load of stuff via get_type_info().
        try:
            # ( var.type_name, typeid, ptr_typeid, var.array_len,
            # members_typeid, varsize)
            (
                var.type_name, _, ptr_typeid, var.array_len,
                members_typeid, _, _, _
            ) = self.get_type_info(var.type)
        except InvalidDebuginfoTypeError:
            # get_type_info has likley failed on a typeid of None or
            # 0xffffffff.  That happens occasionally due to our imperfect
            # types database; If there  is no type information available
            # the variable won't be inspected. This will result to a
            # variable which will only display its raw content (no field
            # or any other info).
            return var

        if ptr_typeid is not None:
            # If the variable is just a pointer, there's not much else to do.
            # We don't want to recurse through all the members.
            pass
        elif var.array_len is not None:
            # If this is an array, split the variable up into individual
            # elements.
            self._inspect_array(var, members_typeid)
        elif members_typeid is not None:
            # If the type has members (i.e. it's a union or a struct),
            # AND it's not an array or pointer, go through the members.
            self._inspect_union_or_struct(var, members_typeid)

        self.mask_var_value(var)
        # We're done.
        return var

    @method_logger(logger)
    def is_maxim_pm_encoding(self, address):
        """Checks whether the provided address is encoded as maxim.

        Look up the contents of Program Memory at the supplied address,
        and work out whether it is encoded as Maxim or Minim.

        Args:
            address: Address to check.

        Returns:
            bool: True if the encoding is Maxim, False if the encoding is
                Minim.

        Raises:
            InvalidPmAddressError: If address is not in PM.
            UnknownPmEncodingError: If address has unknown encoding.
        """
        # pm_section_hdrs is a dict of tuples (start_addr, num_bytes,
        # pm_encoding, flag).
        (start_addr, num_bytes, pm_encoding, _) = list(range(4))
        # where the pm encoding is 1 for maxim code 2 minim code. (0 unknown)
        maxim_code = 1
        minim_code = 2
        unknown_code = 0
        # [DM sections have been pruned from this list]
        pm_address_encoding = unknown_code
        for _, info in self.pm_section_hdrs.items():  # (name, info)
            # Addresses/sizes are all measured in octets. All of our PM is
            # (if you squint a bit) octet-addressed, so the maths is simple.
            # Only check code sections
            address_in_section = (
                address >= info[start_addr] and
                address < (info[start_addr] + info[num_bytes])
            )
            known_section_encoding = (
                info[pm_encoding] in (maxim_code, minim_code)
            )
            if address_in_section and known_section_encoding:
                # Address is in this section
                pm_address_encoding = info[pm_encoding]
                break

        if pm_address_encoding == unknown_code:
            raise UnknownPmEncodingError(
                "Unknown PM encoding for address 0x%08x" % address
            )
        return pm_address_encoding == maxim_code

    @method_logger(logger)
    def is_pm_private(self, address):
        """Checks if the pm address is private or not.

        Args:
            address: pm address.

        Returns:
            bool: True if address private, False otherwise.
        """
        # pm_section_hdrs is a dict of tuples (start_addr, num_bytes,
        # pm_encoding, flag).
        for _, info in self.pm_section_hdrs.items():  # (name, info)
            # Addresses/sizes are all measured in octets. All of our PM is
            # (if you squint a bit) octet-addressed, so the maths is simple.
            # Only check code sections
            if (address >= info[0] and
                    address < (info[0] + info[1]) and
                    (info[2] == 1 or info[2] == 2)):
                # Address is in this section. Check if flag is set.
                if (info[3] & PRIV_SECTION_FLAG) != 0:
                    return True

        return False

    @method_logger(logger)
    def get_mmap_lst(self, elf_id):
        """Returns the mapped listing file from the build.

        Args:
            elf_id: Used to check if the right debuginfo is called.

        Returns:
            Mapped listing file.
        """
        if self.elf_id != elf_id:
            raise InvalidDebuginfoCallError()

        if self.mmap_lst is None:
            try:
                with open(self.lst_path) as lst_file:
                    self.mmap_lst = mmap.mmap(
                        lst_file.fileno(), 0, access=mmap.ACCESS_READ
                    )
            except IOError:
                # the .lst file is missing. Nothing we can do.
                return None

        self.mmap_lst.seek(0)

        return self.mmap_lst

    ##################################################
    # Private methods
    ##################################################

    def _find_nearest_label(self, address):
        """Searches for the nearest code label.

        Args:
            address: The address if the code.
        """
        # force clearing LSB bit for chips
        # that support minim mode
        if Arch.kal_arch == 4:
            address -= address & 0x1

        # No code label for private sections.
        if self.is_pm_private(address):
            # this is private address
            return None

        # search for the nearest label
        nearest_label = None
        for label in self.labels:
            label_address = label[1]
            if address >= label_address:
                nearest_label = label
            else:
                break

        if nearest_label is None:
            raise DebugInfoNoLabelError()
        # convert the label to CodeLabel
        return ct.CodeLabel(nearest_label[0], nearest_label[1])

    def _read_cap_extra_op_data(self):
        """Reads debug information to get the extra operator data type."""
        helper_str = "$_ACAT_INSTANCE_TYPE_CAP_ID_"
        len_helper_str = len(helper_str)
        self.cap_data_type = {}
        for variable in self.var_by_name:
            # Check if the variable name start with the helper string.
            if variable[:len_helper_str] == helper_str:
                # We found a helper variable which stores the extra operator
                # data type name in a debug string.
                var_address = self.var_by_name[variable].address
                # The cap name is actually the variable name without the helper
                # string at the beginning.
                cap_name = variable[len_helper_str:]
                # Now get the extra operator data type name from the debug
                # strings.
                extra_op_data_type = self.debug_strings[var_address]
                # Finally store the value for later use.
                self.cap_data_type[cap_name] = extra_op_data_type

    def _acquire_debug_strings(self):
        """Sets the `debug_strings` instance variable.

        The variable is a dictionary mapping, with addresses as keys:

            address:string
        """
        self.debug_strings = {}
        string_in_progress = ""
        # Start of the Debug region.
        string_start_addr = Arch.dRegions['DEBUG'][0]

        # Go through all the dm_const entries, ignoring everything until
        # we get to the start of the Debug region. Then just build up a list
        # of null-terminated strings.
        if Arch.addr_per_word == 4:
            if self.debug_strings_data is None:
                # No debug strings in the build, no need to read them.
                return
            # counts the number of characters in a string in order to obtain
            # the start address of the next string.
            no_of_addr_units = 0
            debug_values = self.debug_strings_data[4]
            for entry in debug_values:
                string = cu.get_string_from_word(Arch.addr_per_word, entry)
                for char in string:
                    if char != '\0':
                        string_in_progress += char
                        no_of_addr_units += 1
                    else:
                        self.debug_strings[string_start_addr] = string_in_progress
                        # Next address is start of next string
                        string_start_addr += no_of_addr_units + 1
                        string_in_progress = ""
                        no_of_addr_units = 0

        else:
            for addr, entry in sorted(self.dm_const.items()):
                saddr = cu.strip_elf_addr(addr, Arch.kal_arch)
                if Arch.get_dm_region(saddr, False) == 'DEBUG':
                    if entry == 0:
                        # null-terminator - save the string
                        self.debug_strings[string_start_addr] = string_in_progress
                        string_in_progress = ""
                        # Next address is start of next string
                        string_start_addr = saddr + 1
                    else:
                        string_in_progress += chr(entry)

    @staticmethod
    def mask_var_value(var):
        """Masks the variable value.

        Converts the value of the variable from arrays (like [123]) to
        ints (to 123) and masks the value when needed.

        Args:
            var: A Variable instance.
        """

        try:
            if var.size <= Arch.addr_per_word and len(var.value) == 1:
                var.value = var.value[0]
        except TypeError:
            pass

        # special case
        if Arch.addr_per_word == 4 and var.val_mask is None \
                and var.size < Arch.addr_per_word and var.parent is None:
            var.value = var.value >> (8 * (var.address % Arch.addr_per_word))
            # size is measured in octets
            var.value &= int(math.pow(2, 8 * var.size) - 1)

    @method_logger(logger)
    def search_matches(self, search_dict_name, name):
        """Search a name.

        Searches for potential matches in a dictionary. The dictionary
        is pointed as a attribute name of the current object.

        Args:
            search_dict_name: Current object's dictionary name.
            name: Name to search for.

        Returns:
            list: All the potential matches.
        """
        ret_matches = []
        search_dict = self.__getattribute__(search_dict_name)
        matches = variable_dict_search(search_dict, name)

        # store the elf id information in a dictionary
        for match in matches:
            ret_matches.append(
                {
                    "name": match,
                    "elf_id": self.elf_id,
                    "file_path": self.debuginfo_path,
                }
            )
        return ret_matches

    @method_logger(logger)
    def strict_search_matches(self, search_dict_name, name):
        """Searches for a name in a dictionary.

        The dictionary is pointed as a attribute name of the current
        object.

        Args:
            search_dict_name: Current object's dictionary name.
            name (str): Name to search for.

        Returns:
            list: Matches.
        """
        ret_matches = []
        search_dict = self.__getattribute__(search_dict_name)

        if name in search_dict:
            ret_matches.append(
                {
                    "name": name,
                    "elf_id": self.elf_id,
                    "file_path": self.debuginfo_path,
                }
            )

        return ret_matches

    def set_bundle(self, is_bundle):
        """Sets the bundle.

        Set the Debuginfo to a bundle if the input is True.  Unsets if the
        input is false.

        Args:
            is_bundle (bool): Debuginfo is a bundle.
        """
        self.debuginfo_is_bundle = is_bundle

    @method_logger(logger)
    def is_bundle(self):
        """Returns True if the debug info is a bundle.

        Returns:
            bool: True if the debug info is a bundle, False otherwise.
        """
        return self.debuginfo_is_bundle

    @contextmanager
    def _get_kal_elf_reader(self):
        logger.info("Loading '%s'", self.elf_path)
        ker = cu.global_options.ker.Ker()
        opened = False
        try:
            if is_zip(self.elf_path):
                with NamedTemporaryFile(delete=False) as temp_file:
                    temp_file.write(extract(self.elf_path))
                    elf_path = temp_file.name
            else:
                elf_path = self.elf_path

            ker.open_file(elf_path, ["debug_strings"])
            opened = True

            yield ker

        finally:
            if opened:
                ker.close_file()

            if elf_path != self.elf_path:
                os.unlink(elf_path)
