'''
Hydra sco endpoint class
'''

import logging

from kats.framework.library.docstring import inherit_docstring
from kats.library.registry import get_instance
from ..endpoint_base import EndpointBase


class EndpointHydraSco(EndpointBase):
    '''
    Hydra sco endpoint

    This is an endpoint that is destroyed in the stream as part of the hydra sco
    data service creation/destruction.

    - *endpoint_type* can be:

        - *source*, pushing data to the uut.
        - *sink*, extracting data from the uut.

    - *stream* is the index to the sco stream this endpoint is connected to

    Args:
        kymera (kats.kalimba.kymera.kymera_base.KymeraBase): Instance of class Kymera
        endpoint_type (str): Type of endpoint source or sink
        stream (int): Hydra sco stream index
    '''

    platform = ['crescendo', 'stre', 'streplus']
    interface = 'sco'

    def __init__(self, kymera, endpoint_type, stream):
        self._log = logging.getLogger(__name__) if not hasattr(self, '_log') else self._log
        inherit_docstring(self)
        self.__stream = stream

        super(EndpointHydraSco, self).__init__(kymera, endpoint_type)

    def create(self, *_, **__):
        stream = get_instance('stream_sco', self.__stream)
        self._create('sco', [stream.get_hci_handle(), 0x0000])

    def config(self):
        pass

    def destroy(self):
        '''
        Override the base class' destroy() method but we don't need to
        worry about destroying any SCO endpoints.
        SCO Processing Service stop_service() will destroy the endpoints.
        '''
        pass
