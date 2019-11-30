############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2012 - 2018 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
"""
This module implements the interactive formatter which is used during an
interactive session.
"""
import datetime

from ACAT.Display.PlainTextFormatter import PlainTextFormatter


class InteractiveFormatter(PlainTextFormatter):
    """
    @brief Extends the PlainTextFormatter class for interactive use. Everything is
    basically the same except events are automatically printed to stdout. If
    requested open a log file and enable timestamps
    """

    def __init__(self, log_file=None, timestamp=False):
        """
        @brief Initialises the object.
        @param[in] self Pointer to the current object
        @param[in] log_file = None
        @param[in] timestamp = False
        """
        PlainTextFormatter.__init__(self)
        # Initialise here so that there is always one when we start logging
        self.log_file = None
        self.timestamp = timestamp  # display timestamps
        if log_file:
            self.change_log_file(log_file)

    def section_end(self):
        """
        @brief End a section.
        @param[in] self Pointer to the current object
        """
        PlainTextFormatter.section_end(self)
        print('')  # extra spacing for interactive mode

    def flush(self):
        """
        @brief Flush the log file.
        @param[in] self Pointer to the current object
        """
        if self.log_file:
            self.log_file.flush()

    ##################################################
    # Private methods
    ##################################################

    def _log(self, output_str, indent=True):
        """
        @brief Print some text. Optionally, override the indentation rules.
        If 'can_suppress' is True, this output *may* in some circumstances
        be suppressed on stdout.
        @param[in] self Pointer to the current object
        @param[in] output_str
        @param[in] indent = True
        @param[in] can_suppress = True
        """
        if indent:
            str_to_write = self._indent(output_str)
        else:
            str_to_write = output_str

        # In some cases (mainly hydra log run from FATS) we want timestamps on
        # output
        if self.timestamp:
            now = datetime.datetime.now()
            str_to_write = now.strftime("%H.%M.%S.%f ") + str_to_write

        if self.log_file:
            self.log_file.write(u"%s\n" % str_to_write)
            if self.suppress_stdout_when_logging:
                return  # return early so we don't print to stdout
        print(str_to_write)
