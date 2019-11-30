# Copyright (c) 2014 - 2018 Qualcomm Technologies International, Ltd
# Part of the Python bindings for the kalaccess library.

from ctypes import c_int, c_uint, c_byte, c_void_p, POINTER, byref
from ka_ctypes import ka_err


class KaBreak:
    """
    Functions to set, clear, and query program and data breakpoints.
    """

    KA_INSN_SET_MAXIMODE = 0
    KA_INSN_SET_MINIMODE = 1

    def __init__(self, core):
        self._core = core
        self._cfuncs = {}
        self._core.add_cfunc(
            self._cfuncs,
            'ka_init_bp_system',
            POINTER(ka_err),
            [c_void_p]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_is_valid_pm_break_location',
            POINTER(ka_err),
            [c_void_p, c_uint, POINTER(c_byte), c_int]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_set_pm_break',
            POINTER(ka_err),
            [c_void_p, c_uint, c_int]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_clear_pm_break',
            POINTER(ka_err),
            [c_void_p, c_uint]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_set_dm_break',
            POINTER(ka_err),
            [c_void_p, c_uint, c_uint, c_byte, c_byte]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_clear_dm_break',
            POINTER(ka_err),
            [c_void_p, c_uint, c_uint, c_byte, c_byte]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_clear_all_pm_breaks',
            POINTER(ka_err),
            [c_void_p]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_clear_all_dm_breaks',
            POINTER(ka_err),
            [c_void_p]
        )

    def _lib(self):
        return self._core.get_ka()

    def _handle_return_code(self, err):
        self._core.handle_error(err)

    def reinit_breakpoint_system(self):
        """
        Reinitialise the breakpoint system.
        This function clears ALL existing PM and DM breakpoints on the target, and also resets the library's internal
        breakpoint state.
        """
        res = self._cfuncs['ka_init_bp_system'](self._lib())
        self._handle_return_code(res)
        
    def is_pm_break_hit(self):
        """
        Query if the processor is currently waiting on a PM breakpoint.
        :return True or False
        """
        state = self._core.other.get_dsp_state()
        return state.is_on_pm_break() or state.is_on_instruction_break()

    def is_valid_pm_break_location(self, address, instruction_set):
        """
        Query if the specified address is a valid location for a PM breakpoint.
        :param address: a PM address
        :param instruction_set: the instruction set of the code in memory at the specified address. This
        must be either
        KaBreak.KA_INSN_SET_MAXIMODE or
        KaBreak.KA_INSN_SET_MINIMODE.
        :return True or False
        """
        ok = c_byte()
        err = self._cfuncs['ka_is_valid_pm_break_location'](self._lib(), address, byref(ok), instruction_set)
        self._handle_return_code(err)
        return bool(ok.value)
        
    def is_dm_break_hit(self):
        """
        Query if the processor is currently waiting on a DM breakpoint.
        :return True or False
        """
        return self._core.other.get_dsp_state().is_on_dm_break()

    def clear_all_pm_breakpoints(self):
        """
        Remove ALL hardware PM breakpoints, irrespective of who set them.
        """
        res = self._cfuncs['ka_clear_all_pm_breaks'](self._lib())
        self._handle_return_code(res)
        
    def clear_all_dm_breakpoints(self):
        """
        Remove ALL hardware DM breakpoints, irrespective of who set them.
        """
        res = self._cfuncs['ka_clear_all_dm_breaks'](self._lib())
        self._handle_return_code(res)

    def set_pm_breakpoint(self, address, instruction_set):
        """
        Set a PM breakpoint at, or as close as possible to, the supplied PM address.
        :param address: a PM address
        :param instruction_set: the instruction set of the code in memory at the specified address. This
        must be either
        KaBreak.KA_INSN_SET_MAXIMODE or
        KaBreak.KA_INSN_SET_MINIMODE
        """
        res = self._cfuncs['ka_set_pm_break'](self._lib(), address, instruction_set)
        self._handle_return_code(res)

    def clear_pm_breakpoint(self, address):
        """
        Clear any PM breakpoint previously set at the given address.
        :param address: a PM address; this is the address requested in the call to set_pm_breakpoint().
        """
        res = self._cfuncs['ka_clear_pm_break'](self._lib(), address)
        self._handle_return_code(res)

    def set_dm_breakpoint(self, start_address, end_address, trigger_on_read, trigger_on_write):
        """
        Set a DM breakpoint.
        :param start_address: the first address for the data breakpoint to trigger on.
        :param end_address: the last address for the data breakpoint to trigger on (inclusive), so the range is
        [start_addr, end_addr].
        :param trigger_on_read: True if the breakpoint should trigger on read accesses to [start_addr, end_addr]
        :param trigger_on_write: True if the breakpoint should trigger on write accesses to [start_addr, end_addr]
        """
        res = self._cfuncs['ka_set_dm_break'](
            self._lib(), start_address, end_address, trigger_on_read, trigger_on_write
        )
        self._handle_return_code(res)

    def clear_dm_breakpoint(self, start_address, end_address, trigger_on_read, trigger_on_write):
        """
        Clear any DM breakpoint matching the given start address, end address and triggering conditions, as previously
        passed to set_dm_breakpoint().
        """
        res = self._cfuncs['ka_clear_dm_break'](
            self._lib(), start_address, end_address, trigger_on_read, trigger_on_write
        )
        self._handle_return_code(res)
