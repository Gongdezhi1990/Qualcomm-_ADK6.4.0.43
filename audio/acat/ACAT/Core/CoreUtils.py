############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2012 - 2019 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
"""
Module holding all the helper functions and classes in ACAT.
"""
import imp
import importlib
import io as si
import logging
import os
import subprocess
import sys
from collections import OrderedDict
from contextlib import contextmanager

from ACAT.Core import Arch
from ACAT.Core import compression
from ACAT.Core.constants import IS_PY2

##########################################################################
#                               Globals
##########################################################################
logger = logging.getLogger(__name__)


def dict_to_ordered_dict(dictionary):
    """Converts an instance of built in Dict object to an ordered dictionary.

    The result dictionary is ordered by keys.

    Args:
        dictionary (dict): A dictionary.

    Return:
        An OrderedDict instance.
    """
    list_key_value = []
    for key, value in dictionary.items():
        list_key_value.append((key, value))

    # Order by keys and return
    return OrderedDict(sorted(list_key_value, key=lambda x: x[0]))


def get_title(callable_obj):
    """Parse the given callable's docstring and extract the title.

    By convention, the first line of every docstring is the title. This
    method will parse that and extract it from the whole docstring.

    Args:
        callable_obj: Any callable object with valid docstring.

    Returns:
        A string if there is a title, None otherwise.
    """
    if callable_obj.__doc__ is None:
        return None

    title_lines = []
    lines = callable_obj.__doc__.strip().split('\n')
    for line in lines:
        if len(line.strip()):
            title_lines.append(line.strip())

        else:
            break

    return ' '.join(title_lines)


# Command-line options are all stored globally, since some other modules
# will want to use them (and it's not good to pass them through long chains
# of constructors).
class GlobalOptions(object):
    """Encapsulates all of the options supplied on the command-line.

    No members are defined here; they're part of the top-level script
    (e.g. ACAT.py).
    """

    def __init__(self):
        self.options = {
            'build_output_path_p0': "",
            'kymera_p0': None,
            'build_output_path_p1': "",
            'kymera_p1': None,
            'bundle_paths': [],
            'bundles': None,
            'patch': None,
            # This flag is only used when ACAT is launched from QDME. When this
            # flag is set ACAT will wait until the chip is booted.
            'wait_for_proc_to_start': False,

            'coredump_path': "",
            'coredump_lines': None,
            'html_path': "",
            'pythontools_path': "",  # path to the pythontools
            'interactive': False,
            'use_ipython': False,
            # We are analysing a live chip not a coredump (i.e. over SPI)
            'live': False,
            'spi_trans': "lpt",
            'processor': 0,  # Default to p0
            'build_mismatch_allowed': False,
            'is_dualcore': False,
            'kalcmd_object': None,
            'ker': None,
            'kmem': None,  # kalmemaccessors
            'kal': None,
            'kal2': None,
            'dependency_check': False,
            'cache_validity_interval': 3.0,
            # test related globals.
            'under_test': False,
            'verbose': False,
        }
        self.set_defaults()

    def set_defaults(self):
        """Set default value all the options."""
        for option_name, default_value in self.options.items():
            setattr(self, option_name, default_value)


# this is not a constant hence the lower case characters
global_options = GlobalOptions()  # pylint: disable=invalid-name


##########################################################################
#                      Internal Heper Functions
##########################################################################
def create_counter(initial):
    """Increments the counter if it exists otherwise create a counter.

    Args:
        initial: Value of the counter which has been hard coded to 0.

    Returns:
        Incremented output parameter.
    """

    def counter():
        """Returns incremental numbers."""
        retval = counter.i
        counter.i += 1
        return retval

    counter.i = initial
    return counter


# initialise the create_counter function and use get_non_rep_nuber where
# required to get next index
get_non_rep_nuber = create_counter(initial=0)  # pylint: disable=invalid-name


def reset_package():
    """Resets every global ACAT variable."""
    global global_options, get_non_rep_nuber
    global_options = GlobalOptions()
    get_non_rep_nuber = create_counter(initial=0)
    Arch.chip_clear()


