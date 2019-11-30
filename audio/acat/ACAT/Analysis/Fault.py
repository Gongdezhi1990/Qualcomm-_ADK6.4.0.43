############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2016 - 2019 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
"""
Module displays the faults on Kymera.
"""
from . import Analysis

VARIABLE_DEPENDENCIES = {
    'strict': ('L_faultinfo', '$_preserved', '$_preserved')
}
ENUM_DEPENDENCIES = {'not_strict': ('faultid',)}


class Fault(Analysis.Analysis):
    """Fault analysis.

    Retrieves information about faults that have occurred on the chip. An
    array faultinfo records instances of faults. The last fault to have
    occurred has it's diatribe and id stored in the preserve memory block.

    Args:
        **kwarg: Arbitrary keyword arguments.
    """

    def __init__(self, **kwarg):
        Analysis.Analysis.__init__(self, **kwarg)

    def run_all(self):
        """Perform all useful analysis and spew the output to the formatter.

        Displays the contents of the faults from faultinfo array and the
        preserve block.
        """
        self.formatter.section_start('Fault Info')
        self.analyse_faultinfo()
        self.analyse_preserve_block_fault()
        self.formatter.section_end()

    @staticmethod
    def _last_faultrecord(faultlist):
        """Generates the last fault.

        last_faultrecord(iterable) -> generates (thing, islast) pairs

        Generates pairs where the first element is an item from the iterable
        source and the second element is a boolean flag indicating if it is
        the last item in the sequence. Used to determine last element in
        faultinfo[].

        Args:
            faultlist
        """
        record = iter(faultlist)
        prev = next(record)
        for entry in record:
            yield prev, False
            prev = entry
        yield prev, True

    def analyse_faultinfo(self):
        """Retrieve and display data from faultinfo[]."""
        fault_record_exists = False
        empty_record_present = False
        faultinfo = self.chipdata.get_var_strict('L_faultinfo')

        # Loop checks for last entry
        for entry, islast in self._last_faultrecord(faultinfo):

            fault_id = entry.get_member('f').value

            try:
                fault_name = self.debuginfo.get_enum('faultid', fault_id)[0]
                if fault_id != 0:
                    fault_record_exists = True
                    try:
                        self.formatter.alert(
                            (
                                'Chip faulted with fault id %s and diatribe '
                                '0x%x on a total of %d occasions'
                            ) % (
                                fault_name,
                                entry.get_member('arg').value,
                                entry.get_member('tn').value
                            )
                        )
                    except BaseException:
                        self.formatter.alert(
                            (
                                'Chip faulted with fault id %s and diatribe '
                                '0x%x on %d occasions since last published'
                            ) % (
                                fault_name,
                                entry.get_member('arg').value,
                                entry.get_member('n').value
                            )
                        )
                    self.formatter.output(
                        'Time fault first reported: %d' %
                        entry.get_member('st').value
                    )

                    # The array faultinfo has a size of 4. After 4 types of
                    # fault have occurred, any new ones flagged are discarded
                    if islast and empty_record_present is not True:
                        self.formatter.output(
                            'Faultinfo[] full. Compare last faultinfo[] '
                            'entry (above) with last recorded fault \n'
                            'to check if more than 4 fault types have '
                            'occurred in the last cycle.'
                        )
                else:
                    empty_record_present = True
            # Invalid faultids found through exception
            except KeyError as exception:
                if 'Invalid enum value' in str(exception):
                    self.formatter.output('Invalid fault_id present')

        if fault_record_exists is not True:
            self.formatter.output('No faults recorded in coredump')

    def analyse_preserve_block_fault(self):
        """Retrieve and display last fault recorded from preserve block."""
        if self.chipdata.processor == 0:
            preserved = self.chipdata.get_var_strict('$_preserved')
            fault = preserved.get_member("fault")
            last_fault_id = fault.get_member('last_id').value
            last_fault_diatribe = fault.get_member('last_arg').value
        else:
            preserved = self.chipdata.get_var_strict('$_preserved1')
            fault = preserved.get_member("fault")
            last_fault_id = fault.get_member('last_id').value
            last_fault_diatribe = fault.get_member('last_arg').value

        if last_fault_id != 0 and (last_fault_id & 0xFF0000 == 0):
            # for Crescendo, the preserved block is populated with random
            # values, so unless  a fault has occurred the fault_id will most
            # probably be invalid
            try:
                # The chip faulted at some point
                fault_id_str = self.debuginfo.get_enum(
                    'faultid', last_fault_id
                )[0]  # enum name
                self.formatter.alert(
                    'Last recorded fault on chip: '
                    'fault id %s and diatribe 0x%x' %
                    (fault_id_str, last_fault_diatribe))
            except KeyError as exception:
                if 'Invalid enum value' in str(exception):
                    self.formatter.output(
                        'Invalid fault_id on preserve block -'
                        ' might be because the chip has never faulted.'
                    )
        else:
            self.formatter.output('Chip has never faulted')
