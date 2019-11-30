############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2016 - 2019 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
"""
Module which analyses the heap PM ram.
"""
from ACAT.Analysis.Heap import Heap
from ACAT.Core.exceptions import (
    DebugInfoNoVariableError, OutdatedFwAnalysisError
)

CONSTANT_DEPENDENCIES = {
    'not_strict': (
        '$__pm_heap_start_addr', '$PM_RAM_P0_CODE_START'
    )
}
VARIABLE_DEPENDENCIES = {
    'not_strict': (
        'L_freelist_pm', 'L_freelist_pm_p1', 'L_heap_pm_start',
        'L_heap_pm_end', 'L_heap_pm_p1_start', 'L_heap_pm_p1_end',
        'L_addnl_heap_pm_p1_start', 'L_addnl_heap_pm_p1_end',
        'L_pm_reserved_size'
    )
}
TYPE_DEPENDENCIES = {
    'mem_node_pm': (
        'struct_mem_node.u.magic', 'struct_mem_node.length_32',
        'struct_mem_node.u.next'
    )
}


class HeapPmMem(Heap):
    """Encapsulates an analysis for heap program memory usage.

    Args:
        **kwarg: Arbitrary keyword arguments.
    """
    # heap names
    heap_names = ["PM heap P0", "PM heap P1", "PM heap P1+"]
    max_num_heaps = len(heap_names)
    internal_heap_names = [
        "heap_pm",
        "heap_pm_p1",
        "addnl_heap_pm_p1"

    ]
    internal_free_list_names = [
        "freelist_pm",
        "freelist_pm_p1",
        "freelist_pm_p1"
    ]
    memory_type = "pm"

    def __init__(self, **kwarg):
        Heap.__init__(self, **kwarg)

        # Look up the debuginfo once. Don't do it here though; we don't want
        # to throw an exception from the constructor if something goes
        # wrong.
        self._do_debuginfo_lookup = True
        self.patch_size = None
        self.pmalloc_debug_enabled = False

        self.heaps = []

        self._check_kymera_version()

    def _check_kymera_version(self):
        """Checks if the Kymera version is compatible with this analysis.

        Raises:
            OutdatedFwAnalysisError: For outdated Kymera.
        """
        # There is an older version of PM heap that had fixed sizes for P0
        # and P1 heap. If newer variables are not present, raise the outdated
        #  firmware exception.
        try:
            self.debuginfo.get_var_strict("L_heap_pm_start")
        except DebugInfoNoVariableError:
            # fallback to the old implementation
            raise OutdatedFwAnalysisError()

    def run_all(self):
        """Perform all analysis and spew the output to the formatter.

        Displays the heap_pm memory usage.
        """
        if self.chipdata.processor != 0:
            self.formatter.section_start(
                'Heap %s Memory Info' % (self.memory_type.upper())
            )
            self.formatter.output(
                "Heap PM Memory Analysis is not available for processor %d. \n"
                "The control variables for Heap PM are only available in p0 "
                "domain." % self.chipdata.processor
            )
            self.formatter.section_end()
            return

        Heap.run_all(self)

    #######################################################################
    # Analysis methods - public since we may want to call them individually
    #######################################################################

    def display_memory_map(self):
        """Displays PM memory map based on current settings."""
        self._lookup_debuginfo()

        self.formatter.section_start('PM Memory Map')

        (_, p0_size, p0_start, p0_end, _) = \
            self._get_heap_property(0)
        (_, p1_size, p1_start, p1_end, _) = \
            self._get_heap_property(1)
        (_, addnl_p1_size, addnl_p1_start, addnl_p1_end, _) = \
            self._get_heap_property(2)

        if self.patch_size is not None:
            patch_size = self.chipdata.get_data(self.patch_size)

        code_start = self.debuginfo.get_constant_strict(
            '$PM_RAM_P0_CODE_START'
        ).value
        try:
            code_end = self.debuginfo.get_constant_strict(
                '$__pm_heap_start_addr'
            ).value
        except DebugInfoNoVariableError:
            code_end = p0_start - patch_size
            self.formatter.alert(
                'Constants for static code start and '
                'end address is not specified in build.'
            )

        # Now that all values are read, create the memory map
        output_str = ""
        if addnl_p1_size != 0:
            output_str += (
                "==============================<-{:8x}\n"
                "|   Additional P1 PM Heap    |\n"
                "|        {:6d} Bytes        |\n"
                "==============================<-{:8x}\n"
                "|         P1 Cache           |\n"
                "==============================<-{:8x}\n".format(
                    addnl_p1_end,
                    addnl_p1_size,
                    addnl_p1_start,
                    p1_end
                )
            )

        if p1_size != 0:
            if addnl_p1_size == 0:
                output_str += (
                    "==============================\n"
                    "|         P1 Cache           |\n"
                    "==============================<-{:8x}\n".format(p1_end)
                )
            output_str += (
                "|        P1 PM Heap          |\n" +
                "|       {:7d} Bytes        |\n".format(p1_size) +
                "==============================<-{:8x}\n".format(p1_start)
            )

        if p0_size != 0:
            if p1_size == 0 and addnl_p1_size == 0:
                # No P1
                output_str += (
                    "==============================<-{:8x}\n".format(p0_end)
                )
            output_str += (
                "|        P0 PM Heap          |\n" +
                "|       {:7d} Bytes        |\n".format(p0_size) +
                "==============================<-{:8x}\n".format(p0_start)
            )

        if patch_size != 0:
            output_str += (
                "|        Patch Code          |\n"
                "|         {:5d} Bytes        |\n".format(patch_size)
            )

        if code_start != 0 and code_end != 0:
            if patch_size != 0:
                output_str += (
                    "==============================<-{:8x}\n".format(code_end)
                )
            output_str += (
                "|                            |\n"
                "|       PM RAM Code          |\n"
                "|                            |\n"
                "==============================<-{:8x}\n".format(code_start)
            )

        output_str += (
            "==============================\n"
            "|         P0 Cache           |\n"
            "=============================="
        )
        self.formatter.output_raw(output_str)
        self.formatter.section_end()

    @staticmethod
    def ret_get_watermarks():
        """Same as get_watermarks, but it will return values.

        Returns:
            tuple: The heap usage.
        """
        return 0, -1, -1

    def _get_heap_property(self, heap_number):
        """Gets information about a specific heap.

        Args:
            heap_number (int): The heap number specifies the heap from
                which information is asked.

        Returns:
            tuple: Containing information about heap.

                (available, heap_size, heap_start, heap_end, heap_free_start)

                available - True, if the heap is present in the build.
                heap_size - Size in octets.
                heap_start - Start address.
                heap_end - The last valid address.
                heap_free_start - The address of the first available block.
        """
        internal_name = self.internal_heap_names[heap_number]

        available = False
        heap_size = 0
        heap_start = 0
        heap_end = 0
        heap_free_start = 0
        try:
            heap_start = self.chipdata.get_var_strict(
                'L_' + internal_name + '_start'
            ).value
            heap_end = self.chipdata.get_var_strict(
                'L_' + internal_name + '_end'
            ).value
            heap_free_start = self.chipdata.get_var_strict(
                'L_' + self.internal_free_list_names[heap_number]
            ).value
            heap_size = heap_end - heap_start
            heap_end = heap_end - 1
            # heap_start can be different than 0 for disabled heaps
            available = heap_size != 0
        except DebugInfoNoVariableError:
            pass

        return available, heap_size, heap_start, heap_end, heap_free_start

    def _get_heap_config(self, processor_number, heap_number):
        """Get heap configuration.

        In dual core configuration information about the heap can be read
        for the other processor too.

        Args:
            processor_number (int): The processor where the heap lives.
            heap_number (int): The heap number specifies the heap from
                which information is asked.

        Returns:
            tuple: Containing information about heap.

                (available, heap_size, heap_start, heap_end)

                available - True, if the heap is present in the build.
                heap_size - Size in octets.
                heap_start - Start address.
                heap_end - The last valid address.
        """
        if self.chipdata.processor == processor_number:
            # properties is (available, heap_size, heap_start, heap_end, _)
            properties = self._get_heap_property(heap_number)
            return properties[:-1]

        self.formatter.error(
            "_get_heap_config called for heap pm" +
            "processor_number = %d, heap_number = %d" % (
                processor_number, heap_number
            )
        )
        return False, 0, 0, 0

    def _lookup_debuginfo(self):
        """Queries debuginfo for information.

        We need this information to get the heap pm memory usage.
        """
        if not self._do_debuginfo_lookup:
            return

        self._do_debuginfo_lookup = False

        try:
            self.patch_size = self.debuginfo.get_var_strict(
                'L_pm_reserved_size'
            ).address
        except DebugInfoNoVariableError:
            self.patch_size = 0
