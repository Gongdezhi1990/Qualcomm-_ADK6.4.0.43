############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2012 - 2019 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
"""
Module responsible for analysing Kymera Streams.
"""
try:
    from future_builtins import hex
except ImportError:
    pass

import logging
import os
import re
import sys
import tempfile
import threading
import time
import traceback
import urllib
try:
    import urlparse
except ImportError:
    # `urllib` and `urlparse` have Changed in Python3
    import urllib.parse as urlparse
    import urllib.request as urllib  # Redefine the imported urllib

from . import Analysis
from ..Core import Graphviz
from ACAT.Core import CoreUtils as cu
from ACAT.Core.exceptions import (
    DebugInfoNoVariableError, InvalidDebuginfoTypeError
)
from ACAT.Display.HtmlFormatter import HtmlFormatter
from ACAT.Display.InteractiveFormatter import InteractiveFormatter
from ACAT.Display.PlainTextFormatter import PlainTextFormatter

CURRENT_DIRECTORY = os.path.dirname(os.path.realpath(__file__))
TEMP_DIR = os.path.join(CURRENT_DIRECTORY, '..', '..', 'temp')

INSTALL_GRAPHVIZ_MSG = (
    "Graph image cannot be produced as graphviz python is not "
    "installed.\n"
    "To install it, please use the following command:\n\n"
    "{} -m pip install graphviz==0.8.4".format(sys.executable)
)

VARIABLE_DEPENDENCIES = {
    'strict': (
        'L_kick_object_list', '$_transform_list', '$_source_endpoint_list',
        '$_sink_endpoint_list', 'L_system_stream_rate', 'L_system_kick_period'
    ),
    'not_strict': (
        '$_stream_connect_fault',
        'L_rm_list',
    )
}
ENUM_DEPENDENCIES = {
    'strict': ('ENDPOINT_DIRECTION', 'STREAM_CONNECT_FAULT', 'ENDPOINT_TYPE')
}
TYPE_DEPENDENCIES = {
    'ENDPOINT': (
        'key', 'id', 'can_be_closed', 'can_be_destroyed',
        'destroy_on_disconnect', 'direction', 'is_real', 'con_id', 'cbops',
        'ep_to_kick', 'state'
    ),
    'TRANSFORM': (
        'id', 'source', 'sink', 'created_buffer', 'shared_buffer', 'buffer'
    ),
    'RATEMATCH_PAIR': ('ep1', 'ep2', 'enacting_ep', 'next'),
    'KICK_OBJECT': ('sched_ep', 'kick_ep')
}

HW_MASK = 0xFF00
INSTANCE_MASK = 0xF0
CHANNEL_MASK = 0x000F

logger = logging.getLogger(__name__)


