############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2014 - 2019 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
"""
Module to analyse the scratch memory.
"""
from . import Analysis

VARIABLE_DEPENDENCIES = {'strict': ('L_shared_memory_list',)}
TYPE_DEPENDENCIES = {'SHARED_MEMORY_TABLE': ('next',)}


class SharedMem(Analysis.Analysis):
    """Encapsulates an analysis for usage of shared memory.

    Args:
        **kwarg: Arbitrary keyword arguments.
    """

    def __init__(self, **kwarg):
        Analysis.Analysis.__init__(self, **kwarg)
        # Shared memory list
        self._shared_memory_list = self.debuginfo.get_var_strict(
            'L_shared_memory_list'
        )

    def run_all(self):
        """Performs analysis and spew the output to the formatter.

        Displays the contents of the shared memory blocks.
        """
        self.formatter.section_start('Shared Memory Info')
        self._shared_memory_blocks()
        self.formatter.section_end()

    ##################################################
    # Private methods
    ##################################################

    def _shared_memory_blocks(self):
        self._shared_memory_list = self.debuginfo.get_var_strict(
            'L_shared_memory_list'
        ).address

        sh_tables = self.parse_linked_list(
            self._shared_memory_list,
            'next'
        )
        for sh_table in sh_tables:
            self.formatter.output(str(sh_table))
