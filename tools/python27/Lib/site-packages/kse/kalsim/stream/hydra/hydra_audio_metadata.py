'''
Kalsim Hydra Audio Metadata streams
'''

import logging

from kats.framework.library.docstring import inherit_docstring
from kats.framework.library.log import log_input
from kats.library.registry import get_instance
from .hydra import StreamHydra, HYDRA_TYPE, HYDRA_TYPE_SUBSYSTEM, HYDRA_BAC_HANDLE
from ..kalsim_stream import STREAM_FLOW_CONTROL_BLOCK_SIZE, STREAM_TYPE_SOURCE

PARAM_DEFAULT = 'default'

HYDRA_AUDIO = 'hydra_audio'
KICK_ENABLE = 'kick_enable'
EXTERNAL_PACKETISER = 'external_packetiser'


class StreamHydraAudioMetadata(StreamHydra):
    '''
    Hydra Audio Metadata Kalsim Stream

    Args:
        stream_type (str): Type of stream source or sink
        hydra_audio (int): Hydra audio stream index
        kick_enable (bool): Enable kick on insert
        external_packetiser (bool): Do not stream the data but provide an insert function to
            stream from outside
    '''

    platform = ['crescendo', 'stre', 'streplus']
    interface = 'hydra_audio_metadata'
    __param_schema = {
        HYDRA_AUDIO: {PARAM_DEFAULT: 0},
        KICK_ENABLE: {PARAM_DEFAULT: False},
        EXTERNAL_PACKETISER: {PARAM_DEFAULT: False},
    }

    def __init__(self, stream_type, **kwargs):
        self._log = logging.getLogger(__name__) if not hasattr(self, '_log') else self._log
        inherit_docstring(self)

        # initialise default values
        self.__parameters = {}
        for entry in self.__param_schema:
            if entry in kwargs or self.__param_schema[entry][PARAM_DEFAULT] is not None:
                self.__parameters[entry] = kwargs.pop(entry,
                                                      self.__param_schema[entry][PARAM_DEFAULT])

        # only if the external packetiser is not used then we can handle backing and control drive
        if not self.__parameters[EXTERNAL_PACKETISER]:
            raise RuntimeError('hydra_audio_metadata requires external packetiser')

        # this instance is associated with a hydra_audio stream
        self._stream_hydra_audio = get_instance('stream_hydra_audio',
                                                self.__parameters[HYDRA_AUDIO])

        self._hydra_kwargs = kwargs
        self._hydra_kwargs.update({
            HYDRA_TYPE: HYDRA_TYPE_SUBSYSTEM,
            HYDRA_BAC_HANDLE: 0,  # we will modify this when we know the handle
            STREAM_FLOW_CONTROL_BLOCK_SIZE: 1,
        })

        super(StreamHydraAudioMetadata, self).__init__(stream_type, **self._hydra_kwargs)

    def __config(self, **kwargs):
        '''
        extract values from kwargs that we handle
        '''
        for entry in self.__param_schema:
            if entry in kwargs:
                self.__parameters[entry] = kwargs.pop(entry)
        return kwargs

    @log_input(logging.INFO)
    def config(self, **kwargs):

        if self._stream_type == STREAM_TYPE_SOURCE:
            bac_handle = self._stream_hydra_audio.get_metadata_write_bac_handle()
        else:
            bac_handle = self._stream_hydra_audio.get_metadata_read_bac_handle()

        kwargs[HYDRA_BAC_HANDLE] = bac_handle
        super(StreamHydraAudioMetadata, self).config(**kwargs)

    def insert(self, data):
        '''
        Inserts data with the size specified in stream_format property

        Args:
            data (list[int]): Data to insert
        '''
        self._kalcmd.stream_insert(self.get_id(), self.get_data_width(), data)
        # we do not kick the metadata stream as it should be synchronised with
        # audio insertion and that will already kick the service
        if self.__parameters[KICK_ENABLE]:
            self._stream_hydra_audio.kick()
