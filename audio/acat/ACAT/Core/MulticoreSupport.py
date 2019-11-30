############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2015 - 2019 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
"""
Module responsible to enable dual core support.
"""
from __future__ import print_function

import copy
import logging
import numbers
import os
import re

from ACAT.Core import Arch
from ACAT.Core import CoreTypes as ct
from ACAT.Core import CoreUtils as cu
from ACAT.Core.DebugInfoAdapter import DebugInfoAdapter as DebugInformation
from ACAT.Core.DownloadedCapability import MappedTable
from ACAT.Core.KerDebugInfo import KerDebugInfo
from ACAT.Core.patch import ProcessorPatcher
from ACAT.Core.exceptions import (
    UsageError, AnalysisError, AmbiguousSymbolError,
    InvalidDmConstAddressError, InvalidDmConstLengthError,
    InvalidDmAddressError, InvalidDmLengthError
)
from ACAT.Core.logger import function_logger

logger = logging.getLogger(__name__)


@function_logger(logger)
def load_bundle(bundle_path):
    """Loads a bundle (also known as KDCs) and puts it to a dictionary.

    Args:
        bundle_path: Path to the boundle's elf file.
    """
    # Dict representing a bundle
    bundle_dictionary = {}

    # Create separate Debug info for each bundle
    bundle_debuginfo = KerDebugInfo(bundle_path)
    bundle_debuginfo.read_debuginfo()
    bundle_dictionary[bundle_debuginfo.get_elf_id()] = bundle_debuginfo

    return bundle_dictionary


@function_logger(logger)
def get_build_output_path(chipdata):
    """Returns the path to the release build.

    Reads the build ID from the chip and tries to search for the released
    build based on the ID.

    Args:
        chipdata: Access to the chip.
    """
    # If the build_output_path was not supplied, try to work it out from the
    # build ID. This obviously doesn't work for unreleased builds.
    build_id = chipdata.get_firmware_id()
    build_output_path = None

    if build_id == 0xFFFF:
        # Unreleased build!
        raise UsageError(
            "ERROR: Path to build output not supplied, "
            "and build is unreleased!")
    else:
        try:
            from ACAT.Core.BuildFinder import BuildFinder
            build_finder = BuildFinder(build_id)
            build_output_path = os.path.join(
                build_finder.rom_build_path, 'debugbin'
            )
        except ImportError:
            raise UsageError("ERROR: Path to build output not supplied.")

    return build_output_path


