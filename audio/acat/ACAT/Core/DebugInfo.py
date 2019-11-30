############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2012 - 2019 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
"""
The interface to the debug information is defined in this module.
"""
import abc
import logging

from . import Arch
from ACAT.Core.exceptions import DebugInfoNoLabelError
from ACAT.Core.logger import method_logger

logger = logging.getLogger(__name__)


##################################################
# This is an abstract base class, but I am avoiding use of Python's ABC
# syntax.
class DebugInfoInterface(object):
    """Debug Info Interface.

    Defines an interface for extracting debug information from the build
    output.
    """
    __metaclass__ = abc.ABCMeta

    def __init__(self):
        pass

    @abc.abstractmethod
    def get_cap_data_type(self, cap_name, elf_id=None):
        """Returns the data type name for a given capability.

        Note:
            The data type is used to hold all the information for a
            capability.

        Args:
            cap_name (str): Capability name.

        Returns:
            The name of the extra data type for the given capability.
                returns None if no information found.
        """

    @abc.abstractmethod
    def get_constant_strict(self, name, elf_id=None):
        """Return value is a ConstSym object (which may be None).

        If 'name' is not the exact name of a known constant, a KeyError
        exception will be raised.

        Args:
            name (str): Name of the constant.
            elf_id (int, optional): Used by DebugInfoAdapter to select the
                debug information.
        """

    @abc.abstractmethod
    def get_var_strict(self, identifier, elf_id=None):
        """Search list of variables for the supplied identifier (name/address).

        If 'identifier' is not the exact name or start address of a known
        variable, a KeyError exception will be raised.  ('identifier' is
        interpreted as an address if it is an integer, and a name
        otherwise).

        Args:
            identifier
            elf_id (int, optional): Used by DebugInfoAdapter to select the
                debug information.
        """

    @abc.abstractmethod
    def get_dm_const(self, address, length=0, elf_id=None):
        """Get the contents of DM const(NVMEM).

        If the address is out of range, a KeyError exception will be raised.
        If the address is valid but the length is not (i.e. address+length
        is not a valid address) an OutOfRangeError exception will be raised.

        Args:
            address
            length (int, optional)
            elf_id (int, optional)

        Returns:
            int
        """

    @abc.abstractmethod
    def get_source_info(self, address):
        """Get information about a code address.

        The address can be a real address (integer) or a module name
        (string).

        Args:
            address

        Returns:
            a SourceInfo object.
        """

    @abc.abstractmethod
    def get_nearest_label(self, address):
        """Finds the nearest code label to the supplied address.

        Args:
            address

        Returns:
            a CodeLabel object.
        """

    @method_logger(logger)
    def get_label(self, address):
        """Finds the code label at the exact supplied address.

        Args:
            address

        Returns:
            a CodeLabel object, None if no label found at the address.
        """
        label = self.get_nearest_label(address)
        if label is not None:
            if Arch.kal_arch == 4:
                # for kal_arch4 clear minim bit
                address -= address & 0x1
            if address != label.address:
                # expect label at exact address
                label = None
        if label is None:
            raise DebugInfoNoLabelError()
        return label

    @abc.abstractmethod
    def get_instruction(self, address):
        """Return the contents of Program Memory at the supplied address.

        The width of the return value depends on whether the instruction
        is encoded as Minim (16-bit) or Maxim (32-bit).

        Will throw a KeyError exception if the address is not in PM
        RAM/ROM, or the address is not the start of an instruction.

        Args:
            address
        """

    @abc.abstractmethod
    def get_enum(self, enum_name, member=None, elf_id=None):
        """Gets an enum.

        If supplied just with a name, returns a dictionary mapping member
        name->value. If also supplied with a member name/value, returns a
        tuple containing any matching values/names.

        e.g.
            get_enum('ACCMD_CON_TYPE')
            Returns a dictionary of all enum members.

            get_enum('ACCMD_CON_TYPE', 'ACCMD_CON_TEST')
            Returns value of ACCMD_CON_TEST member (1 in this case)

            get_enum('ACCMD_CON_TYPE', 1)
            Returns the name(s) of any entries with value 1
            ('ACCMD_CON_TEST' in this case)

        If either enum_name or member is not found, a KeyError exception
        will be raised.

        Args:
            enum_name
            member
            elf_id (int, optional): Used by DebugInfoAdapter to select the
                debug information.
        """

    @abc.abstractmethod
    def get_type_info(self, type_name_or_id, elf_id=None):
        """Returns information about a type name/id.

        Takes a type name (e.g. 'ENDPOINT' or 'audio_buf_handle_struc') or
        a valid typeId, and looks up information in the type database.
        Returns a tuple containing:
            * The (fully-qualified) name of the type.
            * The typeid (redundant if 'type' is already a typeid)
            * The pointed-to typeid (if the type is a pointer)
            * The array length (if the type or pointed-to type is an
              array).
            * The typeid which describes any members of this type (if it
              is a structure or union).
            * Type size in addressable units (if the type or pointed-to
              type is a structure or union)

        If an error occurs, an empty tuple is returned.

        Args:
            type_name_or_id
            elf_id (int, optional): Used by DebugInfoAdapter to select the
                debug information.
        """

    @abc.abstractmethod
    def read_const_string(self, address, elf_id=None):
        """Takes the address of a (filename) string in const, returns a string.

        Args:
            address
            elf_id (int, optional): Used by DebugInfoAdapter to select the
                debug information.
        """

    @abc.abstractmethod
    def inspect_var(self, var, elf_id=None):
        """Inspects a variable.

        Inspecting a variable will build the variable members.

        Args:
            var: Variable to inspect.
            elf_id (int, optional)
        """

    @abc.abstractmethod
    def is_maxim_pm_encoding(self, address):
        """Checks whether an address is encoded as Max or Min.

        Look up the contents of Program Memory at the supplied address,
        and work out whether it is encoded as Maxim or Minim.

        Args:
            address: Address to check.

        Returns:
            bool: True if the encoding is Maxim. False if encoding is
                Minim.

        Raises:
            raises InvalidPmAddressError - if address is not in PM
            raises UnknownPmEncodingError - if address has unknown encoding.
        """

    @abc.abstractmethod
    def is_pm_private(self, address):
        """Checks if the pm address is private or not.

        Args:
            address: pm address.

        Returns:
            bool: True if address private, false otherwise.
        """

    @abc.abstractmethod
    def get_mmap_lst(self, elf_id):
        """Returns the mmap_lst for the boundle.

        Args:
            elf_id: The bundle elf id.
        """
