############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2015 - 2019 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
"""
Module used to profiler Kymera.
"""
import math
import operator
import threading
import time as import_time
from collections import Counter, OrderedDict
try:
    # Python 3
    from queue import Queue
except ImportError:
    # Python 2
    from Queue import Queue

from ACAT.Analysis import Analysis
from ACAT.Analysis import DebugLog
from ACAT.Core import Arch
from ACAT.Core import ChipData
from ACAT.Core import CoreTypes as ct
from ACAT.Core.exceptions import DebugInfoNoVariableError

VARIABLE_DEPENDENCIES = {
    'not_strict': (
        '$_current_id', '$_profiler_list', '$interrupt.cpu_fraction',
        '$interrupt.cpu_fraction_with_task'
    )
}
TYPE_DEPENDENCIES = {'profiler': ('next', 'id', 'cpu_fraction')}


def output_percentage(output_function, occurrence, total_measurements=100,
                      key_transform_function=None, sort=True, column=56):
    """Displays a dictionary occurrences.

    Args:
        output_function
        occurrence: A dictionary or an OrderedDict instance.
        total_measurements (int, optional)
        key_transform_function (obj, optional)
        sort (bool, optional)
        column (int, optional): Which terminal column to left-justify the
            formatted percentage to.
    """
    if key_transform_function is None:
        def _dummy(input_param):
            """Dummy function."""
            return input_param
        key_transform_function = _dummy

    # outputs the percentage
    if sort:
        sorted_occurrence = sorted(
            list(occurrence.items()), key=operator.itemgetter(1)
        )
        sorted_occurrence.reverse()
    else:
        sorted_occurrence = zip(occurrence.keys(), occurrence.values())

    for element in sorted_occurrence:
        output_string = "%-*s " % (
            column - 1, key_transform_function(element[0])
        )
        if isinstance(element[1], tuple):
            for values in element[1]:
                output_string += "%7.3f%% " % (
                    float(values * 100) / total_measurements
                )
        else:
            output_string += "%7.3f%%" % (
                float(element[1] * 100) / total_measurements
            )
        output_function(output_string)


