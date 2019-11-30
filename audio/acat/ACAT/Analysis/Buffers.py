############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2012 - 2019 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
"""
Module resposible for analysing buffers in Kymera.
"""
import math
import numbers
import os
import re
import sys
import threading
import time
from collections import OrderedDict

if sys.version_info.major == 2:
    # Python 2
    # pylint: disable=redefined-builtin
    from future_builtins import hex

from . import Analysis
from ..Core import Arch
from ACAT.Core import CoreTypes as ct
from ACAT.Core import CoreUtils as cu
from ACAT.Core.exceptions import DebugInfoNoVariableError
from ACAT.Display.HtmlFormatter import HtmlFormatter
from ACAT.Display.InteractiveFormatter import InteractiveFormatter
from ACAT.Display.PlainTextFormatter import PlainTextFormatter

TYPE_DEPENDENCIES = {
    'tCbuffer': (
        'descriptor', 'base_addr', 'read_ptr', 'write_ptr', 'size', 'metadata'
    ),
    'audio_buf_handle_struc': ('buffer_offset',),
    'metadata_list': ('next', 'tags.head', 'tags.tail'),
    'metadata_tag': ('next',)
}


class Cbuffer(ct.Variable):
    """Class representing a circular buffer.

    Args:
        var (obj:`Variable'): CBuffer variable that has been casted from a
            pointer.
        is_old_version (bool)

    Attributes:
        is_old_version (bool)
        buffer_type (str): `SW`/`MMU`/`Remote MMU`
        buffer_size
        rd_offset
        wr_offset
        aux_offset
        base_addr (int)
        rd_type (str): `SW` or `MMU`
        wr_type (str): `SW` or `MMU`
        aux_type (str): `read` or `write`
        remote_rdh (tuple): Tuple of (SSID, index) or None
        remote_wrh (tuple): Tuple of (SSID, index) or None
        amt_data
        av_space
        usable_octets
        metadata_list
        in_place
        head_of_in_place_chain
    """
    def __init__(self, var, is_old_version):
        ct.Variable.__init__(
            self,
            name=var.name,
            address=var.address,
            size=var.size,
            value=var.value,
            var_type=var.type,
            debuginfo=var.debuginfo,
            members=var.members,
            parent=var.parent
        )

        self.is_old_version = is_old_version
        self.buffer_type = None  # String: "SW"/"MMU"/"Remote MMU"

        # Offsets into the buffer (in words)
        self.buffer_size = None  # NB self.size is already in use!
        self.rd_offset = None
        self.wr_offset = None
        self.aux_offset = None
        self.base_addr = None

        # Metadata about the handles
        self.rd_type = None  # Could be "SW" or "MMU"
        self.wr_type = None  # Could be "SW" or "MMU"
        self.aux_type = None  # Could be "read" or "write"
        self.remote_rdh = None  # None, or a tuple containing (SSID, index)
        self.remote_wrh = None  # None, or a tuple containing (SSID, index)
        self.amt_data = None
        self.av_space = None
        if is_old_version is False:
            self.usable_octets = None
        # None, or a string containing the metadata structures and no of tags.
        self.metadata_list = None
        # A buffer can be part of an in-place buffer chain. In which case
        # the available space in the buffer is calculated differently.
        self.in_place = False
        self.head_of_in_place_chain = False

    def __str__(self, debug=False):
        if self.buffer_type is None:
            # Class hasn't been analysed yet.
            return ct.Variable.__str__(debug)

        # Otherwise, we override Variable.__str__, because none
        # of that stuff is especially useful.
        # Re-use the indent mechanism from Variable, though.

        mystr = (self.indent + 'Buffer type: ' + str(self.buffer_type) + '\n')
        mystr += (
            self.indent + 'Buffer struct address: ' + hex(self.address) + '\n'
        )
        mystr += (
            self.indent + 'Buffer descriptor: ' +
            bin(self.get_member("descriptor").value) + '\n'
        )

        if self.buffer_type == "Remote MMU":
            mystr += (
                self.indent + 'Buffer comes from subsystem: ' +
                hex(self.remote_rdh[0]) + '\n'
            )
            mystr += (
                self.indent + '  Read handle index: ' +
                hex(self.remote_rdh[1]) + '\n'
            )
            mystr += (
                self.indent + '  Write handle index: ' +
                hex(self.remote_wrh[1]) + '\n'
            )
        else:
            mystr += (
                self.indent + 'Base address: ' +
                hex(self.get_member('base_addr').value) + '\n'
            )
            mystr += (self.indent + 'Size: ' + str(self.buffer_size) + '\n')
            mystr += (
                self.indent + self.rd_type + ' read offset: ' +
                str(self.rd_offset) + '\n'
            )
            mystr += (
                self.indent + self.wr_type + ' write offset: ' +
                str(self.wr_offset) + '\n'
            )

            mystr += (
                self.indent + 'Amount of Data: ' + str(self.amt_data) + '\n'
            )
            mystr += (
                self.indent + 'Amount of Space: ' + str(self.av_space) + '\n'
            )

            if self.is_old_version is False:
                mystr += (
                    self.indent + 'Usable octets: ' +
                    str(self.usable_octets) + '\n'
                )

            if self.in_place:
                mystr += (self.indent + 'This is an in-place buffer')
                if self.head_of_in_place_chain:
                    mystr += " and it is the head of the in-place chain.\n"
                else:
                    mystr += ".\n"
            if self.aux_offset is not None:
                mystr += (
                    self.indent + 'Aux ' + self.aux_type + ' offset: ' +
                    str(self.aux_offset) + '\n'
                )

            if self.metadata_list is not None:
                mystr += (
                    self.indent + 'Metadata list:\n' + self.metadata_list +
                    '\n'
                )

        return mystr

    def get_base_address(self):
        """Returns the base address of the buffer.

        Returns:
            int: Base address.
        """
        return self.base_addr

    def get_write_address(self):
        """Returns the write address of the buffer.

        Returns:
            int: Write address.
        """
        return self.base_addr + self.wr_offset * Arch.addr_per_word

    def get_read_address(self):
        """Returns the read address of the buffer.

        Returns:
            int: Read address.
        """
        return self.base_addr + self.rd_offset * Arch.addr_per_word

    def get_previous_address(self, address):
        """Returns the previous address.

        Calculate the address which is before the given address in the
        buffer and return it. This will count for buffer wrap.

        Args:
            address (int): Address in the buffer.

        Returns:
            int: Read address.
        """
        end_of_buffer = self.base_addr + self.buffer_size * Arch.addr_per_word
        if (address < self.base_addr) or (address >= end_of_buffer):
            raise Exception(
                "Address 0x%08x is not in the buffer" % address
            )
        address -= Arch.addr_per_word
        if address < self.base_addr:
            address += self.buffer_size * Arch.addr_per_word

        return address


