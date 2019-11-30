############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2012 - 2019 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
"""
Analysis to read debug logs form the chip.
"""
import math
import os
import re
import sys
import threading
import time as import_time
from collections import deque
from functools import wraps

from . import Analysis
from ACAT.Core import Arch
from ACAT.Core import CoreUtils as cu
from ACAT.Core.exceptions import (
    UsageError, AnalysisError, DebugInfoNoVariableError
)

try:
    # pylint: disable=redefined-builtin
    from future_builtins import hex
except ImportError:
    pass

# Strict are the ones that are definitely called within method, not strict
# may or may not be called
VARIABLE_DEPENDENCIES = {
    'not_strict': (
        '$_debugLogLevel', '$_debugBufferSizeMask', 'L_debugBufferPos'
    ),
    'one_strict': (('L_debugBuffer', 'L_debugBuffer1'),)
}


class LogController(object):
    """Control the debuglog enable/disable with context manager.

        With this class the debuglog enable/disable can be encapsulated in
        a "with" statement.
    """

    def __init__(self, analysis, sleep_time=0):
        # init function of the controller.
        self.debuglog_was_enabled = False
        self.analysis = analysis
        self.sleep_time = sleep_time

    def __enter__(self):
        # Debug logging can cause erroneous profiling in many ways*, for
        # simplicity always disable it.
        # *The SPI can wake up the chip ( Amber chips will wake up whenever
        # live chip access is done) or the SPI bandwidth comes to a limit.
        if self.analysis.interpreter.get_analysis(
                "debuglog", self.analysis.chipdata.processor
            ).logging_active():
            print("Disable logging for the profiling")
            self.debuglog_was_enabled = True

            self.analysis.interpreter.get_analysis(
                "debuglog", self.analysis.chipdata.processor
            ).stop_polling()

            if self.sleep_time != 0:
                print("Sleep for %d" % self.sleep_time)
                import_time.sleep(self.sleep_time)

        # we don't need to return any resource
        return None

    def __exit__(self, *_):
        # re- enable debuglog if it was disabled.
        if self.debuglog_was_enabled:
            print("Re-enable logging for the profiling")
            self.analysis.interpreter.get_analysis(
                "debuglog", self.analysis.chipdata.processor
            ).poll_debug_log()


def suspend_log_decorator(sleep_time):
    """Creates a function decorator.

        It disables and re-enables debug logging (see below) and waits for
        a predefined time before the decorated function call.

    Args:
        sleep_time (int) Time to sleep before the decorated function will
        be called.
    """
    def decorator(func):
        """Disables and re-enables debug logging for the function call.

        Only does this if debuglog was enabled.

        Args:
            func
        """

        @wraps(func)
        def _new_method(self, *arg, **kws):
            with LogController(self, sleep_time):
                # call the actual function.
                return func(self, *arg, **kws)

        return _new_method

    return decorator


def is_debuglog_enabled_decorator(func):
    """Only calls the decorated function if debuglog is enabled.

    Args:
        func
    """

    @wraps(func)
    def _new_method(self, *arg, **kws):

        if self.is_debuglog_enabled:
            # call the actual function.
            return func(self, *arg, **kws)
        else:
            # displaying alerts for each function is a bit overkill but is
            # good for testing.
            # self.formatter.alert(
            #    "DebugLog is disabled in the build. debuglog.%s"
            #    " will have no affect."%(func.__name__)
            # )
            pass

    return _new_method


