############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2012 - 2018 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
"""
This module enables users to run ACAT interactively.
"""
from __future__ import print_function
import code
import inspect
import logging
import os
import sys
from collections import OrderedDict

from ACAT.Core import Arch
from ACAT.Core import CoreUtils as cu
from ACAT.Core import MulticoreSupport
from ACAT.Core.LiveSpi import LiveSpi
from ACAT.Display.table import HelpSection
from ACAT.Interpreter.Interpreter import Interpreter

if sys.version_info.major == 3:
    # Python 3
    import builtins
else:
    # Python 2
    import __builtin__ as builtins

logger = logging.getLogger(__name__)

# It looks like nothing uses this, but it needs to be imported before
# readline.parse_and_bind() is called, in order to make tab completion work.
try:
    import readline
except ImportError:
    logger.warning(("Install readline package if you want commandline "
                    "editing & history"))
    readline = None


def str_compare(str_a, str_b):
    """
    @brief Compares two string.
    @param[in] str_a First string.
    @param[in] str_b Second string.
    @param[out] Returns:
        -1 if str_a should be before str_b in the ordered list.
        1 if str_a should be after str_b in the ordered list.
        0 the wo string should be in the same position.

    This function is a simple string compare with the exception that "_" is
    counted as the last element to put private functions and variables at the
    end of the list.
    Note: The function also assumes that if the two function contains "." the
    part before the last "." could be ignored.
    """
    # ignore everything before the last "."
    str_a = str_a.split(".")[-1]
    str_b = str_b.split(".")[-1]

    # If only one of hte string contains "_" put the one with "_" as the later
    # element.
    if str_a.startswith("_") and not str_b.startswith("_"):
        return 1
    if not str_a.startswith("_") and str_b.startswith("_"):
        return -1

    # if non or both of the string contains "_" just use a simple string.
    # compair.
    if str_a == str_b:
        return 0
    if str_a > str_b:
        return 1

    return -1


def get_potential_object_names(object_dict, object_start_name):
    """
    @brief Returns the available variables of given a path (see example below).
    @param[in] object_dict Dictionary containing all the available object in
        the environment.
    @param[in] object_start_name The name of the object in the enviroment.
                This name can be a relative name for example:
                vaible1.attribute1.sub_attribute1

    Lets assume we have an environment with two variables each with a few
    attributes.
    variable1,
    variable1.attribute1
    variable1.attribute1.sub_attribute1,
    variable1.attribute1.sub_attribute2,
    variable1.attribute2,
    variable2,
    variable2.attribute1
    As we can see each attribute counts as a separate variable.

    This function will return all the potential variables with the given start
    name. For example if the given name is "" the function will search in the
    environment and finds two potential matches:
    variable1
    variable2

    On the other hand if the object_start_name is "variable1." the
    function will return the full name of the variable1 attributes which
    are:
    variable1.attribute1
    variable1.attribute2
    Therefore "variable1." is like a scope in the environment.

    Note. "variable1.blah" will return:
    variable1.attribute1
    variable1.attribute2
    because "blah" is not a valid of attribute variable1.
    """
    # the prefix will hold all
    prefix = ""
    attribute_path = object_start_name.split(".")
    for attr in attribute_path:
        # print "attr = ", attr
        if attr in list(object_dict.keys()):
            # add the attribute name to the variable
            prefix += attr + "."

            # save the local_object
            local_object = object_dict[attr]
            # now create a new environment with the attributes of the object.
            object_dict = {}
            for temp_attr in dir(local_object):
                object_dict[temp_attr] = getattr(local_object, temp_attr)
        else:
            # exit recursive search.
            break

    ret_var_list = []

    # make a list from the current available environment.
    for i in object_dict.keys():
        # get the object attribute full name
        object_attr_name = prefix + i
        # add a parenthesis to mark the attribute as a function
        if callable(object_dict[i]):
            object_attr_name += "("
        # Add the  object_attr_name to the returned list.
        ret_var_list.append(object_attr_name)

    ret_var_list.sort(cmp=str_compare)
    return ret_var_list