DESCRIPTOR_FLAGS = (
    # 0 - cbuffer wraps a pure SW buffer  |  1 - cbuffer wraps an MMU buffer
    # (local or remote)
    'BUFFER_TYPE_MMU',
    # [If BUFFER_TYPE_MMU] 0 - cbuffer struct wraps a local MMU buffer;
    # 1 - cbuffer wraps a remote MMU buffer
    'IS_REMOTE_MMU',
    # cbuffer read_ptr points to 0 - SW buffer | 1 - local MMU buffer.
    # Note - not set for remote MMU buffers.
    'RD_PTR_TYPE_MMU',
    # cbuffer write_ptr points to 0 - SW buffer | 1 - local MMU buffer.
    # Note - not set for remote MMU buffers.
    'WR_PTR_TYPE_MMU',
    # The Audio Subsystem cannot / can modify the remote MMU handle
    # pointed to by read_ptr
    'REMOTE_RDH_MOD',
    # The Audio Subsystem cannot / can modify the remote MMU handle
    # pointed to by write_ptr
    'REMOTE_WRH_MOD',
    # 0 - cbuffer aux_ptr absent | 1 - cbuffer aux_ptr present. Note - if set,
    # implies bits 2 & 3 are also both set.
    'AUX_PTR_PRESENT',
    # [If AUX_PTR_PRESENT], the aux_ptr points to the MMU read / write handle
    'AUX_PTR_TYPE',
    # The HostIO subsystem can / cannot modify the read handle(s).
    # Note - only valid for local MMU buffers
    'RD_PTR_HOSTXS_PROT',
    # Remote subsystems can / cannot modify the read handle(s).
    # Note - only valid for local MMU buffers
    'RD_PTR_XS_PROT',
    # The HostIO subsystem can / cannot modify the write handle(s).
    # Note - only valid for local MMU buffers
    'WR_PTR_HOSTXS_PROT',
    # Remote subsystems can / cannot modify the write handle(s).
    # Note - only valid for local MMU buffers
    'WR_PTR_XS_PROT',
    # 0 - Remote read handle has no byte-swap | 1 - has a byte-swap applied.
    # Note - only valid for remote MMU buffers
    'REMOTE_RDH_BSWAP',
    # 0 - Remote write handle has no byte-swap | 1 - has a byte-swap applied.
    # Note - only valid for remote MMU buffers
    'REMOTE_WRH_BSWAP',
    # 0 - CBOPS is NOT scratch | 1 - CBOPS_IS_SCRATCH
    'BUF_DESC_CBOPS_IS_SCRATCH_SHIFT',
    # NOTE: This field is obsolete. We no longer check on this field for
    #       meta data.
    # 0 - no metadata associated (metadata field should be ignored)
    # 1 - may have metadata associated (metadata field should point to a valid
    # metadata list, or be NULL)
    'BUF_DESC_METADATA_SHIFT',
    # 0 - if bit 0 is set, cbuffer is not part of an in place chain
    # 1 - if bit 0 is set, cbuffer is part of an in place chain
    'BUF_DESC_IN_PLACE_SHIFT',
    # bits 17-18: usable_octets:2; number of usable octets in a word
    # (0 means all)
    'BUF_DESC_USABLE_OCTETS_SHIFT',
    'second_bit_BUF_DESC_USABLE_OCTETS'
)


