############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2012 - 2019 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
"""
Module to analyse the operator manager.
"""
import re
import time as import_time
import types

try:
    # pylint: disable=redefined-builtin
    from future_builtins import hex
except ImportError:
    pass

from . import Analysis
from ..Core import Arch
from ACAT.Core import CoreUtils as cu
from ACAT.Core.exceptions import (
    UsageError, AnalysisError, BundleMissingError,
    DebugInfoNoVariableError, InvalidDebuginfoTypeError
)

VARIABLE_DEPENDENCIES = {
    'strict': ('$_capability_data_table', '$_oplist_head')
}
ENUM_DEPENDENCIES = {'strict': ('STOP_KICK', 'OP_STATE', 'CAP_ID', 'OPCMD_ID')}
TYPE_DEPENDENCIES = {
    'OPERATOR_DATA': (
        'con_id', 'id', 'task_id', 'creator_client_id', 'cap_data', 'state',
        'stop_chain_kicks', 'kick_propagation_table', 'extra_op_data'
    ),
    'KP_TABLE': (
        'table_length', 'table', 'num_op_sources', 'num_op_sinks',
        'num_ep_sources', 'num_ep_sinks'
    ),
    'KP_ELEMENT': ('kt', 'kt.ep'),
    'BGINT': ('id',),
    'CAPABILITY_DATA': (
        'id', 'version_msw', 'version_lsw', 'max_sinks', 'max_sources',
        'handler_table', 'opmsg_handler_table', 'process_data',
        'instance_data_size'
    )
}

# Here's a quick sketch to aid digestion of the different terms below.
#
# +---------------------------+
# |          0x4040           |  <-- op_ep_id
# |                           |
# | +--------+    +--------+  |
# | | 0xe040 |    | 0x6040 |  |  <-- ep_ids
# | +--------+    +--------+  |
# |   Sink 0        Src 0     |  <-- port numbers
# +---------------------------+
#          operator 0            <-- opid


