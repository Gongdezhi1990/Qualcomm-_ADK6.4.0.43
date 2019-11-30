'''
Hydra a2dp endpoint class
'''

import logging

from kats.framework.library.docstring import inherit_docstring
from kats.library.registry import get_instance
from ..endpoint_base import EndpointBase


class EndpointHydraA2dp(EndpointBase):
    '''
    Hydra a2dp endpoint

    This is an endpoint that is created and destroyed in the stream as part of the hydra audio
    data service creation/destruction.
    From here we just get the endpoint id from the stream

    - *endpoint_type* can be:

        - *source*, pushing data to the uut.
        - *sink*, extracting data from the uut.

    - *stream* is the index to the hydra a2dp stream this endpoint is connected to

    Args:
        kymera (kats.kalimba.kymera.kymera_base.KymeraBase): Instance of class Kymera
        endpoint_type (str): Type of endpoint source or sink
        stream (int): Hydra audio stream index
    '''

    platform = ['crescendo', 'stre', 'streplus']
    interface = 'a2dp'

    def __init__(self, kymera, endpoint_type, stream):
        self._log = logging.getLogger(__name__) if not hasattr(self, '_log') else self._log
        inherit_docstring(self)
        self.__stream = stream

        super(EndpointHydraA2dp, self).__init__(kymera, endpoint_type)

    def get_id(self):
        stream = get_instance('stream_a2dp', self.__stream)
        return stream.get_endpoint_id()

    def create(self, *_, **__):
        pass

    def config(self):
        pass

    def destroy(self):
        pass