class Endpoint(object):
    """Base class to represent an endpoint in Kymera.

    Args:
        ep
        is_state_type_equal_to_name (bool, optional)
        name
        stream
    """
    raw_data = None  # Endpoint-shaped variable we used to create this object
    # transform =  None # Used for finding the transform

    # The key of the endpoint.
    key = 0

    # The id of the endpoint. This is unique across all endpoint of the same
    # direction, i.e. it is possible for a source and a sink endpoint to have
    # the same id.
    id = 0  # Always represented in External format

    # Flag to say if the endpoint can be closed
    can_be_closed = None

    # Flag to say if the endpoint can be destroyed
    can_be_destroyed = None

    # Flag to say if an endpoint is created by streams
    internally_created = None

    # Flag to say if an endpoint is a source or a sink
    direction = None

    # Enum to say how the endpoint should be connected
    # type = 0 is already specified by the type of the object

    # Flag to say if an endpoint is a 'real' endpoint
    is_real = None

    # Flag to say if an endpoint has been started
    # (is processing data).
    is_enabled = None

    # Connection id of the owner of this endpoint
    con_id = None

    # Pointer to cbops_manager that encapsulates the cbops
    # information associated with the endpoint
    cbops = None

    # Pointer to the endpoint that this endpoint has a connection to.
    _connected_to = None

    # Pointer to the endpoint to kick
    _ep_to_kick = None

    # Pointer to next endpoint in the list.
    # next = 0 Additional information for the list not used in the stream
    # analysis

    # Information for latency control purposes.
    latency_ctrl_info = None

    state = None

    def __init__(self, ep, name, stream, is_state_type_equal_to_name=True):
        try:
            cbops_analysis = stream.interpreter.get_analysis(
                "cbops", stream.chipdata.processor
            )
        except KeyError:
            cbops_analysis = None
        self.raw_data = ep
        self.stream = stream

        self.address = ep.address
        self.key = ep.get_member('key').value
        self.hardware = None
        self.instance = None
        self.codec = None

        if name == "audio":
            stream_device = self.stream.debuginfo.get_enum("STREAM_DEVICE")
            hardware_value = (self.key & HW_MASK) >> 8
            self.instance = (self.key & INSTANCE_MASK) >> 4
            self.channel = self.key & CHANNEL_MASK

            for key, value in stream_device.items():
                if value == hardware_value:
                    self.hardware = key

        self.id = Stream.convert_ep_id(ep.get_member('id').value, 1)
        self.can_be_closed = ep.get_member('can_be_closed').value
        if self.can_be_closed == 0:
            self.can_be_closed = False
        else:
            self.can_be_closed = True

        self.can_be_destroyed = ep.get_member('can_be_destroyed').value
        if self.can_be_destroyed == 0:
            self.can_be_destroyed = False
        else:
            self.can_be_destroyed = True

        self.destroy_on_disconnect = ep.get_member(
            'destroy_on_disconnect'
        ).value
        if self.destroy_on_disconnect == 0:
            self.destroy_on_disconnect = False
        else:
            self.destroy_on_disconnect = True

        self.direction = stream.debuginfo.get_enum(
            'ENDPOINT_DIRECTION', ep.get_member('direction').value
        )[0]
        self.is_real = ep.get_member('is_real').value
        if self.is_real == 0:
            self.is_real = False
        else:
            self.is_real = True

        self.con_id = ep.get_member('con_id')
        if self.con_id is not None:
            self.con_id = self.con_id.value

        # cbops mgr
        self.cbops = ep.get_member('cbops')
        if self.cbops.value != 0:
            self.cbops = stream.chipdata.cast(self.cbops.value, "cbops_mgr")
            if stream.cbopsanalysis_on and cbops_analysis is not None:
                self.cbops = cbops_analysis.analyse_endpoints_cbops(self.cbops)
            else:
                self.cbops = str(self.cbops)
        else:
            self.cbops = str(self.cbops)

        self._connected_to = ep.get_member("connected_to").value
        self._ep_to_kick = ep.get_member('ep_to_kick').value

        self.state = ep.get_member('state')

        self.name = name

        self.is_state_type_equal_to_name = is_state_type_equal_to_name
        if is_state_type_equal_to_name:
            setattr(self, name, self.state.get_member(name))

    def __str__(self):
        """Returns a tidy string representation of an endpoint object."""
        return self.title_str + '\n' + self.desc_str

    @property
    def title_str(self):
        """Returns the title string of the endpoint."""
        return self.compact_name()

    @property
    def desc_str(self):
        """Returns the description string of the endpoint."""
        ep_str = "Address: " + hex(self.address) + "\n"
        ep_str = ep_str + "Key: " + hex(self.key) + "\n"
        # it's enough to check with one variable that it's the AudioEP
        if self.instance is not None:
            ep_str += "Device: " + str(self.hardware) + "\n"
            ep_str += "Instance: " + str(self.instance) + "\n"
            ep_str += "Channel: " + str(self.channel) + "\n"

        ep_str = ep_str + "is_real: " + \
            str(self.is_real) + " can_be_closed: " + \
            str(self.can_be_closed) + "\n"
        ep_str = ep_str + "can_be_destroyed: " + \
            str(self.can_be_destroyed) + " destroy_on_disconnect: " + \
            str(self.destroy_on_disconnect) + "\n"
        if self.con_id is not None:
            ep_str = ep_str + "Connection ID: " + str(self.con_id) + "\n"

        ep_str = ep_str + "connected_to: "
        try:
            connected_to = self.connected_to
            if connected_to is None:
                ep_str += "NULL\n"
            else:
                ep_str += hex(connected_to.id) + \
                    "(" + connected_to.compact_name() + ") \n"
        except AttributeError:
            ep_str += (
                "0x%08x is not a valid endpoint address\n" % self._connected_to
            )

        ep_str += "ep_to_kick: "
        try:
            endpoint_to_kick = self.ep_to_kick
            if endpoint_to_kick is None:
                ep_str += "NULL\n"
            else:
                ep_str += hex(endpoint_to_kick.id) + \
                    "(" + endpoint_to_kick.compact_name() + ") \n"
        except AttributeError:
            ep_str += (
                "0x%08x is not a valid endpoint address\n" % self._ep_to_kick
            )
        ep_str = ep_str + str(self.cbops)

        if self.is_state_type_equal_to_name:
            ep_str = ep_str + \
                str(getattr(self, self.name))

        return ep_str

    @property
    def connected_to(self):
        """Returns the connected_to endpoint as an object."""
        return self.stream.get_endpoint_by_address(self._connected_to)

    @property
    def ep_to_kick(self):
        """Returns the ep_to_kick endpoint as an object."""
        return self.stream.get_endpoint_by_address(self._ep_to_kick)

    def compact_name(self):
        """Returns a string representation of an endpoint object."""
        ep_str = self.name.title() + " " + self.direction + " " + hex(self.id)

        return ep_str


class AudioEndpoint(Endpoint):
    """Audio Endpoint.

    Audio endpoint is treated differently to display the associated timed
    playback module.
    """
    audio = None

    @property
    def desc_str(self):
        """Returns the description string of the endpoint."""
        # Call the base class property function cannot be done otherwise
        ep_str = Endpoint.desc_str.fget(self)  # pylint: disable=no-member

        tp_ptr_var = self.audio.get_member("timed_playback")
        if tp_ptr_var:
            # timed playback is enabled in the build
            timed_playback_ptr = tp_ptr_var.value

            if timed_playback_ptr != 0:
                # the endpoint has a timed playback instance.
                timed_playback_var = self.stream.chipdata.cast(
                    addr=timed_playback_ptr,
                    type_def="TIMED_PLAYBACK",
                    elf_id=self.stream.debuginfo.get_kymera_debuginfo().elf_id
                )
                ep_str = ep_str.replace(
                    "timed_playback: " + hex(timed_playback_ptr),
                    str(timed_playback_var).replace("\n", "\n  ")
                )

        return ep_str