class Operator(object):
    """Encapsulates an operator.

    Args:
        op_entry
        helper
    """

    def __init__(self, op_entry, helper):
        # Operator variables are initialised from an existing ct.Variable
        # extracted from the oplist. Also takes an Analysis object as a
        # 'helper' so that we can make use of analysis fundamentals like
        # cast().

        # All of our info will come from the op_entry structure.
        # This has been cast from malloc'd data so members are difficult
        # to get at.
        self.helper = helper
        self.op_entry = op_entry  # For posterity

        # Connection ID
        self.con_id = op_entry.get_member('con_id').value

        # operator ID
        self.id = op_entry.get_member('id').value

        # Task ID
        self.task_id = op_entry.get_member('task_id').value

        # Creator client ID
        self.creator_client_id = op_entry.get_member('creator_client_id').value

        # Flag indicating running / stopped etc. state
        self.state = op_entry.get_member('state').value

        #  Field indicating the direction in which the kicks are ignored.
        self.stop_chain_kicks = helper.debuginfo.get_enum(
            'STOP_KICK', op_entry.get_member('stop_chain_kicks').value
        )[0]

        # Pointer to a next operator in a list, e.g. kept my OpMgr
        # next used for storing the operator data

        try:
            # logging_enabled doesn't exist on really old builds
            self.logging_enabled = op_entry.get_member('logging_enabled').value
        except AttributeError:
            self.logging_enabled = None

        try:
            # additional operator debug informations
            self.pio_num = op_entry.get_member('pio_num').value
            self.pio_pattern = op_entry.get_member('pio_pattern').value
        except AttributeError:
            self.pio_num = None
            self.pio_pattern = None

        # get the propagation table
        if op_entry.get_member("kick_propagation_table").value != 0:
            self.kpt_table = helper.chipdata.cast(
                addr=op_entry.get_member("kick_propagation_table").value,
                type_def="KP_TABLE",
                elf_id=helper.debuginfo.get_kymera_debuginfo().elf_id
            )

            amount = 0
            table_length = self.kpt_table.get_member("table_length").value
            table_address = self.kpt_table.get_member("table").address
            # the element size
            table_size = helper.debuginfo.get_type_info("KP_ELEMENT")[5]

            self.prop_table = []
            for i in range(0, table_length):
                self.prop_table.append(
                    helper.chipdata.
                    cast(table_address + i * table_size, "KP_ELEMENT")
                )

            num_op_sources = self.kpt_table.get_member("num_op_sources").value
            self.kicked_op_sources = self.prop_table[amount:amount + num_op_sources]
            amount += num_op_sources

            num_op_sinks = self.kpt_table.get_member("num_op_sinks").value
            self.kicked_op_sinks = self.prop_table[amount:amount + num_op_sinks]
            amount += num_op_sinks

            num_ep_sources = self.kpt_table.get_member("num_ep_sources").value
            self.kicked_ep_sources = self.prop_table[amount:amount + num_ep_sources]
            amount += num_ep_sources

            num_ep_sinks = self.kpt_table.get_member("num_ep_sinks").value
            self.kicked_ep_sinks = self.prop_table[amount:amount + num_ep_sinks]
            amount += num_ep_sinks

        # Some extra data needed by specific instance
        self.extra_op_data_address = op_entry.get_member('extra_op_data').value
        self.extra_op_data_type = None

        # Additional information for an operator

        # Operator endpoint IDs
        self.op_ep_id = Opmgr.get_opidep_from_opid(self.id)

        # Pointer to static capability data
        self.cap_data = helper.find_capability(
            op_entry.get_member('cap_data').value
        )

        if not helper.debuginfo.is_elf_loaded(self.cap_data.elf_id):
            # bundle not loaded bail out early.
            return

        # try to get the extra operator data type from the debug information.
        # If this returns None then we are dealing with older FW.
        self.extra_op_data_type = helper.debuginfo.get_cap_data_type(
            self.cap_data.name,
            self.cap_data.elf_id
        )

        # search for casting in the data process function.
        if self.extra_op_data_type is None:
            self.extra_op_data_type = \
                helper.get_extra_op_data_type(
                    self.cap_data.process_data,
                    self.cap_data.elf_id
                )

        # if not type cast found continue with the operator message handlers
        if self.extra_op_data_type is None:
            for handler_entry in self.cap_data.opmsg_handler_table:
                self.extra_op_data_type = \
                    helper.get_extra_op_data_type(
                        handler_entry[1],
                        self.cap_data.elf_id
                    )
                if self.extra_op_data_type is not None:
                    break

        # Continue with the operator command handlers (it is last because
        # many of these function are just a reuse from the basic operator).
        if self.extra_op_data_type is None:
            for handler_entry in self.cap_data.handler_table:
                self.extra_op_data_type = \
                    helper.get_extra_op_data_type(
                        handler_entry[1],
                        self.cap_data.elf_id
                    )
                if self.extra_op_data_type is not None:
                    break

        # Could be that the operator is reusing everything from the main
        # (kymera)build. This means that the extra_op_data_type should be
        # in the main build.
        if self.extra_op_data_type is None:
            self.extra_op_data_type = \
                helper.get_extra_op_data_type(
                    self.cap_data.process_data,
                    helper.debuginfo.get_kymera_debuginfo().elf_id
                )

        self.extra_op_data = None
        if self.extra_op_data_type is None:
            pass
        else:
            try:
                self.extra_op_data = helper.chipdata.cast(
                    self.extra_op_data_address,
                    self.extra_op_data_type,
                    elf_id=self.cap_data.elf_id
                )
            except InvalidDebuginfoTypeError:
                # probably the operator was patched
                self.extra_op_data_type = None

    def __get_kick_table_op_string(self, kick_table):
        mask = kick_table.get_member("t_mask").value
        task_id = self.helper.chipdata.cast(
            kick_table.get_member("kt").get_member("ep").value, "BGINT"
        ).get_member("id").value
        operator = self.helper.get_operator_by_taskid(task_id)

        result = "  Operator %s %s (mask: %s)\n" % (
            hex(operator.op_ep_id),
            operator.cap_data.name,
            bin(mask)
        )
        return result

    def __get_kick_table_ep_str(self, kick_table):
        mask = kick_table.get_member("t_mask").value
        ep_compact_name = self.helper.streams_analysis.get_endpoint_by_address(
            kick_table.get_member("kt").get_member("ep").value
        ).compact_name()
        return "  " + ep_compact_name + " (mask: " + bin(mask) + ')\n'

    def __get_raw_kick_table_string(self):
        kick_table_string = "+++++++++++++++++++++++++++\n"
        for prop in self.prop_table:
            kick_table_string += "--\n"
            kick_table_string += str(prop)
        kick_table_string += "+++++++++++++++++++++++++++\n"
        return kick_table_string

    def __str__(self):
        return self.title_str + '\n' + self.desc_str

    @property
    def title_str(self):
        """Returns the title string of the object."""
        return 'Operator ' + hex(self.id) + ', ep_op_id ' + \
            hex(self.op_ep_id) + ', ' + self.cap_data.name

    @property
    def desc_str(self):
        """Returns the description string of the object."""
        # Pylint thinks that _desc_str is not callable due to the @property
        retval = self._desc_str()  # pylint: disable=not-callable
        return retval

    def _channel_string(self):
        """Returns the common channel data description string."""
        mystr = ""
        channel_p = self.op_entry.get_member("cap_class_ext").value
        if channel_p != 0:
            channel = self.helper.chipdata.cast(channel_p, "MULTI_CHANNEL_DEF")
            mystr += (str(channel))

            # Remove the last line and first line. Note -2 is used because the
            # string terminates in \n
            mystr = "\n".join(mystr.split("\n")[1:-2])
            # Give a better title for the channel description
            mystr = "Multi channel data:\n" + mystr + "\n"

            first_channel = channel.get_member("first_active")
            if first_channel != 0:
                channels_str = ""
                channels = self.helper.parse_linked_list(
                    first_channel,
                    "next_active"
                )
                for chan in channels:
                    chanstr = str(chan)
                    # remove the first line
                    chanstr = "\n".join(chanstr.split("\n")[1:])
                    chanstr = insert_spaces(chanstr)
                    channels_str += insert_spaces("channel:\n" + chanstr)

                mystr += insert_spaces(channels_str)

            chan_obj_size = channel.get_member("chan_obj_size").value
            chan_data_address = channel.get_member("chan_data").value
            channel_data = self.helper.chipdata.get_data(
                chan_data_address,
                self.cap_data.max_sources * Arch.addr_per_word * chan_obj_size
            )
            channel_data_str = "[ "
            count = 1
            for val in channel_data:
                channel_data_str += "0x%08x" % val
                if count == len(channel_data):
                    channel_data_str += " ]"
                else:
                    if count % chan_obj_size == 0:
                        channel_data_str += ",\n  "
                    else:
                        channel_data_str += ", "
                count += 1

            channel_data_str = insert_spaces(channel_data_str)
            # And now print the channel data
            channel_data_str = "chan_data at 0x08%x:\n" % chan_data_address + \
                channel_data_str
            mystr += insert_spaces(channel_data_str + "\n")

        return mystr

    def _encoder_string(self):
        """Returns the common encoder description string."""
        mystr = ""
        encoder_p = self.op_entry.get_member("cap_class_ext").value
        if encoder_p != 0:
            encoder_param = self.helper.chipdata.cast(
                encoder_p, "ENCODER_PARAMS"
            )
            mystr += (str(encoder_param))

            # Remove the last line and first line. Note -2 is used because the
            # string terminates in \n
            mystr = "\n".join(mystr.split("\n")[1:-2])
            # Give a better title for the encoder_param description
            mystr = "Common encoder param:\n" + mystr + "\n"

        return mystr

    def _common_base_class_data(self):
        """Returns the common base class data string from cap_class_ext.

        This can be encoder or channel data dependent on the operator
        type.
        """
        # The current implementation is slightly fragile because it relies on
        # the capability name to decide if the operator is an encoder.
        # Currently celt, aptx and sbc encode are the only operators using
        # encoder common parameters. All the others operators based on the
        # base_op_multi_channel (dbe, vse and peq) are using the common base
        # class data to store channel information.
        if "encode" in self.cap_data.name.lower():
            return self._encoder_string()

        return self._channel_string()

    def _desc_str(self):
        mystr = 'Connection ID: ' + hex(self.con_id) + '\n'
        mystr += 'Task ID: ' + hex(self.task_id) + '\n'
        mystr += ('Creator client ID: ' +
                  hex(self.creator_client_id) + '\n')
        op_state = self.helper.debuginfo.get_enum('OP_STATE', self.state)[0]
        mystr += 'State: ' + op_state.replace("OP_", "") + '\n'
        mystr += 'Kicks ignored at %s \n' % self.stop_chain_kicks

        if self.logging_enabled is not None:
            mystr += 'logging_enabled: ' + str(self.logging_enabled) + '\n'
        if self.pio_num is not None and self.pio_num is not None:
            mystr += 'pio_num: ' + str(self.pio_num) + '\n'
            mystr += 'pio_pattern: ' + str(self.pio_pattern) + '\n'

        src_endpoints = []  # List of Stream.Endpoint objects
        sink_endpoints = []  # List of Stream.Endpoint objects
        for port in range(self.cap_data.max_sources):
            ep_id = Opmgr.ep_id_from_port_opid_and_dir(port, self.op_ep_id, 1)
            associated_ep = self.helper.streams_analysis.get_endpoint(ep_id)
            if associated_ep is not None:
                # associated_ep could be None if we've destroyed (or never
                # created) the Endpoint to go with this operator.
                src_endpoints.append(associated_ep)

        for port in range(self.cap_data.max_sinks):
            ep_id = Opmgr.ep_id_from_port_opid_and_dir(port, self.op_ep_id, 0)
            associated_ep = self.helper.streams_analysis.get_endpoint(ep_id)
            if associated_ep is not None:
                # associated_ep could be None if we've destroyed (or never
                # created) the Endpoint to go with this operator.
                sink_endpoints.append(associated_ep)

        mystr += str(len(src_endpoints)) + ' source endpoint(s): '
        for source in src_endpoints:
            mystr += (hex(source.key) + ', ')
        # Remove the last ", "
        mystr = mystr[:-2]
        mystr += ('\n')

        mystr += (str(len(sink_endpoints)) + ' sink endpoint(s): ')
        for sink in sink_endpoints:
            mystr += (hex(sink.key) + ', ')
        # Remove the last ", "
        mystr = mystr[:-2]
        mystr += ('\n')

        # Print out the raw table. For test only
        # mystr += self.__get_raw_kick_table_string()
        try:
            if self.kicked_op_sources:
                mystr += ("kicked operator sources (forward kicks):\n")
                for kick_table in self.kicked_op_sources:
                    mystr += self.__get_kick_table_op_string(kick_table)

            if self.kicked_ep_sources:
                mystr += ("kicked source endpoints (forward kicks):\n")
                for kick_table in self.kicked_ep_sources:
                    mystr += self.__get_kick_table_ep_str(kick_table)

            if self.kicked_op_sinks:
                mystr += ("kicked operator sinks (backward kicks):\n")
                for kick_table in self.kicked_op_sinks:
                    mystr += self.__get_kick_table_op_string(kick_table)

            if self.kicked_ep_sinks:
                mystr += ("kicked sink endpoints (backward kicks):\n")
                for kick_table in self.kicked_ep_sinks:
                    mystr += self.__get_kick_table_ep_str(kick_table)
        except BaseException:
            # the operator does not kick so just pass
            pass

        # print out the common base class data (cap_class_ext). If all the
        # debug information is loaded and ready.
        if self.cap_data.handler_table:
            mystr += (self._common_base_class_data())

        if self.extra_op_data_type is None:
            mystr += (
                "extra_op_data address: " + hex(self.extra_op_data_address)
            )
            elf_id = self.cap_data.elf_id
            if self.helper.debuginfo.is_elf_loaded(elf_id):
                mystr += (
                    "\nUse the following command to properly display the"
                    " operator data structure"
                )
                mystr += (
                    '\nopmgr.set_extra_op_data_type( %s,\n\t\"<the type '
                    'used to cast '
                    'op_data->extra_op_data to>\")' % hex(self.op_ep_id)
                )
            else:
                mystr += "\nCannot display operator.\n"\
                    "Bundle with elf id {0} is missing!\n"\
                    "Use -j option or load_bundle(r\"<path>\") in "\
                    "interactive "\
                    "mode to add bundles to ACAT!".format(hex(elf_id))

        else:
            mystr += str(self.extra_op_data)
        return mystr

    def cast_extra_data(self, type_str):
        """Sets the extra operator data type for an operator.

        Args:
            type_str (str): Type string for example "RTP_DECODE_OP_DATA".
        """
        self.extra_op_data_type = type_str
        self.extra_op_data = self.helper.chipdata.cast(
            self.extra_op_data_address, type_str, elf_id=self.cap_data.elf_id
        )