def run_plotter(pt_input):
    """Runs matplotlib plotter."""
    matplotlib = MatplotlibImporter()
    plt = matplotlib.pyplot

    if plt is None:
        return "matplotlib.pyplot not available"
    title_string = pt_input["title_string"]
    conversion_function = pt_input["conversion_function"]
    buffer_dict = pt_input["buffer_dict"]
    output_format = pt_input["output_format"]

    with change_matplotlib_log_level(logging.ERROR):
        # Without disabling the matplotlib warnings it keeps complaining about
        # the lack of icon for the figure. It's a known issue with matplotlib
        # dealing with tkinter GUI. Still we get error and exception messages
        # anyway.
        fig = plt.figure(figsize=(10, 5))

    subplot_axes = fig.add_subplot(111)

    # use tight_layout to save space, get rid of the white borders
    # plt.tight_layout()
    plt.grid(alpha=0.2)
    markers = []
    converted_values = OrderedDict()
    for key in buffer_dict:
        if isinstance(key, str):
            # for markers the value is the address
            marker_address = buffer_dict[key]
            marker_name = key
            markers.append([marker_name, marker_address])
        # make the conversion if needed
        if conversion_function:
            converted_values[key] = conversion_function(buffer_dict[key])

    # if no conversion was done the converted values will be the same as
    # the one returned form _get_content
    if not conversion_function:
        converted_values = buffer_dict
    # remove the markers to avoid failures when plotting the content
    for marker_name, marker_address in markers:
        converted_values.pop(marker_name)

    subplot_axes.plot(
        list(converted_values.keys()),
        list(converted_values.values()),
        label="waveform"
    )
    bottom, top = subplot_axes.get_ylim()
    for marker_name, marker_address in markers:
        # read and write pointer marker should be distinguished from other
        # markers
        if marker_name == "read pointer":
            # config for the vertical line
            line_address = marker_address + 1
            linestyle = 0, (1, 1)  # dotted 1-dot, 3-space
            # text config
            alignment = 'left'
            verticalalignment = 'top'
            color = 'green'
            y_location = top
        elif marker_name == "write pointer":  # config for the vertical line
            line_address = marker_address - 1
            linestyle = 0, (1, 2)  # dotted 1-dot, 3-space
            # text config
            alignment = 'right'
            color = 'red'
            verticalalignment = 'bottom'
            y_location = bottom
        else:
            # Default mark up configuration.
            # config for the vertical line
            line_address = marker_address
            linestyle = 0, (1, 3)  # dotted 1-dot , 3-space
            # text config
            alignment = 'right'
            color = 'orange'
            verticalalignment = 'top'
            y_location = top
        # add some space at both two sides.
        marker_name = "  " + marker_name + "  "
        subplot_axes.axvline(
            x=line_address,
            color=color,
            linestyle=linestyle
        )
        # display the marker
        subplot_axes.text(
            marker_address, y_location, marker_name,
            style='normal',  # style must be normal, italic or oblique
            ha=alignment,
            va=verticalalignment,
            bbox={'facecolor': color, 'alpha': 0.2, 'pad': 0}
        )

    axes = plt.gca()
    # provide a function which displays the addresses
    axes.get_xaxis().set_major_formatter(
        matplotlib.ticker.FuncFormatter(
            lambda x, _: " 0x%08x " % int(x)
        )
    )
    plt.legend(loc='lower right')
    if title_string:
        plt.title("Buffer content at {}".format(title_string))
    plt.xlabel("Addresses")
    plt.ylabel("Values in " + conversion_function.__name__)
    if output_format == "svg":
        imgdata = si.StringIO()
        plt.savefig(imgdata, format='svg')
        plt.close(fig)
        imgdata.seek(0)
        svg_dta = imgdata.getvalue().encode(errors="ignore")
        # closet the figure
        return svg_dta

    elif output_format == "window":
        plt.show()
        # closet the figure
        plt.close(fig)
        return None

    elif output_format == "bytes_handler":
        imgdata = si.BytesIO()
        plt.savefig(imgdata, format='png', bbox_inches='tight', pad_inches=0)
        plt.close(fig)
        # Caller expects to read the data from the beginning.
        imgdata.seek(0)
        return imgdata


