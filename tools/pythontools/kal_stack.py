# Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd
from __future__ import print_function


class Stack(object):
    """
    This class provides basic functionality to decode and display the call stack.
    """

    def __init__(self, core):
        self._core = core

    def trace(self):
        """
        A simple stack trace routine, just basically looks into the stack buffer.
        """
        if self._core.arch.get_arch() > 3:
            raise RuntimeError("Kalimba architecture {0} is not supported by Stack. Please use pydbg or QMDE.".format(
                self._core.arch.get_arch())
            )

        # pause chip if running
        was_running = False
        if self._core.is_running():
            was_running = True
            self._core.pause()

        # find the stack
        if self._core.arch.get_chip_rev() == 1:
            stack_start = self._core.sym.varfind("$stack.buffer$").addr
            stack_buf_size = self._core.sym.varfind("$stack.buffer$").size_in_addressable_units
            stack_pntr = self._core.reg.r9
            if stack_pntr < stack_start or stack_pntr > (stack_start + stack_buf_size):
                print("Stack pointer is invalid")
                return
        else:
            stack_start_addr = self._core.sym.constfind("$STACK_START_ADDR$").value
            stack_pntr_addr  = self._core.sym.constfind("$STACK_POINTER$").value

            # where is the buffer
            stack_start = self._core.dm[stack_start_addr]
            stack_pntr  = self._core.dm[stack_pntr_addr]

        # Store current pc and rLink before running
        pc = self._core.reg.pc
        rLink = self._core.reg.rlink

        # Print header
        print("%-40s PC          (Source)" % ("Module"))

        module_name = ""
        count = 0
        # is there anything on the stack
        if stack_pntr == stack_start:
            if was_running:
                self._core.run()
        else:
            # Read the stack buffer
            stack_buf = self._core.dm[stack_start:stack_pntr]

            # Restart chip if was running
            if was_running:
                self._core.run()

            # walk the stack
            for s in stack_buf:
                # If item on stack points one instruction after a call, then assume it is a stored rlink
                # Call instructions start with 6'b111000 = 56
                if s > 0 and ((self._core.pm[s-1] >> 26 ) & 63) == 56:
                    if count > 0:
                        print()
                    count = 0
                    module_name = self._core.sym.modname(s-1)
                    if module_name == '$M.interrupt.handler':
                        print("\nServing an interrupt ---------------------------->>>")
                    print("%-40s (0x%06X)  (STACK)" % (self._core.sym.modname(s-1), s-1))
                else:
                    if count == 0:
                        print("    DATA: "),
                    print("0x%06X, " % (s)),
                    count += 1
                    if count >= 6:
                        print()
                        count = 0

        if count > 0:
            print()
        # print the current rLink (May or may not be useful)
        if self._core.sym.modname(rLink-1) != self._core.sym.modname(pc) and \
                        self._core.sym.modname(rLink-1) != module_name:
            print("%-40s (0x%06X)  (rLink)" % (self._core.sym.modname(rLink-1), rLink-1))

        # print the current location (PC)
        print("%-40s (0x%06X)  (PC)" % (self._core.sym.modname(pc), pc))