class OperatorEndpoint(Endpoint):
    """Operator Endpoint.

    Args:
        ep
        name
        stream
    """
    operator = None

    def __init__(self, ep, name, stream):
        Endpoint.__init__(self, ep, 'operator', stream, False)
        self.operator = self.state.get_member('operator')
        self.name = self._get_op_name()

    # We don't need to override title_str as that delegate to compact_name

    # We don't need to override __str__ as that delegates to title_str and
    # desc_str.

    @property
    def desc_str(self):
        """Returns the description string of the operator."""
        # Call the base class property function cannot be done otherwise
        ep_str = Endpoint.desc_str.fget(self)  # pylint: disable=no-member
        ep_str = ep_str + self.operator.__str__()

        return ep_str

    def _get_op_name(self):
        opmgr = self.stream.interpreter.get_analysis(
            "opmgr", self.stream.chipdata.processor
        )
        # for each operator ep, it gets the associated operator type
        name = opmgr.get_operator(self.id).cap_data.name
        return name


RE_ENDPOINT_TYPE = re.compile(r'endpoint_(\S+)')


class Transform(object):
    """A structure for storing transform info.

    Args:
        t
        stream
    """
    raw_data = None  # Transform-shaped variable we used to create this object

    # source endpoint of the transform
    source = None

    # sink endpoint of the transform
    sink = None

    # Unique ID of the transform
    # Always represented in External format.
    id = 0

    # Indicating whether the transform created this buffer
    created_buffer = 0

    # transform buffer
    buffer = None

    def __init__(self, t, stream):
        # Populate the variables common to every transform
        self.raw_data = t
        self.address = t.address
        self.id = Stream.convert_transform_id(t.get_member('id').value, 1)

        src = t.get_member('source')
        self.source = stream.get_endpoint_by_address(src.value)

        sink = t.get_member('sink')
        self.sink = stream.get_endpoint_by_address(sink.value)

        self.created_buffer = t.get_member('created_buffer').value
        if self.created_buffer == 0:
            self.created_buffer = False
        else:
            self.created_buffer = True

        self.shared_buffer = t.get_member('shared_buffer').value
        if self.shared_buffer == 0:
            self.shared_buffer = False
        else:
            self.shared_buffer = True

        self.buffer = t.get_member('buffer').value
        buffers = stream.interpreter.get_analysis(
            "buffers", stream.chipdata.processor
        )
        self.cbuffer = buffers.inspect_cbuffer(self.buffer)

    def __str__(self):
        """Returns a tidy string representation of a transform object."""
        return self.short_title_str + '\n' + self.desc_str

    @property
    def title_str(self):
        """Returns the title string of the transform."""
        return self.short_title_str + ' from ' + self.direction_str

    @property
    def direction_str(self):
        """Returns the direction string of the transform."""
        return self.source.compact_name().replace(' SOURCE ', ' ') + ' to ' + \
            self.sink.compact_name().replace(' SINK ', ' ')

    @property
    def short_title_str(self):
        """Returns a shorter title than title_str."""
        return "Transform " + hex(self.id)

    @property
    def desc_str(self):
        """Returns the description string of the transform."""
        tf_str = "Address: " + hex(self.address) + "\n"
        tf_str = tf_str + "Source: " + self.source.compact_name() + "\n"
        tf_str = tf_str + "Sink: " + self.sink.compact_name() + "\n"
        if self.created_buffer:
            tf_str = tf_str + "The transform created the buffer \n"
        else:
            tf_str = tf_str + "The buffer was supplied by an endpoint \n"
        if self.shared_buffer:
            tf_str = tf_str + "The buffer shared \n"
        else:
            tf_str = tf_str + "The buffer is not shared \n"

        tf_str = tf_str + 'Buffer: ' + hex(self.buffer) + '\n'

        return tf_str


