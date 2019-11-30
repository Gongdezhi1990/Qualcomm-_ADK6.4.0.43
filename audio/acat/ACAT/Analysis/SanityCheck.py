############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2012 - 2019 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
"""
Module checks FW id and other useful sanity checks.
"""
import re

from . import Analysis
from ..Core import Arch
from ACAT.Core import CoreUtils as cu
from ACAT.Core.exceptions import (
    FatalAnalysisError, BundleMissingError, DebugInfoNoVariableError
)

try:
    from future_builtins import hex
except ImportError:
    pass

# Assumes p0 is used
VARIABLE_DEPENDENCIES = {
    'strict': (
        '$_audio_slt_table', '$_build_identifier_integer',
        '$_build_identifier_string'
    ),
    'not_strict': ('L_panic_on_fault_enabled', 'L_panic_on_fault_id'),
    'one_strict': (('$_preserved', '$_preserved1'),)
}
ENUM_DEPENDENCIES = {
    'not_strict': ('panicid', 'exception_type_enum', 'faultid')
}
TYPE_DEPENDENCIES = {
    '$_preserved': (
        'panic', 'panic.last_id', 'panic.last_arg', 'panic.last_time'
    )
}


class SanityCheck(Analysis.Analysis):
    """Performs high-level sanity checks.

    Simple sanity checks like 'has the chip panicked' and so on.

    Args:
        **kwargs: Arbitrary keyword arguments.
    """

    def __init__(self, **kwargs):
        # Call the base class constructor.
        Analysis.Analysis.__init__(self, **kwargs)

    def run_all(self):
        """Performs analysis and spew the output to the formatter.

        It analyses the firmware ID to make sure we supplied the correct
        build output, if the chip has panicked and it also analyses the
        Symbol Look-Up Table.
        """
        self.formatter.section_start('Firmware info')
        self.analyse_firmware_id()  # Always do this first!
        # If the chip has not panicked, the preserved
        # block is populated with random values, therefore
        # until the magic_value is implemented, do a try and except
        self.analyse_panic_state()
        self.analyse_slt()  # Kind of pointless but why not.
        self.formatter.section_end()

    def chip_has_panicked(self):
        """Works out whether we are currently sitting in the panic code.

        Returns:
            bool: True if the chip has panicked, False otherwise.
        """
        try:
            program_counter = self.where_is_pc()  # a SourceInfo
        except BundleMissingError:
            # The PC is in a downloadable capability which elf is not loaded
            # in ACAT. One is for sure the the PC is not in panic because
            # the panic is implemented in the main elf which is always present.
            return False

        if re.search("panic_diatribe", program_counter.module_name):
            return True

        return False

    def get_panic_time(self):
        """Get the panic time.

        If we panicked at some point in the past, returns the value of
        TIMER_TIME at which we did so. If we haven't ever panicked,
        returns None.
        """
        preserve_block = self.get_preserve_block()

        # Look for a valid panic code at the start of the preserve block.
        # If we've never panicked, the preserve block will contain garbage
        # values.  Try and filter them out by ignoring anything with the
        # top 8 bits set.  No valid panic code (at the time of writing!)
        # has these bits set.
        panic_var = preserve_block.get_member('panic')
        time_arr = panic_var.get_member('last_time').value
        if Arch.kal_arch == 4:
            # 32 bit
            panic_time = time_arr
        else:
            # 24 bit
            panic_time = ((time_arr[0] << 24) | time_arr[1])
        return panic_time

    def panic_on_fault_enabled(self):
        """Check whether panic on fault value is set.

        Returns:
            tuple: containing:
                 * True/False answer to whether panic_on_fault is enabled.
                 * The fault ID for which we have panic_on_fault enabled
                   (0 implies we panic on all faults).
        """
        # The panic_on_fault mechanism might not even be included in the
        # build (in which case the panic_on_fault variables won't exist),
        # so be defensive.
        try:
            enabled = self.chipdata.get_var_strict(
                'L_panic_on_fault_enabled'
            ).value
            fault_id = self.chipdata.get_var_strict(
                'L_panic_on_fault_id'
            ).value
        except DebugInfoNoVariableError:
            enabled = False
            fault_id = 0
        return (enabled, fault_id)

    def where_is_pc(self):
        """Returns the current location of the Program Counter.

        Returns a SourceInfo. Function can raise BundleMissingError if
        current program_counter is in a downloadable capability which is
        not loaded to ACAT.
        """
        program_counter = self.chipdata.get_reg_strict('REGFILE_PC')
        return self.debuginfo.get_source_info(program_counter.value)

    def get_preserve_block(self):
        """Returns a variable containing the preserve block data.

        This is the current format of the preserve_block. It's obviously
        subject to change in future.

        $_preserved
          $_preserved.header
            $_preserved.header.magic_value
            $_preserved.header.checksum
            $_preserved.header.length
          $_preserved.panic
            $_preserved.panic.last_id
            $_preserved.panic.last_arg
            $_preserved.panic.last_time (2 words)
          $_preserved.fault
            $_preserved.fault.last_id
            $_preserved.fault.last_arg
        """
        if self.chipdata.processor == 0:
            return self.chipdata.get_var_strict('$_preserved')

        return self.chipdata.get_var_strict('$_preserved1')

    def get_slt(self):
        """Gets the SLT.

        Returns:
            tuple: Of (key, entry) pairs.
        """
        try:
            slt_ptr = self.chipdata.get_var_strict('$_audio_slt_table').address
        except DebugInfoNoVariableError:
            try:
                # old implementation
                slt_ptr = self.chipdata.get_var_strict(
                    '$_audio_slut_table'
                ).address
            except DebugInfoNoVariableError:
                return None

        # If we read 20 words without stopping, something went wrong!
        failsafe = slt_ptr + 20 * Arch.addr_per_word
        slt_table = []

        while slt_ptr < failsafe:
            slt_key = self.debuginfo.get_dm_const(slt_ptr, 0)
            slt_value = self.debuginfo.get_dm_const(
                slt_ptr + Arch.addr_per_word,
                0
            )
            slt_table.append((slt_key, slt_value))
            slt_ptr += 2 * Arch.addr_per_word

            if slt_key == 0:
                break

        return tuple(slt_table)

    def get_debug_firmware_id(self):
        """Returns the firmware id.

        Returns:
            int: the firmware ID (integer) from our debug information.
        """
        # Read the address via get_var_strict; this will fetch the value
        # from chipdata as well, but we can ignore it.
        int_addr = self.debuginfo.get_var_strict(
            '$_build_identifier_integer'
        ).address
        build_id_int = self.debuginfo.get_dm_const(int_addr, 0)
        if Arch.addr_per_word == 4:
            # in a 32 bit word, the 16 bit build ID int can be in either the 2
            # MS bytes or 2 LS bytes
            if int_addr % Arch.addr_per_word != 0:
                build_id_int >>= 16
            build_id_int &= 0xFFFF
        return build_id_int

    def get_debug_firmware_id_string(self):
        """Gets firmware id in string.

        Returns:
            int: The firmware ID string from our debug information.
        """
        # Read the address via get_var_strict; this will fetch the value
        # from chipdata as well, but we can ignore it.
        chip_str = self.chipdata.get_var_strict('$_build_identifier_string')
        rawstr = self.debuginfo.get_dm_const(chip_str.address, chip_str.size)

        decoded_str = ""
        for chars in rawstr:
            if Arch.addr_per_word == 4:
                # The raw string is encoded with four chars per word
                string = cu.get_string_from_word(Arch.addr_per_word, chars)
                stop_decoding = False
                for char in string:
                    if char != '\0':
                        decoded_str += char
                    else:
                        stop_decoding = True
                        break
                if stop_decoding:
                    break
            else:
                # The raw string is encoded with two chars per word
                upper_part = (chars & 0xFF00) >> 8
                lower_part = chars & 0x00FF
                # strip the null terminator.
                if upper_part != 0:
                    decoded_str += chr(upper_part)
                else:
                    break
                if lower_part != 0:
                    decoded_str += chr(lower_part)
                else:
                    break

        return decoded_str.strip()  # Strip any leading/trailing whitespace

    #######################################################################
    # Analysis methods - public since we may want to call them individually
    #######################################################################

    def analyse_firmware_id(self, id_mismatch_allowed=None):
        """Check the firmware id for mismatch.

        Compare the firmware ID reported in the chipdata with the one in
        the debug information (checks we have got the correct build
        output) id_mismatch_allowed is a boolean. If supplied, it
        overrides any command-line setting.

        Args:
            id_mismatch_allowed (int)
        """
        if Arch.chip_arch == "KAS":
            self.formatter.section_start('Firmware')
            self.formatter.alert("KAS.. ID not currently supported")
            self.formatter.section_end()
            return

        if id_mismatch_allowed is not None:
            permit_mismatch = id_mismatch_allowed
        else:
            permit_mismatch = cu.global_options.build_mismatch_allowed

        if self.chipdata.processor == 1:
            permit_mismatch = True
        # Perform the ID check even if there is no formatter supplied.
        # If we fail this test then we won't need one anyway.
        debug_id = self.get_debug_firmware_id()
        chipdata_id = self.chipdata.get_firmware_id()

        # The build ID check does not work for old style
        # Bluecore coredumps because the firmware
        # ID did not use to be recorded and the ID returned was 0.
        # The Coredump tool was modified to do
        # this in B-204537.
        # For this particular case, the check is being skipped.
        if (
                not self.chipdata.is_volatile() and
                Arch.chip_arch == "Bluecore" and
                chipdata_id == 0
        ):
            permit_mismatch = True
            self.formatter.alert(
                "Might be dealing with an old Bluecore coredump when "
                "ID check was not supported as build ID is 0."
            )

        if debug_id != chipdata_id:
            grave_warning = 'Chip does not match supplied build output! '
            # (Firmware IDs are stored in decimal, so output them in decimal.)
            grave_warning += 'Chip ID is %d, build output is ID %d' % (
                chipdata_id, debug_id
            )
            if not permit_mismatch:
                self.formatter.error(grave_warning)
                raise FatalAnalysisError(grave_warning)
            else:
                self.formatter.alert(grave_warning)

        debug_id_string = self.get_debug_firmware_id_string()
        chipdata_id_string = self.chipdata.get_firmware_id_string()

        # Whitespace is already stripped, should be able to just check for
        # equality.
        if debug_id_string != chipdata_id_string:
            grave_warning = 'Chip does not match supplied build output! '
            grave_warning += (
                "Chip ID string is '%s', build output is ID '%s'" % (
                    chipdata_id_string,
                    debug_id_string
                )
            )
            if not permit_mismatch:
                self.formatter.error(grave_warning)
                raise FatalAnalysisError(grave_warning)
            else:
                self.formatter.alert(grave_warning)

        # Now we can raise an exception if necessary.
        self.formatter.section_start('Firmware')
        self.formatter.output('Firmware ID: ' + str(debug_id))
        self.formatter.output('Firmware ID String: ' + debug_id_string)
        self.formatter.section_end()

    def analyse_panic_state(self):
        """Checks if the chip is panicked.

        Determines whether the chip has panicked? Or faulted previously?
        """
        preserve_block = self.get_preserve_block()
        # If we've ever panicked, the preserve block should contain details.
        preserve_panic_valid = (self.get_panic_time() is not None)

        # This isn't especially sophisticated, but it gets the job done.
        # Other things we could do here:
        #  ~ Look at the Debug Log to see if there are any other previous
        #    faults.
        panic_id = preserve_block.get_member(
            'panic'
        ).get_member('last_id').value
        panic_diatribe = preserve_block.get_member('panic').get_member(
            'last_arg'
        ).value

        self.formatter.section_start('Panic status')
        if self.chip_has_panicked() and preserve_panic_valid:
            # for Crescendo, the preserved block is populated with random
            # values, so unless a panic has occurred the panic_id will
            # most probably be invalid
            try:
                panic_id_str = self.debuginfo.get_enum(
                    'panicid', panic_id
                )[0]  # enum name
                self.formatter.alert(
                    'Chip has panicked with panic id %s and diatribe 0x%x' %
                    (panic_id_str, panic_diatribe)
                )
                self.formatter.output(
                    'Time of panic was ' + hex(self.get_panic_time())
                )
            except KeyError as exception:
                if "Invalid enum" in str(exception):
                    self.formatter.output(
                        'Invalid panic_id '
                        '- might be because the chip has never panicked.'
                    )
        elif self.chip_has_panicked():
            self.formatter.alert(
                'Chip has panicked, but no details are known!'
            )
        elif preserve_panic_valid:
            # for Crescendo, the preserved block is populated with random
            # values, so unless a panic has occurred the panic_id will
            # most probably be invalid
            try:
                panic_id_str = self.debuginfo.get_enum(
                    'panicid', panic_id
                )[0]  # enum name
                self.formatter.alert(
                    (
                        'Chip has not panicked, but did previously '
                        '- with panic id %s and diatribe 0x%x'
                    ) % (
                        panic_id_str,
                        panic_diatribe
                    )
                )
                self.formatter.output(
                    'Time of panic was ' + hex(self.get_panic_time())
                )
            except KeyError as exception:
                if "Invalid enum value" in str(exception):
                    self.formatter.output(
                        'Invalid panic_id '
                        '- might be because the chip has never panicked.'
                    )
        else:
            self.formatter.output('Chip has never panicked')
        try:
            exception_type = self.chipdata.get_reg_strict('$EXCEPTION_TYPE')
            if (
                self.debuginfo.get_enum(
                    'exception_type_enum',
                    'EXCEPTION_TYPE_NONE'
                ) != exception_type.value
            ):
                self.formatter.output(
                    'EXCEPTION_TYPE: ' + self.debuginfo.
                    get_enum('exception_type_enum', exception_type.value)[0]
                )
            if (
                self.debuginfo.get_enum(
                    'exception_type_enum',
                    'EXCEPTION_TYPE_OTHER'
                ) == exception_type.value
            ):
                ext_exception_type = self.chipdata.get_reg_strict(
                    '$EXT_EXCEPTION_TYPE'
                )
                self.formatter.output(
                    'EXT_EXCEPTION_TYPE: ' + self.debuginfo.
                    get_enum(
                        'ext_exception_type_enum',
                        ext_exception_type.value
                    )[0]
                )
        except BaseException:
            pass

        # Retrieves fault on preserve block using fault module
        fault_analysis = self.interpreter.get_analysis(
            "fault", self.chipdata.processor
        )
        fault_analysis.analyse_preserve_block_fault()

        panic_on_fault_setting = self.panic_on_fault_enabled()
        if panic_on_fault_setting[0]:
            panic_on_fault_str = 'Panic on fault is set'
            if panic_on_fault_setting[1] != 0:
                try:
                    p_fault_id_str = self.debuginfo.get_enum(
                        'faultid', panic_on_fault_setting[1]
                    )[0]  # enum name
                    panic_on_fault_str = panic_on_fault_str + \
                        ' for fault id ' + p_fault_id_str + ' only'
                except KeyError as exception:
                    if 'Invalid enum value' in str(exception):
                        self.formatter.output(
                            "Invalid panic_on_fault_id "
                            "- might be because the chip has never panicked."
                        )
            self.formatter.output(panic_on_fault_str)

        self.formatter.section_end()  # end of 'panic status' section

    def analyse_slt(self):
        """Displays the contents of the SLT."""
        slt_content = self.get_slt()
        # SLT is only available in internal builds.
        if slt_content is not None:
            self.formatter.section_start('SLT')
            self.formatter.output_list(self.get_slt())
            self.formatter.section_end()

    def analyse_processor_regs(self):
        """Prints out all of the processor registers."""
        # Can't always work out the address of registers to sort them,
        # so they'll appear in some random dictionary order (sadly).

        self.formatter.section_start('Processor registers')
        # Sort by address
        for name, val in self.chipdata.get_all_proc_regs().items():
            self.formatter.output(name + "\t" + hex(val))
        self.formatter.section_end()

    def analyse_all_mmr(self):
        """Prints out all of the memory-mapped registers.

        Reg names are not precise (i.e. they are guesses), so this should
        only really be used in Interactive mode.
        """
        # The whole memory-mapped register range
        mmr_range = list(
            range(Arch.dRegions['MMR'][0], Arch.dRegions['MMR'][1])
        )
        mmr_list = []
        for i in mmr_range:
            try:
                # Tuple containing (address, register name, register contents)
                mmr_list.append(
                    (
                        i, self.chipdata.get_reg_strict(i).name,
                        self.chipdata.get_reg_strict(i).value
                    )
                )
            except BaseException:
                # Not found
                pass

        self.formatter.section_start('All memory-mapped registers')
        for i in mmr_list:
            self.formatter.output(
                hex(
                    i[0]) +
                " " +
                i[1] +
                " " +
                hex(
                    i[2]))
        self.formatter.section_end()