class Functions(object):
    """Encapsulates the functions used in Interactive mode.

    Args:
        chipdata
        debuginfo
        formatter
    """

    def __init__(self, chipdata, debuginfo, formatter):
        self.chipdata = chipdata
        self.debuginfo = debuginfo
        self.formatter = formatter

        # Will be used in verbose to define indentation steps when the
        # method is being called recursively.
        self._indent_step = 2

    def _verbose(self, obj, indent=0):
        """Prints an object to the output.

        Args:
            obj: Can be any Python standard type.

        Raises:
            RuntimeError: If the given object is not supported by the
                method.
        """
        if isinstance(obj, dict):
            for key, value in obj.items():
                if value is None or len(value) == 0:
                    # The value is either None or empty, so there is
                    # little value to show it.
                    continue

                print('\n{}{}:'.format(indent * ' ', key))
                self._verbose(value, indent=indent+self._indent_step)

        elif isinstance(obj, (list, tuple, set)):
            for item in sorted(obj):
                self._verbose(item, indent=indent+self._indent_step)

        elif isinstance(obj, (int, str)):
            print('{}{}'.format(indent * ' ', obj))

        elif isinstance(obj, Exception):
            # Remove quotes and print the clean message
            error_message = str(obj)
            while '\'' in error_message or '\"' in error_message:
                error_message = error_message.strip('\'\"')

            print(error_message)

        else:
            raise RuntimeError(
                "Unsupported type for {}({})".format(str(obj), type(obj))
            )

    def search(self, phrase, ignore_case=True, elf_id=None, verbose=True):
        """Perform search in registers and variable.

        The search phrase can only contain * as wildcard. For example:

            search('pcm*')  Will find all the constants/variables/registers
                            which start with `pcm`.
            search('*pcm*') Will find all the constants/variables/registers
                            which contains `pcm`.

        When the verbose is disabled(by default), the output will be a
        python dictionary, otherwise the method prints the result in a
        human readable form.

        Args:
            phrase (str): A searching phrase in string. The phrase can
                contain * as wildcard.
            ignore_case (bool, optional): A flag to indicate whether the
                search should be case sensitive or not.
            elf_id: The bundle elf id if the variable is in a downloadable
                capability.
            verbose (bool, optional): Instead of returning an object
                prints the results.

        Returns:
            dict: A dictionary of the results.
            None: If it's in verbose mode.
        """
        # Convert patterns with `*` to an understandable regex and escape
        # any possible regex characters.
        phrase = '.*'.join(re.escape(part) for part in phrase.split('*'))
        phrase = '^{}$'.format(phrase)
        if ignore_case is True:
            flags = re.IGNORECASE
        else:
            flags = 0

        # Building up the results by going through all the given elfs.
        # However, if the elf_id is specified by the user, only the given
        # KerDebugInfo instance with the given elf_id is going to be
        # parsed.
        results = {}
        for ker_debug_info in self.debuginfo.get_ker_debug_infos():
            if elf_id is not None:
                # When the elf_id is given
                if elf_id != ker_debug_info.elf_id:
                    # We are only interested in a specific elf file
                    continue

            constants_and_variables = ker_debug_info.get_constants_variables()
            matches = [
                constant
                for constant in constants_and_variables
                if re.search(phrase, constant, flags=flags) is not None
            ]
            elf_file = os.path.split(ker_debug_info.elf_path)[-1]
            elf_file_id = '{} ({})'.format(elf_file, ker_debug_info.elf_id)
            if len(matches):
                results[elf_file_id] = sorted(matches)

        # Search all the Chip Data registers as well
        matches = [
            constant
            for constant in self.chipdata.get_all_proc_regs().keys()
            if re.search(phrase, constant, flags=flags) is not None
        ]
        if len(matches):
            results['Hardware Registers'] = sorted(matches)

        if verbose:
            self._verbose(results)

        else:
            return results

    def get(self, identifier, elf_id=None, datalen=None):
        """
        Get a variable, register or range of addresses.

        'identifier' can be an address, or the name of a register or variable
        (note constant names are not handled). If it's a name, we attempt to
        find the closest match in our lists of registers and variables.
        However, please note that there is a 'search' function to conduct
        searches, i.e. search('*[IDENTIFIER]*'), and it's advised to use that
        instead of 'get'.

        If the identifier is ambiguous, an AmbiguousSymbolError exception may
        be thrown.

        Like `get_var`, it's also possible to pass in a data length to request
        a slice of data (this only makes sense for things in DM).

        Obviously there are things that could go wrong here, especially if
        the user passes in a random number and we try to guess what it points
        to. That's why we only provide this function in Interactive mode!

        Args:
            identifier: Could be name or address
            elf_id: The bundle elf id if the variable is in a downloadable
                capability.
            datalen: If the identifier is an address the data length is
                specified by this input.

        Returns:
            a DataSym (which may be a Variable) or a SourceInfo (if a
            code address was supplied).
        """
        pm_addr = None
        reg = None
        var = None
        apology = ""
        hit_flag = False
        # For Crescendo, data can only be fetched as words. Since it is
        # octet-addressed, the addresses must be divisible by the number of
        # addresses per word (32 bit words - 4 octets, therefore addresses must
        # be divisible by 4).
        if isinstance(identifier, numbers.Integral):
            identifier = cu.get_correct_addr(identifier, Arch.addr_per_word)

        # Same as above. The lengths are measured in addressable units.
        if datalen is not None:
            datalen = cu.convert_byte_len_word(datalen, Arch.addr_per_word)

        # Look to see whether it's an address in PM.
        # Since the PM range can be huge (and may overlap with DM), perform a
        # sanity check to avoid false positives.
        if (isinstance(identifier, numbers.Integral) and
                Arch.get_pm_region(identifier, False) in ("PMROM", "PMRAM") and
                datalen is None):
            # Is it the address of a valid instruction?
            try:
                self.debuginfo.get_instruction(identifier)
            except KeyError:
                pass
            else:
                pm_addr = self.debuginfo.get_source_info(identifier)

        # Look to see whether it's the name of a module in PM. This needs to
        # include all the fluff at the start, (i.e. '$M.foo.bar' rather than
        # just 'foo.bar' or 'bar') so as to avoid clashes with names in DM
        # space (which are more likely to be what's wanted).
        if isinstance(identifier, numbers.Integral):
            try:
                pm_addr = self.debuginfo.get_source_info(identifier)
                hit_flag = True
            except KeyError:
                pass
            except AmbiguousSymbolError as amb_pm:
                apology += str(amb_pm) + "\n"

        # Look to see whether it's a register.
        try:
            reg = self.get_reg(identifier)
            hit_flag = True
        except AmbiguousSymbolError as amb_reg:
            logger.warning(
                "Multiple registers found for {}. Use `search('*{}*')` to see "
                "the results clearer.\n".format(
                    str(identifier),
                    str(identifier)
                )
            )
            apology += str(amb_reg) + "\n"

        # Also look to see whether it's a variable name/address.
        try:
            var = self.get_var(identifier, elf_id, datalen)
            hit_flag = True
        except AmbiguousSymbolError as amb_var:
            logger.warning(
                "Multiple variables found for {}. Use `search('*{}*')` to see "
                "the results clearer.\n".format(
                    str(identifier),
                    str(identifier)
                )
            )
            apology += str(amb_var) + "\n"

        # We got at least one hit
        if hit_flag:
            if not pm_addr and not reg and not var:
                # We didn't find anything at all
                logger.warning(
                    "Nothing was found! You can also use `search('*%s*')`.",
                    identifier
                )
                return None
            elif pm_addr and not reg and not var:
                return pm_addr
            elif reg and not var and not pm_addr:
                return reg
            elif var and not reg and not pm_addr:
                return var
            else:
                # We got unique, valid hits for one or more of the above.
                apology = (
                    "Multiple matches found for {}. Use `search('*{}*')` to "
                    "see the results clearer.\n".format(
                        str(identifier),
                        str(identifier)
                    )
                )

        # If we get here then we couldn't resolve ambiguous symbols in one or
        #  more cases. Note any genuine match of the other types as well.
        if pm_addr:
            apology += " code address in module " + pm_addr.module_name + "\n"
        if var:
            apology += " variable " + var.name + "\n"
        if reg:
            apology += " register " + reg.name + "\n"
        raise AmbiguousSymbolError(apology)

    def get_reg(self, identifier):
        """Get register.

        Like Analysis.get_reg_strict(), except it's not strict (!)
        'identifier' can be a register name, or address. If it's a name,
        we attempt to find the closest match in our list of registers.

        Args:
            identifier

        Returns:
            a DataSym instance.

        Raises:
            AmbiguousSymbolError: If more than one match is found.
        """

        reg = None  # Will be a ConstSym.

        # If the user supplied an address, and it smells like a register,
        # attempt to look it up.
        if isinstance(identifier, numbers.Integral):
            if Arch.get_dm_region(identifier) == "MMR":
                # Look for constants that have a value of the supplied
                # address.  Inherently risky, since any random constant
                # could have a value that looks like a register address.
                # Since we only do this to set the name, it should be ok.
                possible_regs = [
                    item[0] for item in list(self.debuginfo.constants.items())
                    if item[1] == identifier
                ]
                if possible_regs:
                    reg_name = " or ".join(possible_regs)
                    reg = ct.ConstSym(reg_name, identifier)
        else:
            # Look up the supplied name in our list of constants. If the
            # name is found, reg.value is actually going to be the address
            # of the register.

            # get_constant might throw an AmbiguousSymbolError exception
            # here; in this case we want to catch it, and weed out any
            # matches that aren't register names.
            try:
                if 'regfile' in identifier:
                    return self.chipdata.get_reg_strict(identifier)

                return self.chipdata.get_reg_strict(
                    'regfile_' + identifier
                )
            except KeyError:
                pass
            except BaseException:
                # This shoud be on UnknownRegister but is too hard to import
                pass

            try:
                reg = self.debuginfo.get_constant(identifier)
            except AmbiguousSymbolError as ambs:
                # We helpfully store the ambiguous matches in the exception
                # args
                ambiguous_matches = ambs.args[1]

                actual_ambiguous_matches = []
                for match in ambiguous_matches:
                    amconst = self.debuginfo.get_constant_strict(
                        match["name"], match["elf_id"]
                    )
                    if Arch.get_dm_region(amconst.value, False) == "MMR":
                        actual_ambiguous_matches.append(match)

                if not actual_ambiguous_matches:
                    # We actually ended up finding no real registers
                    reg = None
                else:
                    # If all the matches are aliases for each other, we can
                    # return that value. if they're different,
                    # admit our mistake.
                    val = self.debuginfo.get_constant_strict(
                        actual_ambiguous_matches[0]["name"],
                        actual_ambiguous_matches[0]["elf_id"]
                    )
                    # The first is always the same with the first.
                    success = True
                    # Skip the first which is used to check against.
                    for match in actual_ambiguous_matches[1:]:
                        try:
                            variable = self.debuginfo.get_constant_strict(
                                match["name"],
                                match["elf_id"]
                            )
                            if val != variable:
                                success = False
                                break
                        # Todo remevoe this if B-242063 is corrected.
                        except BaseException:
                            success = False
                            break

                    if success:
                        # We actually got only one register match - work with
                        # it.
                        reg = self.debuginfo.get_constant_strict(
                            actual_ambiguous_matches[0]["name"]
                        )
                    else:
                        apology = "Multiple potential matches found " + \
                            "for register name '" + identifier + "': "
                        raise AmbiguousSymbolError(
                            apology, actual_ambiguous_matches)
            try:
                if reg and (Arch.get_dm_region(reg.value) != "MMR"):
                    # Reg.value wasn't the address of a memory-mapped
                    # register; it was probably a random symbolic
                    # constant. Oh well.
                    return None
            except Arch.NotDmRegion:
                if reg.value == 0xfffffff:
                    # For Crescendo, it has been noticed that the register are
                    # being treated as constants with the value 0xfffffff.
                    # Furthermore, must strip the C and asm specific symbols
                    # for get_reg_strict().
                    try:
                        if '$_' in reg.name:
                            reg_name = reg.name[2:]
                            return self.chipdata.get_reg_strict(reg_name)
                        elif '$' in reg.name:
                            reg_name = reg.name[1:]
                            return self.chipdata.get_reg_strict(reg_name)
                    except BaseException:
                        return self.chipdata.get_reg_strict(reg.name)

        if reg is None:
            return None

        # If we get here, we found something.
        # There's a small chance we've got an unrelated constant, if its
        # value looks sufficiently like the address of a memory-mapped
        # register (e.g. $codec.stream_decode.FAST_AVERAGE_SHIFT_CONST).
        # Can't do much about that.

        # Look up the register contents.
        try:
            regcontents = self.chipdata.get_data(reg.value)
            fullreg = ct.DataSym(reg.name, reg.value, regcontents)
        except KeyError:
            # Reg.value wasn't the address of a register, for some reason.
            fullreg = None

        return fullreg

    def get_var(self, identifier, elf_id=None, datalen=None):
        """Get a variable.

        Like Analysis.get_var_strict(), except it's not strict (!)
        'identifier' can be a variable name, or address. If it's a name,
        we attempt to find the closest match in our list of variables.

        In this case the user can also provide a data length; if it is set,
        we return a slice of data, 'datalen' addressable units long starting at
        the address specified by 'identifier'.

        Args:
            identifier: Could be name or address.
            elf_id (int, optional): The bundle elf id if the variable is
                in a downloadable capability.
            datalen: If the identifier is an address the data length is
                specified by this input.

        Returns:
            A Variable.

        Raises:
            AmbiguousSymbolError: If more than one match is found.
        """
        # For Crescendo, data can only be fetched as words. Since it is
        # octet-addressed, the addresses must be divisible with the number of
        # addresses per word (32 bit words - 4 octets, therefore addresses must
        # be divisible with 4).
        if isinstance(identifier, numbers.Integral):
            identifier = cu.get_correct_addr(identifier, Arch.addr_per_word)

        # Same as above. The lengths are measured in addressable units.
        if datalen is not None:
            datalen = cu.convert_byte_len_word(datalen, Arch.addr_per_word)
        # The following is necessary since we can't rely on variable
        # sizes. If a (say) register address was passed in here we will likely
        # match a variable entry for $flash.data24.__Limit.
        if isinstance(identifier, numbers.Integral) and \
                Arch.get_dm_region(identifier) == "MMR":
            return None

        # First, look up the variable in the debug information.
        # Even if the user supplied an address rather than a name, it's nice
        # if we can tell them which variable it might be part of.

        # Might throw an AmbiguousSymbolError exception here; can't get that
        # with an address but can with a variable name.
        var = None
        try:
            var = self.debuginfo.get_var(identifier, elf_id)
        except AmbiguousSymbolError as amb:
            # Filter out matches of struct/array members, where their parent is
            # also in the list of matches.
            matches = amb.args[1]
            quarantine_list = []
            for match in matches:
                try:
                    mvar = self.debuginfo.get_var_strict(
                        match["name"], match["elf_id"]
                    )
                    if mvar.parent is not None and mvar.parent.name in matches:
                        # This is a struct/array member
                        quarantine_list.append(match)
                    else:
                        possible_parent = mvar
                except ValueError:
                    # Out of memory can be seen for asm memory reagions. Ignore
                    # them.
                    quarantine_list.append(match)

            # If the number of things in the quarantine list is EXACTLY
            # ONE MORE than the number of things in the matches list, then
            # we probably have found a single variable and all its
            # members.
            if len(matches) == len(quarantine_list) + 1:
                var = possible_parent
            else:
                # Give up
                raise AmbiguousSymbolError(amb.args[0], amb.args[1])

        if var is None:
            return None

        # Don't necessarily want to modify the actual variable entry below*,
        # so maybe create a copy here.
        # * Why? Well var is just a reference to the original variable in the
        # debuginfo class - we ought not to change it frivolously, since it
        # could break some other analysis.
        # In this case, we don't want to permanently munge the name
        # just because we're doing a slice this time.
        ret_var = var
        if datalen:
            if isinstance(identifier, numbers.Integral):
                if var.address == identifier and var.size <= datalen:
                    ret_var = copy.deepcopy(var)
                    ret_var.name = "User-defined slice, part of: " + \
                        var.name + " ???"
                    # We want to get a slice of data, not just the variable
                    # entry.
                    ret_var.size = datalen
                    # If the identifier is a variable name, don't include any
                    # members we might have already inspected.
                    ret_var.members = None
                else:
                    ret_var = ct.Variable("???", identifier, datalen)
        else:
            # Mitigation: we can't rely on 'var' actually containing the
            # supplied address, due to the lack of size information (see
            # KerDebugInfo.py). So work around it here.
            if (isinstance(identifier, numbers.Integral) and
                    identifier >= var.address + Arch.addr_per_word * var.size):
                # Just return the value at the given address.
                ret_var = ct.Variable(var.name + " ???", identifier, 1)

        # Now get the data value(s) from chipdata. Look in DM first, only
        # try const if we run out of options.
        try:
            ret_var.value = self.chipdata.get_data(
                ret_var.address, ret_var.size
            )
        except InvalidDmLengthError as oor:
            # Address was valid, but size was not.
            # oor.args[1] contains the last valid address in the supplied
            # range.
            valid_size = (oor.max_length - ret_var.address) + 1
            ret_var.value = self.chipdata.get_data(ret_var.address, valid_size)
            ret_var.size = valid_size
        except InvalidDmAddressError:
            # The address wasn't valid. Could be that this variable is
            # actually in dm const.
            try:
                ret_var.value = self.debuginfo.get_dm_const(
                    ret_var.address, ret_var.size
                )
            except InvalidDmConstLengthError as oor:
                # Address was valid, but size was not.
                valid_size = oor.max_length - ret_var.address
                ret_var.value = self.debuginfo.get_dm_const(
                    ret_var.address, valid_size
                )
            except InvalidDmConstAddressError:
                # Ok we really are stuck. Return variable with a value of None.
                debug_info = self.debuginfo.get_kymera_debuginfo()
                ret_var.value = debug_info .debug_strings[ret_var.address]
                return ret_var

        # Need a way to work out whether we've already inspected this
        # variable, so we can avoid doing it more than once.
        # An inspection *should* result in a non-empty type_name string.
        # Also, don't inspect the slices. It would be bad.
        ret_var.members = []
        var_elf_id = self.debuginfo.table.get_elf_id_from_address(
            ret_var.address
        )
        if not var_elf_id:
            var_elf_id = self.debuginfo.get_kymera_debuginfo().elf_id

        self.debuginfo.inspect_var(ret_var, var_elf_id)
        return ret_var


