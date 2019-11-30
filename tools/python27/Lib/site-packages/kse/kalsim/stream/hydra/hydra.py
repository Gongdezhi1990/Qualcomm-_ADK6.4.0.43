'''
Kalsim Hydra streams
'''

import logging

from kats.framework.library.docstring import inherit_docstring
from kats.framework.library.log import log_input
from kats.library.registry import get_instance
from ..kalsim_stream import KalsimStream

PARAM_DEFAULT = 'default'

HYDRA_TYPE = 'hydra_type'
HYDRA_TYPE_AUDIO_SLOT = 'audio_slot'
HYDRA_TYPE_SUBSYSTEM = 'subsystem'

HYDRA_BAC_HANDLE = 'hydra_bac_handle'
HYDRA_AUDIOSLOT = 'hydra_audioslot'


class StreamHydra(KalsimStream):
    '''
    Hydra Kalsim Stream

    Args:
        stream_type (str): Type of stream source or sink
        hydra_type (str): Come in two flavours, namely audio_slot streams and subsystem streams.
            Audio slot streams resemble the actual hardware devices present on the chip.
            Audioslot streams have a register control interface and can have stream monitors
            attached to them.
            Subsystem streams represent streams between different subsystems on the chip.
            As such their presence is only noticeable via the bac_handles
        hydra_bac_handle (int): 0 to 128
        hydra_audioslot (int): 0 to 16
    '''

    platform = ['crescendo', 'stre', 'streplus']
    interface = 'hydra'
    __param_schema = {
        HYDRA_TYPE: {PARAM_DEFAULT: HYDRA_TYPE_AUDIO_SLOT},
        HYDRA_BAC_HANDLE: {PARAM_DEFAULT: 0},
        HYDRA_AUDIOSLOT: {PARAM_DEFAULT: 0},
    }

    def __init__(self, stream_type, **kwargs):
        self._log = logging.getLogger(__name__) if not hasattr(self, '_log') else self._log
        inherit_docstring(self)
        self._kalcmd = get_instance('kalcmd')

        # initialise default values
        self.__parameters = {}
        for entry in self.__param_schema:
            if entry in kwargs or self.__param_schema[entry][PARAM_DEFAULT] is not None:
                self.__parameters[entry] = kwargs.pop(entry,
                                                      self.__param_schema[entry][PARAM_DEFAULT])
        super(StreamHydra, self).__init__(stream_type, **kwargs)

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
        kwargs = self.__config(**kwargs)
        for key in self.__parameters:
            self._config_param(key, self.__parameters[key])

        super(StreamHydra, self).config(**kwargs)

        stream_id = self.get_id()
        for key in self.__parameters:
            _ = self._kalcmd.stream_query_property(stream_id, key)
