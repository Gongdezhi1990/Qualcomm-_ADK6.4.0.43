'''
Hydra sco streams
'''

import argparse
import copy
import logging
from functools import partial

from kats.framework.library.docstring import inherit_docstring
from kats.framework.library.log import log_input
from kats.framework.library.schema import DefaultValidatingDraft4Validator
from kats.kalimba.hydra_service.sco_processing_service import HydraScoProcessingService, \
    TIMESLOT_DURATION
from kats.kalsim.stream.kalsim_helper import get_user_stream_config, get_file_user_stream_config
from kats.kalsim.stream.kalsim_stream import KalsimStream
from kats.kalsim.stream.stream_base import STREAM_TYPE_SOURCE, STREAM_TYPE_SINK, \
    CALLBACK_EOF, STREAM_NAME, STREAM_RATE, STREAM_DATA_WIDTH, STATE_STARTED
from kats.library.audio_file.audio import audio_get_instance
from kats.library.registry import register_instance, get_instance, get_instance_num
from .hydra import HYDRA_TYPE, HYDRA_TYPE_SUBSYSTEM, HYDRA_BAC_HANDLE

BACKING = 'backing'
BACKING_FILE = 'file'
BACKING_DATA = 'data'
FILENAME = 'filename'
CHANNELS = 'channels'
CHANNELS_DEFAULT = 1
CHANNEL = 'channel'
CHANNEL_DEFAULT = 0
SAMPLE_RATE = 'sample_rate'
SAMPLE_RATE_DEFAULT = 8000
SAMPLE_WIDTH = 'sample_width'
SAMPLE_WIDTH_DEFAULT = 16
FRAME_SIZE = 'frame_size'
FRAME_SIZE_DEFAULT = 1
DELAY = 'delay'
DELAY_DEFAULT = 0
LOOP = 'loop'
LOOP_DEFAULT = 1
CALLBACK_CONSUME = 'callback_consume'
METADATA_ENABLE = 'metadata_enable'
METADATA_ENABLE_DEFAULT = False
METADATA_CHANNEL = 'metadata_channel'
# METADATA_CHANNEL_DEFAULT = 0
STREAM = 'stream'
STREAM_DEFAULT = None

SERVICE_TAG = 'service_tag'
SERVICE_TAG_DEFAULT = None

PARAM_SCHEMA = {
    'oneOf': [
        {
            'type': 'object',
            'required': [BACKING, FILENAME],
            'properties': {
                BACKING: {'type': 'string', 'enum': [BACKING_FILE]},
                FILENAME: {'type': 'string'},
                CHANNELS: {'type': 'integer', 'minimum': 1},
                CHANNEL: {'type': 'integer', 'minimum': 0},
                SAMPLE_RATE: {'type': 'number', 'minimum': 0},
                SAMPLE_WIDTH: {'type': 'integer', 'enum': [8, 16, 24, 32]},
                FRAME_SIZE: {'type': 'integer', 'minimum': 1},
                DELAY: {'type': 'number', 'minimum': 0},
                LOOP: {'type': 'integer', 'minimum': 1},

                METADATA_ENABLE: {'type': 'boolean', 'default': METADATA_ENABLE_DEFAULT},
                # METADATA_CHANNEL: {'type': 'boolean', 'default': METADATA_CHANNEL_DEFAULT},
                STREAM: {'default': STREAM_DEFAULT},

                SERVICE_TAG: {'default': SERVICE_TAG_DEFAULT},
            }
        },
        {
            'type': 'object',
            'required': [BACKING],
            'properties': {
                BACKING: {'type': 'string', 'enum': [BACKING_DATA]},
                SAMPLE_RATE: {'type': 'number', 'minimum': 0},  # only for sink
                SAMPLE_WIDTH: {'type': 'integer', 'enum': [8, 16, 24, 32]},
                FRAME_SIZE: {'type': 'integer', 'minimum': 1},

                METADATA_ENABLE: {'type': 'boolean', 'default': METADATA_ENABLE_DEFAULT},
                # METADATA_CHANNEL: {'type': 'boolean', 'default': METADATA_CHANNEL_DEFAULT},
                STREAM: {'default': STREAM_DEFAULT},

                SERVICE_TAG: {'default': SERVICE_TAG_DEFAULT},
            }
        }
    ]
}