def strip_elf_addr(addr, kal_arch):
    """Strip an address.

    Addresses stored in the elf file have 32 bits, while the processor
    sees 24 bits. Often, the 'invisible' top 8 bits are used by the
    toolchain to store some flags. For example, code address 0x400000 is
    stored in the elf as 0x81400000, and const address 0xa26000 is stored
    as 0x01a26000.  This function strips off the 'invisible' bits of an
    elf-derived address.

    Kalarch4: for PM addresses the 31 bit is set (and the 30th bit is
    always unset) in the elf file, therefore the 31 bit is stripped.

    Args:
        addr
        kal_arch
    """
    if kal_arch == 4 and (addr >> 31) & 0x1 and not (addr >> 30) & 0x1:
        return addr & 0x7FFFFFFF

    return addr & 0x00FFFFFF


def swap_endianness(num, number_width):
    """Converts an int/long of arbitrary length to bytes reversed.

    In other words it does a endianness swap.

    Examples:
        swap_endianness(0x1234, 2) -> 0x3412
        swap_endianness(0x1234, 4) -> 0x34120000
        swap_endianness(0xaaa, 2) -> 0xaa0a

    Args:
        num: The number
        number_width: The width of the number in bytes.

    Returns:
        The endiannes swapped number.
    """
    if number_width == 1:
        return num & 0x000000FF
    elif number_width == 2:
        return (
            ((num << 8) & 0xFF00) |
            ((num >> 8) & 0x00FF)
        )
    elif number_width == 3:
        return (
            ((num << 16) & 0x00FF0000) |
            ((num >> 0) & 0x0000FF00) |
            ((num >> 16) & 0x000000FF)
        )
    elif number_width == 4:
        return (
            ((num << 24) & 0xFF000000) |
            ((num << 8) & 0x00FF0000) |
            ((num >> 8) & 0x0000FF00) |
            ((num >> 24) & 0x000000FF)
        )
    else:
        raise TypeError(
            "Cannot change endianness for a variable "
            "with length %d bytes." % (number_width)
        )


def inspect_bitfield(value, parity, size_bits=24):
    """Inspects a bitfield.

    Takes an int representing a bitfield, and returns a tuple containing:

    * parity = True: a list of which bits are set.
    * parity = False: a list of which bits are not set (requires 'size_bits'
      to be set).

    In both cases, an empty list can be returned.
    Examples:
      inspect_bitfield(0xf01, True) -> (0, 8, 9, 10, 11)
      inspect_bitfield(0xf01, False, 12) -> (1, 2, 3, 4, 5, 6, 7)
      inspect_bitfield(0, True) -> ()

    Args:
        value
        parity
        size_bits (int, optional)
    """
    result = []
    for i in range(size_bits):
        if parity:
            if value & (1 << i):
                result.append(i)
        else:
            if not value & (1 << i):
                result.append(i)
    return tuple(result)


def get_correct_addr(address, addr_per_word):
    """Fixes the address and returns it.

    For Crescendo, sometimes the address that is supplied does not start
    at the beginning of the word. Kalelfreader only returns values in
    words, therefore this function returns the value that must be
    subtracted from the address to get to the start of the word.

    Args:
        address
        addr_per_word
    """
    if addr_per_word == 4:
        if address % addr_per_word == 0:
            return address

        value = address % addr_per_word
        return address - value

    return address


def convert_byte_len_word(length, addr_per_word):
    """Adjusts the length value.

    For Crescendo, if the value of a length is not divisible with the
    number of addr per word, its value must be adjusted so it will be
    divisible with the number of addr per word, in order for kalelfreader
    to return the corresponding number of words.

    Args:
        length
        addr_per_word
    """
    div = length % addr_per_word
    if div == 0:
        return length

    return length + addr_per_word - div