class RunClksReader(threading.Thread, Analysis.Analysis):
    """Runs Clocks Reader.

    This class is responsible for reading the run clocks counter
    repeatedly (every 1 s) until a stop event arrives. After each reading
    it checks if an overflow occurred and corrects the total clks counter
    accordingly.

    Args:
        measure_event
        **kwarg: Arbitrary keyword arguments.
    """

    def __init__(self, measure_event, **kwarg):
        threading.Thread.__init__(self)
        Analysis.Analysis.__init__(self, **kwarg)
        self._measure_event = measure_event
        try:
            # check if we are in a 32 bit machine
            self.chipdata.get_reg_strict("$NUM_RUN_CLKS")
            self.register_size = 32
            self.register_shift = 0
        except KeyError:
            # check if we are in a 24 bit machine
            # if this fails maybe we have an other platform to worry about
            self.chipdata.get_reg_strict("$NUM_RUN_CLKS_MS")
            self.register_size = 8
            self.register_shift = 24

        self._mcps_usage = 0
        self._mcps_percent = 0
        self._chip_cpu_speed_mhz = 0

    def run_all(self):
        """Does nothing.

        This analysis does not implement a run_all function because it is
        a helper module.
        """
        # Nothing we can usefully do here.
        return

    def run(self):
        """Calculates the total wake time.

        It uses the NUM_RUN_CLKS hardware register. It reads the register
        value and re-reads it after the given time; it calculates the
        cycle count (MCPS = millions of cycles per second) using the
        following formula:

                    delta * 2^register_shift
             MCPS = ------------------------
                             10^6

        The upper bound on MCPS is the CPU speed in MHz. During idle
        periods, the CPU will be in 'shallow sleep' (not clocked), during
        which time the NUM_RUN_CLKS register will not count up.

        If the CPU's speed is known, a wake-time percentage is also
        derived.
        """
        overflow_value = int(math.pow(2, self.register_size))
        total_overflow = 0

        # wait until we the measure event is set
        self._measure_event.wait()

        # read the initial value of the run clks and the start time.
        start_time = import_time.time()
        if Arch.addr_per_word != 4:
            start_clk = self.chipdata.get_reg_strict("$NUM_RUN_CLKS_MS").value
        else:
            start_clk = self.chipdata.get_reg_strict("$NUM_RUN_CLKS").value
        prev_clk = start_clk
        cur_clk = start_clk

        while self._measure_event.is_set():
            import_time.sleep(1)
            if Arch.addr_per_word != 4:
                cur_clk = self.chipdata.get_reg_strict(
                    "$NUM_RUN_CLKS_MS"
                ).value
            else:
                cur_clk = self.chipdata.get_reg_strict("$NUM_RUN_CLKS").value
            if cur_clk < prev_clk:
                self.formatter.output("Overflow detected")
                total_overflow += overflow_value
            prev_clk = cur_clk

        # read end time
        end_time = import_time.time()
        delta_time = (end_time - start_time)
        # print "delta_time (should be close to the given value)= ", delta_time

        # calculate the MCPS usage
        delta_clk = cur_clk + total_overflow - start_clk
        # print "delta_clk = ",  delta_clk

        mcps_usage = (
            float(delta_clk) * math.pow(2, self.register_shift)
        ) / (delta_time * math.pow(10, 6))
        try:
            chip_cpu_speed_mhz = self.chipdata.get_var_strict(
                "$profiler.cpu_speed_mhz"
            ).value
            if chip_cpu_speed_mhz == 0:
                # invalid value. Raise exception to handle the error and
                # print error message.
                raise DebugInfoNoVariableError()
        except DebugInfoNoVariableError:
            # On CSRA681xx this is static and comes from Curator (see
            # SERVICE_ADVICE for CCP_FACTS_ABOUT_SUBSYSTEM IE).
            # On QCC3/5xxx it's more dynamic (and there's currently
            # no arrangement to communicate that to ACAT).
            self.formatter.output(
                "Can't determine current CPU speed from chip.\n"
                "CPU speed is an assumption!")
            chip_cpu_speed_mhz = Arch.chip_cpu_speed_mhz

        mcps_percent = (mcps_usage * 100) / chip_cpu_speed_mhz
        self.formatter.output(
            "Total MCPS used = %3.3f (CPU active for ~%6.3f%% @%dMHz)" %
            (mcps_usage, mcps_percent, chip_cpu_speed_mhz)
        )

        self._mcps_usage = mcps_usage
        self._mcps_percent = mcps_percent
        self._chip_cpu_speed_mhz = chip_cpu_speed_mhz


