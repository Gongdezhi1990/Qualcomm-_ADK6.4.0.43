############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2012 - 2019 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
"""
Module which describes the base class for all the analyses.
"""
import abc
import math
import time
from functools import wraps

from ACAT.Core import Arch
from ACAT.Core import CoreTypes as ct
from ACAT.Core import CoreUtils as cu
from ACAT.Display import InteractiveFormatter


def cache_decorator(function):
    """An analysis function return value can be cached with this decorator.

    Args:
        function (obj): A callable.
    """

    @wraps(function)
    def _new_method(self, *arg, **kws):
        """Wrapping function."""
        # The dictionary type is unhashable. The input parameters of the
        # function need  to be converted to a tuple.
        arguments_tuples = tuple(arg) + tuple(kws) + tuple(kws.values())

        # check if we need to empty our cache.
        now = time.time()
        if self.cache_expire_time < now:
            self.cache = {}

        # Check if the function is already cached for the analysis.
        if function in self.cache:
            function_cache = self.cache[function]
        else:
            # Function is not cached. Create a new cache for the function
            function_cache = {}

        # check if the function cache already contains the current argument.
        if arguments_tuples in function_cache:
            retval = function_cache[arguments_tuples]
        else:
            # call the function
            retval = function(self, *arg, **kws)
            # set the cache expire time.
            self.cache_expire_time = self.cache_validity_interval + now

        # put the return value of the function to the function cache
        function_cache[arguments_tuples] = retval

        # Update the operator cache.
        self.cache[function] = function_cache
        return retval

    # return the new function created by the decorator
    return _new_method