class Buffers(Analysis.Analysis):
    """Encapsulates analysis of buffers.

    Args:
        **kwarg: Arbitrary keyword arguments.
    """

    def __init__(self, **kwarg):
        Analysis.Analysis.__init__(self, **kwarg)
        self.is_old_version = True
        usable_octets = self.chipdata.cast(
            0, "metadata_list"
        ).get_member("usable_octets")
        if usable_octets is None:
            self.is_old_version = False

        matplotlib = cu.MatplotlibImporter()
        self._plt = matplotlib.pyplot

    def run_all(self):
        """It does nothing.

        This analysis does not implement a run_all function because it is
        a helper module.
        """
        # Nothing we can usefully do here.
        return

    def plot_buffers(self, buffers, screen_width=120, live_plotting=False):
        """Display buffer all the given buffers.

        Displays the buffer usage in a console progress bar way [####   ]
        for a given list of buffers.

        Args:
            buffers (:obj:`list` of :obj:`tuple'): Each buffer described
                as a tuple of (name, address).
            screen_width (int): Screen width in characters.
            live_plotting (bool): The live_plotting mode will continuously
                display the buffer usage until an exit event is received.
        """
        exit_message = "Press enter to exit.\n"
        # While the measure event is not PrintBufferUsage is reading the buffer
        # data usage.
        exit_event = threading.Event()
        exit_event.clear()
        reader = PrintBufferUsage(
            exit_event,
            buffers,
            self,
            exit_message,
            screen_width,
            live_plotting
        )
        if live_plotting:
            reader.start()
            # wait until a key is pressed.
            sys.stdin.read(1)
            exit_event.set()
            # wait for the task to finish
            reader.join()
        else:
            self.formatter.section_start("Buffer Usage")
            self.formatter.output_raw(reader.get_buffer_usage_table())
            self.formatter.section_end()

    def analyse_transform_buffers(self, analyse_metadata=False,
                                  plot_contents=False):
        """Prints out the buffer details for the transform buffers.

        If True is passed to it, the contents of the metadata structures is
        also displayed, with the numbers of tags. If tag names are passed as
        a list, these tags are displayed e.g ['index', 'flags', 'data']. If
        empty list [] is passed  , all tags are displayed

        Args:
            analyse_metadata (bool): If True the metadata of the buffer is
                displayed. False by default.
            plot_contents (bool): Whether display the buffer's contest in
                an image or not.
        """
        try:
            # Get the stream analysis from the interpreter which is used to
            # get the available transforms and the buffer associated with.
            streams_analysis = self.interpreter.get_analysis(
                "stream", self.chipdata.processor
            )

            for transform in streams_analysis.get_transforms():
                self.analyse_cbuffer(
                    transform.buffer,
                    transform.title_str,
                    analyse_metadata,
                    plot_content=plot_contents,
                )

        except KeyError:
            self.formatter.output(
                "Buffers analysis cannot find the Stream analysis for getting "
                "transform buffers."
            )

    #######################################################################
    # Analysis methods - public since we may want to call them individually
    #######################################################################

    def analyse_cbuffer(self, buffer_p, title_str="None",
                        analyse_metadata=False, plot_content=False):
        """Takes a pointer to a cbuffer and inspects its contents.

        If True is passed to it, the contents of the metadata structures is
        also displayed, with the numbers of tags. If tag names are passed as
        a list, these tags are displayed e.g ['index', 'flags', 'data']. If
        empty list [] is passed  , all tags are displayed

        Args:
            buffer_p (int): Buffer pointer.
            title_str (str): Description of the transform to be displayed
                as title for plot.
            analyse_metadata (bool): If True the metadata of the buffer is
                displayed. False by default.
            plot_content (bool): If True generates a diagram and display
                it.
        """
        buff_var = self.inspect_cbuffer(buffer_p, analyse_metadata)
        namestr = str(buff_var.buffer_type) + '-flavoured cbuffer'
        self.formatter.section_start(namestr)
        self.formatter.output_raw(buff_var)
        self.draw_cbuffer(buffer_p)
        self.display_content(buff_var)
        if plot_content and self._plt is not None:
            # provide a better way to decide the type of the buffer content.
            self.plot_content(buff_var, title_str, cu.u32_to_frac32)
        self.formatter.section_end()

    def _get_content(self, cbuffer, start_with="base",
                     only_available_data=False, ret_format="dict"):
        """Returns the cbuffer content as a list or string.

        This function is the base for get_content and display content.

        Args:
            cbuffer (:obj:`Cbuffer`): Circular buffer pointer or buffer
                variable.
            start_with (str, optional): Where to start to read the buffer.
                The following options are supported:

                    "base" - Reading will start at the base address of the
                        buffer.
                    "read" - Starting with the read address of the buffer.
                    "write" - Starting with the write address of the
                        buffer. This can be useful when you want to display
                        the oldest data in the buffer first.
            only_available_data (bool, optional): If true, only the
                available data in the buffer will be displayed. If false,
                everything in the buffer is displayed (This can be useful
                if you want to see some old packets/ data in the buffer).
            ret_format (str, optional): Specifies the return format:
                "dict" - returns a dictionary of addresses and values
                    (with additional mark-ups) which represents the buffer
                    content.
                "list" - returns a list of integers with the buffer
                    values.

        Returns:
            dict, list: buffer content as a dictionary or list of values
                depending on the specified format.
        """
        if isinstance(cbuffer, numbers.Integral):
            cbuffer = self.inspect_cbuffer(cbuffer)

        if cbuffer.buffer_type != "SW" and cbuffer.buffer_type != "MMU":
            self.formatter.output("Content cannot be displayed!")
            return

        # Get the content and information about the cbuffer
        buffer_base_addr = cbuffer.get_base_address()
        write_address = cbuffer.get_write_address()
        read_address = cbuffer.get_read_address()
        buffer_content = self.chipdata.get_data(
            buffer_base_addr, cbuffer.buffer_size * Arch.addr_per_word
        )

        # Set the start and stop addresses which will be used to decide from
        # where to start reading the cbuffer.
        if start_with == "read":

            start_address = read_address
            if only_available_data:
                stop_address = cbuffer.get_previous_address(write_address)
            else:
                stop_address = cbuffer.get_previous_address(start_address)

        elif start_with == "write":

            start_address = write_address
            if only_available_data:
                raise Exception("Unsupported configuration")
            else:
                stop_address = cbuffer.get_previous_address(start_address)

        elif start_with == "base":

            start_address = buffer_base_addr
            if only_available_data:
                raise Exception("Unsupported configuration")
            else:
                stop_address = cbuffer.get_previous_address(start_address)

        else:
            raise Exception(
                start_with + " - invalid option. " +
                "Only the following options are supported: \"read\", \"write\""
                + " and \"base\"")

        if start_address == stop_address and only_available_data:
            # Buffer empty no need to go trough the memory if only the
            # available data is needed.
            if ret_format == "list":
                return []

        buffer_dict = OrderedDict()
        buffer_list = []
        start_logging = False
        stop_address_found = False
        for offset, value in enumerate(buffer_content):
            current_address = offset * Arch.addr_per_word + buffer_base_addr
            if current_address == start_address:
                start_logging = True

            if start_logging:
                if current_address == read_address:
                    buffer_dict["read pointer"] = current_address
                if current_address == write_address:
                    buffer_dict["write pointer"] = current_address

                buffer_dict[current_address] = value
                buffer_list.append(value)

                if current_address == stop_address:
                    stop_address_found = True
                    # exit if we it is not relevant to display the non
                    # available data.
                    if only_available_data:
                        break

        if not stop_address_found:
            # there is still some data left
            buffer_dict["cbuffer wrap"] = \
                cbuffer.get_previous_address(buffer_base_addr)

            for offset, value in enumerate(buffer_content):
                address = offset * Arch.addr_per_word + buffer_base_addr
                if address == read_address:
                    buffer_dict["read pointer"] = address
                if address == write_address:
                    buffer_dict["write pointer"] = address

                buffer_dict[address] = value
                buffer_list.append(value)

                if address == stop_address:
                    # the second time we are going trough the cbuffer
                    break

        if ret_format == "list":
            return buffer_list
        elif ret_format == "dict":
            return buffer_dict
        else:
            raise Exception("Format not supported!")

    def get_content(self, cbuffer, start_with="read",
                    only_available_data=False, conversion_function=None):
        """Returns the buffer content.

        Returns the buffer content as a list of words. Use this function
        if you want to use the buffer content in a analysis or a separate
        script/program.

        Examples:
            The usage of this function can be something similar to below:

                buffer_content =  buffers.get_content(
                    <cbuffer pointer>,
                    start_with="read",
                    only_available_data = True,
                    conversion_function = cu.u32_to_frac32)
                # displaying the content using matplotlib
                import matplotlib.pyplot as plt
                plt.plot(buffer_content)
                plt.show()

        Args:
            cbuffer (:obj:`Cbuffer`): Circular buffer pointer or buffer
                variable.

            start_with (str, optional): Where to start to read the buffer.
                The following options are supported:
                    "base" - Reading will start at the base address of the
                        buffer.
                    "read" - Starting with the read address of the buffer.
                    "write" - Starting with the write address of the
                        buffer. This can be useful when you want to
                        display the oldest data in the buffer first.
            only_available_data (bool, optional): If true, only the
                available data in the buffer will be displayed. If false,
                everything in the buffer is displayed (This can be useful
                if you want to see some old packets/ data in the buffer).
            conversion_function (obj, optional): A conversion function can
                be applied to the return values by setting this parameter.

        Returns:
            list: unsigned 32 bit integer values.
        """
        retval = self._get_content(
            cbuffer,
            start_with=start_with,
            only_available_data=only_available_data,
            ret_format="list"
        )

        if conversion_function is not None:
            # convert the values using the conversion function
            return [conversion_function(i) for i in retval]

        return retval

    def display_content(self, cbuffer, start_with="base",
                        only_available_data=False, words_per_line=8):
        """Displays the buffer content in a humanly readable manner.

        Args:
            cbuffer (:obj:`Cbuffer`): Circular buffer pointer or buffer
                variable.
            start_with (str, optional): Where to start to read the buffer.
                The following options are supported:
                    "base" - Reading will start at the base address of the
                        buffer.
                    "read" - Starting with the read address of the buffer.
                    "write" - Starting with the write address of the
                        buffer. This can be useful when you want to
                        display the oldest data in the buffer first.
            only_available_data (bool, optional): If true, only the
                available data in the buffer will be displayed. If false,
                everything in the buffer is displayed (This can be useful
                if you want to see some old packets/ data in the buffer).
            words_per_line (int, optional): How many words will be
                displayed in one line.

        Returns:
            str: buffer content as a formatted string.
        """
        buffer_dict = self._get_content(
            cbuffer,
            start_with=start_with,
            only_available_data=only_available_data,
            ret_format="dict"
        )
        output_str = cu.mem_dict_to_string(buffer_dict, words_per_line)
        # and finally display the buffer content
        self.formatter.output_raw(output_str)

    def plot_cbuffer_live(self, cbuffer_addr, conversion_function=None):
        """Displays the cbuffer live.

        Constantly reads the content of a cbuffer and displays it using
        `matplotlib` library. The action ends when user closes the figure
        window.

        Args:
            cbuffer_addr (int): cbuffer's struct address.
            conversion_function (object): A conversion function. When not
                given, the method uses `u32_to_frac32` function in
                CoreUtils.py package.
        """
        if self._plt is None:
            self.formatter.output(
                "Please install matplotlib using: pip install matplotlib\n"
                "For more info visit "
                "https://matplotlib.org/users/installing.html"
            )
            return

        if type(self.formatter) is PlainTextFormatter:
            # Do not draw when in plain text mode.
            self.formatter.output(
                "The functionality is unable to proceed in Plain Text "
                "mode. Try to execute ACAT in Interactive mode instead."
            )
            return

        if conversion_function is None:
            conversion_function = cu.u32_to_frac32

        # When `matplotlib` library is installed it is safe to assume
        # `pylab` is installed as well.
        import pylab as pl

        buff_var = self.inspect_cbuffer(cbuffer_addr)
        image = None
        fig_manager = None
        try:
            # Reads the buffer infinitely until the user closes the
            # window or a keyboard interruption happens.
            while True:
                buffer_dict = self._get_content(
                    buff_var,
                    start_with='base',
                    only_available_data=False,
                    ret_format='dict'
                )

                pt_input = {
                    'conversion_function': conversion_function,
                    'buffer_dict': buffer_dict,
                    'output_format': 'bytes_handler',
                    'title_string': None
                }

                bytes_handler = cu.run_plotter(pt_input)

                image_content = pl.imread(bytes_handler)
                if image is None:
                    image = pl.imshow(image_content, aspect='equal')
                    fig_manager = pl.get_current_fig_manager()
                    pl.axis('off')
                    pl.suptitle("Buffer content at %s" % hex(cbuffer_addr))

                else:
                    image.set_data(image_content)

                pl.pause(0.001) # Allows the plotting to proceed.
                if pl.fignum_exists(fig_manager.num):
                    # The figure window is open.
                    pl.draw()
                else:
                    # The figure is closed, breaking the loop up.
                    break

        except KeyboardInterrupt:
            pl.close()

        finally:
            pl.close()

    def plot_content(self, cbuffer, title_str, conversion_function=None):
        """Plots the content of the buffer using matplotlib.

        Args:
            cbuffer (:obj:`Cbuffer`): Circular buffer pointer or buffer
                variable.
            conversion_function (obj, optional): A conversion function can
                be applied to the buffer values by setting this parameter.

        Returns:
            str: buffer content as a formatted string.
        """
        if self._plt is None:
            self.formatter.output(
                "Please install matplotlib using: pip install matplotlib\n"
                "For more info visit "
                "https://matplotlib.org/users/installing.html"
            )
            return
        if type(self.formatter) is PlainTextFormatter:
            # do not draw when in plain text mode
            return

        # just plot the raw data from the buffer if no conversion function is
        # given
        if conversion_function is None:
            self.formatter.output(
                "A conversion function should be used as "
                "raw data is not useful to plot.\n"
                "Already defined conversion functions can "
                "be found in CoreUtils.\n"
                "Like: cu.u32_to_frac32 or cu.u32_to_s32"
            )

            def raw_data(value):
                """
                dummy conversion function which returns the value.
                """
                return value

            conversion_function = raw_data

        buffer_dict = self._get_content(
            cbuffer,
            start_with="base",
            only_available_data=False,
            ret_format="dict"
        )

        if type(self.formatter) is HtmlFormatter:
            output_format = "svg"
        elif type(self.formatter) is InteractiveFormatter:
            output_format = "window"

        pt_input = {
            "conversion_function": conversion_function,
            "buffer_dict": buffer_dict,
            "output_format": output_format,
            "title_string": title_str

        }

        pt_output = cu.run_plotter(pt_input)

        if cu.global_options.under_test:
            # Remove unwanted symbols.
            pt_output = re.sub(
                u"\u2212", u"",
                pt_output.decode('utf-8', errors="ignore"),
                flags=re.M
            )
            # Remove any random IDs from the svg data
            pt_output = re.sub(
                r"#[a-z0-9]{11}", "",
                str(pt_output),
                flags=re.M
            )
            pt_output = re.sub(
                r"id=\"[a-z0-9]{11}\"", "id=\"\"",
                str(pt_output),
                flags=re.M
            )

        if output_format == "svg":
            self.formatter.output_svg(pt_output)
        elif output_format == "window":
            self.formatter.output("Plotting  the buffer content.")

    def get_mmu_buffer_width(self, handle):
        """Get the width of an MMU buffer.

        Note:
            Only has been tried on Crescendo.

        Args:
            handle: MMU buffer handle.

        Returns:
            int: The width of MMU buffer in octets. i.e. 32bit -> 4
        """
        try:
            # get the mask for 32-bit mmu buffers, this might fail in some
            # architectures
            bac_sample_size = self.debuginfo.get_constant_strict(
                '$BAC_BUFFER_SAMPLE_SIZE_POSN'
            ).value
            bac_buf_size = self.debuginfo.get_constant_strict(
                '$BAC_BUFFER_SIZE_POSN'
            ).value
            sample_size_shift = bac_sample_size - bac_buf_size
            sample_size_32bit_mask = self.debuginfo.get_constant_strict(
                '$BAC_BUFFER_SAMPLE_32_BIT_ENUM'
            ).value
            handle_size = handle.get_member('buffer_size').value
            if ((handle_size >> sample_size_shift) &
                    sample_size_32bit_mask) == sample_size_32bit_mask:
                # 32 bit which is 4 octets
                return 4

            # if not 32-bit, we assume it's 16 bit (might not be valid in
            # long run)
            return 2

        except DebugInfoNoVariableError:
            # In some builds, above constant variables are being removed
            # so it's impossible to determine the buffer width correctly,
            # assume 32 bit. Please note, this assumption may generate
            # invalid output.
            self.formatter.output(
                "Warning, unable to determine width of mmu buffer, "
                "assumed 32-bit\n"
            )
            return 4

    # @Analysis.cache_decorator
    def inspect_cbuffer(self, buffer_p, analyse_metadata=False):
        """Inspect the given cbuffer.

        If True is passed to it, the contents of the metadata structures is
        also displayed, with the numbers of tags. If tag names are passed as
        a list, these tags are displayed e.g ['index', 'flags', 'data']. If
        empty list [] is passed  , all tags are displayed.

        Args:
            buffer_p (int): Buffer pointer.
            analyse_metadata (bool, optional): If True the metadata of the
                buffer is displayed. False by default.

        Returns:
            A cbuffer variable.
        """
        buffer_var = Cbuffer(
            self.chipdata.cast(buffer_p, "tCbuffer"), self.is_old_version
        )
        descriptor = buffer_var.get_member('descriptor').value

        # What kind of buffer is it?
        if descriptor & (1 << DESCRIPTOR_FLAGS.index('BUFFER_TYPE_MMU')):
            if descriptor & (1 << DESCRIPTOR_FLAGS.index('IS_REMOTE_MMU')):
                buffer_var.buffer_type = "Remote MMU"
            else:
                buffer_var.buffer_type = "MMU"
        else:
            buffer_var.buffer_type = "SW"

        # Where are the rd/wr pointers?
        if buffer_var.buffer_type == "SW" or buffer_var.buffer_type == "MMU":
            # This is true for the SW buffer, and a reasonable set of defaults
            # for an MMU buffer.
            base_addr = buffer_var.get_member('base_addr').value
            buffer_var.base_addr = base_addr
            buffer_var.rd_offset = (
                buffer_var.get_member('read_ptr').value - base_addr
            ) // Arch.addr_per_word
            buffer_var.wr_offset = (
                buffer_var.get_member('write_ptr').value - base_addr
            ) // Arch.addr_per_word
            buffer_var.rd_type = "SW"
            buffer_var.wr_type = "SW"

        if buffer_var.buffer_type == "MMU":
            if descriptor & (1 << DESCRIPTOR_FLAGS.index('RD_PTR_TYPE_MMU')):
                handle_p = buffer_var.get_member('read_ptr').value
                handle = self.chipdata.cast(handle_p, "audio_buf_handle_struc")
                rd_offset = handle.get_member('buffer_offset').value
                buffer_var.rd_offset = rd_offset // self.get_mmu_buffer_width(
                    handle
                )
                buffer_var.rd_type = "MMU"

            if descriptor & (1 << DESCRIPTOR_FLAGS.index('WR_PTR_TYPE_MMU')):
                handle_p = buffer_var.get_member('write_ptr').value
                handle = self.chipdata.cast(handle_p, "audio_buf_handle_struc")
                wr_offset = handle.get_member('buffer_offset').value
                buffer_var.wr_offset = wr_offset // self.get_mmu_buffer_width(
                    handle
                )
                buffer_var.wr_type = "MMU"

            if descriptor & (1 << DESCRIPTOR_FLAGS.index('AUX_PTR_PRESENT')):
                handle_p = buffer_var.get_member('aux_ptr').value
                handle = self.chipdata.cast(handle_p, "audio_buf_handle_struc")
                aux_offset = handle.get_member('buffer_offset').value
                buffer_width = self.get_mmu_buffer_width(handle)

                buffer_var.aux_offset = aux_offset // buffer_width

                # AUX_PTR_TYPE
                if descriptor & (1 << DESCRIPTOR_FLAGS.index('AUX_PTR_TYPE')):
                    buffer_var.aux_type = "write"
                else:
                    buffer_var.aux_type = "read"

            # TODO - sanity-check that the contents of the buffer handle
            # matches the cbuffer?

        if buffer_var.buffer_type == "Remote MMU":
            # Do mmu_handle_unpack on rd and wr pointers
            packed_rdh = buffer_var.get_member('read_ptr').value
            packed_wrh = buffer_var.get_member('write_ptr').value
            # SSID, MMU index
            buffer_var.remote_rdh = (packed_rdh >> 8, packed_rdh & 0xFF)
            # SSID, MMU index
            buffer_var.remote_wrh = (packed_wrh >> 8, packed_wrh & 0xFF)

            # TODO: We could look at the contents of
            # the remote subsystem to work out where the read and write offsets
            # for the remote handles are.

        buffer_var.buffer_size = \
            buffer_var.get_member('size').value // Arch.addr_per_word

        if buffer_var.rd_offset > buffer_var.wr_offset:
            buffer_var.amt_data = (
                buffer_var.wr_offset + buffer_var.buffer_size -
                buffer_var.rd_offset
            )
        else:
            buffer_var.amt_data = (buffer_var.wr_offset - buffer_var.rd_offset)

        if analyse_metadata:
            buffer_var.metadata_list = \
                self._analyse_buffer_metadata(buffer_var)

        buffer_var.av_space = buffer_var.buffer_size - buffer_var.amt_data - 1

        if self.is_old_version is False:
            buffer_var.usable_octets = descriptor & \
                (3 << DESCRIPTOR_FLAGS.index('BUF_DESC_USABLE_OCTETS_SHIFT'))
            if buffer_var.usable_octets != 0:
                buffer_var.usable_octets = buffer_var.usable_octets >> \
                    DESCRIPTOR_FLAGS.index('BUF_DESC_USABLE_OCTETS_SHIFT')

        # For in-place buffer the available space is calculated differently if
        # the buffer is the head of the in-place chain
        if descriptor & 1 << DESCRIPTOR_FLAGS.index('BUF_DESC_IN_PLACE_SHIFT'):
            # mark the buffer as in-place
            buffer_var.in_place = True

            tail_buff = buffer_var.get_member('aux_ptr').value
            if tail_buff != 0:
                # the buffer is the head of the in-place chain. Get the tail
                # buffer read offset. This read offset will be used to
                # calculate the available space.
                buffer_var.head_of_in_place_chain = True
                tail_buffer_var = self.chipdata.cast(tail_buff, "tCbuffer")
                new_rd_offset = (
                    tail_buffer_var.get_member('read_ptr').value - base_addr
                ) // Arch.addr_per_word

                # Now calculate the avalable space for the circular buffer.
                if new_rd_offset > buffer_var.wr_offset:
                    buffer_var.av_space = (
                        new_rd_offset - buffer_var.wr_offset - 1
                    )
                else:
                    buffer_var.av_space = (
                        buffer_var.buffer_size + new_rd_offset -
                        buffer_var.wr_offset - 1
                    )

        return buffer_var

    def draw_cbuffer(self, buffer_p, rd_side='right'):
        """Displays the diagram of the given buffer.

        Optionally, provide a preference for which side of the diagram the
        read/aux_rd pointers are drawn (wr/aux_wr will be on the opposite
        side to rd).

        Args:
            buffer_p (int): Buffer address.
            rd_side (str, optional)

        """
        cbuffer = self.inspect_cbuffer(buffer_p)

        if cbuffer.buffer_type is None:
            return

        diag = []
        diag.append("+----------+")
        diag.append("|          |")
        diag.append("|  {0:^6d}  |".format(cbuffer.buffer_size))
        diag.append("|          |")
        diag.append("+----------+")

        # Most of this information isn't present for remote buffers.
        if cbuffer.buffer_type == "Remote MMU":
            for idx, entry in enumerate(diag):
                diag[idx] = "          " + diag[idx]
            diag.insert(
                0, (" "*12 + "SS: 0x{0:0>2x}" + " "*12).
                format(cbuffer.remote_rdh[0])
            )  # SSID
            diag.append((" "*12 + "0x{0:0>8x}" + " "*12).format(buffer_p))
            return diag

        # whole-number percentage, e.g. 65
        rd_height = (cbuffer.rd_offset * 100) // cbuffer.buffer_size
        # which of the rows it should be in
        rd_height = (rd_height * len(diag)) // 100
        wr_height = (cbuffer.wr_offset * 100) // \
            cbuffer.buffer_size  # percentage
        # which of the rows it should be in
        wr_height = (wr_height * len(diag)) // 100
        aux_height = None

        if cbuffer.aux_offset is not None:
            # whole-number percentage, e.g. 65
            aux_height = (cbuffer.aux_offset * 100) // cbuffer.buffer_size
            # which of the rows it should be in
            aux_height = (aux_height * len(diag)) // 100
            if cbuffer.aux_type == "read":
                aux_label = "rdx"
                aux_side = rd_side
            else:
                aux_label = "wrx"
                if rd_side == 'left':
                    aux_side = 'right'
                else:
                    aux_side = 'left'

        # Unless they happen to be on separate lines, draw any aux handle
        # in place of the 'real' one
        for idx, entry in enumerate(diag):
            if idx == aux_height and aux_side == 'left':
                diag[idx] = aux_label
                diag[idx] += "> {0:>5d} >".format(cbuffer.aux_offset)
                diag[idx] += entry
            elif idx == rd_height and rd_side == 'left':
                diag[idx] = "rd > {0:>5d} >".format(
                    cbuffer.rd_offset
                )
                diag[idx] += entry
            elif idx == wr_height and rd_side == 'right':
                diag[idx] = "wr > {0:>5d} >".format(
                    cbuffer.wr_offset
                )
                diag[idx] += entry
            else:
                diag[idx] = " "*12 + entry

            if idx == aux_height and aux_side == 'right':
                diag[idx] += "< {0:>5d} <".format(cbuffer.rd_offset)
                diag[idx] += aux_label
            elif idx == rd_height and rd_side == 'right':
                diag[idx] += "< {0:>5d} < rd".format(cbuffer.rd_offset)
            elif idx == wr_height and rd_side == 'left':
                diag[idx] += "< {0:>5d} < wr".format(cbuffer.wr_offset)
            else:
                diag[idx] += " "*12

        # The header is the base address of the circular buffer.
        diag.insert(
            0, (" "*12 + " 0x{0:0>8x} " + " "*12).
            format(cbuffer.base_addr)
        )
        # The footer will show the first address not in the circular buffer.
        diag.append(
            (" "*12 + " 0x{0:0>8x} " + " "*12).format(
                cbuffer.base_addr + cbuffer.buffer_size * Arch.addr_per_word
            )
        )

        self.formatter.output_list(diag)

    #######################################################################
    # Private methods - don't call these externally.
    #######################################################################
    def _analyse_buffer_metadata(self, buffer_var):
        """Analyse cbuffer's metadata.

        It returns the members of the linked list and the number of tags
        associated with them.

        Args:
            buffer_var (:obj:`Cbuffer`): Buffer variable.

        Returns:
            str: Metadata items in a list. When there is no metadata, `No
                metadata` string will be returned.
        """
        # Historically, there was a flag in the descriptor to indicate
        # the existence of metadata:
        #
        #     (1 << DESCRIPTOR_FLAGS.index('BUF_DESC_METADATA_SHIFT'))
        #
        # However, we no longer set that flag. `metadata' member still
        # exists in the buffer structure and that should be enough to
        # indicate that there is a metadata associated with the given
        # buffer. The following check is backward compatible check:
        if buffer_var.get_member('metadata').value:
            metadata_list = self.chipdata.cast(
                buffer_var.get_member('metadata').value, 'metadata_list'
            )

            metadata_list_str = self._get_linked_list_contents(
                metadata_list
            )
            # Although, the metadata is linked together; if an operator's
            # endpoints are connected to two different operators; those
            # metadata should show up in other buffers. So there is no need
            # to go through the linked list.
            return metadata_list_str

        return 'No metadata.'

    def _get_linked_list_contents(self, metadata_list):
        """Parse metadata_list structure and return it.

        It takes a metadata_list structure and returns a string containing
        the formatter structure and the tags associated with it.

        Args:
            metadata_list (:obj:`CoreType.Variable`): Metadata list variable.

        Return:
            str: metadata list in string format.
        """
        metadata_list_str = str(metadata_list)

        try:
            head_value = metadata_list.get_member(
                'tags').get_member('head').value
        except AttributeError:  # For backwards compatibility
            head_value = metadata_list.get_member(
                'tags').get_member('local').get_member('head').value

        if head_value != 0:
            head = self.chipdata.cast(head_value, 'metadata_tag')
            count = 0
            metadata_list_str += 'Metadata tags:\n'
            for tag in self.parse_linked_list(head, 'next'):
                count += 1
                new_tag = MetadataTag(tag)
                metadata_list_str += "  " + str(new_tag) + "\n"
            metadata_list_str += 'Number of tags: ' + str(count)

        return metadata_list_str


