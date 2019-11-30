############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2014 - 2019 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
"""
Module responsible for analysing a cbops graph.
"""
from . import Analysis
from ACAT.Core import Arch
from ACAT.Core.CoreUtils import dict_to_ordered_dict
from ACAT.Core.exceptions import (
    OutdatedFwAnalysisError, BundleMissingError, InvalidDebuginfoEnumError
)

TYPE_DEPENDENCIES = {
    "cbops_mgr": ("graph", "req_ops", "dir"),
    "cbops_buffer": (
        "type", "buffer", "base", "size", "rw_ptr", "transfer_ptr",
        "transfer_amount"
    ),
    "cbops_functions": ("amount_to_use", "process"),
    "cbops_param_hdr": ("operator_data_ptr", "nr_inputs", "nr_outputs"),
    "cbops_op": (
        "prev_operator_addr", "next_operator_addr", "function_vector"
    ),
    "cbops_graph": ("first", "last", "num_io")
}


class Cbops(Analysis.Analysis):
    """Provides a capability for analysing a cbops chain/graph.

        Args:
            **kwarg: Arbitrary keyword arguments.
    """
    # flags for cbops operation
    CBOPS_FLAGS = (
        "DC_REMOVE",  # 0
        "U_LAW",  # 1
        "A_LAW",  # 2
        "RATEADJUST",  # 3
        "SHIFT",  # 4
        "MIX",  # 5
        "SPLIT",  # 6
        "UPSAMPLE",  # 7
        "DISCARD",  # 8
        "UNDERRUN"  # 9
    )

    # the structure of cbops operator is searched using the label of
    # it's processing function. Each cbops operator type needs to
    # define its unique main or amount to use function in the dictionary below,
    # if not found in this dictionary the operator specific data will not be
    # analysed
    CBOPS_OPERATORS = {
        "$cbops.copy_op.main": None,
        "$cbops.dc_remove.main": "cbops_dc_remove",
        "$cbops.dc_remove2.main": "cbops_dc_remove", # label is used in patches only
        "$cbops.g711.main": "cbops_g711",
        "$cbops.rate_adjustment_and_shift.main": "cbops_rate_adjustment_and_shift",
        "$cbops.shift.main": "cbops_shift",
        "$cbops.discard_op.main": "cbops_discard_op",
        "$cbops.underrun_comp_op.main": "cbops_underrun_comp_op",
        "$cbops.iir_resamplev2.main": "cbops_iir_resampler_op",
        "$cbops.sidetone_filter_op.main": "cbops_sidetone_filter_op",
        "$cbops.sink_overflow_disgard_op.amount_to_use": "cbops_sink_overflow_disgard_op",
        "$cbops.rate_monitor_op.main": "cbops_rate_monitor_op",
        "$cbops.aec_ref_spkr_op.amount_to_use": "cbops_aec_ref_op",
        "$cbops.mixer_op.main": "cbops_mixer_op",
        "$cbops.aec_ref_latency_speaker.main": "latency_op",
        "$cbops.aec_ref_latency_speaker.amount_to_use": "latency_op",
        "$cbops.aec_ref_latency_mic.main": "latency_op",
        "$cbops.aec_ref_latency_mic.amount_to_use": "latency_op",
        "$cbops.mute.main": "cbops_mute",
        "$cbops.sidetone_mix_op.main": "cbops_sidetone_mix_op"}

    # maximum cbops operator in cbops graph
    MAX_CBOPS_OPERATORS = 20

    # maximum input+output channels in a cbops graph
    MAX_CBOPS_IO = 32

    def __init__(self, **kwarg):
        # Call the base class constructor.
        Analysis.Analysis.__init__(self, **kwarg)

        # set the values of type and incr depending on the current architecture
        if Arch.chip_arch == "Hydra":
            self.native_word_size = 4
            self.null = 0xFFFFFFFF
        elif Arch.chip_arch == "Bluecore":
            self.native_word_size = 1
            self.null = 0xFFFFFF

        try:
            self.buffer_types = dict_to_ordered_dict(
                self.debuginfo.get_enum("cbops_buffer_type", None)
            )

        except InvalidDebuginfoEnumError:
            # fallback to the old implementation
            raise OutdatedFwAnalysisError()

        self.buffers_indexes = []

    def run_all(self):
        """Does nothing.

        Note:
            Not implemented for this module.
        """
        return

    #######################################################################
    # Analysis methods - public since we may want to call them individually
    #######################################################################

    def analyse_endpoints_cbops(self, cbops_mgr):
        """Analyses cbops endpoints.

        The function takes the cbops_mgr struct as a parameter and
        performs an analysis on it, if the graph exists.

        Args:
            cbops_mgr
        """
        self.formatter.section_start(
            "Analyse endpoint cbops, cbops manager: 0x{0:x}".
            format(cbops_mgr.address)
        )

        if cbops_mgr.get_member("graph") is not None:
            cbops_graph_p = cbops_mgr.get_member("graph").value
            if cbops_graph_p != 0:
                # decode cbops required flags into a friendly name
                # these are flags at the creation time
                req_ops = cbops_mgr.get_member("req_ops").value
                self.formatter.output(
                    "req_ops: {0}".format(self._decode_cbops_req_ops(req_ops))
                )

                # get the cbops direction
                cbops_dir = cbops_mgr.get_member("dir").value
                self.formatter.output("dir: 0x{0:x}".format(cbops_dir))

                # analyse cbops graph
                self.analyse_cbops_graph(cbops_graph_p)
            else:
                # no analysis for a cbops manager that Don"t have a graph
                self.formatter.output(
                    "cbops manager doesn't have allocated graph"
                )
        else:
            self.formatter.error(
                "input cbops manager doesn't have a graph member!!!"
            )
        self.formatter.section_end()

    def analyse_cbops_graph(
            self,
            cbops_graph_p,
            analyses_ops=False,
            show_graph_buffers=False):
        """Analyses the cbops graph.

        It takes as a parameter the address of the cbops graph structure
        and extra optional parameters.

        Args:
            cbops_graph_p (int): Address of the cbops graph structure.
            analyses_ops (bool, optional): If True it also analyses
                individual cbops operators in the graph.
            show_graph_buffers (bool, optional): If True the cbops buffers
                in the graph will also be processed and displayed.
        """
        # get the cbops graph
        cbops_graph = self.chipdata.cast(cbops_graph_p, "cbops_graph")

        # output the graph structure as is
        self.formatter.output(str(cbops_graph))

        # check total number of ios
        num_io = cbops_graph.get_member("num_io").value
        if num_io > self.MAX_CBOPS_IO:
            self.formatter.error(
                "seems corrupted cbops graph structure - num_io is too big:{}".
                format(num_io)
            )
            return

        # Get cbops buffers
        self._get_cbops_graph_buffers(cbops_graph, show_graph_buffers)

        # analyse operators in the cbops graph
        if analyses_ops:
            self._analyse_cbops_graph_operators(cbops_graph)

    #######################################################################
    # Private methods - don't call these externally.
    #######################################################################
    def _decode_cbops_req_ops(self, req_ops):
        """Converts cbops manager required ops to readable string.

        Args:
            req_ops:  bitmap of required ops.

        Returns:
            str: readable string of the required ops.
        """
        if req_ops == 0:
            # it's copy only
            return "COPY_ONLY"

        req_ops_str = ""
        op_flag_ix = 0
        while req_ops != 0 and op_flag_ix < len(self.CBOPS_FLAGS):
            if req_ops & 1:
                # flag is set, insert a "|" if not the first flag
                req_ops_str += " | " if req_ops_str else ""
                req_ops_str += self.CBOPS_FLAGS[op_flag_ix]
            # move to next flag
            req_ops = req_ops >> 1
            op_flag_ix = op_flag_ix + 1
        return req_ops_str

    def _get_cbops_graph_buffers(self, cbops_graph, show_graph_buffers=False):
        """Gets the cbops buffers used in the cbops graph and displays it.

        Args:
            cbops_graph (:obj:`Variable`): The cbops graph structure.
            show_graph_buffers (bool, optional): Whether to display graph
                buffers info or not.
        """

        self.buffers_indexes = []

        # get the address of cbops buffers
        buffers_p = cbops_graph.get_member("buffers").address

        # total number of buffers (not all used)
        num_io = cbops_graph.get_member("num_io").value

        if show_graph_buffers:
            self.formatter.output(
                "cbops buffers 0x{0:x}, total: {1}".format(buffers_p, num_io)
            )

        # There are a total of num_io buffers for the graph, half of these
        # are inputs and half are output buffer(in current design at least).
        # Not all of the cbops buffers are valid(used), this function only
        # extracts the indexes that are in use by the cbops graph.
        for buf_no in range(num_io):
            # get the next cbops buffer structure and see if it has a valid
            # cbuffer
            cbops_buffer = self.chipdata.cast(buffers_p, "cbops_buffer")
            if cbops_buffer.get_member("buffer").value != 0:
                # This cbops buffer include a valid cbuffer, get all the info
                if show_graph_buffers:
                    self.formatter.section_start(
                        "Cbops Buffers[{0}]: 0x{1:x}".
                        format(buf_no, buffers_p)
                    )
                    buf_type = self._get_cbops_buffer_type(
                        cbops_buffer.get_member("type").value
                    )
                    self.formatter.output("type: {}".format(buf_type))
                    self.formatter.output(
                        "buffer: 0x{0:x}".
                        format(cbops_buffer.get_member("buffer").value)
                    )
                    self.formatter.output(
                        "base: 0x{0:x}".
                        format(cbops_buffer.get_member("base").value)
                    )
                    self.formatter.output(
                        "size: 0x{0:x}".
                        format(cbops_buffer.get_member("size").value)
                    )
                    self.formatter.output(
                        "rw_ptr: 0x{0:x}".
                        format(cbops_buffer.get_member("rw_ptr").value)
                    )
                    self.formatter.output(
                        "transfer_ptr: 0x{0:x}".
                        format(cbops_buffer.get_member("transfer_ptr").value)
                    )
                    self.formatter.output(
                        "transfer_amount: 0x{0:x}".format(
                            cbops_buffer.get_member("transfer_amount").value
                        )
                    )
                    self.formatter.section_end()
                self.buffers_indexes.append(buf_no)
            buffers_p = buffers_p + cbops_buffer.size

        # also for information display number of unused cbops buffers
        if show_graph_buffers:
            unused_buffers = [
                i for i in range(num_io) if i not in self.buffers_indexes
            ]
            self.formatter.output(
                "unused buffer indexes: {0}".format(unused_buffers)
            )

    def _get_cbops_buffer_type(self, buffer_type):
        """Decode the cbops buffer type to a human readable string.

        Args:
            buffer_type (int): buffer type value.

        Returns:
            str: buffer type.
        """
        if buffer_type == 0:
            return "Unknown"
        buffer_type_str = ""
        for buf_type in self.buffer_types:
            # see if the bitmap includes this type
            if buffer_type & self.buffer_types[buf_type]:
                # insert a "|" if not the first flag
                buffer_type_str += (" | " if buffer_type_str else "")
                buffer_type_str += buf_type
        return buffer_type_str

    def _analyse_cbops_graph_operators(self, cbops_graph):
        """Displays the analyses of all the cbops operators in a cbops graph.

        Args:
            cbops_graph (:obj:`Variable`):  the cbops graph structure.
        """
        # Get the head of the cbops graph operators list
        cbops_head = cbops_graph.get_member("first")
        tot_operators = 0

        try:
            parsed_linked_list = self.parse_linked_list(
                cbops_head, 'next_operator_addr'
            )
            # traverse through the list
            for cbops_op in parsed_linked_list:
                # analyse next cbops operator in the graph
                self._analyse_cbops_operator(cbops_op)

                # we don't expect too many operators in the graph, if we see
                # we terminate further processing as it is mostly a sign of
                # broken chain
                tot_operators += 1
                if tot_operators > self.MAX_CBOPS_OPERATORS:
                    self.formatter.error(
                        "Too many operators in the cbops graph, "
                        "possibly corrupted chain"
                    )
                    return

            # Analyse override operator
            override_op_exist = self._analyse_override_operator(cbops_graph)
            # also output the total number of operators in the cbops graph
            self.formatter.output(
                "Total operators in the graph: {}".format(tot_operators)
            )
            self.formatter.output(
                "Override operator exists: {}".format(override_op_exist)
            )

        except Exception as exception:
            # This should be error happening during parsing the linked list
            # perhaps when reading attempt from un-aligned or unmapped memory
            #  address just output the type and message and end the analysis
            self.formatter.error(
                "Problem while reading the graph operator list,"
                " Perhaps corrupted input:{0}:{1}".format(
                    type(exception).__name__, exception.message
                )
            )

    def _analyse_override_operator(self, cbops_graph):
        """Analyses override operator if exists in the graph.

        Args:
            cbops_graph (:obj:`Variable`): The cbops operator structure.

        Return:
            bool: True if override operator exists, False otherwise.
        """
        # get overrid info from the graph
        override_data = cbops_graph.get_member("override_data").value
        override_funcs = cbops_graph.get_member("override_funcs")
        if override_funcs is None or override_data == 0:
            # override operator doesn't exist
            return False

        # there is an override operator in the graph
        self.formatter.section_start(
            "  cbops override operator data=0x{0:x}, funcs=0x{1:x}".
            format(override_data, override_funcs.address))

        # get the override operator
        op_size = self.debuginfo.get_type_info("cbops_op")[5]
        override_op = self.chipdata.cast(override_data - op_size, "cbops_op")

        # analyse the operator
        self._analyse_cbops_operator(override_op)
        self.formatter.section_end()
        return True

    def _analyse_cbops_operator(self, cbops_op):
        """Display the analyses of a cbops operator.

        Args:
            cbops_graph (:obj:`Variable`): The cbops operator structure.
        """
        # get the address to read the cbops op parameters from
        cbops_op_parameter_area_start = cbops_op.get_member(
            "parameter_area_start"
        ).address

        # now read the header parameters
        cbops_param_hdr = self.chipdata.cast(
            cbops_op_parameter_area_start, "cbops_param_hdr"
        )
        nr_inputs = cbops_param_hdr.get_member("nr_inputs").value
        nr_outputs = cbops_param_hdr.get_member("nr_outputs").value
        index_table_addr = cbops_param_hdr.get_member("index_table").address

        # read the input and output buffer indexes that are used by the
        # operators.
        # we have the start of index table, there will be nr_inputs
        # indexes for inputs followed by nr_outputs indexes for outputs
        # the indexes are filtered, so only those with valid buffer will
        # be extracted
        input_indexes = (
            self.chipdata.get_data(
                index_table_addr, self.native_word_size * nr_inputs
            )
        ) if nr_inputs > 0 else []
        input_indexes = [m for m in input_indexes if m in self.buffers_indexes]
        index_table_addr += self.native_word_size * nr_inputs
        output_indexes = (
            self.chipdata.get_data(
                index_table_addr, self.native_word_size * nr_outputs
            )
        ) if nr_outputs > 0 else []
        output_indexes = [
            m for m in output_indexes if m in self.buffers_indexes
        ]

        # get previous and next operator
        prev_operator_addr = cbops_op.get_member("prev_operator_addr").value
        next_operator_addr = cbops_op.get_member("next_operator_addr").value
        prev_operator_struct = self._search_cbops_op_name_by_addr(
            prev_operator_addr
        )
        next_operator_struct = self._search_cbops_op_name_by_addr(
            next_operator_addr
        )

        # get the address of operator specific data
        index_table_addr += self.native_word_size * nr_outputs
        op_specific_data_ptr = index_table_addr

        # search for cbops op name
        cbops_struct = self._search_cbops_op_name(cbops_op)

        # output the info that found
        self.formatter.section_start(
            "  cbops operator 0x{0:x}, {1}".
            format(cbops_op.address, cbops_struct)
        )
        self.formatter.output(
            "prev_operator_addr: 0x{0:x}({1})".
            format(prev_operator_addr, prev_operator_struct)
        )
        self.formatter.output(
            "next_operator_addr: 0x{0:x}({1})".
            format(next_operator_addr, next_operator_struct)
        )
        self.formatter.output(
            "nr_inputs: {0}({1} used)".format(nr_inputs, len(input_indexes))
        )
        self.formatter.output(
            "input buffer indexes: {0}".format(input_indexes)
        )
        self.formatter.output(
            "nr_outputs: {0}({1} used)".
            format(nr_outputs, len(output_indexes))
        )
        self.formatter.output(
            "output buffer indexes: {0}".format(output_indexes)
        )
        self.formatter.output(
            "in-place processing: {0}".format(output_indexes == input_indexes)
        )

        # if a valid operator structure name found, we also output
        # the content of specific data structure for the operator
        if cbops_struct is not None:
            op_specific_data = self.chipdata.cast(
                op_specific_data_ptr, cbops_struct
            )
            self.formatter.output(str(op_specific_data))
        self.formatter.section_end()

    def _get_cbops_op_function(self, cbops_op, func_type):
        """Finds the function label associated with the cbops operator.

        Args:
            cbops_op (:obj:`Variable`): The cbops operator structure.
            func_type (str): Expected to be 'process' or 'amount_to_use'.

        Returns:
            str: The function label.
            None: If the function is invalid or the lable can not be found.
        """
        # get the function vector from the cbops op structure
        function_vector = cbops_op.get_member("function_vector")
        function_vector = self.chipdata.cast(
            function_vector.value, "cbops_functions"
        )

        # get the function
        func = function_vector.get_member(func_type)
        if func is None:
            return None

        # get the function address
        func_p = func.value
        if func_p == 0:
            return None

        # find nearest label
        try:
            # find the cbops function, it needs
            # to be at exact address
            label = self.debuginfo.get_label(func_p)
        except BundleMissingError:
            label = None

        if label is not None:
            return label.name

        # No match.
        return None

    def _search_cbops_op_name(self, cbops_op):
        """Finds operator structure name.

        Args:
            cbops_op (:obj:`Variable`): The cbops operator structure.

        Returns:
            str: Operator's structure name.
        """

        # see if a match exists for "process" function
        func_label = self._get_cbops_op_function(cbops_op, "process")
        if func_label is not None:
            # match exists, now get the structure from dictionary
            if func_label in self.CBOPS_OPERATORS:
                return self.CBOPS_OPERATORS[func_label]

        # failed to find the process function, either label not found
        # or the dictionary hasn't an entry for that. No see if the
        # amount_to_use function can be found
        func_label = self._get_cbops_op_function(cbops_op, "amount_to_use")
        if func_label is not None:
            if func_label in self.CBOPS_OPERATORS:
                return self.CBOPS_OPERATORS[func_label]

        # perhaps better to distinguish between "not found"
        # and "Null found", but we trust that dictionary stays
        # up to data
        return None

    def _search_cbops_op_name_by_addr(self, cbops_op_p):
        """Finds operator structure name using address of cbops op.

        Args:
            cbops_op_p (int): The address of cbops operator structure.

        Returns:
            str: Operator structure name.
        """
        # if Null address return Null
        if cbops_op_p == 0:
            return None

        # search the name
        cbops_op = self.chipdata.cast(cbops_op_p, 'cbops_op')
        return self._search_cbops_op_name(cbops_op)