def insert_spaces(input_string):
    """Inserts spaces before each line for a given string."""
    return "  " + "  ".join(input_string.splitlines(True))


def source_sync_str(self):
    """Descriptor string function for the source sync capability."""
    try:
        op_str = Operator._desc_str(self)

        sinks = self.extra_op_data.get_member("sinks")
        replace_str = "sinks: \n"
        for sink in sinks.value:
            if sink != 0:
                replace_str += insert_spaces(
                    str(
                        self.helper.chipdata.cast(sink, "SRC_SYNC_SINK_ENTRY")
                    )
                )
        op_str = op_str.replace(
            sinks.var_to_str(1), insert_spaces(replace_str)
        )

        sources = self.extra_op_data.get_member("sources")
        replace_str = "sources: \n"
        for source in sources.value:
            if source != 0:
                replace_str += insert_spaces(
                    str(
                        self.helper.chipdata.
                        cast(source, "SRC_SYNC_SOURCE_ENTRY")
                    )
                )
        op_str = op_str.replace(
            sources.var_to_str(1), insert_spaces(replace_str)
        )

        sink_groups = self.extra_op_data.get_member("sink_groups")
        replace_str = "sink_groups: \n"
        parsed_sing_groups = self.helper.parse_linked_list(
            sink_groups,
            "common.next",
            sink_groups.type
        )
        for sink_group in parsed_sing_groups:
            replace_str += insert_spaces(str(sink_group))
            rm_state_p = sink_group.get_member("rm_state").value
            if rm_state_p != 0:
                rm_state = self.helper.chipdata.cast(
                    rm_state_p, "SRC_SYNC_RM_STATE"
                )
                replace_str += insert_spaces(insert_spaces(str(rm_state)))
        op_str = op_str.replace(
            sink_groups.var_to_str(1), insert_spaces(replace_str)
        )

        source_groups = self.extra_op_data.get_member("source_groups")
        replace_str = "source_groups: \n"
        parsed_source_groups = self.helper.parse_linked_list(
            source_groups, "common.next",
            source_groups.type
        )
        for source_group in parsed_source_groups:
            replace_str += insert_spaces(str(source_group))
        op_str = op_str.replace(
            source_groups.var_to_str(1), insert_spaces(replace_str)
        )

        return op_str
    except BaseException:
        return Operator._desc_str(self)


