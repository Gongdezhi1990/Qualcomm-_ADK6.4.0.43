############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2012 - 2018 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
"""
Module for defining the Formatter interface.
"""
try:
    from future_builtins import hex
except ImportError:
    pass


class Formatter(object):
    """
    @brief This class defines an interface for outputting analysis data.
    A typical analysis might do something like:
      self.formatter.section_start('Widget Frobnication status')
      self.formatter.output('Widget is:')
      self.formatter.output(self.get_widget())
      if self.widget_is_clearly_corrupt:
        self.formatter.alert('Corrupt widget!')
      self.formatter.section_end()

    """

    def __init__(self):
        """
        @brief Initialises the Formatter object.
        @param[in] self Pointer to the current object
        """
        # set processor number
        self.proc = "P0"

    def section_start(self, header_str):
        """
        @brief Starts a new section. Sections can be nested.
        @param[in] self Pointer to the current object
        @param[in] header_str
        """
        raise NotImplementedError()

    def section_end(self):
        """
        @brief End a section.
        @param[in] self Pointer to the current object
        """
        raise NotImplementedError()

    def section_reset(self):
        """
        @brief Reset the section formatting, ending all open sections.
        This is provided so that in case of an error we can continue safely.
        @param[in] self Pointer to the current object
        """
        raise NotImplementedError()

    def set_proc(self, proc):
        """
        @brief record the current processor being analysed to be used in
        creating links
        @param[in] self Pointer to the current object
        @param[in] proc Current processor being analysed
        """
        self.proc = proc

    def output(self, string_to_output):
        """
        @brief Normal body text. Lists/dictionaries will be compacted.
        @param[in] self Pointer to the current object
        @param[in] string_to_output
        """
        raise NotImplementedError()

    def output_svg(self, string_to_output):
        """
        @brief svg body text. Lists/dictionaries will be compacted.
        @param[in] self Pointer to the current object
        @param[in] string_to_output
        """
        raise NotImplementedError()

    def output_raw(self, string_to_output):
        """
        @brief Unformatted text output. Useful when displaying tables.
        @param[in] self Pointer to the current object
        @param[in] string_to_output
        """
        raise NotImplementedError()

    def output_list(self, string_to_output):
        """
        @brief Normal body text. Lists/dictionaries will be printed in
            long-form.
        @param[in] self Pointer to the current object
        @param[in] string_to_output
        """
        if isinstance(string_to_output, (list, tuple)):
            # Less-easy
            # Printing to the screen is slow, so do it as one big string to go
            # faster
            list_string = []
            for t in string_to_output:
                list_string.append(self._prepare_output(t))

            # Generate a multi line plain-text
            self.output('\n'.join(list_string))

        else:
            # Easy
            self.output(string_to_output)

    def alert(self, alert_str):
        """
        @brief Raise an alert - important information that we want to be highlighted.
        For example, 'pmalloc pools exhausted' or 'chip has panicked'.
        @param[in] self Pointer to the current object
        @param[in] alert_str
        """
        raise NotImplementedError()

    def error(self, error_str):
        """
        @brief Raise an error. This signifies some problem with the analysis tool
        itself, e.g. an analysis can't complete for some reason.
        @param[in] self Pointer to the current object
        @param[in] error_str
        """
        raise NotImplementedError()

    def flush(self):
        """
        @brief Output all logged events (body text, alerts, errors etc.), then forget
        about them. If outputting to a file then the file will be (over)written
        at this point.
        @param[in] self Pointer to the current object
        """
        raise NotImplementedError()

    def _prepare_output(self, output):
        """
        Recursive method to prepare the given output

        @param[in]  output  This can be any type but ultimately it should be
                            possible to cast it to string.
        @param[out] String
        """
        if isinstance(output, (tuple, list)):
            prepared_output = []
            for item in output:
                prepared_output.append(self._prepare_output(item))

            return str(prepared_output)
        else:

            try:
                return hex(output)
            except TypeError:
                return str(output)