class MetadataTag(object):
    """
    Encapsulates the analysis of metadata tags.

    Attributes:
        METADATA_STREAM_START_SHIFT (int)
        METADATA_STREAM_START_MASK (int)
        METADATA_STREAM_END_SHIFT (int)
        METADATA_STREAM_END_MASK (int)
        METADATA_PACKET_START_SHIFT (int)
        METADATA_PACKET_START_MASK (int)
        METADATA_PACKET_END_SHIFT (int)
        METADATA_PACKET_END_MASK (int)
        METADATA_TIMESTAMP_SHIFT (int)
        METADATA_TIMESTAMP_MASK (int)
        METADATA_TIME_OF_ARRIVAL_SHIfT (int)
        METADATA_TIME_OF_ARRIVAL_MASK (int)

    Args:
        tag (:obj:`CoreTypes.Variable`)
    """
    tag_display_names = OrderedDict(
        [
            ('timestamp', 'ttp'),
            ('sp_adjust', 'sp'),
            ('length', 'L'),
            ('index', 'I'),
            ('xdata', 'priv'),
            ('flags', 'flags')
        ]
    )

    METADATA_STREAM_START_SHIFT = 0
    METADATA_STREAM_START_MASK = (1 << METADATA_STREAM_START_SHIFT)

    METADATA_STREAM_END_SHIFT = 1
    METADATA_STREAM_END_MASK = (1 << METADATA_STREAM_END_SHIFT)

    METADATA_PACKET_START_SHIFT = 2
    METADATA_PACKET_START_MASK = (1 << METADATA_PACKET_START_SHIFT)

    METADATA_PACKET_END_SHIFT = 3
    METADATA_PACKET_END_MASK = (1 << METADATA_PACKET_END_SHIFT)

    METADATA_TIMESTAMP_SHIFT = 4
    METADATA_TIMESTAMP_MASK = (3 << METADATA_TIMESTAMP_SHIFT)

    METADATA_TIME_OF_ARRIVAL_SHIFT = 6
    METADATA_TIME_OF_ARRIVAL_MASK = (1 << METADATA_TIME_OF_ARRIVAL_SHIFT)

    def __init__(self, tag):
        self.tag = tag
        self.address = tag.address
        self.tag_members = OrderedDict()
        # tag members and their values are stored in dictionary
        for member in self.tag.members:
            self.tag_members[member.name.split('.')[-1]] = member.value


    def is_stream_start_tag(self):
        """Checks if the tag is stream start tag."""
        return (
            (self.tag_members['flags'] & self.METADATA_STREAM_START_MASK) != 0
        )

    def is_stream_end_tag(self):
        """Checks if the tag is stream end tag."""
        return (
            (self.tag_members['flags'] & self.METADATA_STREAM_END_MASK) != 0
        )

    def is_packet_start_tag(self):
        """Checks if the tag is pack start tag."""
        return (
            (self.tag_members['flags'] & self.METADATA_PACKET_START_MASK) != 0
        )

    def is_packet_end_tag(self):
        """Checks if the tag is pack end tag."""
        return (
            (self.tag_members['flags'] & self.METADATA_PACKET_END_MASK) != 0
        )

    def is_time_to_play_tag(self):
        """Checks if the tag holds time to play information."""
        flags = self.tag_members['flags']
        return (
            ((flags & self.METADATA_TIMESTAMP_MASK) != 0) and
            ((flags & self.METADATA_TIME_OF_ARRIVAL_MASK) == 0)
        )

    def is_time_of_arrival_tag(self):
        """Checks if the tag holds time of arrival information."""
        flags = self.tag_members['flags']
        return (
            ((flags & self.METADATA_TIMESTAMP_MASK) == 0) and
            ((flags & self.METADATA_TIME_OF_ARRIVAL_MASK) != 0)
        )

    def is_void_tag(self):
        """Checks if the tag is void.

        Note:
            Samples belonging to a void tag should be played as soon as
            possible.
        """
        flags = self.tag_members['flags']
        return (
            ((flags & self.METADATA_TIMESTAMP_MASK) != 0) and
            ((flags & self.METADATA_TIME_OF_ARRIVAL_MASK) != 0) and
            (self.tag_members['timestamp'] == 0)
        )

    def is_empty_tag(self):
        """Checks if the tag is empty.

        Note:
            Empty tags are generated when an operator source without
            metadata support is connected to an operator sink with
            metadata support.
        """
        flags = self.tag_members['flags']
        return (
            ((flags & self.METADATA_TIMESTAMP_MASK) == 0) and
            ((flags & self.METADATA_TIME_OF_ARRIVAL_MASK) == 0)
        )

    def __str__(self):
        tag_str = self.tag.indent
        for tag_mem_name in self.tag_display_names.keys():
            tag_str += self.tag_display_names[tag_mem_name] + ':'
            if tag_mem_name == 'flags':
                # EMPTY, VOID, TOA and TTP tags are mutual exclusive.
                if self.is_empty_tag():
                    tag_str += 'EMP'
                elif self.is_void_tag():
                    tag_str += 'VOI'
                elif self.is_time_of_arrival_tag():
                    tag_str += 'TOA'
                elif self.is_time_to_play_tag():
                    tag_str += 'TTP'
                else:
                    # this tag is erroneous
                    tag_str += 'ERR'

                # Check for packet/stream start and end.
                if self.is_stream_start_tag():
                    tag_str += ',SS'
                if self.is_stream_end_tag():
                    tag_str += ',SE'
                if self.is_packet_start_tag():
                    tag_str += ',PS'
                if self.is_packet_end_tag():
                    tag_str += ',PE'
            else:
                tag_str += '0x%08x ' % (self.tag_members[tag_mem_name])

        return tag_str