#########################################
class Processor(Functions):
    """ Creates objects with specific information for all the cores.

    The formatter is included because of the differences between the
    Automatic and Interactive modes.

    Args:
        coredump_lines
        build_output_path
        processor
        formatter
    """

    def __init__(self, coredump_lines, build_output_path,
                 processor, formatter):
        # Initialise the specific information for a core: chipdata,
        # debuginfo and the functions used in Interactive mode that use
        # the specific information.
        self.processor = processor
        self._kalaccess = None
        self.available_analyses = {}
        if cu.global_options.live:
            if self.processor == 0:
                self._kalaccess = cu.global_options.kal
            else:
                self._kalaccess = cu.global_options.kal2

            if cu.global_options.kalcmd_object is not None:
                from ACAT.Core.LiveKalcmd import LiveKalcmd
                self.chipdata = LiveKalcmd(
                    cu.global_options.kalcmd_object, self.processor
                )
            else:
                from ACAT.Core.LiveSpi import LiveSpi
                if self.processor == 0:
                    self.chipdata = LiveSpi(
                        cu.global_options.kal,
                        cu.global_options.spi_trans,
                        self.processor,
                        wait_for_proc_to_start=cu.global_options.
                        wait_for_proc_to_start
                    )
                else:
                    self.chipdata = LiveSpi(
                        cu.global_options.kal2,
                        cu.global_options.spi_trans,
                        self.processor,
                        wait_for_proc_to_start=False
                    )
        else:
            from ACAT.Core.Coredump import Coredump
            self.chipdata = Coredump(coredump_lines, processor)

        if build_output_path == "":
            # Try to get the build path automatically. If this fails it will
            # throw an exception and we'll bail.
            build_output_path = get_build_output_path(self.chipdata)

            # search for the elf file name
            import glob
            elf_files = glob.glob(os.path.join(build_output_path, '*.elf'))
            # Filter out the "_external.elf" files generated by some _release
            # builds -- we want the corresponding internal one with full
            # symbols (which we assume must exist).
            elf_files = [
                elf_file for elf_file in elf_files
                if not elf_file.endswith("_external.elf")
            ]
            if len(elf_files) > 1:
                raise UsageError(
                    "ERROR: Multiple elf files in the build output, "
                    "don't know which to use.")
            # remove the .elf extension
            build_output_path = elf_files[0].replace(".elf", "")
            cu.global_options.build_output_path_p0 = build_output_path
            cu.global_options.build_output_path_p1 = build_output_path

        if processor == 0:
            if cu.global_options.kymera_p0 is None:
                self.debuginfo = DebugInformation(cu.global_options.ker)
                self.debuginfo.read_kymera_debuginfo(build_output_path)
                cu.global_options.kymera_p0 = self.debuginfo

                # If the two build are the same set the main build for the
                # other processor
                if cu.global_options.build_output_path_p0 == \
                        cu.global_options.build_output_path_p1:

                    cu.global_options.kymera_p1 = \
                        cu.global_options.kymera_p0
            else:
                self.debuginfo = cu.global_options.kymera_p0
        else:
            if cu.global_options.kymera_p1 is None:
                self.debuginfo = DebugInformation(cu.global_options.ker)
                self.debuginfo.read_kymera_debuginfo(build_output_path)
                cu.global_options.kymera_p1 = self.debuginfo

                # If the two build are the same set the main build for the
                # other processor
                if cu.global_options.build_output_path_p0 == \
                        cu.global_options.build_output_path_p1:

                    cu.global_options.kymera_p0 = \
                        cu.global_options.kymera_p1
            else:
                self.debuginfo = cu.global_options.kymera_p1

        # check if there are any bundles that needs reading
        if cu.global_options.bundle_paths is not None:
            # check if they were already read
            if cu.global_options.bundles is None:
                # if not than read all of them
                bundles_dictionary = {}
                for bundle_path in cu.global_options.bundle_paths:
                    bundles_dictionary.update(load_bundle(bundle_path))
                # and save it in global_options to avoid reading them multiple
                # times.
                cu.global_options.bundles = bundles_dictionary

            # set the bundle dictionary.
            self.debuginfo.update_bundles(cu.global_options.bundles)

        self.debuginfo.set_table(MappedTable(self.chipdata, self.debuginfo))

        # Set the debug info for the chipdata
        self.chipdata.set_debuginfo(self.debuginfo)

        super(Processor, self).__init__(
            self.chipdata,
            self.debuginfo,
            formatter
        )

        # Patch the processor
        ProcessorPatcher(self, patch_path=cu.global_options.patch).apply()

        self.formatter = formatter

    @property
    def kalaccess(self):
        """Return the kalaccess instance.

        The return value will be None if there is no kalaccess instance
        available for this processor.

        Returns:
            Kalaccess instance
        """
        return self._kalaccess

    def get_patch_id(self):
        """Reads the relevant variable and return the patch ID.

        Returns:
            An integer, zero if there is no patch detected.
        """
        return self.chipdata.get_patch_id()

    def is_booted(self):
        """Check if the processor is booted.

        The booted state is decided by the Program Counter value.

        Returns:
            bool: True if the processor was booted, False otherwise.
        """
        return self.chipdata.is_booted()

    def __getattribute__(self, name):
        available_analyses = object.__getattribute__(
            self,
            "available_analyses"
        )

        if name not in available_analyses:
            # Default behaviour
            return object.__getattribute__(self, name)

        proc_nr = object.__getattribute__(self, "chipdata").processor
        if proc_nr != 1:
            # Default behaviour
            return object.__getattribute__(self, name)

        booted = object.__getattribute__(self, "is_booted")()
        if booted:
            # Default behaviour
            return object.__getattribute__(self, name)

        raise AnalysisError("Processor 1 not booted!")

    def get_analysis(self, name):
        """Returns the analysis asked by name.

        Args:
            name (str): Name of the analyses.
        """
        return self.available_analyses[name]
