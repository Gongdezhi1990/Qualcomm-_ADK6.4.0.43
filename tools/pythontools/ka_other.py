# Copyright (c) 2014 - 2018 Qualcomm Technologies International, Ltd
# Part of the Python bindings for the kalaccess library.

from ctypes import c_uint, c_void_p, POINTER, c_uint32
from ka_ctypes import ka_err, ctypes_arr_type


class KaDspState(object):

    KAL_STATUS_EXTERNAL_BREAK = None
    KAL_STATUS_EXCEPTION = None
    KAL_STATUS_INSTR_BREAK = None
    KAL_STATUS_DM_BREAK = None
    KAL_STATUS_PM_BREAK = None
    KAL_STATUS_RUNNING = None

    @staticmethod
    def init_constants(core):
        KaDspState.KAL_STATUS_PM_BREAK = core.extract_c_integer_constant("KAL_STATUS_PM_BREAK")
        KaDspState.KAL_STATUS_DM_BREAK = core.extract_c_integer_constant("KAL_STATUS_DM_BREAK")
        KaDspState.KAL_STATUS_RUNNING = core.extract_c_integer_constant("KAL_STATUS_RUNNING")
        KaDspState.KAL_STATUS_INSTR_BREAK = core.extract_c_integer_constant("KAL_STATUS_INSTR_BREAK")
        KaDspState.KAL_STATUS_EXCEPTION = core.extract_c_integer_constant("KAL_STATUS_EXCEPTION")
        KaDspState.KAL_STATUS_EXTERNAL_BREAK = core.extract_c_integer_constant("KAL_STATUS_EXTERNAL_BREAK")

    def __init__(self, raw_state):
        self.raw_state = raw_state
        
    def __repr__(self):
        state = ["DSP state: %#x" % self.raw_state]
        if self.is_running():
            state.append("Running")
        if self.is_on_pm_break():
            state.append("PM breakpoint")
        if self.is_on_dm_break():
            state.append("DM breakpoint")
        if self.is_on_instruction_break():
            state.append("Instruction break")
        if self.is_on_exception_break():
            state.append("Exception break")
        if self.is_on_external_break():
            state.append("External break")
        return "\n\t".join(state)
            
    def is_running(self):
        return self.raw_state & KaDspState.KAL_STATUS_RUNNING != 0
        
    def is_on_pm_break(self):
        return self.raw_state & KaDspState.KAL_STATUS_PM_BREAK != 0
        
    def is_on_dm_break(self):
        return self.raw_state & KaDspState.KAL_STATUS_DM_BREAK != 0
        
    def is_on_instruction_break(self):
        return self.raw_state & KaDspState.KAL_STATUS_INSTR_BREAK != 0
    
    def is_on_exception_break(self):
        return self.raw_state & KaDspState.KAL_STATUS_EXCEPTION != 0
        
    def is_on_external_break(self):
        return self.raw_state & KaDspState.KAL_STATUS_EXTERNAL_BREAK != 0
        

class KaOther:
    def __init__(self, core):
        self._core = core

        self._cfuncs = {}
        self._core.add_cfunc(
            self._cfuncs,
            'ka_reset',
            POINTER(ka_err),
            [c_void_p]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_hal_get_dsp_state',
            POINTER(ka_err),
            [c_void_p, POINTER(c_uint)]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_hal_pcprofile',
            POINTER(ka_err),
            [c_void_p, POINTER(c_uint32), c_uint]
        )
        
        KaDspState.init_constants(self._core)
            
    def reset(self):
        """Performs a reset by disabling the Kalimba, and then enabling it again."""
        err = self._cfuncs['ka_reset'](self._core.get_ka())
        self._core.handle_error(err)

    def get_dsp_state(self):
        """
        Reads the state of the Kalimba core directly from the DSP_STATUS register.
        Returns a KaDspState object representing the state of that register.
        Note that for most purposes KaExec.get_exec_state() provides the core state in a simpler manner.
        """
        result = c_uint()
        err = self._cfuncs['ka_hal_get_dsp_state'](self._core.get_ka(), result)
        self._core.handle_error(err)
        return KaDspState(result.value)

    def pcprofile(self, num_samples):
        """
        Reads Kalimba's program counter repeatedly, as fast as possible, until num_samples have
        been read. Returns the PC values as a list.

        NOTE: if using the Kalimba Python tools, consider using kal_lib.Lib.pcprofiler (accessible
        as kal.lib.pcprofiler from a Kalaccess object) instead of this low-level function.
        kal.lib.pcprofiler() maps PC locations to function names using the symbolic information,
        and produces a frequency report in a printable object.
        """
        pc_samples = ctypes_arr_type(c_uint32, num_samples)()
        err = self._cfuncs['ka_hal_pcprofile'](self._core.get_ka(), pc_samples, num_samples)
        self._core.handle_error(err)
        return list(pc_samples)  # Convert away from ctypes array