def get_data_octet_based(buffers, buffer_p, usable_octets):
    """Get data Octet based in a list."""
    return_list = []
    for i in buffers.get_content(buffer_p, "read"):
        if usable_octets == 4:
            return_list.append((i >> 24) & 0xff)
            return_list.append((i >> 16) & 0xff)
        return_list.append((i >> 8) & 0xff)
        return_list.append(i & 0xff)

    return_list.reverse()
    return return_list


def rtp_str(self):
    """Descriptor string function for the splitter capability."""
    try:
        op_str = Operator._desc_str(self)

        buffers = self.helper.interpreter.get_analysis(
            "buffers", self.helper.chipdata.processor
        )
        # Display all internal buffers. The input and output should already
        # be displayed in stream analysis.

        clone_frame_buffer = self.extra_op_data.get_member(
            "clone_frame_buffer"
        ).value
        op_str += "\n clone_frame_buffer:\n"
        buff_var = buffers.inspect_cbuffer(clone_frame_buffer, True)
        op_str += insert_spaces(str(buff_var))
        op_str += cu.mem_dict_to_string(buffers._get_content(buff_var))

        frame_buffer = self.extra_op_data.get_member("frame_buffer").value
        op_str += "\n frame_buffer:\n"
        buff_var = buffers.inspect_cbuffer(frame_buffer, True)
        op_str += insert_spaces(str(buff_var))
        op_str += cu.mem_dict_to_string(buffers._get_content(buff_var))

        internal_buffer = self.extra_op_data.get_member(
            "internal_buffer"
        ).value
        op_str += "\n internal_buffer:\n"
        buff_var = buffers.inspect_cbuffer(internal_buffer, True)
        op_str += insert_spaces(str(buff_var))
        op_str += cu.mem_dict_to_string(buffers._get_content(buff_var))

        return op_str
    except AttributeError:
        return Operator._desc_str(self)


def splitter_str(self):
    """Descriptor string function for the splitter capability."""
    try:

        buffers = self.helper.interpreter.get_analysis(
            "buffers", self.helper.chipdata.processor
        )

        op_str = ""
        channels = self.extra_op_data.get_member("channel_list")

        # check the metadata first
        internal_metadata = self.extra_op_data.get_member(
            "internal_metadata"
        )
        internal_metadata_buffer = internal_metadata.get_member(
            "buffer"
        ).value
        if internal_metadata_buffer != 0:
            metadata_read_index_output = internal_metadata.get_member(
                "prev_rd_indexes"
            )
            # get the proper variable
            internal_metadata_buffer = self.helper.chipdata.cast(
                internal_metadata_buffer,
                "tCbuffer"
            )
            # don't handle errors here as this buffer must have metadata!
            metadata = internal_metadata_buffer.get_member("metadata").value
            metadata = self.helper.chipdata.cast(metadata, "metadata_list")
            metadata_size = metadata.get_member('buffer_size').value
            metadata_write_index = metadata.get_member('prev_wr_index').value
            # ok now display the buffer usage.
            for output_index in range(2):
                data = metadata_write_index - metadata_read_index_output[output_index].value
                if data < 0:
                    data += metadata_size
                percent = (float(data) * 100) / metadata_size
                op_str += "Channel %d : usage %3.2f%% data %d size %d\n" % (
                    output_index, percent, data, metadata_size
                )
            op_str += "Internal Metadata buffer"
            op_str += insert_spaces(str(internal_metadata_buffer))
            metadata_str = buffers._analyse_buffer_metadata(internal_metadata_buffer)
            op_str += insert_spaces(str(metadata_str))

        for channel in self.helper.parse_linked_list(channels, "next"):
            input_buff = channel.get_member("input_buffer").value
            channel_id = channel.get_member("id").value
            op_str += "Channel %d info \n" % channel_id
            op_str += str(channel)
            if input_buff != 0:
                op_str += "\nchannel %d input:\n" % channel_id
                # Note: this could be ext_buffer.
                cbuffer_var = self.helper.chipdata.cast(input_buff, "tCbuffer")
                op_str += insert_spaces(str(cbuffer_var))
                buff_var = buffers.inspect_cbuffer(input_buff, True)
                op_str += insert_spaces(str(buff_var))
                # the input buffers are transform buffers so content should
                # already be displayed
                # op_str += cu.mem_dict_to_string(buffers._get_content(buff_var))

            internal_buff = channel.get_member("internal").get_member("buffer").value

            if internal_buff != 0:
                op_str += "channel %d internal buffer:\n" % channel_id
                buff_var = buffers.inspect_cbuffer(internal_buff, True)
                cbuffer_var = self.helper.chipdata.cast(
                    internal_buff, "tCbuffer"
                )
                op_str += insert_spaces(str(cbuffer_var))
                base_addr = cbuffer_var.get_member("base_addr").value
                op_str += insert_spaces(
                    "valid address range 0x%08x = 0x%08x\n\n" % (
                        base_addr,
                        base_addr + cbuffer_var.get_member("size").value - 1
                    )
                )
                op_str += insert_spaces(str(buff_var))
                # Display the content of the internal buffer.
                op_str += cu.mem_dict_to_string(buffers._get_content(buff_var))

            op_str += "\n"
            output_buffers = channel.get_member("output_buffer")
            for index, output_buffer in enumerate(output_buffers):
                output_buff = output_buffer.value
                if output_buff != 0:
                    op_str += "channel %d output buffer[%d]:\n" % (channel_id, index)
                    cbuffer_var = self.helper.chipdata.cast(output_buff, "tCbuffer")
                    op_str += insert_spaces(str(cbuffer_var))
                    buff_var = buffers.inspect_cbuffer(output_buff, True)
                    op_str += insert_spaces(str(buff_var))
                    # the output buffers are transform buffers so content should
                    # already be displayed.
                    # op_str += cu.mem_dict_to_string(buffers._get_content(buff_var))

        return Operator._desc_str(self) + "Additional Info:\n" + insert_spaces(op_str)

    except:  # Any error? just fall back to the standard display.
        return Operator._desc_str(self)