def u32_to_s32(uint32):
    """Converts an unsigned 32 bit integer to a signed 32 bit integer.

    Args:
        uint32: Unsigned 32 bit integer.

    Returns:
        Signed 32 bit integer.
    """
    if uint32 >= (1 << 32):
        raise Exception(
            "The value is bigger than a 32 bit unsigned integer"
        )
    if uint32 < 0:
        raise Exception("The values is a signed integer not an unsigned")
    # make the conversion and return the value.
    return uint32 - (uint32 >> 31) * (1 << 32)


def s32_to_frac32(int32):
    """Converts signed 32 bit integer to signed 32 bit fractional value.

    Args:
        int32 Signed 32 bit integer.

    Returns:
        Signed 32 bit fractional value.
    """
    if int32 > ((1 << 31) - 1):
        raise Exception(
            "The value is bigger than a 32 bit signed integer"
        )
    if int32 < (-(1 << 31)):
        raise Exception(
            "The value is smaller than a 32 bit signed integer"
        )
    # make the conversion and return the value.
    return float(int32) / ((1 << 31) - 1)


def u32_to_frac32(uint32):
    """Converts unsigned 32 bit integer to signed 32 bit fractional value.

    Args:
         uint32 Unsigned 32 bit integer.

    Returns:
        Signed 32 bit fractional value.
    """
    return s32_to_frac32(u32_to_s32(uint32))


def get_string_from_word(no_of_chr, word):
    """Converts number of characters from a word.

    It takes the number of characters that one wants to retrieve from a
    word and the word as an argument and turns it into a string.

    Args:
        no_of_chr
        word
    """
    string = ""
    for _ in range(no_of_chr):
        string += chr(word & 0x000000FF)
        word = word >> 8

    return string


def uint32_array_to_uint16(unint32_array):
    """Unpacks a 32bit array to an array of 16bit.

    The values of the new 16 bit array will be calculated using the most
    significant 16 bit and the least significant 16 bit of the 32 bit
    value.

    Args:
        unint32_array

    Returns:
        unint16_array
    """
    unint16_array = []
    for value in unint32_array:
        unint16_array.append(value & 0xffff)
        unint16_array.append((value >> 16) & 0xffff)

    return unint16_array


def uint32_array_to_uint8(unint32_array):
    """Unpacks a 32bit array to an array of 8bit.

    The values of the new 8 bit array will be calculated considering
    little-endian memory layout.

    Args:
        unint32_array

    Returns:
        unint8_array
    """
    unint8_array = []
    for value in unint32_array:
        unint8_array.append(value & 0xff)
        unint8_array.append((value >> 8) & 0xff)
        unint8_array.append((value >> 16) & 0xff)
        unint8_array.append((value >> 24) & 0xff)

    return unint8_array


def add_indentation(input_str, nr_of_spaces):
    """Adds identation before each line in a string.

    Args:
        nr_of_spaces Spaces before each line.

    Returns:
        The formatted string.
    """
    spaces = " "*nr_of_spaces
    # add indentation for a nicer view.
    return "".join(
        spaces + line + "\n" for line in input_str.split("\n") if line != ""
    )


