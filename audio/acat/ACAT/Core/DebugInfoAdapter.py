############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2016 - 2019 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
"""
DebugInfoAdapter is used to read Kymera elf and the downloadable bundles
transparently to the user. This module implements the DebugInfoInterface.
"""
import copy
import logging
import numbers

from . import Arch
from . import DebugInfo as di
from . import KerDebugInfo
from ACAT.Core import CoreTypes as ct
from ACAT.Core.exceptions import (
    AmbiguousSymbolError, BundleMissingError, DebugInfoError,
    DebugInfoNoLabelError, InvalidPmAddressError,
    InvalidDebuginfoCallError
)
from ACAT.Core.logger import method_logger

try:
    from future_builtins import hex
except ImportError:
    pass

logger = logging.getLogger(__name__)

BUNDLE_ERROR_MSG = "Bundle with elf id {0} is missing!\n"\
    "Use -j option or load_bundle(r\"<path>\") in interactive\n"\
    "mode to add bundles to ACAT!"

##############################################################################


class DebugInfoAdapter(di.DebugInfoInterface):
    """Gets debug information from kalelfreader.

    Note:
        self.debug_infos contains all the available debug information.
        The key None is a special one which keep the patch and Kymera
        debug info. The rest is just Debug info mapped based on the elf id
        in integer.

        self.debug_infos = {
            None: ["patch", "kymera"],
            pathc_elf_id: "patch",
            kymera_elf_id: "kymera",
            bundle_elf_id_1: "bundle_1"
            bundle_elf_id_1: "bundle_1"
            ....
        }

    Args:
        ker: Kalimba's .elf file reader object.
    """

    def __init__(self, ker):
        super(DebugInfoAdapter, self).__init__()
        self.ker = ker
        self.table = None

        patch = None
        kymera = None
        self.debug_infos = {
            None: [patch, kymera]
        }

        # which DM related functions need address mapping before call.
        self.dm_address_mappable_function = [
            "get_var_strict",
            "get_dm_const"
        ]
        # PM related functions return code labels. These need different
        # conversions to device address.
        self.pm_address_mappable_function = [
            "get_nearest_label",
            "get_source_info",
            "get_instruction",
            "is_maxim_pm_encoding",
            "is_pm_private"
        ]

    ##################################################
    # Debug info interface
    ##################################################

    def reload(self):
        """Reloads Debug Info of elf files if they have changed."""
        for ker_debug_info in self.get_ker_debug_infos():
            if ker_debug_info.has_changed():
                old_elf_id = ker_debug_info.get_elf_id()
                ker_debug_info.read_debuginfo()
                new_elf_id = ker_debug_info.get_elf_id()

                # When the elf file changes, the elf_id will change as
                # well. The old elf_id needs to be replaced by the new
                # one.
                self.debug_infos.pop(old_elf_id)
                self.debug_infos[new_elf_id] = ker_debug_info

    @method_logger(logger)
    def get_ker_debug_infos(self):
        """Gets all the KerDebugInfo instances."""
        return [
            ker_debug_info
            for elf_id, ker_debug_info in self.debug_infos.items()
            if elf_id is not None
        ]

    def get_constants_variables(self, elf_id=None):
        """Gets all the constant variables."""
        return self._call_debuginfo_elfid(elf_id, "get_constants_variables")

    @method_logger(logger)
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
        return self._call_debuginfo_elfid(
            elf_id,
            "get_cap_data_type",
            cap_name
        )

    @method_logger(logger)
    def get_constant_strict(self, name, elf_id=None):
        """Returns value is a ConstSym object (which may be None).

        If 'name' is not the exact name of a known constant, a KeyError
        exception will be raised.

        Args:
            name: Name of the constant.
            elf_id (int, optional): Which debug information to use. If
                None, patch and Kymera's debug information is used.
        """
        return self._call_debuginfo_elfid(
            elf_id,
            "get_constant_strict",
            name
        )

    @method_logger(logger)
    def get_var_strict(self, identifier, elf_id=None):
        """Searchs list of variables for an identifier (name or address).

        Args:
            identifier: Name or address of a variable. If the type is
                integer then it's an address. If the type is string it's
                a name.
            elf_id (int, optional): Which debug information to use. If None,
                patch and Kymera's debug information is used.

        Raises:
            KeyError: If `identifier` is not the exact name or start address
                of a known variable, a KeyError exception will be raised.
        """
        return self._call_debuginfo_elfid(
            elf_id,
            "get_var_strict",
            identifier
        )

    @method_logger(logger)
    def get_dm_const(self, address, length=0, elf_id=None):
        """Get a const from DM.

        If the address is out of range, a KeyError exception will be
        raised. If the address is valid but the length is not (i.e.
        address+length is not a valid address) an OutOfRangeError
        exception will be raised.

        Args:
            address: Address to read from.
            length
            elf_id (int, optional): Which debug information to use. If
                None, patch and Kymera's debug information is used.
        """
        return self._call_debuginfo_elfid(
            elf_id,
            "get_dm_const",
            address, length
        )

    @method_logger(logger)
    def get_source_info(self, address):
        """Gets information about a code address (integer).

        Args:
            address

        Returns:
            a SourceInfo object.

        Raises:
            KeyError: if the address is not valid.
        """
        try:
            return self._call_debuginfo_pm_addr(address, "get_source_info")
        except DebugInfoNoLabelError:
            name = "Unknown function (symbol may be censored)"
            return_sym = ct.SourceInfo(
                address, name, "Unknown file (symbol may be censored)", 0
            )
            return_sym.nearest_label = None
            return return_sym

    @method_logger(logger)
    def get_nearest_label(self, address):
        """Finds the nearest label to the supplied code address.

        Args:
            address

        Returns:
            a CodeLabel object.
        """
        return self._call_debuginfo_pm_addr(address, "get_nearest_label")

    @method_logger(logger)
    def get_instruction(self, address):
        """Returns the contents of Program Memory at the supplied address.

        The width of the return value depends on whether the instruction
        is encoded as Minim (16-bit) or Maxim (32-bit).  Will throw a
        KeyError exception if the address is not in PM RAM/ROM, or the
        address is not the start of an instruction.

        Args:
            address
        """
        return self._call_debuginfo_pm_addr(address, "get_instruction")

    @method_logger(logger)
    def get_enum(self, enum_name, member=None, elf_id=None):
        """Gets an enum.

        If supplied just with a name, returns a dictionary mapping member
        name->value.

        If also supplied with a member name/value, returns a tuple
        containing any matching values/names.

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
            enum_name
            member
            elf_id (int, optional)
        """
        return self._call_debuginfo_elfid(
            elf_id,
            "get_enum",
            enum_name, member
        )

    @method_logger(logger)
    def get_type_info(self, type_name_or_id, elf_id=None):
        """Gets type information.

        Takes a type name (e.g. 'ENDPOINT' or 'audio_buf_handle_struc') or
        a valid typeId, and looks up information in the type database.
        Returns a tuple containing:
        * The (fully-qualified) name of the type.
        * The typeid (redundant if 'type' is already a typeid).
        * The pointed-to typeid (if the type is a pointer).
        * The array length (if the type or pointed-to type is an array).
        * The typeid which describes any members of this type (if it is a
          structure or union).
        * Type size in addressable units (if the type or pointed-to type
          is a structure or union).

        Note: Unfortunately, a small number of types are defined as
        an array, but have a length of 0. That means to determine whether or
        not the type is an array you have to compare array length to None,
        not zero.

        Consider the cases:
         - pointer to an array (we set pointed_to_type, and also array_len.
         - array of pointers (we set array_len, but not pointed_to_type).
         - array of pointers to structs (array length is set, members typeid
           defines pointer type, not struct type).

        Args:
            type_name_or_id
            elf_id (int, optional)
        """
        return self._call_debuginfo_elfid(
            elf_id,
            "get_type_info",
            type_name_or_id
        )

    @method_logger(logger)
    def read_const_string(self, address, elf_id=None):
        """Takes the address of a (filename) string in const, returns a string.

        Args:
            address
        """
        return self._call_debuginfo_elfid(
            elf_id,
            "read_const_string",
            address
        )

    @method_logger(logger)
    def inspect_var(self, var, elf_id=None):
        """Inspects a variable.

        Inspecting a variable will build the variable members.

        Args:
            var: Variable to inspect.
            elf_id (int, optional)
        """
        return self._call_debuginfo_elfid(
            elf_id,
            "inspect_var",
            var
        )

    @method_logger(logger)
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
            InvalidPmAddressError: When address is not in PM.
            UnknownPmEncodingError: When address has unknown encoding.
        """
        return self._call_debuginfo_pm_addr(address, "is_maxim_pm_encoding")

    @method_logger(logger)
    def is_pm_private(self, address):
        """Checks if the pm address is private or not.

        Args:
            address: pm address.

        Returns:
            bool: True if address private, false otherwise.
        """
        return self._call_debuginfo_pm_addr(address, "is_pm_private")

    @method_logger(logger)
    def get_mmap_lst(self, elf_id):
        """Returns the mmap_lst for the boundle.

        Args:
            elf_id (int): The bundle elf id.
        """
        if elf_id is None:
            raise InvalidDebuginfoCallError(
                "get_mmap_lst needs exact elf ID."
                "None can be patch or Kymera debug information"
            )
        try:
            debug_info = self.debug_infos[elf_id]
        except KeyError:
            raise BundleMissingError()
        return debug_info.get_mmap_lst(elf_id)

    ##################################################
    # Adapter Interface
    ##################################################

    def set_table(self, table):
        """Sets the table for the adapter.

        Args:
            table
        """
        self.table = table

    @method_logger(logger)
    def update_patches(self, patches):
        """Updates the downloaded bundles for the adapter.

        Note:
            The update is similar to a dictionary merge.

        Args:
            bundles: A dictionary of bundles.
        """
        # A patch can overwrite debug info from the main build. Therefore
        # it must be saved to the special None key.
        count = 0
        for _, patch in patches.items():
            self.debug_infos[None][0] = patch
            count += 1

        if count > 1:
            raise Exception(
                "Only one patchpoint debug information source is supported."
            )

        self.debug_infos.update(patches)

    @method_logger(logger)
    def update_bundles(self, bundles):
        """Updates the downloaded bundles for the adapter.

        Note:
            The update is similar to a dictionary merge.

        Args:
            bundles: A dictionary of bundles.
        """
        for _, debug_info in bundles.items():
            debug_info.set_bundle(True)
        self.debug_infos.update(bundles)

    @method_logger(logger)
    def read_kymera_debuginfo(self, paths):
        """reads Kymera Debug Info.

        Reads all the bundles (also known as KDCs) and puts it to a
        dictionary based on the elf id. NOTE; a bundle is kept in an elf
        file.

        Args:
            paths: list of bundles.
        """
        kymera_debug_info = KerDebugInfo.KerDebugInfo(paths)
        try:
            kymera_debug_info.read_debuginfo()
        except TypeError:
            raise Exception("The Pythontools version is too old.")
        # Add to the special key None
        self.debug_infos[None][1] = kymera_debug_info
        # Add to the general dictionary
        self.debug_infos[kymera_debug_info.get_elf_id()] = kymera_debug_info

    @method_logger(logger)
    def is_elf_loaded(self, elf_id):
        """Checks if the elf file is loaded.

        Args:
            elf_id (int): The bundle elf id.
        """
        return elf_id in self.debug_infos

    def _map_data_from_file_to_build(self, var, elf_id):
        """maps Variable to a global addresses and values.

        This method maps variable referred to local addresses of
        downloaded capability back to global addresses and values.

        Args:
            var: a Variable.
            elf_id: elf id to whom the variable structure definition
                belongs to.
        """
        # Deep copy is used for the value to avoid modifying the original
        # value.
        var = ct.Variable(
            name=var.name,
            address=var.address,
            size=var.size,
            value=copy.deepcopy(var.value),
            var_type=var.type,
            members=var.members,
            parent=var.parent,
            debuginfo=var.debuginfo
        )
        var.address = self.table.convert_addr_to_build(
            var.address, elf_id
        )
        return var

    def _address_in_table(self, address):
        """Checks if and address is in the downloaded boundle table.

        Args:
            address
        """
        if self.table is not None and self.table.is_addr_in_table(address):
            return True

        return False

    ##################################################
    # Adapter Searchable Interface
    ##################################################

    @method_logger(logger)
    def get_constant(self, name):
        """Gets a symbolic constant.

        If 'name' is not the exact name of a known constant, a number of
        variations will be tried in an attempt to find a match.

        This method should only be used in interactive mode; it's risky to
        call it as part of an analysis. Use get_constant_strict instead.

        Return value is a ConstSym object (which may be None).
        An AmbiguousSymbolError exception will be raised if the supplied
        name can't be matched to a single constant.

        Args:
            name
        """

        # todo add search for downloadable capabilities
        matches = self._search_by_name("constants", "search_matches", name)

        if matches == []:
            result = None
        else:
            # We may have found multiple matches for the symbol name.
            # If all the matches are aliases for each other, we can return
            # that value. if they're different, admit our mistake.
            val = self.get_constant_strict(
                matches[0]["name"], matches[0]["elf_id"]
            )
            # The first is always the same with the first.
            success = True
            # Skip the first which is used to check against.
            for match in matches[1:]:
                try:
                    constant = self.get_constant_strict(
                        match["name"],
                        match["elf_id"]
                    )
                    if (val != constant):
                        success = False
                        break
                # Todo remevoe this if B-242063 is corrected.
                except BaseException:
                    success = False
                    break

            if success:
                result = val
            else:
                apology = (
                    "Multiple potential matches found for "
                    "constant '%s': " % name
                )
                raise AmbiguousSymbolError(apology, matches)

        return result

    @method_logger(logger)
    def get_var(self, identifier, elf_id=None):
        """Searches variables for the supplied identifier (name or address).

        If a supplied name is not the exact name of a known variable, a
        number of variations will be tried in an attempt to find a match.

        If a supplied address is not the start of a known variable, the
        containing variable (if any) will be returned.

        For example, get_var(0x600) and get_var(0x604) will both
        return the variable $stack.buffer, starting at address 0x600.

        This method should only be used in interactive mode; it's risky
        to call it as part of an analysis. Use get_var_strict instead.

        Args:
            identifier: Name or address of a variable. If the type is
                integer then it's an address. If the type is string it's
                a name.
            elf_id (int, optional): The bundle elf id if the variable is
                in a downloadable capability.

        Returns:
            value is a Variable object (which may be None).

        Raises:
            AmbiguousSymbolError: When the supplied name can't be matched
                to a single variable.
        """

        if isinstance(identifier, numbers.Integral):
            return self._search_var_by_address(identifier)

        # First search for strict matches
        matches = self._search_by_name(
            "var_by_name", "strict_search_matches", identifier, elf_id
        )

        if matches == []:
            # No strict matches: search if there is any similarly named
            # variables
            matches = self._search_by_name(
                "var_by_name", "search_matches",
                identifier, elf_id
            )

        if matches == []:
            result = None
        else:
            # We found one or more matches for the symbol name.  If all
            # the matches are aliases for each other, just return that
            # value.  if they're different, admit our mistake.
            val = self.get_var_strict(matches[0]["name"], matches[0]["elf_id"])
            # The first is always the same with the first.
            success = True
            # Skip the first which is used to check against.
            for match in matches[1:]:
                try:
                    variable = self.get_var_strict(
                        match["name"],
                        match["elf_id"]
                    )
                    if (val != variable):
                        success = False
                        break
                # Todo remevoe this if B-242063 is corrected.
                except BaseException:
                    success = False
                    break

            if success:
                result = val
            else:
                apology = (
                    "Multiple potential matches found for "
                    "variable '%s': " % identifier
                )
                raise AmbiguousSymbolError(apology, matches)

        return result

    @method_logger(logger)
    def get_kymera_debuginfo(self):
        """Returns Kymera's debug information.

        Returns:
            Kymera's debug information.
        """
        # Kymera is the last debuginfo in the special list mapped to key None.
        # See __init__ for more.
        return self.debug_infos[None][1]

    @method_logger(logger)
    def get_patch_debuginfo(self):
        """Returns the patch's debug information.

        Returns:
            Patch's debug information or None if patch is not loaded.
        """
        # The patch debug info is the first debuginfo in the special list
        # mapped to key None. See __init__ for more.
        return self.debug_infos[None][0]

    def _search_by_name(self, search_dict_name, function_name, name,
                        elf_id=None):
        """Searches for a variable by name. Accepts partial matches.

        Args:
            name: Name to search for.
            function_name: Function name used for searching. As examples,
                the function can be something like `search_matches` and
                `strict_search_matches`.
            elf_id (int, optional): The bundle elf id if the variable is
                in a downloadable capability.

        Returns:
            list: All the matches.
        """
        ret_matches = []
        if elf_id is None:
            # no elf  id so search everywhere
            for cur_elf_id, debug_info in self.debug_infos.items():
                # elf_id None is used for Kymera and patches, but they can be
                # found based on the id too. So don't double check them.
                if cur_elf_id is not None:
                    function_to_call = getattr(debug_info, function_name)
                    ret_matches += function_to_call(
                        search_dict_name, name
                    )
        else:
            # search in a specific elf
            debug_info = self.debug_infos[elf_id]
            function_to_call = debug_info.__getattribute__(function_name)
            ret_matches += function_to_call(search_dict_name, name)

        return ret_matches

    @staticmethod
    def _check_for_addr(variable_by_address, address):
        # Start off at the address given and work downwards.
        checkaddr = address
        while not (checkaddr in variable_by_address) and checkaddr >= 0:
            checkaddr -= Arch.addr_per_word
        if checkaddr >= 0:
            # We found something
            return variable_by_address[checkaddr]
        return None

    def _search_var_by_address(self, address):
        """Searches for a variable by address.

        Args:
            address: Address to search for.

        Returns:
            list: All the matches.
        """
        # If address provided maps to address for downloaded capability, the
        # variable we are searching for is actually stored in downloaded
        # capability
        if self._address_in_table(address):
            elf_id_from_address = self.table.get_elf_id_from_address(address)
            variable_by_address = self.debug_infos[
                elf_id_from_address
            ].var_by_addr
            return self._check_for_addr(variable_by_address, address)

        # search in Patches and Kymera
        for debug_info in self.debug_infos[None]:
            variable_by_address = debug_info.var_by_addr
            retval = self._check_for_addr(variable_by_address, address)
            if retval:
                return retval

        return None

    def _call_debuginfo_pm_addr(self, address, function_name):
        """Invokes _call_debuginfo_elfid method.

        Decides if the address is part of a downlaodable bundle or not and
        calls _call_debuginfo_elfid with the elf id for further
        processing.  This is necessary because _call_debuginfo_elfid
        assumes that the address is in Kymera or patch elf.

        Args:
            @param[in] address Code address.

        Returns:
            function_name (str): Name of the function to call.
        """
        if Arch.get_pm_region(address, False) is None:
            raise InvalidPmAddressError("Key " + hex(address) + " is not in PM")

        if self._address_in_table(address):
            # elf_id_from_address should be different than None
            elf_id_from_address = self.table.get_elf_id_from_address(
                address
            )
            if elf_id_from_address not in self.debug_infos:
                # No bundles are loaded to ACAT at all
                raise BundleMissingError(
                    BUNDLE_ERROR_MSG.format(
                        hex(elf_id_from_address)
                    )
                )
        else:
            # Use the main Kymera debug info and the patches.
            elf_id_from_address = None

        return self._call_debuginfo_elfid(
            elf_id_from_address, function_name,
            address
        )

    def _call_debuginfo_function(self, elf_id, function_name, *argv, **kwargs):
        """Invokes a function in debuginfo.

        Searches for the right debuginfo based on elf id (Kymera, patch or
        downloadable bundles) and call the function specified at the
        input.

        Args:
            elf_id (int): The Debug info elf id to call.
            function_name (str): Name of the function to call.
            *argv: Function unnamed input parameter.
            **kwargs: Function named input parameter.

        Returns:
            Value returned by the function call.
        """
        # convert addresses to the Bundle address if needed
        in_dm_pm = (
            function_name in self.dm_address_mappable_function or
            function_name in self.pm_address_mappable_function
        )
        if (in_dm_pm and isinstance(argv[0], numbers.Integral)):
            address = argv[0]
            if self._address_in_table(address):
                # Get the elf ID from the address.
                elf_id = self.table.get_elf_id_from_address(
                    address
                )
                address = self.table.convert_addr_to_download(
                    address,
                    elf_id
                )
                argv = (address,)

        # Access the debuginfo in the standard way. Do this after the proper
        # elf_id is selected.
        try:
            debug_info = self.debug_infos[elf_id]
        except KeyError:
            raise BundleMissingError(hex(elf_id))

        # get the function from the debuginfo which will be called.
        function_to_call = debug_info.__getattribute__(function_name)
        # todo: Make all adapted functions to accept elf_id as an input
        # kwargs["elf_id"] = elf_id
        # and call the function.
        return_val = function_to_call(*argv, **kwargs)

        # Remap the addresses from bundle to chip
        if debug_info.is_bundle():
            if function_name in self.dm_address_mappable_function:
                # Convert a variable to chip address
                return_val = self._map_data_from_file_to_build(
                    return_val, elf_id
                )
            if function_name in self.pm_address_mappable_function:
                if hasattr(return_val, "address"):
                    # Convert a code label to chip address.
                    return_val = copy.deepcopy(return_val)
                    return_val.address = self.table.convert_addr_to_build(
                        return_val.address, elf_id
                    )
        return return_val

    def _call_debuginfo_elfid(self, elf_id, function_name, *argv, **kwargs):
        """Calls a function in a debuginfo instance with the given elf_id.

        Searches for the right debuginfo based on elf id (Kymera or
        downloadable bundles) and call the function specified at the input.

        Args:
            elf_id (int): Debug info elf id.
            function_name (str): Name of the function to call.
            *argv: Function unnamed input parameter.
            **kwargs: Function named input parameter.

        Returns:
            Value returned by the function call.
        """
        if elf_id is None:
            # elf_id == None. Meaning to look for patch and then kymera
            pacth, kymera = self.debug_infos[elf_id]
            if pacth:
                try:
                    return self._call_debuginfo_function(
                        pacth.elf_id,
                        function_name, *argv, **kwargs
                    )
                except DebugInfoError:
                    # this is just the pach so ignore any erros.
                    pass

            # try the same for Kymera. This time do not handle the
            return self._call_debuginfo_function(
                kymera.elf_id,
                function_name, *argv, **kwargs
            )

        return self._call_debuginfo_function(
            elf_id, function_name,
            *argv, **kwargs
        )
