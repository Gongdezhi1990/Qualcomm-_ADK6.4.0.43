'''
Kalsim Hydra Audio streams
'''

import argparse
import logging
import os

from kats.framework.library.docstring import inherit_docstring
from kats.framework.library.log import log_input
from kats.kalimba.hydra_service.audio_sink_service import HydraAudioSink
from kats.kalimba.hydra_service.audio_source_service import HydraAudioSource
from kats.kalimba.hydra_service.constants import DEVICE_TYPE_A2DP
from kats.library.registry import get_instance
from .hydra import StreamHydra, HYDRA_TYPE, HYDRA_TYPE_SUBSYSTEM, HYDRA_BAC_HANDLE
from ..kalsim_stream import STREAM_FLOW_CONTROL_DRIVE, STREAM_FLOW_CONTROL_DRIVE_KALCMD, \
    STREAM_FLOW_CONTROL_BLOCK_SIZE, STREAM_BACKING, STREAM_BACKING_FILE, STREAM_FILENAME, \
    STREAM_FORMAT, STREAM_TYPE_SOURCE

PARAM_DEFAULT = 'default'

DEVICE_TYPE = 'device_type'
SERVICE_TAG = 'service_tag'
METADATA_BUFFER_SIZE = 'metadata_buffer_size'
BLOCK_SIZE = 'block_size'
KICK_ENABLE = 'kick_enable'
EXTERNAL_PACKETISER = 'external_packetiser'


