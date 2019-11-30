############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2015 - 2018 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
"""
Interpreter abstraction class.
"""
import traceback
import importlib

from ACAT.Core import CoreTypes as ct
from ACAT.Core import CoreUtils as cu
from ACAT.Core.TypeDependencies import check_dependencies
from ACAT.Core.constants import ANALYSES
from ACAT.Core.exceptions import OutdatedFwAnalysisError


class Interpreter(object):
    """
    @brief Base class for all interpreters.

    @param[in] p0 A Processor instance
    @param[in] p1 A Processor instance
    @param[in] analyses A list of requested analyses. If this
               option is not provided all the default analyses
               will be performed.
    @param[in] analyses_exceptions A list/tuple of analyses
               which should not be included. This option is
               more useful if the child class is relying on
               default analyses.
    """

    def __init__(self, p0=None, p1=None, analyses=None, analyses_exceptions=None):
        self.p0 = p0
        self.p1 = p1
        self.formatter = None

        self.processors = {}
        for p_name, p_instance in (('p0', p0), ('p1', p1)):
            if p_instance:
                self.processors[p_name] = p_instance

                # p0 and p1's formatters are the same, so it doesn't matter
                # if we overwrite.
                self.formatter = p_instance.formatter

        if analyses:
            self.analyses = [analysis.lower() for analysis in analyses]

        else:
            self.analyses = list(ANALYSES.keys())

        if analyses_exceptions:
            lower_case_exceptions = [
                analysis.lower()
                for analysis in analyses_exceptions
            ]
            self.analyses = [
                analysis.lower()
                for analysis in self.analyses
                if analysis.lower() not in lower_case_exceptions
            ]

        self._load_analyses()

    def run(self):
        """
        @brief Interpreter runner
        """
        raise NotImplementedError()

    def get_analysis(self, name, processor):
        """
        @brief  Function to get an initialised analysis. This is used by the
            Analysis module.
        @param[in] self Pointer to the current object
        @param[in] name Analysis name
        @param[in] processor processor number.
        """
        if processor == 0:
            return self.p0.available_analyses[name]

        return self.p1.available_analyses[name]

    def to_file(self, file_name, suppress_stdout=False):
        """
        Directs the output  of all analyses to a file. Analyses can have
        separate files as output by using the analysis to file function
        "[ANALYSIS NAME].to_file()".
        """
        self.formatter.change_log_file(file_name, suppress_stdout)

        for processor in self.processors:
            for analysis in self.processors[processor].available_analyses:
                self.processors[processor].available_analyses[
                    analysis
                ].formatter = self.formatter

    def _load_analysis(self, analysis, processor):
        """
        @brief  Function to import and instantiate one analysis.
        @param[in] self Pointer to the current object
        @param[in] analysis Analysis name
        @param[in] processor processor number.

        Raises:
            ImportError: When the given analysis name is invalid.
        """
        class_name = ANALYSES.get(analysis)
        if class_name is None:
            raise ImportError('Invalid {} analysis'.format(analysis))

        current_proc = self.processors[processor]
        kwargs = {
            "chipdata": current_proc.chipdata,
            "debuginfo": current_proc.debuginfo,
            "formatter": current_proc.formatter,
            "interpreter": self
        }
        # All analyses depend on module Analysis.
        # Python 3 demands that it is imported first.
        importlib.import_module("ACAT.Analysis")
        extensions = ["", "Old1", "Old2", "Old3"]
        for ext in extensions:
            try:
                # Import the analyses.
                imported_analysis = importlib.import_module(
                    '.' + class_name + ext,
                    "ACAT.Analysis"
                )
                # Get the analysis class from the module.
                class_obj = getattr(imported_analysis, class_name)
                # create the analyses object
                analysis_obj = class_obj(**kwargs)
                current_proc.available_analyses[analysis] = analysis_obj
                setattr(current_proc, analysis, analysis_obj)

                if cu.global_options.dependency_check:
                    # check if the analyses loaded are compatible with the debug
                    # information.
                    check_dependencies(
                        current_proc,
                        class_name,
                        imported_analysis
                    )
                # exit from the loop
                return
            except ImportError:
                # If the standard (the one with no extension) analyses is not
                # available it means no other will so bail out.
                if ext == "":
                    raise ImportError()
                continue
            except OutdatedFwAnalysisError:
                # continue to look for compatible analyses
                continue

        # Couldn't find anything. If we are here it is because outdated Kymera.
        raise OutdatedFwAnalysisError()

    def _load_analyses(self):
        """
        @brief  Function to import and instantiate all the available analyses.
        @param[in] self Pointer to the current object
        """
        # Inspect every variable - shouldn't take too long to do this
        # up-front.
        for processor in sorted(self.processors.keys()):
            # Invoke all the other analysis modules.
            # Slightly tricky stuff so that we can import them all and create
            # an instance of them without having to modify this code every time
            # we add a new one.
            # Save any unavailable analyses
            current_proc = self.processors[processor]
            formatter = current_proc.formatter
            if cu.global_options.dependency_check:
                # Type dependency check will run so create a section for it.
                current_proc.formatter.section_start(
                    "Dependency Check For %s" % processor.upper()
                )
            unavailable_analyses = []
            for analysis in self.analyses:
                # pylint: disable=broad-except
                try:
                    self._load_analysis(analysis, processor)
                except ImportError:
                    # Analysis not available for the user.
                    formatter.alert(
                        analysis + ' analysis for ' + processor +
                        ' is not available.'
                    )
                    # in case of an import error the analysis is not available.
                    unavailable_analyses.append(analysis)
                except OutdatedFwAnalysisError:
                    # Analysis not available for the user.
                    formatter.alert(
                        analysis + ' analysis for ' + processor +
                        ' is not available because Kymera is outdated.'
                    )
                    # in case of an import error the analysis is not available.
                    unavailable_analyses.append(analysis)
                except Exception:
                    # The analysis failed to initialise. Not much we can do
                    # about that.
                    formatter.section_start(analysis)
                    formatter.alert(
                        'Analysis ' + analysis + ' failed to initialise for ' +
                        processor
                    )
                    formatter.output(traceback.format_exc() + '\n')
                    formatter.section_reset()
                # pylint: enable=broad-except
            # Remove any unavailable analyses to avoid loading them for the
            # other processor.
            for analysis in unavailable_analyses:
                self.analyses.remove(analysis)
            if cu.global_options.dependency_check:
                # Close the type dependency check.
                formatter.section_end()

    def _flush_output(self):
        """
        Function used to flush the output of the interpreter.
        """
        self.formatter.flush()

        for processor in self.processors:
            for analysis in self.processors[processor].available_analyses:
                cur_proc = self.processors[processor]
                cur_formatter = cur_proc.available_analyses[analysis].formatter
                if self.formatter != cur_formatter:
                    # The analyses have differnt fomatter so flush it
                    cur_proc.available_analyses[analysis].formatter.flush()