METADATA_SYNC_WORD = 0x5c5c


class StreamHydraSco(KalsimStream):
    '''
    Hydra sco streams

    *stream_type* can be:

        - *source*, pushing data to the uut.
        - *sink*, extracting data from the uut.

   - ``backing`` defines the origin (for sources) or destination (for sinks) of data,
     could be file or data. In the case of data it cones or goes to an external software component,
     this allow to have loops where a sink stream loops back to a source stream.

    *backing=file*

    - *filename* is the file to back the stream (mandatory).

        - raw files only store audio data but no information about number of channels,
          sampling frequency or data format.
          This information (*channels*, *sample_rate*, *sample_width*) has to be supplied
        - wav files store number of channels, sampling frequency and sample data format,
          Note that if *sample_rate* is provided then information in the file is overriden
        - qwav files store number of channels, sampling frequency, sample data format, optional
          metadata and optional packet based information
    - *channels* is the number of channels/streams in the audio file,
      only for source streams, sink streams are always created with 1 channel (optional default=1).
    - *channel* is the channel index in the audio file,
      only for source streams, sink streams are always created with 1 channel (optional default=0).
    - *sample_rate* is the sampling frequency in hertzs,
      for raw source files (mandatory), wav source files (optional) and all sink files (mandatory).
      If it is 0 it means as fast as possible.
    - *sample_width* is the number of bits per sample,
      for raw source files (mandatory) and all sink files (mandatory).
    - *frame_size* is the number of samples per transaction,
      valid for all file types and stream types (optional, default=1). Currently unused
    - *delay* indicates the delay between the stream start command and the actual start in seconds,
      only for source streams (optional default=0.0)
    - *loop* indicates the number of times the source is played, when the source gets to end of file
      it is rewinded and replayed, only for source streams (optional default=1)
    - *metadata_enable* indicates if metadata should be sent alongside the audio data, this metadata
      will be auto-generated if the format is not qwav or extracted from the qwav file,
      for source streams (optional default=False)
    - *metadata_channel* indicates the metadata channel in the qwav file,
      for source qwav streams (optional default=0). Currently unused
    - *stream* indicates the parent stream, the sco services supports one tx and one rx channel,
      for the second stream in the service this provides the parent sco stream index
    - wallclock_accuracy (float): Wallclock simulation accuracy in part per million.
    - role (str): Bluetooth link role, master or slave
    - air_compression_factor (int): SPS to/from-air compression factor
      VALID_COMPRESSION_FACTORS = (1, 2)
    - air_buffer_size (int): SPS to/from-air buffer size in octets
    - air_packet_length (int): SPS to/from-air packet length in octets
    - tesco (int): SPS BT link TeSCO (interval between transmissions)
      VALID_TESCOS = (2, 4, 6, 8, 10, 12, 14, 16, 18)
    - wesco (int): SPS BT link WeSCO (retransmission window)
    - slot_occupancy (int): Number of slots on the BT physical channel taken by one packet
      VALID_SLOT_OCCUPANCIES = (1, 3)

    *backing=data*

    - *sample_width* is the number of bits per sample,
      valid for all file types and stream types (mandatory).
    - *frame_size* is the number of samples per transaction,
      only used in sink streams (optional default=1).
    - *callback_data_received*, function to be invoked when data is available,
      only used in sink streams (mandatory).
    - *metadata_enable* indicates if metadata should be sent alongside the audio data, this metadata
      will be auto-generated if the format is not qwav or extracted from the qwav file,
      for source streams (optional default=False)
    - *metadata_channel* indicates the metadata channel in the qwav file,
      for source qwav streams (optional default=0). Currently unused
    - *stream* indicates the parent stream, the sco services supports one tx and one rx channel,
      for the second stream in the service this provides the parent sco stream index
    - wallclock_accuracy (float): Wallclock simulation accuracy in part per million.
    - role (str): Bluetooth link role, master or slave
    - air_compression_factor (int): SPS to/from-air compression factor
      VALID_COMPRESSION_FACTORS = (1, 2)
    - air_buffer_size (int): SPS to/from-air buffer size in octets
    - air_packet_length (int): SPS to/from-air packet length in octets
    - tesco (int): SPS BT link TeSCO (interval between transmissions)
      VALID_TESCOS = (2, 4, 6, 8, 10, 12, 14, 16, 18)
    - wesco (int): SPS BT link WeSCO (retransmission window)
    - slot_occupancy (int): Number of slots on the BT physical channel taken by one packet
      VALID_SLOT_OCCUPANCIES = (1, 3)
    - *callback_consume*, function to be invoked when data is available,
      only used in sink streams (mandatory) but can be set in the config method.

    Args:
        stream_type (str): Type of stream source or sink
        filename (str): Filename to back the stream
        channels (int): Number of channels in file
        channel (int): Channel in the file
        sample_rate (int): Sample rate
        sample_width (int): Number of bit per sample
        frame_size (int): Number of frames per transfer
        delay (float): Delay in seconds from start to real start
        loop (int): Number of loops
        callback_data_received (function(int)): Callback function when data is received
    '''

    platform = ['crescendo', 'stre', 'streplus']
    interface = 'sco'

    def __init__(self, stream_type, **kwargs):
        self._log = logging.getLogger(__name__) if not hasattr(self, '_log') else self._log
        inherit_docstring(self)

        DefaultValidatingDraft4Validator(PARAM_SCHEMA).validate(kwargs)

        self.__helper = argparse.Namespace()  # helper modules
        self.__helper.uut = get_instance('uut')
        self.__helper.hydra_prot = get_instance('hydra_protocol')

        self.__config = argparse.Namespace()  # configuration values
        self.__config.backing = kwargs.pop(BACKING)
        self.__config.callback_data_received = None  # backing=data sink stream callback
        self.__config.delay = None
        self.__config.metadata_enable = kwargs.pop(METADATA_ENABLE)
        # self.__config.metadata_channel = kwargs.pop(METADATA_CHANNEL, METADATA_CHANNEL_DEFAULT)
        self.__config.stream = kwargs.pop(STREAM)

        # self.__config.stream = kwargs.pop('stream')

        self.__config.service_tag = kwargs.pop(SERVICE_TAG)

        self._sco_kwargs = copy.deepcopy(kwargs)

        self.__data = argparse.Namespace()
        self.__data.loop_timer_id = None  # timer when looping
        self.__data.loop = 1
        self.__data.source_packetiser = None  # source with packet based streaming
        self.__data.source_metadata = None
        self.__data.source_timer_id = None
        self.__data.sink_timer_id = None  # sink manual streaming timer id
        self.__data.sink_timer_remain = 0.0  # sink manual timer remainder
        self.__data.sink_audio_buffer = None  # sink manual streaming audio data buffer
        self.__data.stream2 = None
        self.__data.bt_clock = 0
        self.__data.audio_data = None
        self.__data.total_samples = 0
        self.__data.sent_samples = 0

        self.__data.sco_service = None

        if self.__config.backing == BACKING_FILE:
            self.__config.filename = kwargs.pop(FILENAME)
            if stream_type == STREAM_TYPE_SOURCE:
                self.__config.delay = kwargs.pop(DELAY, DELAY_DEFAULT)
            else:
                self.__config.delay = None
            self.__config.loop = kwargs.pop(LOOP, LOOP_DEFAULT)
            self.__config.user_callback_eof = kwargs.get(CALLBACK_EOF, lambda *args, **kwargs: None)

            params = getattr(self, '_init_%s_file' % (stream_type))(kwargs)
            params[CALLBACK_EOF] = self.__eof  # required for loop
        else:  # BACKING_DATA
            if stream_type == STREAM_TYPE_SINK:
                self.__config.sample_rate = kwargs.pop(SAMPLE_RATE)  # needed in sco
            self.__config.sample_width = kwargs.pop(SAMPLE_WIDTH)
            if stream_type == STREAM_TYPE_SINK:
                self.__config.frame_size = kwargs.pop(FRAME_SIZE, FRAME_SIZE_DEFAULT)
                self.__config.callback_consume = kwargs.pop(CALLBACK_CONSUME, None)
            else:
                self.__config.frame_size = None
            self.__config.loop = 1

            params = get_user_stream_config(self.__config.sample_width)

        self.__parameters = {}
        self.__parameters[HYDRA_TYPE] = HYDRA_TYPE_SUBSYSTEM
        self.__parameters[HYDRA_BAC_HANDLE] = 0  # we will modify this when we know the handle

        if kwargs:
            self._log.warning('unknown kwargs:%s', str(kwargs))

        super(StreamHydraSco, self).__init__(stream_type, **params)

    def _init_source_file(self, kwargs):
        channels = kwargs.pop(CHANNELS, CHANNELS_DEFAULT)
        channel = kwargs.pop(CHANNEL, CHANNEL_DEFAULT)
        self.__config.sample_rate = kwargs.pop(SAMPLE_RATE, None)
        self.__config.sample_width = kwargs.pop(SAMPLE_WIDTH, None)
        self.__config.frame_size = kwargs.pop(FRAME_SIZE, FRAME_SIZE_DEFAULT)

        audio_kwargs = {
            'channels': channels,  # some formats do not require
            'sample_rate': self.__config.sample_rate,  # some formats do not require
            'sample_width': self.__config.sample_width,  # some formats do not require
        }
        audio_instance = audio_get_instance(self.__config.filename, **audio_kwargs)
        channels = audio_instance.get_audio_stream_num()
        # we allow overriding sample_rate from what is in the file
        self.__config.sample_width = audio_instance.get_audio_stream_sample_width(channel)

        if channel >= channels:
            raise RuntimeError('channels:%s channel:%s inconsistency' % (channels, channel))
        if audio_instance.get_audio_stream_sample_rate(channel) is None:
            raise RuntimeError('stream filename:%s sample_rate not set' % (self.__config.filename))
        if self.__config.sample_width is None:
            raise RuntimeError('stream filename:%s sample_width not set' % (self.__config.filename))

        # FIXME here we are only supporting streaming manually with kats
        # might be interesting to add kalsim support in the future whenever is possible
        self.__data.audio_data = audio_instance.get_audio_stream_data(channel)
        self.__data.total_samples = len(audio_instance.get_audio_stream_data(channel))
        params = get_user_stream_config(self.__config.sample_width)
        params[STREAM_NAME] = self.__config.filename
        params[STREAM_RATE] = self.__config.sample_rate  # pointless in qwav with packet_info
        params[STREAM_DATA_WIDTH] = self.__config.sample_width

        del audio_instance
        return params

    def _init_sink_file(self, kwargs):
        self.__config.channels = 1
        self.__config.channel = 0
        self.__config.sample_rate = kwargs.pop(SAMPLE_RATE)
        self.__config.sample_width = kwargs.pop(SAMPLE_WIDTH)
        self.__config.frame_size = kwargs.pop(FRAME_SIZE, FRAME_SIZE_DEFAULT)

        # kalsim supports raw and wav files with only 1 stream
        if not self.__config.filename.endswith('.qwav'):
            params = get_file_user_stream_config(self.__config.filename,
                                                 self.__config.sample_rate,
                                                 self.__config.sample_width)
        else:
            self.__data.sink_audio_buffer = []
            params = get_user_stream_config(self.__config.sample_width)
            params[STREAM_NAME] = self.__config.filename
            params[STREAM_RATE] = self.__config.sample_rate
            params[STREAM_DATA_WIDTH] = self.__config.sample_width
        return params

    def _start(self, timer_id):
        _ = timer_id
        self.__data.loop_timer_id = None
        super(StreamHydraSco, self).start()

        if self.get_type() == STREAM_TYPE_SOURCE:
            if self.__config.backing == BACKING_FILE:
                self.__data.sent_samples = 0
                self.__data.sco_service.start_channel(1, partial(self._data_transmit, 0))
            else:
                # we receive data with calls to consume
                pass
        else:
            if self.__config.backing == BACKING_FILE:
                self.__data.sco_service.start_channel(0, partial(self._data_received, 0))
            else:  # backing=data
                # we receive data from sco endpoint, but we need to poll it
                self.__data.sco_service.start_channel(0, partial(self._data_received, 0))

    def _get_metadata(self, data_length):
        # compute metadata in 16 bit native format
        length_in_bytes = int(data_length * self.__config.sample_width / 8.0)
        metadata = [METADATA_SYNC_WORD, 5, length_in_bytes, 0,
                    self.__data.bt_clock & 0xFFFF]
        self.__data.bt_clock += self.__data.sco_service.get_tesco() * 2

        # convert metadata to a different sample_width if required
        if self.__config.sample_width == 8:
            metadata_b = []
            for entry in metadata:
                metadata_b.append(entry & 0xFF)
                metadata_b.append((entry >> 8) & 0xFF)
            metadata = metadata_b
        elif self.__config.sample_width == 16:
            pass
        else:
            raise RuntimeError(
                'sco sink stream with metadata sample_width:%s not supported' %
                (self.__config.sample_width))
        return metadata

    def _data_transmit(self, timer_id):
        _ = timer_id
        self.__data.source_timer_id = None

        data_length = int(
            self.__config.sample_rate * self.__data.sco_service.get_tesco() * TIMESLOT_DURATION)

        if self.get_state() == STATE_STARTED:
            if self.__data.sent_samples < self.__data.total_samples:
                if (self.__data.total_samples - self.__data.sent_samples) < data_length:
                    data_length = self.__data.total_samples - self.__data.sent_samples

                if self.__config.metadata_enable:
                    metadata = self._get_metadata(data_length)

                    data_to_send = metadata + list(
                        self.__data.audio_data[
                        self.__data.sent_samples:self.__data.sent_samples + data_length])
                else:
                    data_to_send = self.__data.audio_data[
                                   self.__data.sent_samples:self.__data.sent_samples + data_length]

                self.insert(data_to_send)
                self.__data.sent_samples += data_length

                self.__data.source_timer_id = self.__helper.uut.timer_add_relative(
                    TIMESLOT_DURATION * self.__data.sco_service.get_tesco(), self._data_transmit)
            else:
                self.eof()

    def _data_received(self, timer_id):
        _ = timer_id
        self.__data.sink_timer_id = None

        data_length = int(
            self.__config.sample_rate * self.__data.sco_service.get_tesco() * TIMESLOT_DURATION)

        if self.get_state() == STATE_STARTED:
            if self.__config.backing == BACKING_FILE:
                if self.__data.sink_audio_buffer is not None:
                    # FIXME not using frame_size
                    # data = self.extract(self.__config.frame_size, sign_extend=True)
                    data = self.extract(data_length, sign_extend=True)
                    self.__data.sink_audio_buffer += data
                else:
                    self.induce(data_length)
            else:
                if self.__config.callback_consume:
                    # FIXME not using frame_size
                    data = self.extract(data_length, sign_extend=True)
                    self.__config.callback_consume[0](data=data)

            self.__data.sink_timer_id = self.__helper.uut.timer_add_relative(
                TIMESLOT_DURATION * self.__data.sco_service.get_tesco(), self._data_received)

    def __eof(self):
        '''
        This is our own callback for an End of File.

        In the case of source file backed streams we install this callback handler when there is a
        stream eof. This will cause to check if there are any additional loops to be done and
        in case there are rewind the stream and replay
        '''
        self.__data.loop = 0 if self.__data.loop <= 1 else self.__data.loop - 1
        if self.__data.loop > 0:
            self.stop()
            self._start(0)
        else:
            self.__config.user_callback_eof()

    @log_input(logging.INFO)
    def create(self):
        '''
        Start service and create stream

        TODO: If stream_type of a SCO Processing Service instance is easily available,
        raise a RuntimeError if we are trying to start two instances with the same
        stream_type and hci_handle.
        '''
        if self.__config.stream is not None:
            stream = get_instance('stream_sco')
            if stream.get_type() == self.get_type():
                raise RuntimeError('trying to start two sco streams of same type')
            self.__data.sco_service = stream.get_sco_service()
        else:
            if self.__config.service_tag is not None:
                service_tag = self.__config.service_tag
            else:
                service_tag = get_instance_num('sco_processing_service') + 100

            self.__data.sco_service = HydraScoProcessingService(service_tag=service_tag,
                                                                **self._sco_kwargs)
            register_instance('sco_processing_service', self.__data.sco_service)
            self.__data.sco_service.start()
            self.__data.sco_service.config()

        return super(StreamHydraSco, self).create()

    @log_input(logging.INFO)
    def config(self, **kwargs):
        if CALLBACK_CONSUME in kwargs:
            self.__config.callback_consume = kwargs.pop(CALLBACK_CONSUME)
            if not isinstance(self.__config.callback_consume, list):
                raise RuntimeError('callback_consume:%s invalid' % (self.__config.callback_consume))
            if len(self.__config.callback_consume) != 1:
                raise RuntimeError('callback_consume:%s invalid' % (self.__config.callback_consume))

        if self.get_type() == STREAM_TYPE_SOURCE:
            bac_handle = self.__data.sco_service.get_from_air_handle()
        else:
            bac_handle = self.__data.sco_service.get_to_air_handle()

        self.__parameters[HYDRA_BAC_HANDLE] = bac_handle

        for key in self.__parameters:
            self._config_param(key, self.__parameters[key])

        super(StreamHydraSco, self).config(**kwargs)

        for key in self.__parameters:
            _ = self.query(key)

    @log_input(logging.INFO)
    def start(self):
        '''
        Start streaming

        Notes:
        Before we start streaming:

        - we check if Audio FW is ready to supply or consume data - check_for_channels_ready()
        - start BT clock if Audio ready
        - Audio FW ready is indicated by a 'run state' message which should have
          already been handled by SCO Processing Service

        Raises:
            RuntimeError: - If Audio FW not ready to process data
        '''

        self.__data.loop = self.__config.loop
        if self.__config.delay:
            self._log.info('delaying start for %s seconds', self.__config.delay)
            self.__data.loop_timer_id = self.__helper.uut.timer_add_relative(self.__config.delay,
                                                                             callback=self._start)
        else:
            self._start(0)

    @log_input(logging.INFO)
    def stop(self):
        '''
        Stop stream
        '''

        if self.__data.loop_timer_id is not None:
            self.__helper.uut.timer_cancel(self.__data.loop_timer_id)
            self.__data.loop_timer_id = None
        if self.__data.source_timer_id:
            self.__helper.uut.timer_cancel(self.__data.source_timer_id)
            self.__data.source_timer_id = None
        if self.__data.sink_timer_id:
            self.__helper.uut.timer_cancel(self.__data.sink_timer_id)
            self.__data.sink_timer_id = None

        super(StreamHydraSco, self).stop()

    @log_input(logging.INFO)
    def destroy(self):

        if self.__config.backing == BACKING_FILE and self.__data.sink_audio_buffer is not None:
            self._log.info('creating file %s', self.__config.filename)
            audio_instance = audio_get_instance(self.__config.filename, 'w')
            audio_instance.add_audio_stream(self.__config.sample_rate,
                                            self.__config.sample_width,
                                            self.__data.sink_audio_buffer)
            audio_instance.write()
            del audio_instance
            self.__data.sink_audio_buffer = []

        # Note that stopping the service will destroy the endpoint associated with it
        # hence we delay stopping the service until we are destroying the stream
        if self.__data.sco_service.check_started():
            self.__data.sco_service.stop()
        super(StreamHydraSco, self).destroy()

        # FIXME unregister instance, be careful with multiple channels

    def check_active(self):
        # FIXME this code assumes that we are connected after a kymera graph
        # if we are not it should be removed
        # a similar thing happens with a2dp and pcm streams
        if self.get_type() == STREAM_TYPE_SOURCE and self.__config.backing == BACKING_DATA:
            return False
        return super(StreamHydraSco, self).check_active()

    def consume(self, input_num, data):
        if (input_num == 0 and
                self.get_type() == STREAM_TYPE_SOURCE and
                self.__config.backing == BACKING_DATA):
            data_length = len(data)
            if self.__config.metadata_enable:
                metadata = self._get_metadata(data_length)
                data_to_send = metadata + list(data)
            else:
                data_to_send = data

            self.insert(data_to_send)

    def eof_detected(self, input_num):
        if (input_num == 0 and
                self.get_type() == STREAM_TYPE_SOURCE and
                self.__config.backing == BACKING_DATA):
            self.eof()

    def get_endpoint_id(self):
        '''
        Get endpoint id

        Returns:
            int: Endpoint id
        '''
        return self.__helper.audio_hydra.get_endpoint_id()

    def get_sco_service(self):
        '''
        Return sco service instance

        Returns:
            HydraScoProcessingService: sco service
        '''
        return self.__data.sco_service

    def get_hci_handle(self):
        '''
        Get hci handle

        Returns:
            int: hci handle
        '''
        return self.__data.sco_service.get_hci_handle()