class Profiler(Analysis.Analysis):
    """Encapsulates analyses for measuring CPU runtime performance."""

    def __init__(self, **kwarg):
        # Call the base class constructor.
        Analysis.Analysis.__init__(self, **kwarg)
        try:
            self._profiler_list_address = self.debuginfo.get_var_strict(
                '$_profiler_list'
            ).address
        except DebugInfoNoVariableError:
            self._profiler_list_address = None

        self.old_formatter = None

    def run_all(self, run_pc_clks=False):
        """Performs analysis and send the output to the formatter.

        If the default values for the different analyses are not suitable
        for you, consider calling them directly.

        run_kymera_builtin() is one of them. It reads the built
            in Kymera profiler (if it exists) and displays it in a more
            useful way.

        run_clks() calculates the total cycle consumption using
            NUM_RUN_CLKS. Please see RunClksReader for more details.
            Only runs if run_pc_clks input is True.

        run_pc() runs the PC profiling analysis for a given time. Only runs
            if run_pc_clks input is True.

        Args:
            run_pc_clks (bool, optional): If set to True PC and RUN_CLKS
                profiler will called.
        """
        self.formatter.section_start('Profiler')
        self.run_kymera_builtin()
        if run_pc_clks:  # for live chips
            self.run_clks(10)
            self.run_pc(10, False)
        self.formatter.section_end()

    @DebugLog.suspend_log_decorator(0)
    def run_pc(self, time=10, task_id_read=False, top=None):
        """This reads the program counter for a period of time.

        After reading the Program Counter numerous times it checks which
        is the nearest code label to those PC values. Finally it puts in
        order the occurrences of the different code labels.

        It can also read the current task id to compare the active times
        of the different tasks.

        Args:
            time (int, optional)
            task_id_read (bool, optional)
            top (int, optional): The top n(int) functions to show.
        """
        queue = Queue()
        _stop_event = threading.Event()

        if task_id_read:
            reader = ChipData.VariablePoller(
                _stop_event, ['$PC_STATUS', '$_current_id'], self.chipdata,
                queue
            )
        else:
            reader = ChipData.VariablePoller(
                _stop_event, ['$PC_STATUS'], self.chipdata, queue
            )

        reader.start()
        self.formatter.output("\nPC profiling started for %d sec." % time)
        import_time.sleep(time)
        _stop_event.set()
        # it will take a almost the same time to analyse the collected data
        reader.join()

        pc_data = queue.get()
        count = len(pc_data)
        self.formatter.output("Total measurements = %d" % count)

        if task_id_read:
            pc_occurrence = Counter([row[0] for row in pc_data])
            task_id_occurrence = Counter([row[1] for row in pc_data])
        else:
            pc_occurrence = Counter(pc_data)

        module_occurrence = OrderedDict()
        for address in pc_occurrence:
            try:
                module_name = self.debuginfo.get_source_info(
                    address
                ).module_name
            except ct.BundleMissingError:
                module_name = (
                    "No source information for 0x%08x. " +
                    "Bundle is missing." % (address)
                )
            module_name = module_name.replace("L_", "")
            if module_occurrence.get(module_name) is None:
                module_occurrence[module_name] = pc_occurrence[address]
            else:
                module_occurrence[module_name] += pc_occurrence[address]

        # Sort the occurrences
        module_occurrence = OrderedDict(
            sorted(
                module_occurrence.items(),
                key=operator.itemgetter(1),
                reverse=True
            )
        )

        # If the `top' argument is given, keep the top occurrences and
        # convert the rest into a single item `Other functions'.
        if top and top < len(module_occurrence):
            for module, usage in list(module_occurrence.items())[top:]:
                current_value = module_occurrence.get('Other functions', 0)
                module_occurrence['Other functions'] = current_value + usage
                del module_occurrence[module]

        # The result should not be sorted by `output_percentage' function
        # since `Other functions' bit should always stay at the bottom of
        # the table.
        output_percentage(self.formatter.output, module_occurrence, count, sort=False)

        if task_id_read:
            self.formatter.output("\nTask ID")
            output_percentage(
                self.formatter.output,
                task_id_occurrence,
                count,
                key_transform_function=lambda x: "0x%x" %
                self.chipdata.get_data(x) if x != 0 else "None")

    @DebugLog.suspend_log_decorator(0)
    def run_clks(self, time):
        """Calculates the total wake time / MCPS using NUM_RUN_CLKS.

        Please see RunClksReader for more details on the method.

        Args:
            time
        """

        self.formatter.output(
            "\nRun clocks profiling started for %d sec." % time
        )

        # While the measure event is set RunClksReader is profiling.
        measure_event = threading.Event()
        measure_event.clear()

        kwargs = {
            "chipdata": self.chipdata,
            "debuginfo": self.debuginfo,
            "formatter":  self.formatter,
            "interpreter": self.interpreter
        }

        reader = RunClksReader(
            measure_event, **kwargs
        )
        reader.start()
        measure_event.set()
        import_time.sleep(time)
        measure_event.clear()

        # wait for the task to finish
        reader.join()

    @DebugLog.suspend_log_decorator(0)
    def ret_run_clks(self, time):
        """Returns the total wake time / MCPS using NUM_RUN_CLKS.

        Please see RunClksReader for more details on the method.

        Args:
            time

        Returns:
            MCPS, percentage (derived), assumed CPU speed (MHz).
        """

        self.formatter.output(
            "\nRun clocks profiling started for %d sec." % time
        )

        # While the measure event is set RunClksReader is profiling.
        measure_event = threading.Event()
        measure_event.clear()

        kwargs = {
            "chipdata": self.chipdata,
            "debuginfo": self.debuginfo,
            "formatter":  self.formatter,
            "interpreter": self.interpreter
        }

        reader = RunClksReader(
            measure_event, **kwargs
        )
        reader.start()
        measure_event.set()
        import_time.sleep(time)
        measure_event.clear()

        # wait for the task to finish
        reader.join()

        mcps_usage = getattr(reader, '_mcps_usage', None)
        mcps_percent = getattr(reader, '_mcps_percent', None)
        chip_cpu_speed_mhz = getattr(reader, '_chip_cpu_speed_mhz', None)
        return mcps_usage, mcps_percent, chip_cpu_speed_mhz

    # The built in profiler is not accurate while the debuglog is enabled.
    # The profiler updates its reading every 1.024 second.
    # Sleeping for 1.024 second is needed to refresh Kymera builtin profiler..
    @DebugLog.suspend_log_decorator(1.024)
    def run_kymera_builtin(self, detailed_debug=False, raw_debug=False):
        """Displays the interpreted measurements.

        Reads the built-in Kymera profiler (if compiled in) and displays
        the interpreted measurements. This is an on-chip task which
        gathers statistics every 1.024 seconds and attempts to account CPU
        time (MCPS) to individual tasks. Its results are naturally a
        proportion of the reporting period, rather than an absolute number
        in MHz.

        On CDA devices, the built-in profiler is disabled by default. It can
        be enabled by setting the MIB key ProfilerEnable=true.

        Args:
            detailed_debug (bool, optional): Whether to also report
                detailed statistics for each profiler entry. Default is
                just a summary of MCPS. The detailed statistics, if
                available, include figures per invocation of the profiler
                (e.g., per-kick), such as INSTRS (from which MIPS can be
                derived).  These performance counters are described in the
                Qualcomm Kalimba Architecture DSP User Guide.
            raw_debug (bool, optional): Whether to also print the raw,
                uninterpreted profiler structure from the chip. Only
                effective with detailed_debug True.
        """

        if self._profiler_list_address is None:
            self.formatter.output("Profiling is not enabled for this build.")
            return

        try:
            enabled = self.chipdata.get_var_strict('$profiler.enabled').value
            if not enabled:
                self.formatter.output(
                    "Profiler is compiled in, but currently disabled.")
                if Arch.chip_arch == 'Hydra':
                    self.formatter.output(
                        "Set MIB key ProfilerEnable=true to enable it.")
                return
        except DebugInfoNoVariableError:
            # In very old builds, the profiler couldn't be turned off at
            # runtime. Assume this is one of those, and plough on.
            pass

        profiler_head_ptr = self.chipdata.get_var_strict(
            self._profiler_list_address
        ).value

        active = None
        sleep = None

        cpu_usage_overview = []
        cpu_usage = {}
        total_user_defined = 0

        # For debugging the builtin profiler the logs are saved.
        if detailed_debug:

            def builtin_profiler_log(string):
                """
                Function which stores the detailed debugging of the profiler.
                """
                builtin_profiler_log.debug_string += string

            builtin_profiler_log.debug_string = "\nDetailed:\n"
        else:

            def builtin_profiler_log(_):
                """
                Dummy function to ignore detailed debugging of the profiler.
                """

        # Check if the profiler list is not empty
        if profiler_head_ptr != 0:
            # read the first element of the list and parse the profiler list
            profiler_list = self.chipdata.cast(profiler_head_ptr, "profiler")

            for profiler in self.parse_linked_list(profiler_list, 'next'):
                # read the name of the profiler
                profiler_name = "%s 0x%x" % (
                    self.debuginfo.get_kymera_debuginfo().debug_strings.
                    get(profiler.get_member("name").value),
                    profiler.get_member("id").value
                )

                if profiler_name == "None 0x0":  # assembly style measurement
                    # get the original name from debuginfo
                    profiler_name = self.debuginfo.get_var_strict(
                        profiler.address
                    ).name.replace("L_", "")
                    builtin_profiler_log(
                        "Assembly style profiler entry: \"%s 0x%x\"," %
                        (profiler_name, profiler.address)
                    )
                else:
                    # Fish out the capability name
                    if profiler_name.startswith("Operator 0x"):
                        opid = int(profiler_name[9:], 16)
                        try:
                            opmgr = self.interpreter.get_analysis(
                                "opmgr", self.chipdata.processor
                            )
                            cid_name = opmgr.get_operator(opid).cap_data.name
                        except BaseException:
                            cid_name = "unknown capability"
                        profiler_name += ' (' + cid_name + ')'
                    builtin_profiler_log(
                        "C style profiler entry: \"%s\"," %
                        (profiler_name)
                    )

                # read the cpu usage of the profiler and convert it to percent.
                # Originally the cpu usage is in thousands.
                cpu_usage_val = float(
                    profiler.get_member("cpu_fraction").value
                ) / 10

                # Calculate kick statistics. (These are only useful for
                # profiler entries corresponding to operator tasks; others
                # will come out as zero.)
                try:
                    # kick_total is the number of kicks during the last
                    # reporting period that caused data to move.
                    # (Only nonzero for operator tasks.)
                    kick_total = float(
                        profiler.get_member("kick_total").value
                    )
                    if kick_total > 0:
                        # Average time between kicks (ms/kick)
                        stat_kick_period = 1024.0 / kick_total
                    else:
                        stat_kick_period = 0.0

                    # count is the total number of times the profiler
                    # was called during the last reporting period.
                    # For operator tasks, this is the total number of
                    # kicks received (including ones that didn't cause
                    # data to move).
                    # We thus expect it to > kick_total
                    count = float(
                        profiler.get_member("count").value
                    )
                    if count > 0 and kick_total > 0:
                        # Ratio of useful kicks to total kicks, as a
                        # percentage
                        stat_kick_ratio = 100.0 * kick_total / count
                    else:
                        stat_kick_ratio = 0.0
                except BaseException:
                    stat_kick_period = 0.0
                    stat_kick_ratio = 0.0

                try:
                    peak_cpu_usage = float(
                        profiler.get_member("peak_cpu_fraction").value
                    ) / 10
                except BaseException:
                    peak_cpu_usage = None
                # Check if the profiler is not a special one. Sleep time and
                # scheduler loop is considered special. They give the user
                # information about the system.
                if profiler_name == "$_sleep_time or $profiler.sleep_time":
                    cpu_usage_overview.insert(0, ("sleep", cpu_usage_val))
                    sleep = cpu_usage_val
                elif profiler_name == "sched_loop":
                    cpu_usage_overview.append(("sched_loop", cpu_usage_val))
                    active = cpu_usage_val
                else:
                    # This is a user defined or a Kymera profiler. Kymera
                    # profiler is used to measure the cpu usage of an
                    # operator. The User defined can be any profiling used
                    # for debug purposes.
                    total_user_defined += cpu_usage_val
                    cpu_usage[profiler_name] = (
                        cpu_usage_val, peak_cpu_usage, stat_kick_period,
                        stat_kick_ratio
                    )
                # Log the cpu usage to the detailed debug string.
                builtin_profiler_log(
                    "\nused %.1f%% of the CPU.\n" % (cpu_usage_val)
                )

                # Report detailed profiler statistics.

                # This reporting is designed to cope gracefully with more
                # measurements being added to the firmware over time, and
                # with builds where some measurements are compiled out; you
                # can just add new members to these arrays without any
                # firmware version checking, and everything should cope.
                # It relies on consistent naming of structure members.

                # Subset of on-chip measurements for which it makes sense to
                # derive a foo-per-clock value:
                per_clock = [
                    "instrs", "core_stalls", "prefetch_wait",
                    "mem_access_stalls", "instr_expand_stalls"
                ]
                # Add other on-chip measurement(s):
                measurements = ["run_clks"] + per_clock
                meas_width = max([len(m) for m in measurements])
                # For each on-chip measurement, there's a consistent set of
                # parameters (e.g., instrs_average, instrs_max, etc).
                on_chip_param_suffixes = ["average", "max"]
                # Names of derived parameters
                params = ["Avg MxPS", "Avg/clk", "Max"]
                # ...and format strings for those
                param_formats = ["%.3f", "%.2f%%", "%d"]
                # ...and sensible column widths
                param_width = max(7, max([len(param) for param in params]))
                raw = {}
                # Collate raw data from chip
                for meas in measurements:
                    cols = {}
                    for typ in on_chip_param_suffixes:
                        # None if member doesn't exist in this firmware:
                        mem = profiler.get_member(meas + "_" + typ)
                        if mem is not None:
                            cols[typ] = mem.value
                    # If no columns at all, probably the measurement isn't
                    # supported by this firmware. Forget it.
                    if len(cols) > 0:
                        raw[meas] = cols
                # Calculate derived values
                rows = {}
                for meas in measurements:
                    if meas in raw:
                        rows[meas] = {}
                        # "average" from the chip is in
                        # whatevers-per-millisecond.
                        # Calculate millions-of-whatevers-per-second
                        # (e.g., MIPS for INSTRS).
                        rows[meas]['Avg MxPS'] = (raw[meas]['average']
                                                  * 1000 / 1000000.0)
                        # Just copy "max" as-is. (This is the biggest
                        # value ever seen in a profiler run; it is not
                        # related to the 1.024s reporting period.)
                        rows[meas]['Max'] = raw[meas]['max']
                # (We do generally expect RUN_CLKS to be present, but
                # let's be robust.)
                if 'run_clks' in raw:
                    for meas in per_clock:
                        if meas in raw:
                            # Calculate all foo-per-clock, as a percentage
                            if raw['run_clks']['average'] != 0:
                                rows[meas]['Avg/clk'] = (
                                    (raw[meas]['average'] * 100.0)
                                    / raw['run_clks']['average']
                                )
                            # else no meaningful statistic, so display
                            # as "-"

                # ('raw' and 'rows' are now nice data structures with all
                # the raw and display data respectively, should we ever
                # decide to expose that programmatically.)

                # Format and print detailed report.
                if len(rows) > 0:
                    # Header
                    builtin_profiler_log(
                        "%*s  " % (meas_width, "") +
                        "  ".join([
                            "%*s" % (param_width, param)
                            for param in params
                        ]) + "\n"
                    )
                    # Table body
                    for meas in measurements:
                        if meas in rows:
                            cols = [
                                (
                                    (param_formats[i] % rows[meas][params[i]])
                                    if (params[i] in rows[meas]) else "-"
                                )
                                for i in range(len(params))
                            ]
                            builtin_profiler_log(
                                "%-*s  " % (meas_width, meas.upper()) +
                                "  ".join([
                                    "%*s" % (param_width, s)
                                    for s in cols
                                ]) + "\n"
                            )

                if raw_debug:
                    # Full raw structure dump
                    builtin_profiler_log(
                        str(profiler)
                    )
                builtin_profiler_log("\n")
        else:
            builtin_profiler_log("The profiler list is empty")

        if active is None or sleep is None:
            # shallow sleep was disabled
            cpu_usage_overview.insert(0, ("sleep not detected", 0))
            # using array because it will be modified after the interrupt time
            # is read.
            cpu_usage_overview.append(["sched_loop", 100])

        try:
            # Analyse the interrupt profiler
            interrupt_cpu_fraction = float(
                self.chipdata.get_var_strict('$interrupt.cpu_fraction').value
            )
            # cpu_usage["interrupt handler"] = interrupt_cpu_fraction/10
            cpu_usage_overview.append(
                (
                    "    interrupt handler (cannot be nested)",
                    interrupt_cpu_fraction / 10
                )
            )

            # read all the nested interrupts
            level = 0
            total_percent = 0
            # Interrupt with task per nested level.
            for intr_nested_level in self.chipdata.get_var_strict(
                    '$interrupt.cpu_fraction_with_task').value:
                percent = float(intr_nested_level) / 10
                total_percent += percent
                cpu_usage_overview.append(
                    (
                        "    interrupt with task at nested level %d" % level,
                        percent
                    )
                )
                level += 1

            cpu_usage_overview.insert(
                2, (
                    "total interrupt = int. handler + int. triggered task",
                    total_percent
                )
            )

            if active is None or sleep is None:
                # correct the scheduler loop value
                active = 100 - total_percent
                cpu_usage_overview[1][1] = active
                # Because the sleep was disabled the sleep percentage is zero.
                sleep = 0

            cpu_usage_overview.append(
                (
                    "sleep + active + int = %d Remains " %
                    (total_percent + sleep + active),
                    100 - (total_percent + sleep + active)
                )
            )

        except DebugInfoNoVariableError:
            # Interrupt profiler is off
            pass

        self.formatter.output("Overview:")
        output_percentage(
            self.formatter.output,
            OrderedDict(cpu_usage_overview),
            sort=False
        )
        if cpu_usage == {}:
            self.formatter.output(
                "\nNo kymera or user defined measurements available. "
            )
        else:
            self.formatter.output("\nKymera and user defined measurements:")
            self.formatter.output(
                " " * 42 + "current   peak    KickPd    KickEffic"
            )

            # outputs the percentage
            sorted_occurrence = sorted(
                list(cpu_usage.items()), key=operator.itemgetter(1)
            )
            sorted_occurrence.reverse()

            total_measurements = 100
            for element in sorted_occurrence:
                output_string = "%-40s " % (element[0])
                values = element[1]
                output_string += "%7.3f%% " % (
                    float(values[0] * 100) / total_measurements
                )
                if values[1] is None:
                    output_string += "         "
                else:
                    output_string += "%7.3f%% " % (
                        float(values[1] * 100) / total_measurements
                    )
                output_string += "%6.3fms " % (float(values[2]))
                output_string += "%8.3f%% " % (float(values[3]))
                self.formatter.output(output_string)

            self.formatter.output("_" * 58)
            output_percentage(
                self.formatter.output,
                OrderedDict([("total user defined", total_user_defined)]),
                sort=False,
                column=41
            )

        if detailed_debug:
            self.formatter.output(builtin_profiler_log.debug_string)

    def clear_peak(self):
        """Clears the peak cpu usage for all the built in Kymera profilers."""
        if self._profiler_list_address is None:
            self.formatter.output("Profiling is not enabled for this build.")
            return

        profiler_head_ptr = self.chipdata.get_var_strict(
            self._profiler_list_address
        ).value

        # Check if the profiler list is not empty
        if profiler_head_ptr != 0:
            # read the first element of the list and parse the profiler list
            profiler_list = self.chipdata.cast(profiler_head_ptr, "profiler")

            for profiler_entry in self.parse_linked_list(
                    profiler_list, 'next'
            ):
                # clear the peak cpu usage.
                self.chipdata.set_data(
                    profiler_entry.get_member("peak_cpu_fraction").address,
                    [0]
                )