class Capability(object):
    """Class to encapsulate a capability.

    Args:
        cap_entry
        helper
        cap_elf_id
    """

    def __init__(self, cap_entry, helper, cap_elf_id):
        self.helper = helper
        self.address = cap_entry.address
        # if this capability is from downloaded capabilities, store the file id
        self.elf_id = cap_elf_id
        # Capability ID
        self.id = cap_entry.get_member('id').value

        recheck_name = False
        # Capability name
        try:
            # try to get the name from the built in capability names
            self.name = helper.debuginfo.get_enum('CAP_ID', self.id)[
                0].replace("CAP_ID_", "")
        except KeyError:
            # Not a built in capability. Capability IDs are allocated as
            # follows:
            #
            # 0x0000-0x3FFF capabilities built in to a Kymera image.
            # 0x4000-0x7FFF Qualcomm-provided downloadable capabilities.
            # 0x8000-0xBFFF are internally reserved IDs.
            # 0xC000-0xC0FF are for private use in customer projects.
            # 0xC100-0xFFFF Qualcomm eXtension programme partners.
            if self.id >= 0x4000 and self.id <= 0x7FFF:
                self.name = "(Qualcomm_Provided_Capability:%s)" % hex(self.id)
            elif self.id >= 0x8000 and self.id <= 0xBFFF:
                # Probably these IDs will never be used.
                self.name = "(Reserved_Capability:%s)" % hex(self.id)
            elif self.id >= 0xC000 and self.id <= 0xC0FF:
                self.name = "(Customer_Capability:%s)" % hex(self.id)
            elif self.id >= 0xC100 and self.id <= 0xFFFF:
                self.name = "(Qualcomm_eXtension_Capability:%s)" % hex(self.id)
            else:
                # could be any downloaded capability
                self.name = "(Downloaded_Capability:%s)" % hex(self.id)
            # try to better guess the name later.
            recheck_name = True

        # Capability version
        # do the masking
        msw = cap_entry.get_member('version_msw').value
        lsw = cap_entry.get_member('version_lsw').value
        self.version = (msw << 16) + lsw

        # max sinks and source
        self.max_sinks = cap_entry.get_member('max_sinks').value
        self.max_sources = cap_entry.get_member('max_sources').value

        self.process_data = cap_entry.get_member('process_data').value

        self.instance_data_size = cap_entry.get_member(
            'instance_data_size'
        ).value

        # Check if the elf file is loaded to the system. Bail out early if not.
        if not helper.debuginfo.is_elf_loaded(cap_elf_id):
            self.opmsg_handler_table = []
            self.handler_table = []
            return

        handler_table = helper.chipdata.get_var_strict(
            cap_entry.get_member('handler_table').value, cap_elf_id
        )
        # If the capability name is still unknown try to guess it from the
        # handle table name.
        if recheck_name:
            if "_handler_table" in handler_table.name:
                # Use the handle table name as capability name
                guessed_name = handler_table.name
                # remove the beginning
                guessed_name = guessed_name.replace("$_", "")
                # remove the ending
                guessed_name = guessed_name.replace("_handler_table", "")
                # make it upper case
                guessed_name = guessed_name.upper()
                # finally mark it as downloaded
                guessed_name = "DOWNLOADED_" + guessed_name

                self.name = guessed_name

        self.handler_table = []
        type_id = helper.debuginfo.get_kymera_debuginfo().types[
            helper.debuginfo.get_kymera_debuginfo().get_type_info(
                'handler_lookup_struct')[1]
        ].ref_type_id

        members = helper.debuginfo.get_kymera_debuginfo().types[type_id].members
        for index, value in enumerate(members):
            self.handler_table.append(
                [index, handler_table.get_member(value.name).value]
            )

        try:
            opmsg_handler_table = helper.chipdata.get_var_strict(
                cap_entry.get_member('opmsg_handler_table').value, cap_elf_id
            )
        except Exception:
            # the capability may reuse a handle table from kymera
            opmsg_handler_table = helper.chipdata.get_var_strict(
                cap_entry.get_member('opmsg_handler_table').value,
                helper.debuginfo.get_kymera_debuginfo().elf_id
            )

        self.opmsg_handler_table = [
            handler_entry.value for handler_entry in opmsg_handler_table
            if handler_entry.value[0] != 0
        ]

    def __str__(self):
        return self.title_str + '\n' + self.desc_str

    @property
    def title_str(self):
        """Returns the title string of the object."""
        return self.name + ' id: ' + hex(self.id)

    @property
    def desc_str(self):
        """Returns the description string of the object."""
        mystr = ('version: ' + hex(self.version) + '\n')
        mystr += ('max_sinks: ' + str(self.max_sinks) + '\n')
        mystr += ('max_sources: ' + str(self.max_sources) + '\n')
        mystr += ('instance_data_size: ' + str(self.instance_data_size) + '\n')
        try:
            process_data_name = self.helper.debuginfo.get_source_info(
                self.process_data
            ).module_name
        except BundleMissingError:
            # Bundle not loaded. Bail out early.
            process_data_name = " - Bundle missing. Cannot display handler.\n"

        mystr += ('handler_table: \n')
        for handler in self.handler_table:
            mystr += (
                " " * 2 +
                self.helper.debuginfo.get_enum('OPCMD_ID', handler[0])[0]
            )
            try:
                mystr += (
                    " - " + self.helper.debuginfo.get_source_info(handler[1])
                    .module_name + '\n'
                )
            except BundleMissingError:
                mystr += (" - Bundle missing. Cannot display handler.\n")

        mystr += ('opmsg_handler_table: \n')
        for handler in self.opmsg_handler_table:
            #  unfortunately the message IDs are in separate enumerations
            mystr += ("  0x" + format(handler[0], '04x'))
            try:
                mystr += (
                    " - " + self.helper.debuginfo.get_source_info(handler[1])
                    .module_name + '\n'
                )
            except BundleMissingError:
                mystr += (" - Bundle missing. Cannot display handler.\n")

        mystr += ('process_data: ' + process_data_name + '\n')
        return mystr