# The abstract Analysis class is used in many other places, but pylint
# can not see them. Hence disabling the check in this package.
# pylint: disable=abstract-class-not-used
class Analysis(object):
    """Base class for all analysis plugins.

    Example:

        Individual analysis can be a subclass of the Analysis class. See
        the example below:

        class MyAnalysis(Analysis.Analysis):
            def __init__(self, **kwarg):
                # Call the base class constructor. This call will set all
                # the values from the dictionary as attributes. In this
                # manner chipdata, debuginfo and formatter will be set.

                super(MyAnalysis, self).__init__(self, **kwarg)
                do_other_stuff()

            def run_all(self):
                run_useful_analysis()

    Convention is for useful methods (which we might want to call from other
    analyses and in Interactive mode) to be public and they should return
    data structures rather than printing it directly. An analysis which
    prints information via the formatter should follow the naming convention
    that has the form analyse_xxx (or run_all). These functions return nothing.

    Do not be afraid to throw exceptions from Analysis functions. In fact,
    it is to be encouraged; when run in (say) Interactive mode we want as much
    direct feedback as possible.

    Analyses should not assume that chipdata is non-volatile, since they may
    be asked to operate on a live chip (e.g. by a framework other than
    CoreReport). ChipData.is_volatile() is provided so that an analysis can,
    if necessary, optimise for the non-volatile case.
    (FWIW debuginfo is inherently non-volatile so there's no need to check
    that.)

    Analyses should generally avoid doing any significant work - including
    any data or symbol lookup - in their constructors. If an error occurs
    that causes an exception, none of the rest of the analysis will run.

    An analysis can cache some of its answer. In other words the return value
    for some of their functions can be cached. Function which are used for
    inter-analyses communication should always be cached. In this manner,
    we can avoid refreshing the internal data of the called analysis and
    creating new variables when providing information to other analyses. To
    mark a function for caching it must be decorated with cache_decorator.
    This cache is cleared after each user instruction in interactive mode.
    With coredumps or in automatic mode there is no need to empty the cahce.

    Args:
        **kwargs: Arbitrary keyword arguments.

    Attributes:
        cache (dict)
        cache_expire_time (timestamp)
        cache_validity_interval (int)
        chipdata (:obj:`ChipData`)
        debuginfo (:obj:`DebugInfo`)
        interpreter (:obj:`Iterpreter`)
        formatter (:obj:`Formatter`)
    """

    def __init__(self, **kwargs):
        self.cache = {}
        # Time showing when the cache expires.
        self.cache_expire_time = time.time()
        # Cache validity in seconds.
        self.cache_validity_interval = cu.global_options.cache_validity_interval

        # Default internal values.
        self.chipdata = None
        self.debuginfo = None
        self.interpreter = None
        # Note that it is valid for formatter to be None; if we just want
        # to analyse data (and not print anything out) it is not necessary
        # to create a pointless formatter object.
        self.formatter = None

        for key in kwargs:
            setattr(self, key, kwargs[key])

        if self.formatter is not None:
            self.default_formatter = self.formatter

        try:
            if not any((self.chipdata, self.debuginfo, self.formatter)):
                raise Exception(
                    "All analyses need at least chipdata, "
                    "debuginfo and formatter."
                )
        except AttributeError:
            raise Exception(
                "All analyses need at least chipdata, "
                "debuginfo and formattter."
            )

    @abc.abstractmethod
    def run_all(self):
        """Performs all the useful analysis this module can do.

        Any useful output should be directed via the Analysis' formatter.
        """

    def set_formatter(self, formatter):
        """Sets the formatter for an analysis.

        Args:
            formatter (obj): Instance of the Formatter class.
        """
        self.formatter = formatter

    def reset_formatter(self):
        """Resets the formatter for an analysis."""
        # Signal to the formatter before swapping.
        self.formatter.flush()
        self.formatter = self.default_formatter

    def to_file(self, file_name, suppress_stdout=False):
        """Sets the output of the analysis to a file.

        Args:
            file_name (str): The output filename.
            suppress_stdout (bool): Whether to suppress the standard
                output or not.
        """
        formatter = InteractiveFormatter.InteractiveFormatter()
        formatter.change_log_file(file_name, suppress_stdout)

        self.formatter = formatter

    def parse_linked_list(self, identifier, next_field_name,
                          override_element_type=None, elf_id=None):
        """A helper function for traversing a linked-list structure.

        This is a generator and yields each element of the list. The input
        parameters are:

        * identifier - the variable at the head of the list. This can be the
        name or address of a (named) variable in firmware, or a Variable
        object (e.g. the result of a call to Analysis.cast). Note that this
        could be the first element in the list, or a pointer to the head of
        the list (e.g. '$_transform_list').

        * next_field_name - the name of the field in the structure that
        points to the next element in the list.

        * override_element_type - by default each list member is interpreted
        as the type as which the next_field_name pointer is declared.
        Optionally specify another type to cast to.

        Args:
            identifier (str): The name of the identifier.
            next_field_name (str):  Next field name.
            override_element_type (int, optional): Override element type.
            elf_id (int, optional): The elf file ID.
        """
        if isinstance(identifier, ct.Variable):
            element_ptr = identifier
        else:
            element_ptr = self.chipdata.get_var_strict(
                identifier,
                elf_id=elf_id
            )

        if element_ptr.get_member(next_field_name) is not None:
            # The user supplied the first element in the list, not
            # a pointer to it.
            # (Either that, or they supplied a pointer that happens to
            # have a member called next_field_name).
            yield element_ptr
            element_ptr = element_ptr.get_member(next_field_name)

        dm_word_width = 24
        if Arch.addr_per_word == 4:
            dm_word_width = 32

        while element_ptr.value != 0:
            # Check if the list terminated with 0xffffff(-1 in 24 bit =
            # 16777215 in 24bit) or 0xFFFFFFFF (32 bit). If so, raise and
            # error because this is not accepted anymore.
            if element_ptr.value == (math.pow(2, dm_word_width) - 1):
                raise Exception(
                    "List terminated with 0x%x" % element_ptr.value
                )
            if override_element_type is not None:
                element = self.chipdata.cast(
                    addr=element_ptr.value,
                    type_def=override_element_type,
                    deref_type=True,
                    elf_id=elf_id
                )
            else:
                if element_ptr.debuginfo is None:
                    elf_id = None
                else:
                    elf_id = element_ptr.debuginfo.elf_id

                element = self.chipdata.cast(
                    addr=element_ptr.value,
                    type_def=element_ptr.type,
                    deref_type=True,
                    elf_id=elf_id,
                )
            yield element
            for field in next_field_name.split("."):
                element = element.get_member(field)
            element_ptr = element

    def find_references(self, value):
        """Searches the whole of DM for the supplied value.

        Returns a tuple of each address that contains that value (empty
        tuple if it was not found anywhere).

        Note:
            It does not search in memory-mapped registers, only in DM1 and
            DM2.

        Args:
            value (int)
        """
        if Arch.kal_arch != 4:
            dm1_start = self.debuginfo.get_var_strict(
                '$DM1_REGION.__Base'
            ).address  # Start of DM1
            if Arch.chip_arch == "Hydra" or Arch.chip_arch == "KAS":
                dm1_limit = self.debuginfo.get_var_strict(
                    '$DM1_ZI_REGION.__Limit'
                ).address  # End of DM1
            else:
                dm1_limit = self.debuginfo.get_var_strict(
                    '$DM1_REGION.__Limit'
                ).address
            dm2_start = self.debuginfo.get_var_strict(
                '$DM2_REGION.__Base'
            ).address
            dm2_limit = self.debuginfo.get_var_strict(
                '$DM2_REGION.__Limit'
            ).address
        else:
            try:
                dm1_start = self.debuginfo.get_constant_strict(
                    '$DM1_RAM_BASE_ADDRESS'
                ).value
            except BaseException:
                dm1_start = self.debuginfo.get_constant_strict(
                    '$DATA_RAM_START_ADDRESS'
                ).value

            try:
                dm1_limit = self.debuginfo.get_constant_strict(
                    '$MEM_P0_PRESERVE_END'
                ).value
            except BaseException:
                dm1_limit = self.debuginfo.get_constant_strict(
                    '$MEM_MAP_STACK_END'
                ).value

            dm2_start = self.debuginfo.get_constant_strict(
                '$DM2_RAM_BASE_ADDRESS'
            ).value
            try:
                dm2_limit = self.debuginfo.get_constant_strict(
                    '$MEM_P1_PRESERVE_END'
                ).value
            except BaseException:
                # Old Crescendo memory map (before 2016/03/22)
                dm2_limit = dm1_limit - dm1_start + dm2_start

        refs = [
            i for i in range(dm1_start, dm1_limit)
            if self.chipdata.get_data(i) == value
        ]
        refs.extend(
            [
                i for i in range(dm2_start, dm2_limit)
                if self.chipdata.get_data(i) == value
            ]
        )
        return tuple(refs)
