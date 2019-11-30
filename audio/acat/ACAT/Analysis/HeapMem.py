############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2014 - 2019 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
"""
Module responsible to analyse the heap memory in Kymera.
"""
from ACAT.Analysis import DebugLog
from ACAT.Analysis.Heap import Heap
from ACAT.Core.exceptions import (
    OutdatedFwAnalysisError, DebugInfoNoVariableError
)


# 'heap_config':() is empty because members are not necessarily accessed,
# 'mem_node' also has members 'line' and 'file' missing since they are in try
VARIABLE_DEPENDENCIES = {
    'strict': (
        'L_processor_heap_info_list', 'L_pheap_info', '$_heap_debug_free',
        '$_heap_debug_min_free'
    )
}
TYPE_DEPENDENCIES = {
    'heap_config': (),
    'heap_info': ()
}

SRAM_START_ADDRESS = 0xfa000000
SRAM_SIZE = 128 * 1024


class HeapMem(Heap):
    """Encapsulates an analysis for heap data memory usage.

    Args:
        **kwarg: Arbitrary keyword arguments.
    """
    # heap names
    heap_names = [
        "Main heap",
        "Shared heap",
        "Slow heap",
        "External heap",
    ]
    # maximum number of heaps per processor.
    max_num_heaps = len(heap_names)

    memory_type = "dm"

    def __init__(self, **kwarg):
        Heap.__init__(self, **kwarg)
        # Look up the debuginfo once. Don't do it here though; we don't want
        # to throw an exception from the constructor if something goes
        # wrong.
        self._do_debuginfo_lookup = True
        self.pmalloc_debug_enabled = None
        self.heap_info = None
        self.heap_info_list = None
        self.freelist = None
        self._check_kymera_version()

    def display_configuration(self):
        """Prints out the heap configuration for both processors."""
        # Look up the debug information.
        self._lookup_debuginfo()

        self.formatter.section_start('Heap Configuration')
        num_heap_processors = len(self.heap_info_list.members)

        for pnum in range(num_heap_processors):
            self.formatter.section_start('Processor %d' % pnum)
            self.formatter.output(
                self._get_heap_config_str(pnum)
            )
            self.formatter.section_end()
        self.formatter.section_end()

    @DebugLog.suspend_log_decorator(0)
    def ret_get_watermarks(self):
        """Same as get_watermarks, but it will return values.

        Returns:
            tuple: The heap usage.
        """
        # Look up the debug information.
        self._lookup_debuginfo()

        total_heap = 0
        free_heap = self.chipdata.get_var_strict("$_heap_debug_free").value
        min_free_heap = self.chipdata.get_var_strict(
            "$_heap_debug_min_free"
        ).value

        for heap_num in range(self.max_num_heaps):
            (available, heap_size, _, _, _) = \
                self._get_heap_property(heap_num)
            if available:
                total_heap += heap_size

        return total_heap, free_heap, min_free_heap

    @DebugLog.suspend_log_decorator(0)
    def clear_watermarks(self):
        """Clears the minimum available memory watermark.

        It's doing it by equating it with the current available memory.
        """
        # Look up the debug information.
        self._lookup_debuginfo()

        free_heap = self.chipdata.get_var_strict("$_heap_debug_free").value
        # Wash down the watermark (min available =  current available)
        self.chipdata.set_data(
            self.chipdata.get_var_strict("$_heap_debug_min_free").address,
            [free_heap]
        )

    ##################################################
    # Private methods
    ##################################################

    def _check_kymera_version(self):
        """Checks if the Kymera version is compatible with this analysis.

        Raises:
            OutdatedFwAnalysisError: For an outdated Kymera.
        """
        try:
            self.debuginfo.get_var_strict("L_heap_single_mode")
        except DebugInfoNoVariableError:
            # fallback to the old implementation
            raise OutdatedFwAnalysisError()

    def _get_heap_property(self, heap_number):
        """Internal function used to get information about a specific heap.

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
        heap_name = self.heap_names[heap_number]
        processor_number = self.chipdata.processor
        heap_free_start = 0

        # when offloading is enabled the private heap property of the
        # second core is not populated. Use the common heap config to
        # decide if the heap is enabled.
        available, heap_size, heap_start, heap_end = self._get_heap_config(
            processor_number, heap_number)

        # Adjust the heap number
        if heap_name == "External heap" and available is True:
            try:
                self.chipdata.get_var_strict("$_slow_heap").value
            except DebugInfoNoVariableError:
                heap_number = heap_number - 1

        if available is True:
            heap_free_start = self.freelist[heap_number].value

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
            heap_num  - The actual heap number
            heap_size - Size in octets.
            heap_start - Start address.
            heap_end - The last valid address.
        """
        heap_name = self.heap_names[heap_number]
        heap_start = 0
        heap_end = 0
        heap_size = 0
        extmem_cntrl = 0

        try:
            self.chipdata.get_var_strict("$_slow_heap").value
        except DebugInfoNoVariableError:
            if heap_name == "Slow heap":
                return False, 0, 0, 0
            elif heap_name == "External heap":
                heap_number = heap_number - 1

        try:
            extmem_cntrl = self.chipdata.get_var_strict("$_extmem_cntrl").value
        except DebugInfoNoVariableError:
            pass

        if heap_name == "External heap":
            sram_enabled = False
            if extmem_cntrl != 0:
                extmem_blk = self.chipdata.cast(
                    extmem_cntrl, "EXTMEM_CNTRL_BLOCK")
                clk = extmem_blk.get_member('cur_clk').value
                # If clk is greater than EXT_CLK_OFF , then sram is available
                if clk > 1:
                    sram_enabled = True

            if sram_enabled is False:
                return False, 0, 0, 0

        # Once we reach here, the heap is avialable

        heap_info = self.heap_info_list[processor_number]
        proc_config = heap_info.get_member("heap")
        heap = proc_config[heap_number]

        heap_size = heap.get_member('heap_size').value
        heap_start = heap.get_member('heap_start').value
        heap_end = heap.get_member('heap_end').value - 1

        return True, heap_size, heap_start, heap_end

    def _lookup_debuginfo(self):
        """Queries debuginfo for information.

        The information is needed to get the heap memory usage.
        """

        if not self._do_debuginfo_lookup:
            return

        self._do_debuginfo_lookup = False

        # Freelist
        self.freelist = self.chipdata.get_var_strict('L_freelist')

        # Check for PMALLOC_DEBUG
        # If L_memory_pool_limits exists then PMALLOC_DEBUG is enabled
        try:
            self.debuginfo.get_var_strict(
                'L_memory_pool_limits'
            )
            self.pmalloc_debug_enabled = True
        except DebugInfoNoVariableError:
            self.pmalloc_debug_enabled = False

        pheap_info = self.chipdata.get_var_strict("L_pheap_info").value
        heap_info = self.chipdata.cast(pheap_info, "heap_config")
        self.heap_info = heap_info.get_member("heap")

        # processor_heap_info_list should be always different than NULL!
        self.heap_info_list = self.chipdata.get_var_strict(
            "L_processor_heap_info_list"
        )
