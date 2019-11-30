############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2018 - 2019 Qualcomm Technologies, Inc. and/or its
# subsidiaries.  All rights reserved.
#
############################################################################
"""
Parse the ACAT arguments

See http://wiki.europe.root.pri/Kymera/Using_ACAT for more information on
how to use ACAT.
"""
import argparse
import logging
import os

from .._version import __version__ as version

logger = logging.getLogger(__name__)

DESCRIPTION = """
Audio Coredump Analysis Tool, v{}
""".format(version)
EPILOG = """multiline|

Example usage:

python ACAT.py -t C:\qtil\ADK_XY\\tools\pythontools
        -b C:\qtil\ADK_XY\audio\kalimba\kymera\output\<conf>\build\debugbin\kymera_<chip>_audio
        -c coredump_file.xcd -d -i
python ACAT.py -t C:\qtil\ADK_XY\\tools\pythontools
        -b C:\qtil\ADK_XY\audio\kalimba\kymera\output\<conf>\build\debugbin\kymera_<chip>_audio
        -s "trb/scar/0" -p 0 -w HTML_output.html
"""


class ACATArgumentParserError(Exception):
    """When something goes wrong with argument parser."""


class MultilineFormatter(argparse.HelpFormatter):
    """Help Formatter that supports multi line help and epilog."""
    multiline_tag = "multiline|"

    def _split_lines(self, text, width):
        """Overrides the original `_split_lines` method.

        Checks for the multiline tag, if present changes the output and
        generate a more readable text with multi lines.

        Args:
            text (str): An arbitrary string.
            width (int): An integer.

        Returns:
            A list of strings.
        """
        if text.startswith(self.multiline_tag):
            # It's a multiline text
            text = text[len(self.multiline_tag):]  # Remove the tag

            output = list()
            lines = text.splitlines()
            for line in lines:
                if len(line):
                    output.extend(
                        super(MultilineFormatter, self)._split_lines(
                            line,
                            width
                        )
                    )

                else:
                    # Handle the empty new line
                    output.append('')

            return output
        else:
            return super(MultilineFormatter, self)._split_lines(text, width)

    def add_text(self, text):
        """Overrides the original add_text to handle the multiline tag.

        Args:
            text (str): An arbitrary string.
        """
        if text is not None and text.startswith(self.multiline_tag):
            # It's a multiline text
            text = text[len(self.multiline_tag):]  # Remove the tag
            self._add_item(lambda x: x, [text])

        else:
            super(MultilineFormatter, self).add_text(text)


class DefaultEnvAction(argparse.Action):
    """Environment Variable default value action.

    An Argument Parser action to handle the case when the argument can be
    read from an environment variable

    Args:
        env_var(str): An environment variable name that can be read from
                      the operating system.
        required(boolean): If the argument is required.
        default(str): Default value for the argument.
    """

    def __init__(self, env_var, required=True, default=None, **kwargs):
        env_var_value = os.environ.get(env_var)

        if env_var_value is not None:
            required = False
            default = env_var_value

        super(DefaultEnvAction, self).__init__(
            default=default,
            required=required,
            **kwargs
        )

    def __call__(self, parser, namespace, values, option_string=None):
        setattr(namespace, self.dest, values)


