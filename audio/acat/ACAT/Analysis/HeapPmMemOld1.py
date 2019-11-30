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
from ..Core import Arch
from ACAT.Analysis.HeapPmMem import HeapPmMem as NewHeapPmMem
from ACAT.Core.exceptions import DebugInfoNoVariableError

CONSTANT_DEPENDENCIES = {'not_strict': ('$PM_HEAP_SIZE', '$PM_P1_HEAP_SIZE')}
VARIABLE_DEPENDENCIES = {
    'not_strict': (
        'L_freelist_pm', 'L_freelist_pm_p1', 'L_heap_pm_adjusted',
        'L_heap_pm_adjusted_p1'
    )
}
TYPE_DEPENDENCIES = {
    'mem_node_pm': (
        'struct_mem_node.u.magic', 'struct_mem_node.length_32',
        'struct_mem_node.u.next'
    )
}


class HeapPmMem(NewHeapPmMem):
    """Encapsulates an analysis for heap program memory usage.

    Args:
        **kwarg: Arbitrary keyword arguments.
    """
    heap_names = ["PM heap P0", "PM heap P1", "PM heap P1 ext."]
    max_num_heaps = len(heap_names)
    internal_heap_names = [
        "heap_pm_adjusted",
        "heap_pm_adjusted_p1"

    ]

    def __init__(self, **kwarg):
        NewHeapPmMem.__init__(self, **kwarg)

        # Look up the debuginfo once. Don't do it here though; we don't want
        # to throw an exception from the constructor if something goes
        # wrong.
        self._do_debuginfo_lookup = True
        self.patch_size = None
        self.pmalloc_debug_enabled = False

    def display_memory_map(self):
        """Displays memory map based on current settings."""

    def _check_kymera_version(self):
        """Check if the Kymera version is compatible with this analysis.

        Raises:
            OutdatedFwAnalysisError: Outdated Kymera.
        """
        pass

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
        is_aurachip = (Arch.kal_arch == 4) and (Arch.chip_arch == "Hydra") and \
            ((Arch.chip_id == 0x49) or (Arch.chip_id == 0x4A))
        try:
            patch_analysis = self.interpreter.get_analysis(
                "patches",
                self.chipdata.processor
            )
            patch_level = patch_analysis.get_patch_level()
            patch_available_p1_ext = (patch_level >= 4881)
            patch_available_p1_dis = (patch_level >= 5303)
            patch_size = patch_analysis.get_patch_size()
        except KeyError:
            # Patch analysis is not released to customers.
            patch_available_p1_ext = False
            patch_available_p1_dis = False
            patch_size = 0
        # Aura has an extra 6K PM Heap for P1 since patch 4881
        aura_pm_heap_p1_extra = 0x1800

        active_num_cores = self.chipdata.get_var_strict(
            'L_active_num_cores'
        ).value

        if patch_available_p1_ext and is_aurachip:
            if heap_number > 2:
                # Only one heap is supported for this configuration.
                return False, 0, 0, 0, 0
            elif heap_number == 2:
                if patch_available_p1_dis and (active_num_cores == 1):
                    # Only one heap is supported for this configuration.
                    return False, 0, 0, 0, 0

                available = True
                heap_size = aura_pm_heap_p1_extra
                heap_start = self.debuginfo.get_constant_strict(
                    '$PM_RAM_END_ADDRESS'
                ).value - heap_size
                heap_end = heap_start + heap_size - 1
                heap_free_start = self.chipdata.get_var_strict(
                    'L_' + self.internal_free_list_names[heap_number]
                ).value
                return (
                    available,
                    heap_size,
                    heap_start,
                    heap_end,
                    heap_free_start
                )
        else:
            if heap_number > 1:
                # Only one heap is supported for this configuration.
                return False, 0, 0, 0, 0

        internal_name = self.internal_heap_names[heap_number]

        available = False
        heap_size = 0
        heap_start = 0
        heap_end = 0
        heap_free_start = 0
        try:
            heap_start = self.chipdata.get_var_strict(
                'L_' + internal_name
            ).value

            if heap_number == 0:
                heap_size = self.debuginfo.get_constant_strict(
                    '$PM_HEAP_SIZE'
                ).value - patch_size
                if patch_available_p1_dis and active_num_cores == 1 and is_aurachip:
                    # Claim all of P1's PM Heap (including the extra 6K) and
                    # cache (2K) if P1 is disabled
                    p1_pm_cache = 0x800
                    total_p1_pm_heap = self.debuginfo.get_constant_strict(
                        '$PM_P1_HEAP_SIZE'
                    ).value
                    heap_size = heap_size + total_p1_pm_heap + p1_pm_cache
            else:
                if patch_available_p1_dis and active_num_cores == 1:
                    # Only one heap is supported for this configuration.
                    return False, 0, 0, 0, 0
                heap_size = self.debuginfo.get_constant_strict(
                    '$PM_P1_HEAP_SIZE'
                ).value
                if patch_available_p1_ext and is_aurachip:
                    heap_start = self.debuginfo.get_constant_strict(
                        '$PM_RAM_CODE_P1_START_ADDRESS'
                    ).value
                    heap_size = heap_size - aura_pm_heap_p1_extra

            heap_end = heap_start + heap_size - 1
            heap_free_start = self.chipdata.get_var_strict(
                'L_' + self.internal_free_list_names[heap_number]
            ).value
            available = heap_start != 0
        except DebugInfoNoVariableError:
            pass

        return available, heap_size, heap_start, heap_end, heap_free_start
