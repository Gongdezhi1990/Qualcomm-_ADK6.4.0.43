# Copyright (c) 2014 - 2018 Qualcomm Technologies International, Ltd
# Part of the Python bindings for the kalaccess library.

from ctypes import c_int, c_uint, c_byte, c_void_p, POINTER, byref
from ka_ctypes import ka_err


class KaExecState:
    """
    Represents the execution state of a Kalimba core -- instances of this class are returned by KaExec.get_exec_state().
    """

    KA_STATE_PM_BREAK           = 0
    KA_STATE_DM_BREAK           = 1
    KA_STATE_RUNNING            = 2
    KA_STATE_STOPPED            = 3
    KA_STATE_EXCEPTION_BREAK    = 4
    KA_STATE_EXTERNAL_BREAK     = 5
    KA_STATE_CLOCK_OR_POWER_OFF = 6
    KA_STATE_SHALLOW_SLEEP      = 7

    _string_reprs = [
        "On PM breakpoint",    # KA_STATE_PM_BREAK,
        "On DM breakpoint",    # KA_STATE_DM_BREAK,
        "Running",             # KA_STATE_RUNNING,
        "Paused",              # KA_STATE_STOPPED,
        "Exception break",     # KA_STATE_EXCEPTION_BREAK,
        "External break",      # KA_STATE_EXTERNAL_BREAK,
        "Clock or power off",  # KA_STATE_CLOCK_OR_POWER_OFF,
        "Shallow sleeping"     # KA_STATE_SHALLOW_SLEEP,
    ]

    def __init__(self, state):
        self.state = state

    def __repr__(self):
        return self._string_reprs[self.state]

    def is_running(self):
        return self.state == self.KA_STATE_RUNNING or self.state == self.KA_STATE_SHALLOW_SLEEP


class KaCouldNotDetermineInstructionSet(RuntimeError):

    def __init__(self):
        RuntimeError.__init__(self, "Could not automatically determine the instruction set at the target address. "
                                    "Please specify it explicitly.")


class KaExec:
    """
    Exposes execution-control functions, such as run, pause, step.
    Also provides the ability to query the execution state of the core.
    """
    KA_INSN_SET_MAXIMODE = 0
    KA_INSN_SET_MINIMODE = 1

    # noinspection PyProtectedMember
    def _declare_cfuncs(self):
        self._core.add_cfunc(
            self._cfuncs,
            'ka_get_instruction_set_from_pc',
            POINTER(ka_err),
            [c_void_p, POINTER(c_int)]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_run',
            POINTER(ka_err),
            [c_void_p, c_int, POINTER(c_byte)]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_run_to',
            POINTER(ka_err),
            [c_void_p, c_uint, c_int]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_pause',
            POINTER(ka_err),
            [c_void_p]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_step',
            POINTER(ka_err),
            [c_void_p, c_int]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_step_over',
            POINTER(ka_err),
            [c_void_p, POINTER(c_byte), c_int]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_get_chip_state',
            POINTER(ka_err),
            [c_void_p, POINTER(c_int)]
        )

    def __init__(self, core):
        self._core = core
        self._cfuncs = {}
        self._declare_cfuncs()
    
    # noinspection PyProtectedMember
    def _lib(self):
        return self._core.get_ka()

    # noinspection PyProtectedMember
    def _handle_return_code(self, res):
        self._core.handle_error(res)
        
    def _get_instruction_set_from_pc(self):
        """
        Retrieve the instruction set of the code at the current execution location.
        :return: KaExec.KA_INSN_SET_MAXIMODE or KaExec.KA_INSN_SET_MINIMODE.
        """
        instruction_set = c_int()
        res = self._cfuncs['ka_get_instruction_set_from_pc'](self._lib(), byref(instruction_set))
        self._handle_return_code(res)
        return instruction_set

    def run(self):
        """
        Set the processor running. Has no effect if the processor is already running.
        """
        # We discard the following out param -- the only interesting information is whether there was an error.
        run_was_actually_called = c_byte()
        # The second parameter is deprecated.
        unused = 0
        res = self._cfuncs['ka_run'](self._lib(), unused, byref(run_was_actually_called))
        self._handle_return_code(res)

    def run_to(self, address, instruction_set_at_target=None):
        """
        Run the processor to the specified address. The processor should be paused before this operation is attempted.
        :param address: A valid program address.
        :param instruction_set_at_target: The instruction set at the target address; valid values are
        KaExec.KA_INSN_SET_MAXIMODE or
        KaExec.KA_INSN_SET_MINIMODE }. If this parameter is None, an attempt will be made to set it automatically.
        In this case, if the instruction set cannot be unambiguously determined, an exception will be raised.
        """
        if instruction_set_at_target is None:
            if not self._core.arch.supports_minim():
                instruction_set_at_target = KaExec.KA_INSN_SET_MAXIMODE
            else:
                raise KaCouldNotDetermineInstructionSet()
        
        if instruction_set_at_target not in [KaExec.KA_INSN_SET_MAXIMODE, KaExec.KA_INSN_SET_MINIMODE]:
            raise ValueError('Please supply a valid instruction set at target address -- see help.')
            
        res = self._cfuncs['ka_run_to'](self._lib(), address, instruction_set_at_target)
        self._handle_return_code(res)

    def pause(self):
        """
        Pause the processor. Has no effect if the processor is already paused.
        """
        res = self._cfuncs['ka_pause'](self._lib())
        self._handle_return_code(res)

    def step(self):
        """
        Step an instruction, stepping into calls. The processor should be paused before this operation is attempted.
        """
        instruction_set = self._get_instruction_set_from_pc()
        res = self._cfuncs['ka_step'](self._lib(), instruction_set)
        self._handle_return_code(res)

    def step_over(self):
        """
        Step over an instruction, stepping over calls. The processor should be paused before this operation is
        attempted.
        """
        instruction_set = self._get_instruction_set_from_pc()
        # We discard the following out param -- the only interesting information is whether there was an error.
        run_was_actually_called = c_byte()
        res = self._cfuncs['ka_step_over'](self._lib(), byref(run_was_actually_called), instruction_set)
        self._handle_return_code(res)

    def get_exec_state(self):
        """
        Returns the execution state of the core, represented as a KaExecState instance.
        """
        result = c_int()
        res = self._cfuncs['ka_get_chip_state'](self._lib(), result)
        self._handle_return_code(res)
        return KaExecState(result.value)