def string_copleter(object_dict):
    """
    @brief Creates a completer function for the readline module.
    @param[in] analyses_names List of the name of the analyses
    @param[in] analyses_dict Dictionary containing all the analyses maped
        to the name.
    """

    def completer_func(text, state):
        """
        Command completer function for the readline module.
        """
        what_to_look = get_potential_object_names(object_dict, text)

        # and now filter out all the irrelevant matches.
        options = [i for i in what_to_look if i.startswith(text)]
        if state < len(options):
            return options[state]
        return None

    return completer_func


class HistoryConsole(code.InteractiveConsole):
    """
    @brief Class used enter to a interactive shell.
    """

    def __init__(self, local_vars, formatter):
        """
        @brief Initialises the console object
        @param[in] self Pointer to the current object
        @param[in] locals Local variables.
        """
        self._formatter = formatter
        # call the parent constructor. Cannot use super because the class is
        # an old style object
        code.InteractiveConsole.__init__(self, locals=local_vars)

        # Enable auto completion if readline is installed in python.
        self.histfile = os.path.expanduser("~/.ACAT.history")
        if readline:
            # enable tab completion
            readline.parse_and_bind("tab: complete")
            readline.set_completer(
                string_copleter(object_dict=local_vars)
            )
            try:
                readline.read_history_file(self.histfile)
            except IOError:
                # Permission problems
                pass
        self.traceback = None

    def push(self, line):
        """Push a line to the interpreter.
        see code.InteractiveConsole.push for more.
        """
        if self._formatter.log_file:
            self._formatter.output('>>> {}'.format(line))

        # Save the history every time because there seems to be no other way
        # to ensure that history is kept when the session is Ctrl+C'd.
        if readline:
            try:
                readline.write_history_file(self.histfile)
            except IOError:
                # Permission problems
                pass

        # Give the line to the Python interpreter
        return code.InteractiveConsole.push(self, line)


#########################################