class StreamHydraAudio(StreamHydra):
    '''
    Hydra Audio Kalsim Stream

    .. code-block:: python

        from kats.kalsim.stream.hydra.hydra_audio import StreamHydraAudio
        from kats.kalimba.endpoint.common.static import EndpointStatic

        kwargs = {
            "stream_backing": "file",
            "stream_filename": "resource/al05_48k.adts",
            "stream_flow_control_rate": 48000,
            "stream_format": 16,
            "stream_flow_control_block_size": 1,
            "device_type": 12
        }

        st = StreamHydraAudio(kalcmd, 'source', **kwargs)
        st.create()
        ep = EndpointStatic(kymera, 'source', 11936)
        ep.create()
        st.config()
        ep.config()

    Args:
        stream_type (str): Type of stream source or sink
        device_type (int): A2DP
        service_tag (int): Hydra service stag
        block_size (int): Block size for streaming audio, only used if external_packetiser is False
        metadata_buffer_size (int): Header length in a metadata buffer record in octets
        external_packetiser (bool): Do not stream the data but provide an insert function to
            stream from outside
    '''

    platform = ['crescendo', 'stre', 'streplus']
    interface = 'hydra_audio'
    __param_schema = {
        DEVICE_TYPE: {PARAM_DEFAULT: DEVICE_TYPE_A2DP},
        SERVICE_TAG: {PARAM_DEFAULT: 1},
        BLOCK_SIZE: {PARAM_DEFAULT: 180},
        METADATA_BUFFER_SIZE: {PARAM_DEFAULT: 0},
        KICK_ENABLE: {PARAM_DEFAULT: True},
        EXTERNAL_PACKETISER: {PARAM_DEFAULT: False},
    }

    def __init__(self, stream_type, **kwargs):
        self._log = logging.getLogger(__name__) if not hasattr(self, '_log') else self._log
        inherit_docstring(self)
        self._helper = argparse.Namespace()
        self._helper.uut = get_instance('uut')
        self._helper.hydra_prot = get_instance('hydra_protocol')
        self._helper.audio_hydra = None
        self._msg_handler = None
        self._total_samples = None
        self._sent_samples = None

        # initialise default values
        self.__parameters = {}
        for entry in self.__param_schema:
            if entry in kwargs or self.__param_schema[entry][PARAM_DEFAULT] is not None:
                self.__parameters[entry] = kwargs.pop(entry,
                                                      self.__param_schema[entry][PARAM_DEFAULT])

        self._hydra_kwargs = kwargs
        self._hydra_kwargs.update({
            HYDRA_TYPE: HYDRA_TYPE_SUBSYSTEM,
            HYDRA_BAC_HANDLE: 0,  # we will modify this when we know the handle
            STREAM_FLOW_CONTROL_BLOCK_SIZE: 1,
        })

        # only if the external packetiser is not used then we can handle backing and control drive
        if not self.__parameters[EXTERNAL_PACKETISER]:
            self._hydra_kwargs.update({
                STREAM_FLOW_CONTROL_DRIVE: STREAM_FLOW_CONTROL_DRIVE_KALCMD,
                STREAM_BACKING: STREAM_BACKING_FILE,
            })

        super(StreamHydraAudio, self).__init__(stream_type, **self._hydra_kwargs)

    def __config(self, **kwargs):
        '''
        extract values from kwargs that we handle
        '''
        for entry in self.__param_schema:
            if entry in kwargs:
                self.__parameters[entry] = kwargs.pop(entry)
        return kwargs

    @log_input(logging.DEBUG)
    def _space_available(self, avail, space):
        avail = int((avail * 8) / self.get_data_width())
        space = int((space * 8) / self.get_data_width())
        if self.__parameters[EXTERNAL_PACKETISER]:
            pass
        elif self._stream_type == STREAM_TYPE_SOURCE:
            if self._sent_samples >= self._total_samples:
                try:
                    self.induce(1)  # this will force EOF to be signalled
                except Exception:  # pylint: disable=broad-except
                    pass
            elif space >= self.__parameters[BLOCK_SIZE]:
                send = self._total_samples - self._sent_samples
                send = (send if send < self.__parameters[BLOCK_SIZE] else
                        self.__parameters[BLOCK_SIZE])

                self.induce(send)
                self._helper.audio_hydra.kick()
                self._sent_samples += send
            else:
                self._helper.audio_hydra.kick()
        else:
            self.induce(avail)
            # self._helper.audio_hydra.kick()

    @log_input(logging.INFO)
    def create(self):
        if self._stream_type == STREAM_TYPE_SOURCE:
            self._helper.audio_hydra = HydraAudioSink(
                self._helper.hydra_prot,
                device_type=self.__parameters[DEVICE_TYPE],
                service_tag=self.__parameters[SERVICE_TAG],
                metadata_buffer_size=self.__parameters[METADATA_BUFFER_SIZE],
                space_handler=self._space_available)
            self._helper.audio_hydra.start()
        else:
            self._helper.audio_hydra = HydraAudioSource(
                self._helper.hydra_prot,
                device_type=self.__parameters[DEVICE_TYPE],
                service_tag=self.__parameters[SERVICE_TAG],
                space_handler=self._space_available)
            self._helper.audio_hydra.start()

        return super(StreamHydraAudio, self).create()

    @log_input(logging.INFO)
    def config(self, **kwargs):

        if self._stream_type == STREAM_TYPE_SOURCE:
            bac_handle = self._helper.audio_hydra.get_data_write_handle()
        else:
            bac_handle = self._helper.audio_hydra.get_data_read_handle()

        kwargs[HYDRA_BAC_HANDLE] = bac_handle
        super(StreamHydraAudio, self).config(**kwargs)

    @log_input(logging.INFO)
    def start(self):
        super(StreamHydraAudio, self).start()

        if self._stream_type == STREAM_TYPE_SOURCE:
            if not self.__parameters[EXTERNAL_PACKETISER]:
                self._total_samples = int(os.path.getsize(self._hydra_kwargs[STREAM_FILENAME]) /
                                          (self._hydra_kwargs[STREAM_FORMAT] / 8))
                # FIXME check that file has at least self.__parameters[BLOCK_SIZE] samples
                self._sent_samples = self.induce(self.__parameters[BLOCK_SIZE])
                self._helper.audio_hydra.kick()

    @log_input(logging.INFO)
    def stop(self):
        super(StreamHydraAudio, self).stop()

    def insert(self, data):
        '''
        Inserts data with the size specified in stream_format property

        Args:
            data (list[int]): Data to insert
        '''
        self._kalcmd.stream_insert(self.get_id(), self.get_data_width(), data)
        if self.__parameters[KICK_ENABLE]:
            self._helper.audio_hydra.kick()

    @log_input(logging.INFO)
    def destroy(self):
        # Note that stopping the service will destroy the endpoint associated with it
        # hence we delay stopping the service until we are destroying the stream
        if self._helper.audio_hydra and self._helper.audio_hydra.check_started():
            self._helper.audio_hydra.stop()
        super(StreamHydraAudio, self).destroy()

    def get_endpoint_id(self):
        '''
        Get endpoint id

        Returns:
            int: Endpoint id
        '''
        return self._helper.audio_hydra.get_endpoint_id()

    def get_metadata_read_bac_handle(self):
        '''
        Get meta data read BAC handle

        Returns:
            int: Read handle
        '''
        return self._helper.audio_hydra.get_meta_data_read_handle()

    def get_metadata_write_bac_handle(self):
        '''
        Get meta data write BAC handle

        Returns:
            int: Write handle
        '''
        return self._helper.audio_hydra.get_meta_data_write_handle()

    def kick(self):
        '''
        Kick service
        '''
        self._helper.audio_hydra.kick()