class Stream(Analysis.Analysis):
    """
    Provides a capability for analysing Streams.
    """

    @staticmethod
    def convert_transform_id(id_to_convert, force_direction=None):
        """Converts the transform ID.

        If 'force' is set to 1, we will force the ID to be external;
        if 'force' is set to 0, we will force the ID to be internal.
        If 'force' isn't set, we toggle the ID between internal/external.
        Defined as a static method so that other classes/analyses can use it.

        Args:
            id_to_convert
            force_direction

        Returns:
            The internal/external version of it.
        """
        # TRANSFORM_COOKIE is defined in stream_connect.c, so this code needs
        # to be kept in sync with it if anything ever changes.

        # define TRANSFORM_COOKIE 0x1C1E
        cookie = 0x1C1E

        if force_direction is not None:
            already_external = False
            if id_to_convert & cookie > 0xff:
                already_external = True
            if ((force_direction == 0 and already_external) or
                    (force_direction == 1 and not already_external)):
                id_to_convert = id_to_convert ^ cookie
        else:
            id_to_convert = id_to_convert ^ cookie

        return id_to_convert

    @staticmethod
    def convert_ep_id(id_to_convert, force_direction=None):
        """Takes the id and return the internal/external version of it.

        This does roughly the same as the firmware macro
        TOGGLE_EP_ID_BETWEEN_INT_AND_EXT.

        Args:
            id_to_convert
            force_direction (int, optional): Forces the ID to be external
                or internal. It can have the following values:

                1: Forces the ID to be external.
                0: Forces the ID to be internal.
                None: Toggles the ID between internal/external.
        """
        # All of this stuff is defined in stream.h and needs to be kept in
        # sync with it if anything ever changes.

        # In general, we don't need to do anthing to convert operator EPIDs
        # since the internal and external representations are the same.
        # Real endpoints have a 'cookie' applied, though.

        # define SOURCE_EP_COOKIE 0x0EA1
        # define SINK_EP_COOKIE   0x0A5E
        # define Stream.convert_ep_id(id_to_convert) \
        #    if ((id_to_convert & STREAM_EP_TYPE_MASK) == STREAM_EP_EXT_SOURCE) \
        #        id_to_convert ^= SOURCE_EP_COOKIE; \
        #    else if ((id_to_convert & STREAM_EP_TYPE_MASK) == STREAM_EP_EXT_SINK) \
        #        id_to_convert ^= SINK_EP_COOKIE;
        stream_ep_type = id_to_convert & 0xe000
        if stream_ep_type == 0x2000:
            # Real endpoint, source
            cookie = 0x0EA1
        elif stream_ep_type == 0xa000:
            # Real endpoint, sink
            cookie = 0x0A5E
        else:
            return id_to_convert  # operator EPID; bail out now

        if force_direction is not None:
            already_external = False
            if id_to_convert & cookie > 0xff:
                already_external = True
            if ((force_direction == 0 and already_external) or
                    (force_direction == 1 and not already_external)):
                id_to_convert = id_to_convert ^ cookie
        else:
            id_to_convert = id_to_convert ^ cookie

        return id_to_convert

    def __init__(self, **kwarg):
        """Initialises the Sps Analysis object.

        Args:
            **kwarg: Arbitrary keyword arguments.
        """
        Analysis.Analysis.__init__(self, **kwarg)
        self.transforms = []
        self.endpoints = []
        self.cbopsanalysis_on = False

    def run_all(self, cbopsanalysis_on=False):
        """Runs a complete analysis of Streams.

        The complete analysis includes transforms, endpoints, chains,
        ratematch pairs and kick objects.

        Args:
            cbopsanalysis_on (bool): If True, the CBOPs mgr will also be
                analysed and displayed.
        """
        # self.cbopsanalysis_on decides on whether CBOPs are analysed or not
        self.cbopsanalysis_on = cbopsanalysis_on
        self.formatter.section_start("Streams")
        if Graphviz.is_grapviz_available():
            self.create_graph_img()
        self.analyse_system_configuration()
        self.analyse_stream_connect_fault()
        self.analyse_transforms(
            True, True
        )
        self.analyse_endpoints(self.cbopsanalysis_on)
        self.analyse_chains()
        self.plot_transforms_buffers(screen_width=120, live_plotting=False)
        self.analyse_rm_pairs()
        self.analyse_kick_objects()
        self.formatter.section_end()

    def get_endpoint(self, id_to_search):
        """Finds the endpoint.

        Args:
            id_to_search (int): It can be an endpoint ID in internal or
                external format.
        """
        self._read_all_endpoints()

        # Force to external representation
        id_to_search = Stream.convert_ep_id(id_to_search, 1)
        return self._find_endpoint(id_to_search)

    def get_endpoint_by_address(self, address):
        """Finds the endpoint by address.

        Args:
            address
        """
        self._read_all_endpoints()

        return self._find_endpoint_by_address(address)

    def get_endpoints(self):
        """Returns a list with all available endpoints."""
        self._read_all_endpoints()

        return self.endpoints

    @Analysis.cache_decorator
    def get_transform(self, id_to_search):
        """Finds the transform.

        Args:
            id_to_search (int): It can be a transform ID in internal or
                external format.
        """
        self._read_all_transforms()

        # Force to external representation
        id_to_search = Stream.convert_transform_id(id_to_search, 1)
        return self._find_transform(id_to_search)

    def get_transforms(self):
        """Returns all the available transforms."""
        self._read_all_transforms()
        return self.transforms

    def get_ep_audio_sink_timed(self):
        """Returns all the audio sink endpoints which have time playback.

        Returns:
            list: All the audio sink endpoints which have time playback.
        """
        ep_audio_sink_timed = []
        for ep in self.get_endpoints():
            if ep.name == 'audio' and ep.direction == 'SINK':
                if ep.audio.get_member('timed_playback').value != 0:
                    ep_audio_sink_timed.append(ep)

        return ep_audio_sink_timed

    def get_ep_audio_sink_timed_dict(self):
        """Returns endpoints in dictionary format.

        This can be used in ACATLab to draw graphs for audio sink
        endpoints which have timed playback.

        Returns:
            dict: Containing audio sink endpoint parameters and their
                addresses that need to be drawn in Matlab.
        """

        ep_audio_sink_timed = self.get_ep_audio_sink_timed()
        ep_draw_dict = {}
        for ep in ep_audio_sink_timed:
            temp_dict = {}
            ep_draw_dict[ep.id] = temp_dict
            ep_chipdata = self.chipdata.cast(
                ep.audio.get_member('timed_playback').value, "TIMED_PLAYBACK"
            )
            current_tag = ep_chipdata.get_member('current_tag')
            playback_time = current_tag.get_member('playback_time').address
            timestamp = current_tag.get_member('timestamp').address

            temp_dict['error'] = current_tag.get_member('error').address
            temp_dict['timestamp_minus_playback_time'] = timestamp - \
                playback_time
            temp_dict['warp'] = ep_chipdata.get_member('pid_state').get_member(
                'warp'
            ).address
            temp_dict['sp_adjust'] = ep_chipdata.get_member(
                'tag_reframe'
            ).get_member('tag').get_member('sp_adjust').address
            ep_draw_dict[ep.id] = temp_dict

        return ep_draw_dict

    #######################################################################
    # Analysis methods - public since we may want to call them individually
    #######################################################################

    def analyse_stream_connect_fault(self):
        """Checks stream connect for any fault.

        It checks if STREAM_CONNECT_FAULT_CODE is defined and if it is,
        its value is displayed.
        """
        try:
            faultvar = self.chipdata.get_var_strict('$_stream_connect_fault')
        except DebugInfoNoVariableError:
            # STREAM_CONNECT_FAULT_CODE probably not defined
            self.formatter.output(
                "STREAM_CONNECT_FAULT_CODE is not defined for this build."
            )
            return

        fault_str = self.debuginfo.get_enum(
            'STREAM_CONNECT_FAULT', faultvar.value
        )[0]  # enum name
        self.formatter.output("Stream connect fault code is " + fault_str)

    def analyse_transforms(self, display_buffer=False, analyse_metadata=False):
        """Displays the transform contents.

        Args:
            display_buffer (bool, optional): If True the transform buffer
                is displayed.  False by default.
            analyse_metadata (bool, optional): If True the metadata of the
                transform buffer is displayed. False by default.
        """
        self._read_all_transforms()
        if display_buffer:
            buffers_analysis = self.interpreter.get_analysis(
                "buffers", self.chipdata.processor
            )
        self.formatter.section_start("Transforms")
        plot = isinstance(self.formatter, HtmlFormatter)
        for t in self.transforms:
            self.formatter.section_start(t.title_str)
            self.formatter.output(t.desc_str)
            if display_buffer:
                buffers_analysis.analyse_cbuffer(
                    t.buffer,
                    t.title_str,
                    analyse_metadata,
                    plot_content=plot
                )

            self.formatter.section_end()

        self.formatter.section_end()

    def get_transforms_buffer_pointer(self):
        """Retruns the transform buffers.

        Returns:
            list of buffers.
        """
        self._read_all_transforms()
        buffers = []
        for t in self.transforms:
            buffers.append((t.direction_str, t.buffer))
        return buffers

    def plot_transforms_buffers(self, screen_width=0, live_plotting=True):
        """Prints out buffers usages.

        Displays the transform buffers usage in a console progress bar way
        [####    ].

        Args:
            screen_width (int, optional): Number of characters.
            live_plotting (bool, optional) The live_plotting mode will
                continuously display the buffer usage until an exit event
                is received.
        """
        if screen_width == 0:
            try:
                # Auto detect mode. Make the width 90% of the terminal
                # width.
                screen_width = int(os.get_terminal_size().columns * 0.9)

            except AttributeError:
                # Python2, set a default screen width.
                screen_width = 120

        self._read_all_transforms()
        buffers = []
        for t in self.transforms:
            buffers.append((t.direction_str, t.buffer))

        if type(self.formatter) is not InteractiveFormatter:
            # live_plotting mode only available in the interactive formatter.
            live_plotting = False

        buffers_analysis = self.interpreter.get_analysis(
            "buffers", self.chipdata.processor
        )
        buffers_analysis.plot_buffers(buffers, screen_width, live_plotting)

    def analyse_service_priv(self, ep):
        """Displays the service_priv info for audio data service endpoints.

        Args:
            ep (:obj:`Endpoint`): Endpoint structure.
        """
        try:
            srv_priv_p = getattr(ep, ep.name).get_member("service_priv").value
            srv_priv = self.chipdata.cast(
                srv_priv_p,
                "AUDIO_SINK_SOURCE"
            )
        except (AttributeError, InvalidDebuginfoTypeError):
            # Do nothing if the field isn't present
            pass
        else:
            self.formatter.output(srv_priv)
            self.formatter.output(
                self.chipdata.
                cast(srv_priv.get_member("meta_desc").value, "META_DESC")
            )

    def analyse_endpoints(self, cbopsanalysis_on=False):
        """Displays the contents of the endpoints.

        Args:
            cbopsanalysis_on (bool, optional): If True, CBOPs mgr analysis
                will also be displayed.
        """
        self.cbopsanalysis_on = cbopsanalysis_on
        # Refresh our lists of endpoints and transforms
        self._read_all_endpoints()
        self._read_all_transforms()

        self.formatter.section_start("Endpoints")
        for endpoint in self.endpoints:
            self.formatter.section_start(endpoint.title_str)
            self.formatter.output(endpoint.desc_str)
            # Check the latency specific information
            if endpoint.name == "audio" and endpoint.direction == "SINK":
                running = endpoint.audio.get_member("running").value
                sync_started = endpoint.audio.get_member("sync_started").value
                latency_ctrl_info_var = endpoint.audio.get_member("latency_ctrl_info")
                silence_samples = latency_ctrl_info_var.get_member(
                    "silence_samples"
                ).value
                if running and sync_started != 0 and \
                        silence_samples != 0:
                    self.formatter.alert(
                        "Silence samples different from 0 for %s" %
                        (endpoint.compact_name())
                    )

            if endpoint.name in {"timestamped", "a2dp", "file", "usb"}:
                self.analyse_service_priv(endpoint)

            self.formatter.section_end()

        self.formatter.section_end()

    def analyse_endpoint(self, ep_id, cbopsanalysis_on=True):
        """Displays the endpoint contents.

        Args:
            ep_id
            cbopsanalysis_on (bool, optional)
        """
        self.cbopsanalysis_on = cbopsanalysis_on
        # Refresh our lists of endpoints and transforms
        # (Need to do both, in this order, if we want endpoints to contain
        # a reference to attached transforms.)
        ep = self.get_endpoint(ep_id)

        self.formatter.output(str(ep))
        if ep.name == "audio" and ep.direction == "SINK":
            running = ep.audio.get_member("running").value
            sync_started = ep.audio.get_member("sync_started").value
            latency_ctrl_info_var = ep.audio.get_member("latency_ctrl_info")
            silence_samples = latency_ctrl_info_var.get_member(
                "silence_samples"
            ).value
            if running and sync_started != 0 and \
                    silence_samples != 0:
                self.formatter.alert(
                    "Silence samples different from 0 for %s" %
                    (ep.compact_name())
                )

        if ep.name in {"timestamped", "a2dp", "file", "usb"}:
            self.analyse_service_priv(ep)

    def analyse_chains(self):
        """Draws operators with their endpoints."""
        self._read_all_endpoints()
        self._read_all_transforms()
        try:
            opmgr = self.interpreter.get_analysis(
                "opmgr", self.chipdata.processor
            )
            # for each operator ep, it gets the associated operator type
            operators = opmgr.get_oplist("object")
        except KeyError:
            operators = []
        # The function `create_graph_dict` is unrelated to Graphviz third
        # party library.
        graph_dict = Graphviz.create_graph_dict(self.endpoints, operators)
        self.formatter.section_start("Chains")
        empty_text_sink = '        |      '
        empty_text_source = '      |'
        for cluster_name in graph_dict:
            output_text = '.          /--------------------\\\n'
            output_text += "." + cluster_name.center(42) + '\n'
            source_list = []
            sink_list = []
            ep_id_details = graph_dict[cluster_name][Graphviz.ELEMENTS].items()
            for ep_id, details in ep_id_details:
                if details[Graphviz.DIRECTION] == 'SOURCE':
                    source_list.append(
                        ep_id + '|->' + details[Graphviz.CONNECTED_TO_ID]
                    )
                else:
                    sink_list.append(
                        details[Graphviz.CONNECTED_TO_ID] + '->|' + ep_id
                    )

            for i in range(max(len(source_list), len(sink_list))):
                text_to_output = ' %s        %s' % (
                    sink_list[i]
                    if i < len(sink_list) else empty_text_sink, source_list[i]
                    if i < len(source_list) else empty_text_source
                )
                output_text += ('. ' + text_to_output + '\n')
            output_text += r'.          \--------------------/'
            output_text += '\n'
            self.formatter.output(output_text)
        self.formatter.section_end()

    def _create_streams_html(self, operators, graph):
        """Creates and opens an html file which will display the streams.

        This html will include the graph, operators, endpoints and
        transforms.

        Args:
            operators: available operators.
            graph: Graphviz object.
        """
        # analyse the buffer with buffers_analysis
        buffers_analysis = self.interpreter.get_analysis(
            "buffers", self.chipdata.processor
        )
        # create a temporary file
        temp_file = tempfile.NamedTemporaryFile(
            mode="w", prefix="ACAT_", suffix=".html", delete=False
        )
        file_lication = temp_file.name
        temp_file.close()
        # create a temporary html formatter
        formtter_temp = HtmlFormatter(file_lication)
        # Display the graph
        formtter_temp.section_start("Streams")
        formtter_temp.output_svg(str(graph.pipe(format='svg')))
        formtter_temp.section_end()
        # display all the operators
        formtter_temp.section_start("Operators")
        for operaor in operators:
            formtter_temp.section_start(operaor.title_str)
            formtter_temp.output(operaor.desc_str)
            formtter_temp.section_end()
        formtter_temp.section_end()
        # display all the endpoints
        formtter_temp.section_start("Endpoints")
        for endpoint in self.endpoints:
            formtter_temp.section_start(endpoint.title_str)
            formtter_temp.output(endpoint.desc_str)
            formtter_temp.section_end()
        formtter_temp.section_end()
        # display all the transforms
        formtter_temp.section_start("Transforms")
        for transform in self.transforms:
            formtter_temp.section_start(transform.title_str)
            formtter_temp.output(transform.desc_str)
            formtter_temp.output(
                buffers_analysis.inspect_cbuffer(transform.buffer, True)
            )
            formtter_temp.section_end()
        formtter_temp.section_end()
        # write the data to the html file
        formtter_temp.flush()
        # finally, open the html file
        self.formatter.output(" Opening " + file_lication)
        cu.open_file_with_default_app(file_lication)

    def _create_and_output_graph(self, operators):
        """Creates and displays the graph.

        Args:
            operators: available operators.
        """
        graph = Graphviz.create_graph(
            self.endpoints, self.transforms, operators, self.formatter.proc
        )
        try:
            if type(self.formatter) is InteractiveFormatter:
                self._create_streams_html(operators, graph)
            elif type(self.formatter) is HtmlFormatter:
                # Create legend for graph image when formatter is Html
                legend = Graphviz.create_legend()
                if not cu.global_options.under_test:
                    self.formatter.output_svg(
                        str(graph.pipe(format='svg'))
                    )
                    self.formatter.output_svg(
                        str(legend.pipe(format='svg'))
                    )

                else:
                    # display the graph content as a string.
                    self.formatter.output_raw(str(graph))
                    self.formatter.output_raw(str(legend))
        except RuntimeError:
            raise Exception(
                "Please install graphviz from www.graphviz.org\n"
                "Make sure the Graphviz is in the PATH"
            )

    def create_graph_img(self):
        """Draws a graphical representation of streams chain.

        It uses graphviz. This is done by first extracting the endpoint
        details from the dictionary and drawing nodes and edges from that
        dictionary.
        """
        if Graphviz.is_grapviz_available() is False:
            logger.warning(INSTALL_GRAPHVIZ_MSG)
            return
        if type(self.formatter) is PlainTextFormatter:
            # do not draw when in plain text mode
            return

        self._read_all_endpoints()
        self._read_all_transforms()

        opmgr = self.interpreter.get_analysis("opmgr", self.chipdata.processor)
        # for each operator ep, it gets the associated operator type
        operators = opmgr.get_oplist("object")

        # call function to create graphviz
        self._create_and_output_graph(operators)

    def create_live_graph_img(self, set_refresh_interval=3):
        """Draws a live graphical representation of streams chain.

        It uses graphviz. This is done by first extracting the endpoint
        details from the dictionary and drawing nodes and edges from that
        dictionary.
        """
        if Graphviz.is_grapviz_available() is False:
            logger.warning(INSTALL_GRAPHVIZ_MSG)
            return
        filename = 'live_graph.html'
        html_file = open(filename, 'w+')
        live_html_text = '''
        <!DOCTYPE html>
        <html>
        <head>
        </head>
        <body>
                <img style='display:none;' id='graph_img' name='graph_img' src="">
        </body>

        <script language='javascript'>
            function updateImg() {
                rand_int = Math.random();
                document.getElementById("graph_img").src = '%s/graph.svg?rnd=' + rand_int;
                document.getElementById("graph_img").style.display = '';
            }

            window.setInterval(function(){updateImg();}, %d);
        </script>
        </html>
        ''' % (_path_to_url(TEMP_DIR), set_refresh_interval * 1000)
        html_file.write(live_html_text)
        html_file.close()

        # try to opend the html file
        if os.name == 'nt':
            os.system("start " + html_file.name)
        elif os.name == 'nt':
            os.system("open " + html_file.name)
        else:
            # Give up. Let the user open it.
            self.formatter.output("Please open " + html_file.name)

        # start_new_thread(self._live_graph, ())

        self.formatter.output("Press enter to exit from graph update.")
        # Create and run the GrapUdater
        exit_event = threading.Event()
        exit_event.clear()
        reader = GraphUpdate(
            exit_event=exit_event,
            helper=self
        )
        reader.start()
        # wait until a key is pressed.
        sys.stdin.read(1)
        exit_event.set()
        # wait for the task to finish
        reader.join()

    def analyse_rm_pairs(self):
        """Prints out the available ratematch pairs."""
        self.formatter.section_start("Ratematch pairs")
        pair_found = False
        try:
            _rm_list = self.debuginfo.get_var_strict('L_rm_list').address

            for rm_pair in self.parse_linked_list(_rm_list, 'next'):
                ep1 = self.get_endpoint_by_address(
                    rm_pair.get_member("ep1").value
                )
                ep2 = self.get_endpoint_by_address(
                    rm_pair.get_member("ep2").value
                )
                enacting_ep = self.get_endpoint_by_address(
                    rm_pair.get_member("enacting_ep").value
                )
                self.formatter.section_start("")
                self.formatter.output(
                    "    pair: %-34s %-34s" %
                    (ep1.compact_name(), ep2.compact_name())
                )
                self.formatter.output(
                    "enacting: %-34s" % (enacting_ep.compact_name())
                )
                self.formatter.section_end()
                pair_found = True
        except DebugInfoNoVariableError:
            pass

        if not pair_found:
            self.formatter.output("No ratematch pair found.")

        self.formatter.section_end()

    def analyse_kick_objects(self):
        """Prints out the available kick objects."""
        self.formatter.section_start("Kick objects")
        _kick_object_list = self.debuginfo.get_var_strict(
            'L_kick_object_list'
        ).address
        kick_object_found = False
        kcik_obj_list = self.parse_linked_list(
            _kick_object_list, 'next'
        )
        for kick_object in kcik_obj_list:
            sched_ep = self.get_endpoint_by_address(
                kick_object.get_member("sched_ep").value
            )
            kick_ep = self.get_endpoint_by_address(
                kick_object.get_member("kick_ep").value
            )
            self.formatter.section_start("")
            self.formatter.output(
                "     kick endpoint: %-34s" % (kick_ep.compact_name())
            )
            self.formatter.output(
                "scheduler endpoint: %-34s" % (sched_ep.compact_name())
            )
            self.formatter.section_end()
            kick_object_found = True

        if not kick_object_found:
            self.formatter.output("No kick object found.")

        self.formatter.section_end()

    def analyse_system_configuration(self):
        """Prints out the system stream configuration."""
        try:
            system_stream_rate = self.chipdata.get_var_strict(
                "L_system_stream_rate"
            )
            system_kick_period = self.chipdata.get_var_strict(
                "L_system_kick_period"
            )
        except DebugInfoNoVariableError:
            self.formatter.output(
                "There is no system stream configuration in the build."
            )
            return
        system_stream_rate = system_stream_rate.value
        system_kick_period = system_kick_period.value
        output_str = "The system sample rate is %s" % (system_stream_rate)
        if system_stream_rate == 0:
            output_str += " (uninitialized)"

        output_str += (
            "\nThe system kick period is %s us" % (system_kick_period)
        )
        if system_kick_period == 0:
            output_str += " (uninitialized)"

        self.formatter.output(
            output_str
        )

    def _read_all_transforms(self):
        """Reads all transforms and populate self.transforms."""
        self.transforms = self._read_raw_transforms()

    @Analysis.cache_decorator
    def _read_raw_transforms(self):
        transforms = []
        for transform in self.parse_linked_list('$_transform_list', 'next'):
            transforms.append(self._read_transform(transform))

        return transforms

    #######################################################################
    # Private methods - don't call these externally.
    #######################################################################

    def _read_transform(self, transform):
        """Reads a transform.

        Takes a raw transform Variable (cast from the linked-list) and
        inspects it properly.

        Args:
            transform: Raw transform Variable.
        """
        return Transform(transform, self)

    def _read_all_endpoints(self):
        """Reads all endpoints and populate self.endpoints."""
        # Initialise the list so that this works on a live chip
        self.endpoints = self._read_raw_endpoints()

    @Analysis.cache_decorator
    def _read_raw_endpoints(self):
        endpoints = []
        for source in self.parse_linked_list('$_source_endpoint_list', 'next'):
            endpoints.append(self._read_endpoint(source))
        for sink in self.parse_linked_list('$_sink_endpoint_list', 'next'):
            endpoints.append(self._read_endpoint(sink))

        return endpoints

    def _read_endpoint(self, ep):
        """Reads an endpoint.

        Takes a raw endpoint Variable (cast from the linked-list) and
        inspects it properly.

        Args:
            ep: Raw endpoint Variable.
        """
        # Check the endpoint_type of the endpoint
        ep_type = self.debuginfo.get_enum(
            'ENDPOINT_TYPE', ep.get_member('stream_endpoint_type').value
        )[0]
        ep_type = RE_ENDPOINT_TYPE.search(ep_type).group(1)

        if ep_type == "operator":  # operator
            return OperatorEndpoint(ep, ep_type, self)
        # audio endpoint is treated differently to show the timedplayback
        # module.
        if ep_type == "audio":
            return AudioEndpoint(ep, ep_type, self)

        # Endpoint name from endpoint ID enum is equal to endpoint state
        # type.
        return Endpoint(ep, ep_type, self)

    @Analysis.cache_decorator
    def _find_endpoint(self, id_to_search):
        """NB searches for a transform using its *external* ID.

        Args:
            id_to_search
        """
        for ep in self.endpoints:
            if ep.id == id_to_search:
                return ep
        return None

    @Analysis.cache_decorator
    def _find_endpoint_by_address(self, address):
        """NB searches for an endpoint by its address.

        Args:
            address
        """
        for ep in self.endpoints:
            if ep.address == address:
                return ep
        return None

    @staticmethod
    def _ep_is_real_or_shadow(ep):
        """Checks if an endpoint is real or shadow.

        Args:
            ep

        Returns:
            bool: True if ep real or shadow, False otherwise.
        """
        return ep.is_real or ep.name == "shadow"

    def _find_real_and_shadow_endpoints(self):
        """Searches for real and shadow endpoint.

        Returns:
            list: Real and shadow endpoints.
        """
        real_ep = []
        for ep in self.endpoints:
            if self._ep_is_real_or_shadow(ep):
                real_ep.append(ep)

        return real_ep

    @Analysis.cache_decorator
    def _find_transform(self, id_to_search):
        """NB searches for a transform using its *external* ID.

        Args:
            id_to_search
        """
        for tfm in self.transforms:
            if tfm.id == id_to_search:
                return tfm
        return None


