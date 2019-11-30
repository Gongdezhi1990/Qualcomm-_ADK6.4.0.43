############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2013 - 2019 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
"""
Analyses for the dorm module in Kymera.
"""
from . import Analysis

VARIABLE_DEPENDENCIES = {
    'one_not_strict': ('$_kip_table', 'L_dorm_table'),
}
ENUM_DEPENDENCIES = {'strict': ('dorm_ids',)}


class Dorm(Analysis.Analysis):
    """Encapsulates an analysis for Dorm/deep sleep.

    Args:
        **kwarg: Arbitrary keyword arguments.
    """

    def __init__(self, **kwarg):
        # Call the base class constructor.
        Analysis.Analysis.__init__(self, **kwarg)

    def run_all(self):
        """Perform all useful analysis and spew the output.

        It analyses the dorm state, whether deep sleep is allowed and if
        there is something preventing it.
        """
        self.formatter.section_start('Dorm')
        self.analyse_dorm_state()
        self.formatter.section_end()

    #######################################################################
    # Analysis methods - public since we may want to call them individually
    #######################################################################
    def analyse_dorm_state(self):
        """Determine whether we are preventing deep sleep."""
        # The elements of kip_table are indexed by dormid, and the values are
        # or's of bits given by dorm_state. Each bit indicates we can or can't
        # go into shallow sleep.
        if self.chipdata.identifier_exists('L_dorm_table'):
            kip_table = self.chipdata.get_var_strict('L_dorm_table')
        elif self.chipdata.identifier_exists('$_kip_table'):
            kip_table = self.chipdata.get_var_strict('$_kip_table')
        else:
            # Maybe this is taken from an old build without deep sleep support?
            self.formatter.error("Deep sleep unsupported in this build??")
            return

        is_deep_sleep_permitted = True
        for i, entry in enumerate(kip_table):
            dorm_id = self.debuginfo.get_enum('dorm_ids', i)[0]

            if entry.value != 0:
                self.formatter.output(
                    str(dorm_id) + ' is preventing deep sleep with state {}.'.
                    format(entry.value)
                )
                is_deep_sleep_permitted = False

        if is_deep_sleep_permitted:
            self.formatter.output('Deep sleep is permitted.')

    #######################################################################
    # Private methods - don't call these externally.
    #######################################################################
    @staticmethod
    def _is_deep_sleep_permitted(kip_table):
        """Check whether the sleep is permitted."""
        for _, entry in enumerate(kip_table):
            state = entry.value
            if state != 0:
                return False

        return True