def mem_dict_to_string(mem_dict, words_per_line=8, compact_mode=True):
    """Converts ordered dictionary of memory addresses and values into string.

    Args:
        mem_dict (OrderedDict): Memory addresses and values.
        words_per_line (int, optional): Number of words to print out.
            Default value is 8 words.
        compact_mode (bool, optional): If true, removes repeating lines in
            the middle. For example:

                line1: Something 1                line1: Something 1
                line2: Something 2                line2: Something 2
                line3: Something 2                line3: *
                line4: Something 2  >conversion>  line4: Something 2
                line5: Something 2                line5: Something 3
                line6: Something 3
    """
    # Convert the buffer content to human readable addresses and values
    output_str = ""
    count = 0
    display_address = False
    for key in mem_dict:
        if isinstance(key, str):
            # String keys are reserved to mark downs
            output_str += "\n--- " + key + " at 0x%08x " % (mem_dict[key])

            # After, displaying the information resume displaying the
            # buffer content.
            display_address = True
        else:
            # display the starting address for each line.
            if count % words_per_line == 0 or display_address:
                display_address = False
                # display the new starting address
                output_str += "\n0x%08x: " % (key)
                # keep the alignment in the same way
                output_str += " " * 9 * (count % words_per_line)

            # format the value
            output_str += "%08x " % (mem_dict[key])
            count += 1

    if not compact_mode:
        return output_str

    # remove same lines to save some space at the output.
    output_str_no_duplicate = ""
    heading_length = 11
    previous_line = " " * heading_length
    same_line_count = 0
    for line in output_str.split("\n"):
        if line[heading_length:] == previous_line[heading_length:]:
            same_line_count += 1
        else:
            # display multiple copies of the same line as a * only if more
            # than 3 lines are the same.
            if same_line_count > 1:
                output_str_no_duplicate += "*\n"
            if same_line_count > 0:
                output_str_no_duplicate += previous_line + "\n"

            output_str_no_duplicate += line + "\n"
            same_line_count = 0

        previous_line = line
    # check if the the buffer ended with same values.
    if same_line_count > 1:
        output_str_no_duplicate += "*\n"
    if same_line_count > 0:
        output_str_no_duplicate += previous_line + "\n"
    return output_str_no_duplicate


def mem_size_to_string(size_in_octets, units="okw"):
    """Returns the memory size as a formatted string in different unit.

    Args:
        size_in_octets: Memory size in octets.
        units (str, optional): Contains the unit to use:
            "o" - octets
            "k" - KiB
            "w" - words

    Returns:
        String which contains the memory size in different units.
    """
    return_str = ""
    if "o" in units:
        # first format the size in octets
        return_str += "{:7,} bytes".format(size_in_octets)
    if "k" in units:
        if "o" in units:
            return_str += " ("
        return_str += "{:5.1f} KiB".format(size_in_octets / 1024.0)
        if "o" in units:
            return_str += ")"
    # if the address per words is different than one, format the size in words.
    if Arch.addr_per_word != 1 and "w" in units:
        # convert to words and round up.
        size_in_words = (
            (size_in_octets + Arch.addr_per_word - 1) // Arch.addr_per_word
        )
        if "o" in units or "k" in units:
            return_str += " or "
        return_str += "{:7,} words".format(size_in_words)

    return return_str


def list_to_string(in_list):
    """Converts a list to a string.

    This happens in a way to hide differences between python 3 and 2.

    Args:
        in_list (list)

    Returns:
        str
    """
    # Remove 'u' which is not present in python 2.
    return str(in_list).replace(" u'", " '")


def import_module_from_path(module_name, path):
    """Imports a python module from a given path.

    Args:
        module_name
        path

    Returns:
        Python module
    """
    file_handler, filename, desc = imp.find_module(module_name, [path])

    return imp.load_module(module_name, file_handler, filename, desc)


def open_file_with_default_app(file_path):
    """Opens a file with the default application.

    Args:
        file_path (str): path to file.

    Raises:
        OSError: Unsupported OS.
    """
    if sys.platform.startswith('darwin'):
        subprocess.call(('open', file_path))
    elif os.name == 'nt':  # For Windows
        os.startfile(file_path)
    elif os.name == 'posix':  # For Linux, Mac, etc.
        subprocess.call(('xdg-open', file_path))
    else:
        raise OSError("Unsupported OS")

##########################################################################
# ACAT Package Helper Functions - function used to start ACAT as a package
##########################################################################