class GraphUpdate(threading.Thread):
    """Creates graph images for the html file which displays the image.

    Args:
        exit_event: Event which stops the thread.
        helper: Pointer to the analyses.
    """

    def __init__(self, exit_event, helper):
        threading.Thread.__init__(self)
        self.exit_event = exit_event
        self.helper = helper

    def run(self):
        """Starts the thread."""
        if Graphviz.is_grapviz_available() is False:
            logger.warning(INSTALL_GRAPHVIZ_MSG)
            # Terminate the thread
            return

        # get the operator manager
        opmgr = self.helper.interpreter.get_analysis(
            "opmgr",
            self.helper.chipdata.processor
        )
        # run until an exit event is received.
        while not self.exit_event.is_set():
            try:
                self.helper._read_all_endpoints()
                self.helper._read_all_transforms()
                # for each operator ep, it gets the associated operator type
                operators = opmgr.get_oplist("object")
                graph = Graphviz.create_graph(
                    self.helper.endpoints,
                    self.helper.transforms,
                    operators,
                    self.helper.formatter.proc
                )
                graph.format = 'svg'
                graph.render("%s/graph" % TEMP_DIR, cleanup=True)
                time.sleep(0.1)

            except BaseException:
                self.helper.formatter.output(
                    "\n" + traceback.format_exc() +
                    "\nException raised! Continuing execution.\n"
                )
                # sleep a few second to avoid entering to the same error
                # too fast.
                time.sleep(2)


def _path_to_url(path):
    """Converts a local path to a browser friendly one.

    Args:
        path (str): An OS path.

    Returns:
        str: The address of a file or location on the drive and is browser
            friendly.
    """
    return urlparse.urljoin('file:', urllib.pathname2url(path))