class AcatArgumentParser(object):
    """
    Parses the ACAT arguments

    Args:
        arguments
    """

    def __init__(self, arguments=None):
        self._arguments = arguments

        self._parser = argparse.ArgumentParser(
            formatter_class=MultilineFormatter,
            description=DESCRIPTION,
            epilog=EPILOG
        )
        self._ns = None
        self._kalcmd2 = None

        self._extract_kalcmd2()

    @property
    def namespace(self):
        """Reads only property to access the namespace of parsed arguments."""
        if self._ns is not None:
            return vars(self._ns)

        else:
            raise ACATArgumentParserError("No argument is parsed")

    def populate_global_options(self, global_options):
        """Populates the given GlobalOptions instance.

        Args:
            global_options (:obj:`GlobalOptions'): A GlobalOptions instance.
        """
        self._parse()
        self._check()

        if self._ns.coredump:
            global_options.coredump_path = self._ns.coredump
            # Increase the cache validity internal because all the data is
            # static. 10 minutes will be probably enough to hold the data
            # for the whole automatic run.
            global_options.cache_validity_interval = 600.0

        is_interactive = self._ns.interactive or self._ns.ipython

        global_options.build_mismatch_allowed = self._ns.build_mismatch
        global_options.build_output_path_p0 = self._ns.p0_build_output
        global_options.build_output_path_p1 = self._ns.p1_build_output
        global_options.bundle_paths.extend(self._ns.downloadable_capabilities)
        global_options.dependency_check = self._ns.elf_check
        global_options.html_path = os.path.normcase(self._ns.write_html)
        global_options.interactive = is_interactive
        global_options.is_dualcore = self._ns.dual_core
        global_options.kalcmd_object = self._kalcmd2
        global_options.live = self._ns.live
        global_options.patch = self._ns.patch
        global_options.processor = self._ns.processor
        global_options.pythontools_path = os.path.abspath(self._ns.py_tools)
        if self._ns.transport is not None:
            global_options.spi_trans = self._ns.transport
        global_options.use_ipython = self._ns.ipython
        global_options.verbose = self._ns.verbose
        global_options.wait_for_proc_to_start = self._ns.boot_wait
        global_options.under_test = self._ns.under_test

    def _parse(self):
        """Parses the given arguments and do the sanity check on them."""
        self._parser.add_argument(
            '-t',
            '--py-tools',
            action=DefaultEnvAction,
            env_var='ACAT_PYTOOLS',
            help="Path to the Python Tools (default $ACAT_PYTOOLS).",
            dest='py_tools',
            type=str,
        )

        self._parser.add_argument(
            '-c',
            '--coredump',
            help=(
                "Path to coredump file to analyse. The coredump can be "
                "compressed as zip file as well as xcd file."
            ),
            dest='coredump',
            default='',
            type=str,
        )

        self._parser.add_argument(
            '-b',
            '--build',
            help=(
                "Path to build output file. It is not required to specify if "
                "the tool is used internally on a released build."
            ),
            dest='p0_build_output',
            default='',
            type=str,
        )

        self._parser.add_argument(
            '-j',
            help=(
                "Path to downloaded capabilities. Put them one after another "
                "when there are more than one."
            ),
            dest='downloadable_capabilities',
            default=[],
            action='append',
        )

        self._parser.add_argument(
            '-l',
            help=(
                "Path to build output for P1. This option will automatically "
                "enables dual core mode. See '-d' option for more."
            ),
            dest='p1_build_output',
            default='',
            type=str
        )

        self._parser.add_argument(
            '-s',
            '--transport',
            help=(
                "multiline|"
                "Allows ACAT to run on a 'live' chip rather than a coredump. "
                "The transport parameter is a standard kalaccess parameter "
                "string that should contain the transport required (e.g. "
                "'usb', 'kalsim' and 'trb') plust a number of other "
                "parameters such as SPIPORT, or SPIMUL.\n\n"
                "The value for transport should be relevant to the protocol "
                "that it uses. Please see the following examples:\n\n"
                "trb --> trb/scar/0\n"
                "kalsim --> kalsim localhost:31400\n"
                "usb --> usb SPIPORT=247264"
            ),
            dest='transport',
            type=str
        )

        self._parser.add_argument(
            '-p',
            '--processor',
            help=(
                "Specify which Kalimba processor (0, 1, ...) to debug"
            ),
            dest='processor',
            default=0,
            type=int
        )

        self._parser.add_argument(
            '-d',
            '--dual-core',
            help=(
                "Required if dualcore and if both cores are using the same "
                "build output. In Interactive mode, to select the processor "
                "that the command is to be run on, it has to be called with "
                "the name of the processor as the instance e.g. p0.<command>, "
                "p1.<command>."
            ),
            dest='dual_core',
            action='store_true'
        )

        self._parser.add_argument(
            '-i',
            '--interactive',
            help=(
                "Run in Interactive mode. Interactive mode accepts individual "
                "commands and will query for missing information."
            ),
            dest='interactive',
            action='store_true'
        )

        self._parser.add_argument(
            '-I',
            '--ipython',
            help=(
                "Same as -i, but using the IPython interpreter."
            ),
            dest='ipython',
            action='store_true'
        )

        self._parser.add_argument(
            '-w',
            '--write-html',
            help=(
                "Filepath (inc. filename) where the html file will be created "
                "containing the results of the automatic analyse. (not "
                "compatible with interactive mode"
            ),
            dest='write_html',
            default='',
            type=str
        )

        self._parser.add_argument(
            '-v',
            '--verbose',
            help=(
                "When set, extra logging information will appear on the "
                "screen and log files will be created in ACAT's package "
                "directory."
            ),
            dest='verbose',
            action='store_true'
        )

        self._parser.add_argument(
            '-q',
            help="Wait until the chip is booted",
            dest='boot_wait',
            action='store_true'
        )

        self._parser.add_argument(
            '-m',
            '--build-mismatch',
            help=argparse.SUPPRESS,
            dest='build_mismatch',
            action='store_true'
        )

        self._parser.add_argument(
            '--elf_check',
            help=argparse.SUPPRESS,
            dest='elf_check',
            action='store_true'
        )

        self._parser.add_argument(
            '--patch',
            help=argparse.SUPPRESS,
            dest='patch',
            type=str
        )

        # When the `--under-test` flag is set, ACTA will enter the test
        # mode which makes the following changes in the behaviour:
        #
        #     - Random IDs will be removed from the matplotlib svg output.
        #       Note: This can cause an erroneous output which cannot be
        #       displayed in a browser.  The reason to do this is to make
        #       test reproduceable.
        #
        #     - Graphviz graphs will be displayed as test. The only way to
        #       test the Graphviz module is to display the graph as a text
        #       because Graphviz svg contains different random IDs with
        #       shuffled order.
        #
        #     - Disables error stack printing for all analyses, because
        #       the relative path to a file is machine dependent.
        self._parser.add_argument(
            '--under-test',
            help=argparse.SUPPRESS,
            dest='under_test',
            action='store_true'
        )

        self._parser.add_argument(
            '--version',
            action='version',
            version='Audio Coredump Analysis Tool, v{}'.format(version)
        )

        self._ns = self._parser.parse_args(self._arguments)

        # Populate the missing namespaces based on some internal logics
        if self._ns.transport or self._kalcmd2:
            self._ns.live = True
        else:
            self._ns.live = False

        # If the Dual Core option is not selected but both p0_build_output and
        # p1_build_output is specified, then it's a Dual Core option anyway.
        if self._ns.p0_build_output and self._ns.p1_build_output:
            self._ns.dual_core = True

        # If it's Dual Core but `p1_build_output` is not specified, the build
        # for it will be exactly the same as p0_build_output.
        if self._ns.dual_core and self._ns.p1_build_output == '':
            self._ns.p1_build_output = self._ns.p0_build_output

    def _check(self):
        """Does the post parse check, main focus on dependencies."""
        is_interactive = self._ns.interactive or self._ns.ipython
        if is_interactive and self._ns.write_html:
            self._parser.error(
                "Write HTML is not supported in interactive mode. This option "
                "can be used only in Automatic mode."
            )

        if self._ns.p1_build_output and self._kalcmd2:
            self._parser.error("Kalcmd does not support dual core")

        if self._ns.dual_core and self._kalcmd2:
            self._parser.error("Kalcmd does not support dual core")

        if self._ns.live is False and self._ns.coredump == '':
            self._parser.error("ERROR: Path to coredump file not supplied")

        if not os.path.exists(self._ns.py_tools):
            self._parser.error(
                "ERROR: Path to Pythontools {} invalid or not supplied".format(
                    self._ns.py_tools
                )
            )

    def _extract_kalcmd2(self):
        """Removes the kalcmd2.

        Extracts the kalcmd2 instance if available and assign it to an
        instance variable.

        kalcmd2 is not a command line argument, however external apis,
        like kats pass the instance by calling the parser.
        """
        if self._arguments:
            # Arguments may not be command line options, so search for kalcmd2
            # object and set the values appropriately.
            try:
                kalcmd2_index = self._arguments.index('-a')
            except ValueError:
                # Kalcmd2 not available in the arguments
                return

            # pop the `-a`
            self._arguments.pop(kalcmd2_index)
            # pop the the kalcmd2 instance
            self._kalcmd2 = self._arguments.pop(kalcmd2_index)