class DebugLog(Analysis.Analysis):
    """Encapsulates an analysis for Debug Log decode.

    Args:
        *arg: Variable length argument list.
        **kws: Arbitrary keyword arguments.
    """

    def __init__(self, **kwarg):
        # Call the base class constructor.
        Analysis.Analysis.__init__(self, **kwarg)
        try:
            # Check if debug log is enabled by reading debugLogLevel variable.
            # This variable can go out of sync if other tool is modifying the
            # debug log level, but we are only using it to get the address of
            # the debug log level variable so that's fine.
            self.log_level = self.chipdata.get_var_strict('$_debugLogLevel')
            self.is_debuglog_enabled = True
            self._stop_event = threading.Event()
            self._stop_event.clear()
            self._polling = threading.Event()
            self._polling.clear()
            self._debug_logging = Logging(
                self.chipdata, self.debuginfo, self._polling, self._stop_event,
                self.formatter
            )
        except DebugInfoNoVariableError:
            # Debug log is disabled for the current build. We still need the
            # analysis because the profiler and fats analyses are calling it.
            self.is_debuglog_enabled = False
            self.formatter.alert("DebugLog is disabled in the build")

    def __del__(self):
        if self.is_debuglog_enabled:
            # Need to make sure we stop polling if the object is destroyed.
            self._polling.clear()
            self._stop_event.set()
            if self._debug_logging.is_alive():
                self._debug_logging.join()

    @is_debuglog_enabled_decorator
    def run_all(self):
        """Outputs the contents of the debug log buffer."""
        self.formatter.section_start('Debug Log')
        self.analyse_debug_log()
        self.formatter.section_end()

    @is_debuglog_enabled_decorator
    def get_debug_log(self):
        """Gets the (decoded) contents of the debug_log buffer.

        This could raise a number of different exception types, including
        AnalysisError.

        Returns:
            A list of debug strings, ordered from oldest to newest.

        Raises:
            AnalysisError: The firmware does not have Debug Log symbols.
            UsageError: Debug Log is being polled.
        """

        if not self._debug_logging.debuginfo_present:
            raise AnalysisError(
                "This firmware does not have symbols for the Debug Log.\n"
                "Most likely it's not compiled in."
            )

        # Don't perform this action if polling is occurring
        if self._polling.is_set():
            raise UsageError(
                "Debug Log is being polled; "
                "you must stop polling to make this call!"
            )

        return self._debug_logging.decode_log()

    @is_debuglog_enabled_decorator
    def set_debug_log_level(self, set_level):
        """Sets the debug log level to use.

        Note:
            This is only available on a live chip.

        Args:
            set_level (int)
        """
        if not self.chipdata.is_volatile():
            raise UsageError(
                "The debug log level can only be set on a live chip.")

        # Force to int, just in case the user supplied a string
        set_level = int(set_level)
        if set_level > 5 or set_level < 0:
            raise UsageError("Level must be in the range 0-5\n")
        else:
            # for the 32-bit architecture, the memory location where log_level
            # is might contain other values, therefore set_data() cannot be
            # used without any algorithm to preserve the other values since it
            #  overwrites the whole word the offset of log_level in the word
            offset = self.log_level.address % Arch.addr_per_word
            # the mask to be applied to get the value of log_level from the
            # word
            mask = (
                int(math.pow(2, 8 * self.log_level.size) - 1)
            ) << (8 * offset)
            value = self.chipdata.get_data(self.log_level.address)
            # set the log_level to 0, while preserving the other bits in the
            # word
            value &= (~mask)
            # add the set_level variable in the location of log_level in the
            # word
            value |= (set_level << (8 * offset))
            self.chipdata.set_data(self.log_level.address, [value])
            self.formatter.alert(
                "Debug log level set to " + str(set_level) + "\n"
            )

    @is_debuglog_enabled_decorator
    def print_log_level(self):
        """Prints the current debug log level."""
        # update the log level variable
        self.log_level = self.chipdata.get_var_strict('$_debugLogLevel')
        self.formatter.alert(
            "Debug log level is currently set to %s \n" % str(
                self.log_level.value
            )
        )

    @is_debuglog_enabled_decorator
    def poll_debug_log(self, wait_for_keypress=None):
        """Polls the debug_log buffer.

        If the contents of the buffer changes then it prints out the new
        extra contents. This function spawns a new thread to perform the
        polling operation.

        Note:
            This is only available on a live chip.

        Raises:
            UsageError: Invoked on something other than a live chip.
            UsageError: Debug Log is being polled.
            AnalysisError: The firmware does not have Debug Log symbols.
        """
        if wait_for_keypress is None:
            if os.name == 'nt':
                wait_for_keypress = False
            else:
                wait_for_keypress = True
        if not self.chipdata.is_volatile():
            raise UsageError(
                "ERROR: This functionality is only available on a live chip.\n"
                "Use get_cu.debug_log()"
            )

        if not self._debug_logging.debuginfo_present:
            raise AnalysisError(
                "This firmware does not have symbols for the Debug log.\n"
                "Most likely it's not compiled in."
            )

        # If we're already polling don't do it again
        if self._polling.is_set():
            raise UsageError(
                "Debug log is being polled; "
                "you must stop polling to make this call!"
            )

        # Use 'alert' to differentiate command responses from the debug log
        # contents
        self.formatter.alert("Polling debug log...")

        # start polling in s separate thread
        if not self._debug_logging.is_alive():
            self._debug_logging.start()
        self._polling.set()

        if wait_for_keypress:
            self.formatter.alert("Press any key to stop.")
            # wait until a key is pressed.
            sys.stdin.read(1)
            self.stop_polling()

    @is_debuglog_enabled_decorator
    def stop_polling(self):
        """Stops the polling.

        Sets a flag that the polling thread can see telling it to stop.
        """
        self._polling.clear()
        self._debug_logging.inactive.wait()
        # Use 'alert' to differentiate command responses from the debug log
        # contents
        self.formatter.alert("Polling debug log terminated")

    def logging_active(self):
        """Check if the ACAT is actively logging the debug log.

        Returns:
            bool: True if the ACAT is actively logging the debuglog, False
                otherwise.
        """
        if self.is_debuglog_enabled:
            return self._polling.is_set()

        return False

    @is_debuglog_enabled_decorator
    def set_formatter(self, formatter):
        """Sets how the result is being formatted.

        Args:
            formatter (:obj:`Formatter`)
        """
        Analysis.Analysis.set_formatter(self, formatter)
        self._debug_logging.formatter = self.formatter

    #######################################################################
    # Analysis methods - public since we may want to call them individually
    #######################################################################

    @is_debuglog_enabled_decorator
    def analyse_debug_log(self):
        """Outputs the contents of the debug_log buffer.

        If something goes wrong (Debug Log not compiled in, Debug Log is
        being polled etc.), an exception is thrown.
        """
        self.formatter.output('Debug log contents is: ')
        self.formatter.output_list(self.get_debug_log())