class PrintBufferUsage(threading.Thread):
    """Reads and displays usage info about buffers.

        Args:
            exit_event (:obj:`threading.Event`): Exit event.
            cbuffers (:obj:`list` of :obj:`tupe`): Each tuple is
                (repr string/name, address).
            buffers_analysis (:obj:`Analysis.Buffers.Buffers`)
            exit_message (str) Exit message prompt.
            screen_width (int, optional): screen width in characters.
            live_plotting (bool, optional): Displays buffers.
    """

    def __init__(self, exit_event, cbuffers, buffers_analysis, exit_message,
                 screen_width=120, live_plotting=False):
        threading.Thread.__init__(self)
        self.exit_event = exit_event
        self.buffers = cbuffers
        self.buffers_analysis = buffers_analysis
        self.exit_message = exit_message
        self.screen_width = screen_width
        self.live_plotting = live_plotting

        # find the longest name to properly align the buffer usage table.
        max_direction_len = 0
        for cbuffer in cbuffers:
            # buffer[0] is the buffer name
            if max_direction_len < len(cbuffer[0]):
                max_direction_len = len(cbuffer[0])

        # add a space between the bar and the name
        max_direction_len += 1
        alignment = "{:<%d}" % max_direction_len
        self.alignment = alignment
        len_alignment_fomatted_str = len(self.alignment.format(" "))

        # The usage consists of two digits, the first is the number of
        # words which are occupying the buffer and the second one is the
        # number of available words.
        self.usage_formatter = " %-5d/%-5d "
        len_usage_formatted_str = len(self.usage_formatter % (0, 0))
        # We assume that the address will be 32 bit. This will add unnecessary
        # padding 0 for 24bit chips. If the chip is more than 32 bit the
        # alignment won't be correct.
        self.base_addr_formatter = "0x%-08x"
        len_base_addr_formatted_str = len(self.base_addr_formatter % (0))
        # In-place will be shortened to IP. People will confuse it with
        # Internet Protocol.
        self.in_place_formatter = " %2s"
        len_in_place_formatter = len(self.in_place_formatter % (""))
        self.progress_bar_len = (
            self.screen_width - (
                len_alignment_fomatted_str + len_base_addr_formatted_str +
                len_usage_formatted_str
            )
        )

        # warn the user if the screen is too small to display the table rows
        # in one line.
        self.warning = None
        if self.progress_bar_len < 10 + 2:
            self.progress_bar_len = 10
            self.warning = (
                "Screen width (%d characters) is too small to " +
                "display the buffer usage in one line!\n"
            ) % (self.screen_width)

        # Create the table header using the length of each formatter string.
        self.header = \
            (
                "{:<" + str(len_alignment_fomatted_str) + "}"
            ).format("Buffer Name") + \
            (
                "{:<" + str(self.progress_bar_len + 2) + "}"
            ).format("Status") + \
            (
                " {:<" + str(len_usage_formatted_str - 1) + "}"
            ).format("Usage") + \
            (
                "{:<" + str(len_base_addr_formatted_str) + "}"
            ).format("Base Addr") + \
            (
                "{:>" + str(len_in_place_formatter) + "}"
            ).format("IP") + "\n"

        self.total_size_formatter = (
            "{:<" + str(self.progress_bar_len + 2) + "}"
        )

        self.legend = (
            "\n\nAdditional information:\n"
            "* Size- Buffer sizes displayed in words. Note: the useful size\n"
            "        is with one word less due to tracking.\n"
            "* Buffer sizes and available space is displayed in words.\n"
            "* Base addr - Base address of the circular buffer.\n"
            "* IP - In place description column:\n"
            "      a) \"\" -  or nothing means that the buffer is not part of\n"
            "        an in-place chain.\n"
            "      b) \"I\" - The buffer is part of an in-place chain, but it\n"
            "        is not the head buffer.\n"
            "      c) \"IH\" - The buffer is the head of the in-place chain.\n"
            "        This means that the available space for this buffer is\n"
            "        calculated using the read offset of the tail buffer from\n"
            "        the in-place chain.\n"
            "* When calculating the total usage, in-pace buffers will count "
            "as\n"
            "one buffer.\n"
            "* The total available space is displayed in words.\n\n")

    def get_buffer_usage_table(self):
        """Returns the buffer usage table.

        Returns:
            String containing the formatted buffer usage table plus some
            information describing the table.
        """
        if not self.buffers:
            return "No transform buffers in the system."
        table_str = self.header
        table_str += "-" * (len(self.header) - 1) + "\n"
        # dictionary containing the buffer usage. Same in-place buffers will
        # only count once.
        dict_buffer = {}
        for buff in self.buffers:
            buffer_var = self.buffers_analysis.inspect_cbuffer(buff[1])
            # calculate the usage. Note: Available space is used to take in
            # account in-place buffers.
            usable_size = buffer_var.buffer_size - 1
            float_usage = float(
                usable_size - buffer_var.av_space
            ) / buffer_var.buffer_size
            usage = float_usage * self.progress_bar_len
            # Round up from 0.5
            usage = int(math.ceil(usage))

            # log the minimum available space for each buffer. Because in-place
            # buffers have the same base address each in-place chain will only
            # count as once. The head of the chain will contain the smallest
            # amount of space, but to avoid errors when the chain is not
            # complete calculate the minimum.
            buff_base_addr = buffer_var.get_base_address()
            if buff_base_addr not in dict_buffer:
                dict_buffer[buff_base_addr] = \
                    (buffer_var.av_space, buffer_var.buffer_size)
            else:
                if buffer_var.av_space < dict_buffer[buff_base_addr][0]:
                    dict_buffer[buff_base_addr] = \
                        (buffer_var.av_space, buffer_var.buffer_size)

            progress_bar = "[{0}{1}]".format(
                "#" * usage,
                " " * (self.progress_bar_len - usage)
            )
            used = buffer_var.buffer_size - buffer_var.av_space - 1
            available = buffer_var.buffer_size - 1

            table_str += self.alignment.format(buff[0])
            table_str += progress_bar
            table_str += self.usage_formatter % (used, available)
            table_str += self.base_addr_formatter % (buff_base_addr)

            if buffer_var.head_of_in_place_chain:
                table_str += self.in_place_formatter % ("IH")
            else:
                if buffer_var.in_place:
                    table_str += self.in_place_formatter % ("I")
                else:
                    table_str += self.in_place_formatter % ("")
            table_str += "\n"

        # -1 because of the new line
        table_str += "-" * (len(self.header) - 1) + "\n"

        sum_space = 0
        sum_size = 0
        for buffer_base_addr in dict_buffer:
            sum_space += dict_buffer[buffer_base_addr][0]
            sum_size += dict_buffer[buffer_base_addr][1]
        # the usable size is with one words less so remove one word for each
        # buffer
        usable_size = (sum_size - len(dict_buffer))
        total_usage = (float(usable_size - sum_space) / usable_size) * 100

        output = "Totals available space %d. Total size %d. "\
            "Overall usage %.2f%%." % (sum_space, sum_size, total_usage)
        table_str += self.alignment.format(output)
        # Add the legend at the end for non live mode.
        if not self.live_plotting:
            table_str += self.legend
        else:
            table_str += "\n\n"

        return table_str

    def run(self):
        """Starts the thread."""
        while not self.exit_event.is_set():
            new_output = ""
            if self.warning:
                new_output += self.warning
            new_output += self.get_buffer_usage_table()
            new_output += self.exit_message
            # print out the new statistic
            os.system('cls' if os.name == 'nt' else 'clear')
            sys.stdout.write(new_output)
            sys.stdout.flush()
            time.sleep(0.2)
