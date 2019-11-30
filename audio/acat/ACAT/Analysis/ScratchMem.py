############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2015 - 2019 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
"""
Module to analyse the scratch memory.
"""
try:
    from future_builtins import hex
except ImportError:
    pass

from . import Analysis

VARIABLE_DEPENDENCIES = {'strict': ('$_per_prio_data', '$_first_scratch_mem')}
TYPE_DEPENDENCIES = {
    'scratch_per_prio_data': (
        'alloc_info_dm1', 'alloc_info_dm1.alloc_size', 'alloc_info_dm2',
        'alloc_info_dm2.alloc_size', 'alloc_info_none',
        'alloc_info_none.alloc_size', 'refcount'
    ),
    'scratch_per_task_data': ('next',)
}


class ScratchMem(Analysis.Analysis):
    """Encapsulates an analysis for usage of scratch memory.

    Args:
        **kwarg: Arbitrary keyword arguments.
    """

    def __init__(self, **kwarg):
        # This call will set all the values from the dictionary as attributes
        Analysis.Analysis.__init__(self, **kwarg)

    def run_all(self):
        """Performs analysis.

        Perform all useful analysis and direct the output via the
        Analysis'formatter.  It analyses the scratch memory usage in DM1,
        DM2 and none, per priority and per task.
        """
        self.formatter.section_start('Scratch Memory Info')
        self.formatter.section_start('Per priority')
        self.analyse_per_priority()
        self.formatter.section_end()
        self.formatter.section_start('Per task')
        self.analyse_per_task()
        self.formatter.section_end()
        self.formatter.section_end()

    ##################################################
    # Private methods
    ##################################################

    def analyse_per_priority(self):
        """Analyses per priority.

        Displays the total scratch memory allocations in DM1, DM2 and none
        and their values per each priority level.  Furthermore, it also
        displays the total number of users per each priority level.  The
        function reads and stores the allocations info from per_prio_data
        in the first for loop in order to display the total values first.
        In the second for loop it simply displays the stored values for
        the scratch memory (total, DM1, DM2, none) for each priority.
        """
        dm1_total = 0
        dm2_total = 0
        none_total = 0
        dm1_size = []
        dm2_size = []
        none_size = []
        users = []
        pdd = self.chipdata.get_var_strict('$_per_prio_data')
        num_entries = len(pdd.members)

        for current in range(0, num_entries):
            dm1_size.append(
                pdd.members[current].get_member('alloc_info_dm1')
                .get_member('alloc_size').value
            )
            dm2_size.append(
                pdd.members[current].get_member('alloc_info_dm2')
                .get_member('alloc_size').value
            )
            none_size.append(
                pdd.members[current].get_member('alloc_info_none')
                .get_member('alloc_size').value
            )
            users.append(pdd.members[current].get_member('refcount').value)
            dm1_total = dm1_total + dm1_size[current]
            dm2_total = dm2_total + dm2_size[current]
            none_total = none_total + none_size[current]

        total_mem = dm1_total + dm2_total + none_total
        self.formatter.output('Total scratch memory used: ' + str(total_mem))
        self.formatter.output('DM1 total: ' + str(dm1_total))
        self.formatter.output('DM2 total: ' + str(dm2_total))
        self.formatter.output('none total: ' + str(none_total))

        for current in range(0, num_entries):
            mem = dm1_size[current] + dm2_size[current] + none_size[current]
            self.formatter.output(
                'For priority ' + str(current) + ' the memory allocated is ' +
                str(mem) + ' and the total no of users is ' +
                str(users[current])
            )
            self.formatter.output('DM1 ' + str(dm1_size[current]))
            self.formatter.output('DM2 ' + str(dm2_size[current]))
            self.formatter.output('none ' + str(none_size[current]))

    def analyse_per_task(self, task_id=None):
        """Displays the scratch memory info per task.

        It can take one parameter that represents the task id and it will
        display the info for it.

        The function goes through the linked list and if no value is
        passed to it, it displays the complete list. Otherwise, it
        displays the info for the task with the id inputted or a message
        (if the task id is invalid).

        Args:
            task_id (int, optional)
        """
        per_task = self.chipdata.cast(
            self.chipdata.get_var_strict('$_first_scratch_mem').address,
            'scratch_per_task_data'
        )
        matching_id = False
        for sc_table in self.parse_linked_list(per_task.address, 'next'):
            if (task_id is None) or (sc_table.value[0] is task_id):
                self.formatter.output(str(sc_table))
                matching_id = True
        if (task_id is not None) and (not matching_id):
            self.formatter.output(
                'There is no task id = ' + str(hex(task_id)) + '!'
            )