class Interactive(Interpreter):
    """
    @brief Class which encapsulates an interactive session.
    Kick things off by calling run_all().

    @param[in] p0 A Processor instance
    @param[in] p1 A Processor instance
    @param[in] analyses A list of requested analyses. If this
               option is not provided all the default analyses
               will be performed.
    """

    def __init__(self, p0=None, p1=None, analyses=None):
        super(Interactive, self).__init__(p0=p0, p1=p1, analyses=analyses)

        # save Arch for matlab
        self.arch = Arch

        self._user_namespace = {}

    def mem_print(self, address, words, processor=0, words_per_line=8):
        """
        Get a slice of DM, starting from address and prints out the
        result in a nicely formatted manner.

        @param[in] address Address to start from.
        @param[in] words Number of words to read from the memory.
        @param[in] processor Processor to use. Default is processor 0.
        @param[in] words_per_line Number of words to print out. Default value
                                 is 8 words.
        """
        if processor == 0:
            content = self.p0.chipdata.get_data(
                address, words * Arch.addr_per_word
            )
        else:
            content = self.p1.chipdata.get_data(
                address, words * Arch.addr_per_word
            )

        # convert the memory content to an ordered dictionary
        mem_content = OrderedDict()
        for offset, value in enumerate(content):
            current_address = address + offset * Arch.addr_per_word
            mem_content[current_address] = value

        self.formatter.output(
            cu.mem_dict_to_string(
                mem_content,
                words_per_line))

    def load_bundle(self, bundle_path):
        """
        @brief Loads a bundle (also known as KDCs) and sets it for
        both processors.
        @param[in] bundle_path Path to the bundle name.
        """
        bundle = MulticoreSupport.load_bundle(bundle_path)
        self.p0.debuginfo.update_bundles(bundle)
        if cu.global_options.is_dualcore:
            self.p1.debuginfo.update_bundles(bundle)

    def reconnect(self):
        """
        Reconnects to all live processors and reload debug info(s)

        @param[in] self Pointer to the current object
        """
        try:
            self.p0.chipdata.reconnect()
            self.p0.debuginfo.reload()
        except AttributeError:
            pass
        try:
            self.p1.chipdata.reconnect()
            self.p1.debuginfo.reload()
        except AttributeError:
            pass

    def run(self):
        """
        @brief Reads and run instruction from the command line.
        @param[in] self Pointer to the current object
        """
        self._populate_namespace()

        if cu.global_options.use_ipython:
            try:
                import IPython

                IPython.start_ipython(
                    argv=[], user_ns=self._user_namespace,
                    display_banner=False,
                    check_same_thread=False
                )
                return

            except ImportError:
                logger.warning("IPython not installed. -I option ignored!")

        history_console = HistoryConsole(
            local_vars=self._user_namespace,
            formatter=self.formatter
        )
        # enter to interactive mode
        history_console.interact("Interactive mode - type help() for help.")

    def help(self, arg=None):
        """
        Prints the help text

        This Overloads the built-in help function.
        help() prints the help text for Interactive mode;
        help(foo) calls the standard help routine for thing 'foo'.
        @param[in] self Pointer to the current object
        @param[in] arg = None
        """
        if len(self._user_namespace) == 0:
            self._populate_namespace()

        if arg is not None:
            return builtins.help(arg)

        else:
            self._print_help()

    def _called_externally(self):
        """
        Check whether the instance is called externally

        KATS and KSE are two major shells which are importing the ACAT and
        then call its functions. This is to make sure help blurb shows the
        right commands in its output.
        """
        current_frame = inspect.currentframe()
        first_frame = inspect.getouterframes(current_frame)[-1]

        acat_executables = ('acat.py', 'acat.exe', 'acat', 'acat-script.py')
        if isinstance(first_frame, tuple):
            # In Python2 inspect's `getouterframes` returns tuple instead of
            # FrameInfo instance. Something similar to:
            # (
            #     <frame object at 0x04B322E0>,
            #     '.\\ACAT.py',
            #     24,
            #     ...
            # )
            return not any(
                [
                    str(item).lower().endswith(acat_executables)
                    for item in first_frame
                ]
            )

        # It's Python3
        return not first_frame.filename.lower().endswith(acat_executables)

    def _print_help(self):
        # Generating all the sections of the Interactive Help
        if self._called_externally():
            command_template = 'acat.p0.{}:'

        else:
            command_template = '{}:'

        title_section = HelpSection(formatter=self.formatter)
        title_section.title = "Audio Coredump Analysis Tool - Interactive mode"
        title_section.description = (
            "For each command, you can use help([command]) to get more "
            "information and its usage. i.e. help(search)"
        )
        title_section.get_help(verbose=True)

        builtin_commands_section = HelpSection(formatter=self.formatter)
        builtin_commands_section.title = "Built-in commands"
        builtin_commands = (
            'help',
            'to_file',
            'get',
            'search',
            'mem_print',
            'kalaccess'
        )
        interpreter_commands = ('help', 'to_file')

        for command in builtin_commands:
            implementation = self._user_namespace.get(command)
            if implementation is None:
                # Built-in commands like kalaccess aren't always
                # available.
                continue

            if self._called_externally() and command in interpreter_commands:
                # Interactive commands don't need `acat.p0` prefix.
                command = 'acat.{}:'.format(command)

            else:
                command = command_template.format(command)

            builtin_commands_section.add_item(
                command,
                cu.get_title(implementation)
            )
        builtin_commands_section.get_help(verbose=True)
        self.formatter.output('')

        builtin_analysis_section = HelpSection(
            command_width=25,
            formatter=self.formatter
        )
        builtin_analysis_section.title = "Built-in analysis is available via:"
        for analysis in self.analyses:
            implementation = self._user_namespace.get(analysis.lower())
            builtin_analysis_section.add_item(
                command_template.format(analysis.lower()),
                cu.get_title(implementation)
            )
        builtin_analysis_section.get_help(verbose=True)
        self.formatter.output('')

        other_method_section = HelpSection(formatter=self.formatter)
        other_method_section.title = "Other methods are available via:"
        for method in ('chipdata', 'debuginfo'):
            implementation = self._user_namespace.get(method)
            other_method_section.add_item(
                command_template.format(method),
                cu.get_title(implementation)
            )
        other_method_section.get_help(verbose=True)
        self.formatter.output('')

        examples_section = HelpSection(
            command_width=40,
            formatter=self.formatter
        )
        examples_section.title = "Examples"
        examples = (
            (
                "get('audio_slt_table')",
                "Get the variable named 'audio_slt_table'"
            ), (
                "get('mm_doloop_start')",
                "Get the register named 'MM_DOLOOP_START'"
            ), (
                "chipdata.get_data(0xd00, 30)",
                "Get a slice of DM, starting from address "
                "0xd00 and 30 addressable units. Alternatively "
                "use mem_print."
            ), (
                "get(0x406c10)",
                "Get the code entry at address 0x406c10"
            )
        )
        for example, description in examples:
            examples_section.add_item(
                command_template.format(example),
                description
            )
        examples_section.get_help(verbose=True)

    def _populate_namespace(self):
        """
        Populate the interpreter's user namespace
        """
        # add the created analyses to the local variables. Use the processor
        # wanted by the user as the default.
        proc_name = "p" + str(cu.global_options.processor)
        processor = self.__getattribute__(proc_name)

        # add the current processors analyses.
        self._user_namespace.update(processor.available_analyses)

        if cu.global_options.is_dualcore:
            self._user_namespace["p0"] = self.p0
            self._user_namespace["p1"] = self.p1

        # add chipdata, debuginfo and formatter to the local variables
        self._user_namespace["chipdata"] = processor.chipdata
        self._user_namespace["debuginfo"] = processor.debuginfo

        # Built-in functions.
        # We used to define 'ihelp' to avoid masking Python's built-in help
        # function. We don't need it any more, but keep it for now to avoid
        # confusing users.
        if processor.kalaccess and processor.kalaccess.is_connected():
            self._user_namespace["kalaccess"] = processor.kalaccess
        self._user_namespace["search"] = processor.search
        self._user_namespace["get"] = processor.get
        self._user_namespace["get_var"] = processor.get_var
        self._user_namespace["get_reg"] = processor.get_reg
        self._user_namespace["load_bundle"] = self.load_bundle

        # Make the default kymera elf id easily accessible
        debug_info = processor.debuginfo.get_kymera_debuginfo()
        self._user_namespace["kymera_debuginfo"] = debug_info
        self._user_namespace["kymera_elf_id"] = debug_info.elf_id

        self._user_namespace["ihelp"] = self.help
        self._user_namespace["help"] = self.help
        self._user_namespace["mem_print"] = self.mem_print
        self._user_namespace["to_file"] = self.to_file

        # reveal some useful conversion functions
        from ..Core.CoreUtils import u32_to_s32
        from ..Core.CoreUtils import s32_to_frac32
        from ..Core.CoreUtils import u32_to_frac32
        self._user_namespace["u32_to_s32"] = u32_to_s32
        self._user_namespace["s32_to_frac32"] = s32_to_frac32
        self._user_namespace["u32_to_frac32"] = u32_to_frac32

        # CoreUtils have some useful functions which a user may want.
        self._user_namespace["cu"] = cu

        # Only live chip can reconnect.
        if isinstance(processor.chipdata, LiveSpi):
            # Utility function to reconnect.
            self._user_namespace["reconnect"] = self.reconnect

        # We need to perform the ID check before people start typing in
        # commands and getting confusing answers.
        processor.sanitycheck.analyse_firmware_id()
##################################################