def operator_factory(operator_var, analysis):
    """Operator creator function."""
    operator = Operator(operator_var, analysis)

    if operator.cap_data.name == "SOURCE_SYNC":
        # use special descriptor function for source sync capability
        operator._desc_str = types.MethodType(source_sync_str, operator)

    if operator.cap_data.name in ("SPLITTER", "DOWNLOAD_SPLITTER"):
        # use special descriptor function for source sync capability
        operator._desc_str = types.MethodType(splitter_str, operator)

    if operator.cap_data.name in ("RTP_DECODE", "DOWNLOAD_RTP_DECODE"):
        # use special descriptor function for source sync capability
        operator._desc_str = types.MethodType(rtp_str, operator)

    return operator


class Opmgr(Analysis.Analysis):
    """Encapsulates analysis for operators.

    Args:
        **kwarg: Arbitrary keyword arguments.
    """

    def __init__(self, **kwarg):
        # Call the base class constructor.
        Analysis.Analysis.__init__(self, **kwarg)
        self.op_list_object = []
        # We need a Streams analysis so that we can look up endpoint/transform
        # information. Give it a null formatter though, just to make sure it
        # doesn't start outputting stuff unbidden.
        try:
            self.streams_analysis = self.interpreter.get_analysis(
                "stream", self.chipdata.processor
            )
        except KeyError:
            raise AnalysisError(
                "OPMGR analysis doesn't work without Stream analysis.  "
                "Please check the available analyses and make sure that "
                "the Stream is ahead of OPMGR."
            )

    @staticmethod
    def get_opid_from_ep_id(ep_id):
        """Returns the opid, given an endpoint id.

        If passed something that was already an opid, will return the
        original value.

            e.g. 'ep_id' = 0x6040 -> opid = 1
                 'ep_id' = 2 -> opid = 2

        Args:
            ep_id
        """
        # All of this stuff is defined in opmgr.c and needs to be kept in
        # sync with it if anything ever changes.
        # static unsigned int get_opid_from_ep(unsigned int opidep)
        # {
        #    return ((opidep & STREAM_EP_OPID_MASK) >> STREAM_EP_OPID_POSN);
        # }
        # define STREAM_EP_OPID_MASK  0x1fc0
        # define STREAM_EP_OPID_POSN  6
        real_opid = (ep_id & 0x1fc0) >> 6
        # opid wraps if (id > (STREAM_EP_OPID_MASK >> STREAM_EP_OPID_POSN))
        if ep_id < (0x1fc0 >> 6):
            # User actually supplied a real opid
            real_opid = ep_id
        return real_opid

    @staticmethod
    def get_opidep_from_opid(opid):
        """Returns the operator endpoint id (op_ep_id), given an opid.

        If passed something that was already an ep_id, will return the
        op_ep_id equivalent.

            e.g. 'opid' = 1 -> op_ep_id = 0x4040
                 'opid' = 0xe040 -> op_ep_id = 0x4040
                 'opid' = 0x4040 -> op_ep_id = 0x4040

        Args:
            opid
        """
        # All of this stuff is defined in opmgr.c and needs to be kept in
        # sync with it if anything ever changes.
        # define STREAM_EP_SINK_BIT  0x8000
        # define STREAM_EP_OP_BIT    0x4000
        # define STREAM_EP_EP_BIT    0x2000
        # static unsigned int get_opidep_from_opid(unsigned int opid)
        # {
        #    return (((opid << STREAM_EP_OPID_POSN) & STREAM_EP_OPID_MASK) |
        #            STREAM_EP_OP_BIT );
        # }
        # See above for where the other magic numbers come from
        if opid & 0x4000 != 0:
            # Already an ep_id, convert to op_ep_id
            return opid & ~0xA000
        return ((opid << 6) & 0x1fc0) | 0x4000

    @staticmethod
    def ep_id_from_port_opid_and_dir(port, op_ep_id, direction):
        """Returns EP ID.

        Takes a port number (e.g. 0), op_ep_id (e.g. 0x4040) and direction
        (0 for sink, 1 for source).

        Args:
            port
            op_ep_id
            direction
        """
        # static unsigned int get_port_from_opidep(unsigned int opidep)
        # {
        #    return ((opidep & STREAM_EP_CHAN_MASK) >> STREAM_EP_CHAN_POSN);
        # }
        # define STREAM_EP_CHAN_MASK  0x003f
        # define STREAM_EP_CHAN_POSN  0
        # See above for where the other magic numbers come from.
        ep_id = op_ep_id | 0x2000  # Set 'is endpoint' bit
        if not direction:
            ep_id = ep_id | 0x8000  # Set 'is sink' bit
        ep_id = ep_id | port
        return ep_id

    def run_all(self, detailed=True):
        """Perform analysis and spew the output to the formatter.

        It analyses all the operators and the available capabilities.

        Args:
            detailed (bool, optional): Detailed capability display.
        """
        self.formatter.section_start('Opmgr')
        self.analyse_all_operators()
        self.analyse_all_capabilities(detailed)
        self.formatter.section_end()

    def get_operator(self, opid):
        """Returns the operator (an Operator object) that has ID 'opid'.

        Note:
            'operator ID' can be an actual opid (1, 2, etc.) or an
            operator ep_id (0x4040, 0xe040) - we'll infer which one it is.
            If force_read == True, the op_list variable is read again (if
            contents have changed).

        Args:
            opid (int)

        Returns:
            None: If opid was not found.
        """
        real_opid = Opmgr.get_opid_from_ep_id(opid)
        self.op_list_object = self.get_oplist('object')
        for operator in self.op_list_object:
            if operator.id == real_opid:
                return operator

        raise AnalysisError("No operator found with id: " + hex(opid))

    def set_extra_op_data_type(self, opid, type_string):
        """Sets the typed of the extra_op_data field for an operator.

        Args:
            opid (int): Operator ID.
            type_str (str): Type string for example "RTP_DECODE_OP_DATA".
        """
        operator = self.get_operator(opid)
        operator.cast_extra_data(type_string)

        return operator

    def get_operator_by_taskid(self, taskid):
        """Returns the operator (an Operator object) that has a given Task ID.

        Args:
            taskid
        """
        self.op_list_object = self.get_oplist('object')
        taskid = taskid & 0x1FFFFF
        for operator in self.op_list_object:
            if (operator.task_id & 0x1FFFFF) == taskid:
                return operator

        return None

    @Analysis.cache_decorator
    def get_oplist(self, mode='id'):
        """Returns a list of all operators in opmgr's oplist.

        Args:
            mode (str, optional): Selects which information to return.
              'entry' - a list of op_entry elements.
              'id' - a list of operator IDs.
              'object' - a list of Operator objects (NB: could be slow).
              'name' - a list of operator names and their ids.

        Raises:
            UsageError: Invalid oplist mode.
        """
        op_entries = self._read_raw_oplist()

        if mode == 'entry':
            return op_entries
        elif mode == 'id':
            return [op.get_member('id').value for op in op_entries]
        elif mode == 'object':
            return [operator_factory(op, self) for op in op_entries]
        elif mode == 'name':
            return_list = []
            for operator in op_entries:
                operator = operator_factory(operator, self)
                return_list.append(
                    operator.cap_data.name +
                    " " +
                    hex(
                        operator.op_ep_id))
            return return_list
        else:
            raise UsageError('Invalid oplist mode')

    @Analysis.cache_decorator
    def _read_raw_oplist(self):
        return [op for op in self.parse_linked_list("$_oplist_head", "next")]

    @Analysis.cache_decorator
    def get_capability_list(self):
        """Returns a list of all capability data."""
        # read the capability table
        capability_data_table = self.chipdata.get_var_strict(
            "$_capability_data_table"
        )
        # the capability table is null terminated and we know the exact size
        # print "capability_data_table =", hex(capability_data_table.value)
        capability_length = (
            capability_data_table.size - 1
        ) // Arch.addr_per_word

        if capability_length == 0:
            raise AnalysisError("Invalid capability table")

        capabilities = [
            Capability(
                self.chipdata.cast(
                    capability_data_table.value[count], "CAPABILITY_DATA"
                ), self, self.debuginfo.get_kymera_debuginfo().elf_id
            ) for count in range(capability_length)
        ]

        # Now deal with downloaded capabilities if they exist
        downloaded_capabilites_entries = self._get_downloaded_capabilities()
        if downloaded_capabilites_entries is not None:
            for entry in downloaded_capabilites_entries:
                # for now there is no direct way of determining the
                # capability id for capability entry, so we rely on
                # extracting that information from the mapped table.
                # For downloaded capablilities at least one (probably all
                # of them) of the members below should be mapped however
                # for the sake of safety check them all
                kymera_elf_id = self.debuginfo.get_kymera_debuginfo().elf_id
                cap_elf_id = kymera_elf_id

                temp_elf_id = self.debuginfo.table.get_elf_id_from_address(
                    entry.get_member('handler_table').value
                )
                if temp_elf_id and temp_elf_id != kymera_elf_id:
                    cap_elf_id = temp_elf_id

                temp_elf_id = self.debuginfo.table.get_elf_id_from_address(
                    entry.get_member('opmsg_handler_table').value
                )
                if temp_elf_id and temp_elf_id != kymera_elf_id:
                    cap_elf_id = temp_elf_id

                temp_elf_id = self.debuginfo.table.get_elf_id_from_address(
                    entry.get_member('process_data').value
                )
                if temp_elf_id and temp_elf_id != kymera_elf_id:
                    cap_elf_id = temp_elf_id

                cap = Capability(entry, self, cap_elf_id)

                capabilities.append(cap)
        return capabilities

    def _get_downloaded_capabilities(self):
        """Looks how many downloaded capability is loaded in the system.

        Note:
            A bundle can contain multipe downloadable capabilities.

        Returns:
            cap_entries The available downloadable capabilities.
        """
        cap_entries = []
        try:
            head = self.chipdata.get_var_strict(
                '$_cap_download_data_list_shared'
            ).value
        except DebugInfoNoVariableError:
            # if previous variable not found that means single core build is
            # used
            try:
                head = self.chipdata.get_var_strict(
                    '$_cap_download_data_list_aux'
                ).value
            except DebugInfoNoVariableError:
                # if this value is not found older build is probably used
                # without support for downloaded capabilities.
                return None
        # if variable exist but it points to NULL, no capabilities are present
        if head == 0:
            return None
        database = self.chipdata.cast(head, 'DOWNLOAD_CAP_DATA_DB')
        for database_entry in self.parse_linked_list(database, 'next'):
            cap_entry = self.chipdata.cast(
                database_entry.get_member('cap').value, 'CAPABILITY_DATA'
            )
            cap_entries.append(cap_entry)
        return cap_entries

    def find_capability(self, address):
        """Search for the capability data by address.

        Args:
            address
        """
        capabilities = self.get_capability_list()
        for capability in capabilities:
            if capability.address == address:
                return capability

        # there was no way that this could be known before so try to create the
        # capability now
        capability = Capability(
            self.chipdata.cast(
                address, "CAPABILITY_DATA"
            ), self, None
        )
        self.formatter.alert(capability.name + " is a patched capability.")
        return capability

    @Analysis.cache_decorator
    def get_extra_op_data_type(self, function_address, elf_id):
        """Finding the casting of extra_op_data.

        It takes the function address and searches for the nearest label
        in the listing file in order to find the casting of extra_op_data.

        Args:
            function_address
            elf_id: The bundle elf id where the capability can be found.
        """
        address_in_dowload_cap = False
        if elf_id != self.debuginfo.get_kymera_debuginfo().elf_id:
            if not self.debuginfo.table.is_addr_in_table(function_address):
                # if we are searching for a type in downloaded capability,
                # we only check the addresses that are mapped from file to
                # coredump. This is not always the case since some entries
                # in handler table and opmsg handler table are referred to
                # original lst file. It happens when their handlers inherit
                # from other  capabilities such as base_op
                # In the later case we are sure that type will not be found
                # from this address thus return None
                return None
            else:
                # the given address is in a downloadable capability, therefore
                # when searching in the .lst file we need to convert the address
                # to capability address.
                address_in_dowload_cap = True

        mmap_lst = self.debuginfo.get_mmap_lst(elf_id)
        if not mmap_lst:
            # the .lst file is missing.
            self.formatter.alert(
                "One of the listing .lst file is missing "
                "which is needed for pre MAP_INSTANCE_DATA capabilities."
            )
            return None

        # adjust the function address and convert it to hex string
        try:
            nearest_label = self.debuginfo.get_nearest_label(function_address)
        except BundleMissingError:
            nearest_label = None

        if nearest_label is not None:
            func_address = nearest_label.address
            if address_in_dowload_cap:
                # Convert the address to capability address (file address)
                # as the function address is in the downloaded table.
                func_address = self.debuginfo.table.convert_addr_to_download(
                    func_address, elf_id
                )
            func_address = hex(func_address)
        else:
            func_address = hex(function_address)

        func_address = func_address.replace("0x", "").encode('utf-8')
        # get the first match of the address
        address_pos = mmap_lst.find(func_address + b":")

        if address_pos == -1:
            # Address not found. Probably it's in another build
            return None

        mmap_lst.seek(address_pos)

        current_line = ""
        start_time = import_time.time()
        gurard_time = 2  # sec

        # until the nearest label is our function search for cast or timeout.
        while import_time.time() - start_time < gurard_time:
            # read new line from correct file opened
            current_line = mmap_lst.readline()
            # An rts instruction means that we are out of the original function
            if b"rts;" in current_line:
                return None

            # filter out lines which has no relation to the extra_op_data.
            if b"extra_op_data" in current_line:
                # regular expression to search for casting of the extra_op_data
                cast_type = re.search(
                    b'\=[\ ]*\([\ ]*([A-Za-z0-9\_]*)[\ ]*\*[\ ]*\).*->[\ ]*extra_op_data.*\;',
                    current_line
                )
                if cast_type is not None:
                    type_name = cast_type.group(1).decode()
                    return type_name
            elif b"get_instance_data" in current_line:
                # regular expression to search for casting of the extra_op_data
                # which is done using get_instance_data
                cast_type = re.search(
                    b'[\ ]*([A-Za-z0-9\_]*)[\ ]*\*[\ ]*[A-Za-z0-9\_]*[\ ]*\=[\ ]*get_instance_data',
                    current_line
                )
                if cast_type is not None:
                    type_name = cast_type.group(1).decode()
                    return type_name

        return None

    #######################################################################
    # Analysis methods - public since we may want to call them individually
    #######################################################################

    def analyse_all_operators(self):
        """Displays all the operators in use."""
        self.formatter.section_start('Operators in use')
        self.op_list_object = self.get_oplist('object')
        for operator in self.op_list_object:
            self.formatter.section_start(operator.title_str)
            self.formatter.output(operator.desc_str)
            self.formatter.section_end()
        self.formatter.section_end()

    def analyse_all_capabilities(self, detailed=True):
        """Displays all the available capabilities.

        Args:
            detailed (bool, optional): Detailed capability display.
        """
        self.formatter.section_start('Available capabilities')
        capabilities = self.get_capability_list()
        for cap in capabilities:
            if detailed:
                self.formatter.section_start(cap.title_str)
                self.formatter.output(cap.desc_str)
                self.formatter.section_end()
            else:
                self.formatter.output(cap.name)
        self.formatter.section_end()

    #######################################################################
    # Private methods - don't call these externally.
    #######################################################################