def import_pythontools():
    """Imports modules from pythontools."""
    logger.info("pythontools_path is %s", global_options.pythontools_path)
    # add pythontools to the system path.
    sys.path.append(global_options.pythontools_path)

    # Import kalelfreader for KerDebugInfo
    ker_module = import_module_from_path(
        'kalelfreader_lib_wrappers', global_options.pythontools_path
    )
    global_options.ker = ker_module

    # Import kalmemaccessors for LiveSPI amd LiveKalcmd
    kmem = import_module_from_path(
        'kalmemaccessors', global_options.pythontools_path
    )
    global_options.kmem = kmem

    if global_options.live:
        # Import kalaccess for LiveSpi
        kalaccess_module = import_module_from_path(
            'kalaccess', global_options.pythontools_path
        )
        global_options.kal = kalaccess_module.Kalaccess()
        if (global_options.is_dualcore) or (global_options.processor == 1):
            global_options.kal2 = kalaccess_module.Kalaccess()

    logger.info("coredump_path is %s", global_options.coredump_path)
    if global_options.live:
        if global_options.kalcmd_object is not None:
            logger.info("Running on a live chip over kalcmd2")
        else:
            logger.info("Running on a live chip")


#########################################


def common_func(proc):
    """Apply common justifications to a processor instance.

    This function contains functionalities that are applied to each
    processor instance that does special decoration for pciespi.

    Args:
        proc: Proccessor object.
    """
    if "pciespi" in global_options.spi_trans:
        # pciespi transport doesn't read good values until the clock is up
        # to the speed in the DUT.  The reading from the dm memory will be
        # decorated to read the first element from the capability data
        # table until is null. ( The first element from the capability
        # tables  is the first capability address. This table is null
        # terminated and we know that at least one capability is
        # installed) Because the read method is static we need to decorate
        # the class.
        var = proc.debuginfo.get_var_strict("$_capability_data_table")
        address = var.address

        def _dm_decorator(getitem, address):
            # function decorator
            def _new_getitem(self, index):
                prev_value = getitem(self, address)
                value = getitem(self, address)
                while (prev_value != value) or (value == 0):
                    prev_value = value
                    value = getitem(self, address)
                return getitem(self, index)

            return _new_getitem

        def _dm_accessor_decorator(kls):
            # class decorator
            kls.__getitem__ = _dm_decorator(kls.__getitem__, address)
            return kls

        dm_accessor_mod = _dm_accessor_decorator(
            global_options.kmem.DmAccessor
        )
        if proc.processor == 0:
            global_options.kal.dm = dm_accessor_mod(global_options.kal)
        else:
            global_options.kal2.dm = dm_accessor_mod(global_options.kal2)
        logger.info("kalmemaccessors.DmAccessor was decorated for pciespi")


#########################################


def load_session(analyses=None):
    """Returns an interpreter loaded with the specified analyses.

    Args:
        analyses (:obj:`list` of :obj:`string`): What analyses to load. If
            none all the existent analyses will be loaded.
    """
    # import here to avoid circular dependency.
    from ..Core.MulticoreSupport import Processor
    from ..Interpreter.Interactive import Interactive
    from ..Interpreter.Automatic import Automatic
    from ..Display.PlainTextFormatter import PlainTextFormatter
    from ..Display.HtmlFormatter import HtmlFormatter
    from ..Display.InteractiveFormatter import InteractiveFormatter

    # In non-interative mode we can use "-" to indicate that the coredump
    # is being supplied on the standard input. Since we need to parse it
    # more than once, we slurp it into an array here so we can iterate over
    # it later. Because this feature was quite a late addition, we signal
    # what's going on by putting the array into coredump_path and detecting
    # it's an array later. This is ugly.
    if global_options.coredump_path == '-':
        # The coredump content is provided via standard input. Decompression
        # isn't supported in this case.
        global_options.coredump_lines = _strip_lines(sys.stdin.readlines())

    elif len(global_options.coredump_path):
        # The coredump filename is provided
        coredump_path = os.path.normcase(global_options.coredump_path)
        if compression.is_zip(coredump_path):
            logger.debug("%s is a zip file", coredump_path)
            global_options.coredump_lines = _strip_lines(
                compression.get_coredump_lines(coredump_path)
            )

        else:
            with open(coredump_path) as handler:
                global_options.coredump_lines = _strip_lines(
                    handler.readlines()
                )

    # decide on the formatter
    if global_options.interactive:
        formatter = InteractiveFormatter()
    else:
        if global_options.html_path == "":
            formatter = PlainTextFormatter()
        else:
            formatter = HtmlFormatter(global_options.html_path)

    # create the processors
    if global_options.processor == 0:
        p0 = Processor(
            global_options.coredump_lines, global_options.build_output_path_p0,
            0, formatter
        )
        logger.info(
            "build_output_path for P0 is %s",
            global_options.build_output_path_p0
        )
        kwargs = {'p0': p0}
        common_func(p0)

        if global_options.is_dualcore:
            p1 = Processor(
                global_options.coredump_lines,
                global_options.build_output_path_p1, 1, formatter
            )
            kwargs['p1'] = p1
            logger.info(
                "build_output_path for P1 is %s",
                global_options.build_output_path_p1
            )
            common_func(p1)
    else:
        p1 = Processor(
            global_options.coredump_lines, global_options.build_output_path_p1,
            1, formatter
        )
        logger.info(
            "build_output_path for P1 is %s",
            global_options.build_output_path_p1
        )
        kwargs = {'p1': p1}
        common_func(p1)

        if global_options.is_dualcore:
            p0 = Processor(
                global_options.coredump_lines,
                global_options.build_output_path_p0, 0, formatter
            )
            logger.info(
                "build_output_path for P0 is %s",
                global_options.build_output_path_p0
            )
            kwargs = {'p0': p0}
            common_func(p0)

    # if analyses is not none only the given analysis will be run.
    if analyses:
        kwargs['analyses'] = analyses

    # return the interpreter
    if global_options.interactive:
        return Interactive(**kwargs)
    return Automatic(**kwargs)


