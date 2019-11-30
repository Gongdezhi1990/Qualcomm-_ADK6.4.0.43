############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2012 - 2018 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
"""
Module responsible for displaying results in text format.
"""
import sys
import threading
import os.path

if sys.version_info.major == 2:
    # Python 2
    from future_builtins import hex

from ACAT.Display.Formatter import Formatter
from ACAT.Core.exceptions import FormatterError


class PlainTextFormatter(Formatter):
    """
    @brief Implements the Formatter interface to provide plain-text output.
    """

    def __init__(self):
        """
        @brief Initialises the object.
        @param[in] self Pointer to the current object
        """
        self.sectionlevel = 0

        # untitled sections
        self.untitled_sectionlevel = 0

        self.log_file = None  # Initialise here so that there is always one when we start logging

        # All the lines we want to output
        self.text = ""

        # Alerts and errors get saved up so that we can highlight them at
        # the end.
        self.alerts = []
        self.errors = []

        self.lock = threading.Lock()
        self.suppress_stdout_when_logging = False
        super(PlainTextFormatter, self).__init__()

    def change_log_file(self, new_log_filename, suppress_stdout=False):
        """
        @brief Creates a log file, and ensures that everything output via
        this formatter (e.g. via formatter.output) will be copied to the
        file as well as to stdout. Setting 'suppress_stdout' to True will
        mean that we don't output to stdout either, unless we're outputting
        an alert or error.

        If we are already logging, setting a new filename allows the
        log file to be changed over to a new file. A filename of None
        stops logging to a file (and stops suppressing stdout, if we
        were).
        @param[in] self Pointer to the current object
        @param[in] new_log_filename
        @param[in] suppress_stdout = False
        """
        self.suppress_stdout_when_logging = suppress_stdout

        # Close the open file before writing to the new one
        if self.log_file:
            # Check that they aren't the same file, otherwise the handles will be
            # the same and we'll close the file we want
            if new_log_filename:  # There is no name attribute if type None
                if self.log_file.name == new_log_filename:
                    return

            self.log_file.close()

        if new_log_filename:
            # (path, filename)
            (path, _) = os.path.split(new_log_filename)
            if path == '':
                path = '.\\'
            if not os.path.exists(path):
                os.makedirs(path)
            self.log_file = open(new_log_filename, 'w')
            # Definitely want the user to see this message, even if
            # suppress_stdout is set.
            self.alert("Now Logging to " + new_log_filename)
        else:
            self.log_file = None
            self.output("Logging to file stopped")

    def set_logfile(self, log_file):
        """
        @brief Sets the log file, and ensures that everything output via
        this formatter (e.g. via formatter.output) will be copied to the
        file.
        @param[in] self Pointer to the current object
        @param[in] log_file This must be a file handler
        """
        self.suppress_stdout_when_logging = True
        self.log_file = log_file

    def section_start(self, header_str):
        """
        @brief Starts a new section. Sections can be nested.
        @param[in] self Pointer to the current object
        @param[in] header_str
        """
        if header_str != "":
            # Increment our section level
            self.sectionlevel += 1

            section_hdr = (
                '-' * len(header_str) + '\n' + header_str + '\n' +
                '-' * len(header_str)
            )
            # Indent each line
            self._log('')
            self._log(section_hdr)
        else:
            self.untitled_sectionlevel += 1
            self._log('-' * 5)

    def section_end(self):
        """
        @brief End a section.
        @param[in] self Pointer to the current object
        """
        if self.untitled_sectionlevel > 0:
            self.untitled_sectionlevel -= 1
        else:
            if self.sectionlevel > 0:
                self.sectionlevel -= 1

    def section_reset(self):
        """
        @brief Reset the section formatting, ending all open sections.
        This is provided so that in case of an error we can continue safely.
        @param[in] self Pointer to the current object
        """
        self.sectionlevel = 0

    def output(self, string_to_output):
        """
        @brief Normal body text. Lists/dictionaries will be compacted.
        @param[in] self Pointer to the current object
        @param[in] string_to_output
        """
        with self.lock:
            if isinstance(string_to_output, (list, tuple)):
                self._log(str([hex(item) for item in string_to_output]))
            else:
                try:
                    self._log(hex(string_to_output))
                except TypeError:
                    self._log(str(string_to_output))

    def output_svg(self, string_to_output):
        """
        @brief svg body text. Lists/dictionaries will be compacted.
        @param[in] self Pointer to the current object
        @param[in] string_to_output
        """
        # we cannot pipe the svg to the output.
        raise FormatterError("Text formatter cannot diplay a svg file")

    def output_raw(self, string_to_output):
        """
        @brief Unformatted text output. Useful when displaying tables.
        @param[in] self Pointer to the current object
        @param[in] string_to_output
        """
        self.output(string_to_output)

    def alert(self, alert_str):
        """
        @brief Raise a alert - important information that we want to be highlighted.
        For example, 'pmalloc pools exhausted' or 'chip has panicked'.
        @param[in] self Pointer to the current object
        @param[in] alert_str
        """
        # Make sure alerts have the same type
        alert_str = str(alert_str)
        self._log("@ ALERT: " + alert_str, True)
        self.alerts.append(alert_str)

    def error(self, error_str):
        """
        @brief Raise an error. This signifies some problem with the analysis tool
        itself, e.g. an analysis can't complete for some reason.
        @param[in] self Pointer to the current object
        @param[in] error_str
        """
        # Make sure errors have the same type
        error_str = str(error_str)
        # Don't indent the output
        self._log("@@@@ ERROR: " + error_str, False)
        self.errors.append(error_str)

    def flush(self):
        """
        @brief Output all logged events (body text, alerts, errors etc.), then forget
        about them. You could then re-use the formatter if you really wanted.
        If outputting to a file then the file will be (over)written at this
        point.
        @param[in] self Pointer to the current object
        """

        # Before we output the log, we can apply some finishing-touches.
        # We want to reproduce any alerts at the start of the log.
        self.section_reset()
        error_summary = ("----------------------------\n" +
                         "Summary of alerts and errors\n" +
                         "----------------------------\n")
        error_summary = self._indent(error_summary)
        for error in sorted(self.errors):
            error_summary += self._indent(error + "\n")

        for alert in sorted(self.alerts):
            error_summary += self._indent(alert + "\n")

        self.text = error_summary + self.text

        if self.log_file:
            self.log_file.write(u"%s" % str(self.text))
            self.log_file.flush()
            if self.suppress_stdout_when_logging:
                return  # return early so we don't print to stdout

        print(self.text)

    ##################################################
    # Private methods
    ##################################################

    def _indent(self, input_str):
        """
        @brief Indent the supplied string by an amount based on section level.
        @param[in] self Pointer to the current object
        @param[in] input_str
        """
        indent = "    " * self.sectionlevel
        input_str = indent + input_str
        return input_str.replace('\n', '\n' + indent)

    def _log(self, output_str, indent=True):
        """
        @brief Log some text. Optionally, override the indentation rules.
        If 'can_suppress' is True, this output *may* in some circumstances
        be suppressed on stdout.
        This doesn't actually output anything; the log needs to be flushed
        for that to happen.
        @param[in] self Pointer to the current object
        @param[in] output_str
        @param[in] indent = True
        """
        if indent:
            str_to_write = self._indent(output_str)
        else:
            str_to_write = output_str

        self.text += (str_to_write + "\n")
