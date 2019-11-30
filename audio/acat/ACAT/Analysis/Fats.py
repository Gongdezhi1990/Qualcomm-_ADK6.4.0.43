############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2015 - 2019 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
"""
Analysis which provides an interface for the FATS test system.
"""
from __future__ import print_function

import os
import re
import traceback
from time import strftime

from . import Analysis
from ..Display.InteractiveFormatter import InteractiveFormatter
from ACAT.Core.exceptions import AnalysisError


class Instruction(object):
    """Class for keeping an ACAT instruction.

    Args:
        instr
        state
    """

    def __init__(self, instr, state):
        self.instr = instr
        self.state = state
        self.instr_to_run = None


RESOURCE_USAGE_ANALYSES = ['profiler', 'heapmem', 'poolinfo']
OTHER_FATS_ANALYSES = ['debuglog']

AVAILABLE_INSTRUCTIONS = {
    "file (.*)": [Instruction(r"self.log_to_file(\1)", 'config')],
    "dbl_lvl (.*)": [
        Instruction(r"debuglog.set_debug_log_level(\1)", 'config'),
        Instruction("debuglog.poll_debug_log(False)", 'start'),
        Instruction("debuglog.stop_polling()", 'reset')
    ],
    # (Pedantry: FATS expects us to call this "MIPS", although it's
    # really about MCPS = millions of cycles per second.)
    "mips_clks (.*)": [
        Instruction(
            r"profiler.formatter.output('\\nSTART mips_clks');"
            r"profiler.run_clks(\1);"
            r"profiler.formatter.output('END mips_clks')",
            'read_resource_usage'
        )
    ],
    "mips_kymera_builtin": [
        Instruction(
            r"profiler.formatter.output('\\nSTART mips_kymera_builtin');"
            r"profiler.run_kymera_builtin();"
            r"profiler.formatter.output('END mips_kymera_builtin')",
            'read_resource_usage'
        )
    ],
    "mips_pc (.*)": [
        Instruction(
            r"profiler.formatter.output('\\nSTART mips_pc');"
            r"profiler.run_pc(\1);"
            r"profiler.formatter.output('END mips_pc')",
            'read_resource_usage'
        )
    ],
    "heap": [
        Instruction(
            r"heapmem.formatter.output('\\nSTART heap');"
            r"heapmem.get_watermarks();"
            r"heapmem.formatter.output('END heap');",
            'read_resource_usage'
        ),
        Instruction('heapmem.clear_watermarks()', 'reset')
    ],
    "pool": [
        Instruction(
            r"poolinfo.formatter.output('\\nSTART pool');"
            r"poolinfo.get_watermarks();"
            r"poolinfo.formatter.output('END pool')",
            'read_resource_usage'
        ),
        Instruction('poolinfo.clear_watermarks()', 'reset')
    ],
}


class Fats(Analysis.Analysis):
    """Encapsulates an analysis for resource usage used by FATS test.

    Args:
        **kwarg: Arbitrary keyword arguments.
    """

    def __init__(self, **kwarg):
        Analysis.Analysis.__init__(self, **kwarg)

    def run_all(self):
        """Run the analysis.

        This analyses cannot run in automatic mode. It is design to be
        used by the FATS system.
        """
        self.formatter.output(
            "FATS analysis does not support run_all() command"
        )

    def config(self, requests):
        """Configure the Fats analysis.

        Also, it executes the config state specific instructions.

        Args:
            requests
        """
        try:
            if not isinstance(requests, (tuple, list)):
                requests = [req.strip() for req in requests.split(";")]
        except BaseException:
            raise AnalysisError("Configuration not recognised.")

        # go trough all request
        for request in requests:
            # translate the requests
            for config in AVAILABLE_INSTRUCTIONS:
                # print "-",config
                match = re.match(config, request)
                if match:
                    print(match.group())
                    try:
                        for instruction in AVAILABLE_INSTRUCTIONS[config]:
                            instruction.instr_to_run = re.sub(
                                config, instruction.instr, request
                            )
                    except BaseException:
                        for instruction in AVAILABLE_INSTRUCTIONS[config]:
                            instruction.instr_to_run = instruction.instr

        self.execute_state_commands("config")

    def start(self):
        """Executes the start state specific instructions."""
        self.execute_state_commands("start")

    def read_resource_usage(self):
        """Executes the read_resource_usage state specific instructions."""
        self.execute_state_commands("read_resource_usage")

    def reset(self):
        """Test finished reset everything."""
        self.execute_state_commands("reset")

        # disable all instructions
        for config in AVAILABLE_INSTRUCTIONS:
            for instruction in AVAILABLE_INSTRUCTIONS[config]:
                instruction.instr_to_run = None

        # disable logging to file
        for analysis in OTHER_FATS_ANALYSES:
            self.interpreter.get_analysis(
                analysis, self.chipdata.processor
            ).reset_formatter()
        for analysis in RESOURCE_USAGE_ANALYSES:
            self.interpreter.get_analysis(
                analysis, self.chipdata.processor
            ).reset_formatter()

    def log_to_file(self, test_name=None):
        """Change the output of all the FATS test to the file.

        Args:
            test_name (str, optional)
        """
        if test_name is not None:
            logfile_name = os.path.splitext(test_name)[0]
        else:
            logfile_name = self.__class__.__name__

        new_logfile_name = logfile_name + strftime("_%Y_%m_%d__%H_%M_%S.log")

        formatter = InteractiveFormatter()
        if os.name == 'nt':
            formatter.change_log_file(new_logfile_name, False)
        else:
            formatter.change_log_file(new_logfile_name, True)

        for analysis in OTHER_FATS_ANALYSES:
            self.interpreter.get_analysis(
                analysis, self.chipdata.processor
            ).set_formatter(formatter)

        new_logfile_name = "%s_resource_usage.log" % (
            os.path.splitext(new_logfile_name)[0]
        )

        formatter = InteractiveFormatter()
        if os.name == 'nt':
            formatter.change_log_file(new_logfile_name, False)
        else:
            formatter.change_log_file(new_logfile_name, True)

        for analysis in RESOURCE_USAGE_ANALYSES:
            self.interpreter.get_analysis(
                analysis, self.chipdata.processor
            ).set_formatter(formatter)

    def execute_state_commands(self, state):
        """Executes the state specific commands.

        Args:
            state
        """
        locals().update(self.interpreter.p0.available_analyses)
        for config in AVAILABLE_INSTRUCTIONS:
            for instruction in AVAILABLE_INSTRUCTIONS[config]:
                if instruction.instr_to_run is None:
                    continue

                if instruction.state == state:
                    try:
                        exec(instruction.instr_to_run)
                    except BaseException:
                        self.formatter.output(traceback.format_exc() + '\n')

        self.formatter.output("fats.%s finished!" % state)