#########################################
def do_analysis(session):
    """Starts the interpreter.

    Args:
        session: An instance of Interpreter class.
    """
    try:
        session.run()

    except SystemExit:
        # Python Interpreter invokes SystemExist at its end of life time
        pass


def _strip_lines(lines):
    """Strips all the strings within given list.

    Args:
        lines (iterable): List of strings.

    Returns:
        list: stripped strings.
    """
    if IS_PY2:
        return [
            line.strip()
            for line in lines
        ]
    else:
        return [
            isinstance(line, bytes) and line.decode().strip()
            or
            line.strip()
            for line in lines
        ]


class MatplotlibImporter(object):
    """A class imitating `matplotlib` library.

    A simple use case of this class is something similar to below:

        >>> matplotlib = MatplotlibImporter()
        >>> plt = matplotlib.pyplot

    Above is similar to:

        >>> import matplotlib.pyplot as plt

    But handles the GUI selection based on the fact whether the application
    under test or not. Since `tkinter`, the default GUI for matplotlib, isn't
    thread-safe pytest crashes out if it's unchanged.
    """

    @staticmethod
    def _get_matplotlib(sub_module=None):
        """Import `matplotlib` library with given sub module(s)."""
        try:
            # pylint: disable=import-error
            matplotlib = importlib.import_module('matplotlib')
            if global_options.under_test:
                # tkinter isn't thread-safe. Replace it with `agg` instead.
                # Since we dynamically import the module the `use` method gets
                # confused and warn that this call may not have any effect
                # since there is a sub-module of `matplotlib` is already
                # loaded. But in fact the effect is already there with the
                # first call. So, turning the warning off and ignore it.
                matplotlib.use('agg', warn=False)

            if sub_module:
                return importlib.import_module(
                    'matplotlib.{}'.format(sub_module)
                )

            # There is no sub-module specified, so returning the matplotlib
            # itself.
            return matplotlib

        except ImportError:
            # matplotlib not installed
            return None

    def __getattr__(self, name):
        return self._get_matplotlib(sub_module=name)


@contextmanager
def change_matplotlib_log_level(log_level):
    """Changes the matplotlib logger's logging level.

    Args:
        log_level (int): Python standard logging level.
    """
    matplotlib_logger = logging.getLogger('matplotlib')
    original_level = matplotlib_logger.level
    try:
        matplotlib_logger.setLevel(log_level)
        yield

    finally:
        matplotlib_logger.setLevel(original_level)