class Logging(threading.Thread):
    """Creates a separate thread for logging debug information.

    Attributes:
        debuginfo
        formatter
        chipdata
        decode_lock
        inactive
        last_read
        debug_log
        debuginfo_present
        current_elf_id
        p_buffer
        p_buffer
        p_buffer_size
        p_buffer_wrp

    Args:
        chipdata
        debuginfo
        polling
        stop_event
        formatter
    """

    def __init__(self, chipdata, debuginfo, polling, stop_event, formatter):
        threading.Thread.__init__(self)
        self._stop_event = stop_event
        self._polling = polling
        self.debuginfo = debuginfo
        self.formatter = formatter
        self.chipdata = chipdata

        self.decode_lock = threading.Lock()
        # This event can give a feedback about the state of the debuglog
        # thread.
        self.inactive = threading.Event()
        self.inactive.set()

        # When polling on a live chip we need to remember where we last read
        # up to; by starting at -10 even if we add 1 to do some checks it
        # doesn't go positive
        self.last_read = -10

        # Our list of formatted debug strings. Set every time we call
        # decode_log.decode_log is a limited queue
        self.debug_log = deque(maxlen=10000)

        # Look up the data addresses of the variables we need.
        # Note: this could fail if debug log was not enabled in the build!
        try:
            if self.chipdata.processor != 0:
                self.p_buffer = self.debuginfo.get_var_strict(
                    'L_debugBuffer1'
                ).address
            else:
                self.p_buffer = self.debuginfo.get_var_strict(
                    'L_debugBuffer'
                ).address  # Address of the debug buffer
            self.p_buffer_size = self.debuginfo.get_var_strict(
                '$_debugBufferSizeMask'
            ).address  # Address of 'buffer size mask'
            self.p_buffer_wrp = self.debuginfo.get_var_strict(
                'L_debugBufferPos'
            ).address  # Address of the debug buffer's write handle
            # Get the buffer size now. This is fixed and saves time in a live
            #  chip scenario the size is in words and it has to be converted
            # to addressable units since get_data takes the size in addr
            # units !! Because debugBufferSizeMask is size-1.
            self.buffer_size = Arch.addr_per_word * \
                (self.chipdata.get_var_strict(self.p_buffer_size).value + 1)
        except DebugInfoNoVariableError:
            # Can't find symbol. Assume firmware doesn't have debug log
            # compiled in.
            self.debuginfo_present = False
            return

        self.debuginfo_present = True

        # current elf id is used to select the correct downloade bundle when
        # reading debug messages.
        self.current_elf_id = 0

    def _get_formatter_string(self, string_ptr, arguments):
        """Searches for the formatter string in the debug information.

        Also checks the downloadable capabilities.

        Args:
            string_ptr: Pointer to the formatter string
            arguments: Arguments for the formatter string. Note this can
                change.
        """
        region = Arch.get_dm_region(string_ptr, False)
        if region == 'DBG_DWL':
            return_address = -1
            try:
                if arguments:
                    # for downloadable capabilities the first argument to the
                    # debug logging is the return address for the debug log
                    # call. This return address is used to identify the
                    # downloadable capability.
                    return_address = arguments[0]
                    elf_id = self.debuginfo.table.get_elf_id_from_address(
                        return_address
                    )

                    if elf_id is not None:
                        self.current_elf_id = elf_id
                        arguments.pop(0)
                # The linker will put the debug messages for downloadable
                # capabilities to 0x15500000, but the elf for some reason
                # leaves it in 0x13500000. Count for the difference
                # 0x15500000 -  0x13500000 = 0x2000000#
                download_bundle = self.debuginfo.debug_infos[self.current_elf_id]
                mystr = (
                    download_bundle.debug_strings[string_ptr - 0x2000000]
                )
            except KeyError:
                mystr = (
                    "@@@@ ERROR: Cannot find %s debug string" +
                    " in downloadable capability (capability elf id %s)."
                ) % (hex(string_ptr), hex(self.current_elf_id))
        elif region == 'DBG_PTCH':
            # Look at the patch elf.
            # No address conversation is needed; the patch is directly mapped.
            patch = self.debuginfo.get_patch_debuginfo()
            if patch is None:
                mystr = (
                    "@@@@ ERROR: Cannot find %s debug string" +
                    " because patch not loaded."
                ) % (hex(string_ptr))
            else:
                try:
                    # The linker will put the debug messages for the patch
                    # to 0x14500000, but in the elf it will be in 0x13500000.
                    # Count for the difference:
                    #   0x14500000 -  0x13500000 = 0x1000000
                    mystr = patch.debug_strings[string_ptr - 0x1000000]
                except KeyError:
                    mystr = (
                        "@@@@ ERROR: Cannot find %s debug string" +
                        " in patch."
                    ) % (hex(string_ptr))
        else:
            # It's almost certainly a valid string pointer.
            # Still a remote chance that we have been very unlucky and hit
            # a numerical argument which coincides with a string address;
            # if so we'll hit an exception when we do 'mystr %
            # tuple(arguments)' below.
            try:
                kymera = self.debuginfo.get_kymera_debuginfo()
                mystr = kymera.debug_strings[string_ptr]
            except KeyError:
                # invalid pointer. This is probably caused by a buffer tear.
                mystr = None
            if mystr is None:
                mystr = "@@@@ ERROR: Cannot find %s debug string." % hex(
                    string_ptr
                )
        return mystr

    def _format_string(self, formatter, arguments):
        """Searches for the formatter string in the debug information.

        Also checks the downloadable capabilities.

        Args:
            string_ptr: Pointer to the formatter string.
            arguments: Arguments for the formatter string. Note this can
                 change.
        """
        try:
            # python can't handle the '%p' format specifier, apparently.
            formatter = re.sub(r'%([0-9]*)p', r'0x%\g<1>x', formatter)
            formattedstr = formatter % tuple(
                arguments
            )  # works for an empty list too
        except TypeError as exception:
            # TypeError: %d format: a number is required, not str
            # Nasty special case: if we incorrectly interpreted a large number
            # as a string, the format specifier won't match here.
            if re.search("arguments number is required", str(exception)):
                specifiers = re.findall(r"%([\d\w]+)", formatter)
                # if len(specifiers) != len(arguments) bad case
                for idx, val in enumerate(specifiers):
                    # If the specifier isn't a string, but we have a string in
                    # args, try to correct our mistake.
                    cs_available = re.search("[cs]", val)
                    if not cs_available and isinstance(arguments[idx], str):
                        # Replace the string with the original large number.
                        # That requires a search through all the debug strings
                        # to work out what the number was.
                        debug_strings = self.debuginfo.get_kymera_debuginfo().debug_strings
                        for addr, string in debug_strings.items():
                            if string == arguments[idx]:
                                arguments[idx] = addr
                                formattedstr = formatter % tuple(arguments)
            else:
                formattedstr = (
                        "\n@@@@ ERROR:" + str(exception) + "\n" +
                        "     formatter string: %s\n" % (formatter) +
                        "     arguments: %s\n" % (cu.list_to_string(arguments))
                    )
        return formattedstr

    def decode_log(self):
        """Decodes the contents of the debug_buffer and returns it.

        This could raise a number of different exception types, including
        AnalysisError. This function could be called from outside of this
        thread. A lock is used to makes sure it is only running on one
        thread.
        """
        with self.decode_lock:
            return self._decode_log()

    def _decode_log(self):
        """Same as decode_log, but is not thread safe."""
        # Form the list on a temporary list so we don't duplicate old entries
        tmp_debug_log = []
        # Read all the debug variables.
        buffer_wrp = self.chipdata.get_data(self.p_buffer_wrp)
        debug_buffer = self.chipdata.get_data(self.p_buffer, self.buffer_size)
        if buffer_wrp == self.last_read + 1:
            # Nothing new in the log so don't bother reading it
            return

        # The debug buffer looks like this:
        #
        #     [appppaapapaa...............ppp]
        #      ^           ^                 ^
        #  debug_buffer    buffer_wrp      len(debug_buffer)
        #
        #  p = string pointer, arguments = string argument

        # Decoding is more complicated than you might think, because:
        # * We can't assume that a value > $flash.debugdata.__Base is a string
        #    pointer (it could just be a large number)
        # * String pointers can in some circumstances be arguments to other
        #   strings (more details below).
        # * The buffer may not be completely full
        # * The buffer may have wrapped unevenly, so that the first few
        #   arguments after buffer_wrp are orphaned from the corresponding
        #   string.
        #
        # So what we need to do is work backwards from buffer_wrp, find the
        # last string pointer, and count the number of format specifiers
        # in it to check that it matches the number of arguments.

        arguments = []  # list of args for each log statement
        i = buffer_wrp - 1  # start with the newest entry in the buffer
        last_read = i  # record how far we will read up to this time
        wrapped = False

        while True:
            # Check for a wrap/repeat
            if i < 0:
                i = len(debug_buffer) - 1
                wrapped = True
            if wrapped and i <= buffer_wrp:
                # we've parsed the whole buffer
                break

            # We need to look for things that look like pointers to debug
            # strings.
            region = Arch.get_dm_region(debug_buffer[i], False)
            if region == 'DEBUG' or region == 'DBG_DWL'or region == 'DBG_PTCH':
                mystr = self._get_formatter_string(debug_buffer[i], arguments)

                # Now it could just so happen that this value is an argument
                # to printf AND a valid debug string pointer, like:
                #    AUDIO_LOG_STRING(hi, "hello");
                #    L0_DBG_MSG1("foo: %s", (DBG_STR)hi);
                # (This construct is especially prevalent in isp_router).
                #
                # We can check that the string pointed-to has the same number
                # of arguments as we have in a[], but that doesn't actually
                # help in this case since it doesn't allow us to distinguish
                # between a string that simply has no arguments, and a string
                # that is an argument to another string.
                # To avoid painful look-ahead, the simple way to solve this
                # is to just assume every complete-looking string is actually
                # complete, and put it into tmp_debug_log[]. If we then come
                # across a string that doesn't seem to have enough
                # arguments, we backtrack. Note that we only support one
                # level of nesting here; if someone has put a formatted
                # string inside a formatted string then all bets are off.

                if mystr.count('%') == 0 and arguments:
                    # This could happen if you had something like:
                    #    L0_DBG_MSG1("foo: %s %d", (DBG_STR)hi, 0x1234)
                    # This string is clearly just actually an argument.
                    arguments.append(mystr)
                    i -= 1
                    continue

                if mystr.count('%') > len(arguments):
                    # We're missing some arguments. We probably put them into
                    #  tmp_debug_log[], thinking they were complete strings.
                    num_args_missing = mystr.count('%') - len(arguments)
                    if num_args_missing == mystr.count('%s'):
                        temp_arguments = tmp_debug_log[-num_args_missing:]
                        if Arch.addr_per_word != 4:
                            # For Hydra platforms, like Crescendo, Aura and
                            # Napier the arguments in tmp_debug_log[] are in
                            # the right order, because the order in which the
                            #  arguments are stored. For Blucore platforms,
                            # like Gordon and Rick, the array slice is in the
                            #  wrong order.
                            temp_arguments.reverse()
                        arguments = temp_arguments + arguments
                        # now arguments[] contains what it would have held,
                        # had we not put the missing args into
                        # tmp_debug_log[]. Last thing to do is disavow them.
                        tmp_debug_log[-num_args_missing:] = []

                # Now we should have the right number of arguments!
                str_cnt = mystr.count('%')

                # Assume this is a complete log.
                # shrink-wrap the string we just read, and save it.
                arguments.reverse()  # because we added args in reverse order.

                # format the string.
                formatter_error = False
                if str_cnt == len(arguments):
                    try:
                        formatted_str = self._format_string(mystr, arguments)
                    except ValueError:
                        # ValueError can be caused by erroneous formatting
                        # string like a "0x%08"
                        formatter_error = True
                else:
                    formatter_error = True
                if formatter_error:
                    formatted_str = (
                        "\n@@@@ ERROR: Wrong number of arguments! " +
                        "This could be caused by buffer tear. \n" +
                        "Buffer tear happens when messages are " +
                        "written too fast to the log buffer\n" +
                        "and the debug interface cannot keep up in reading them. \n" +
                        "     formatter string: %s\n" % (mystr.replace("\n", "")) +
                        "     arguments: %s\n" % (cu.list_to_string(arguments))
                    )

                # Add the formatted string to the debug log.
                tmp_debug_log.append(formatted_str)
                # Clear the arguments for the next run.
                arguments = []

            else:
                # Found an argument.
                arguments.append(debug_buffer[i])

            i -= 1  # next

            if i == self.last_read:  # We've reached where we got to
                break

        self.last_read = last_read
        # invert the debug log so it is printed in the correct order
        tmp_debug_log.reverse()
        return tmp_debug_log

    def run(self):
        """Polls the debug log buffer repeatedly.

        This is called in a separate thread so that command line input
        still works. This could raise a number of different exception
        types, including AnalysisError.
        """
        while not self._stop_event.is_set():
            self.inactive.set()
            self._polling.wait()
            self.inactive.clear()
            try:
                new_debug_log = self.decode_log()
                # new_debug_log can be None
                if isinstance(new_debug_log, list):
                    self.debug_log += new_debug_log
                    self.formatter.output_list(new_debug_log)
            except (TypeError, IndexError, AnalysisError) as excep:
                # It's possible if the buffer tears.
                self.formatter.output("|%s| in Logging.run()" % (str(excep)))
