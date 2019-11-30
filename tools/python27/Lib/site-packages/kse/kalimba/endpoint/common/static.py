'''
Static endpoint class
'''

import logging

from kats.framework.library.docstring import inherit_docstring
from ..endpoint_base import EndpointBase


class EndpointStatic(EndpointBase):
    '''
    Static Endpoint

    This is an endpoint that never gets created or destroyed

    - *endpoint_type* can be:

        - *source*, pushing data to the uut.
        - *sink*, extracting data from the uut.

    - *endpoint_id* is the endpoint identifier kymera has created for this endpoint

    Args:
        kymera (kats.kalimba.kymera.kymera_base.KymeraBase): Instance of class Kymera
        endpoint_type (str): Type of endpoint source or sink
        endpoint_id (int): Endpoint identifier
    '''

    platform = ['common']
    interface = 'static'

    def __init__(self, kymera, endpoint_type, endpoint_id):
        self._log = logging.getLogger(__name__) if not hasattr(self, '_log') else self._log
        inherit_docstring(self)
        self.__endpoint_id = endpoint_id

        super(EndpointStatic, self).__init__(kymera, endpoint_type)

    def get_id(self):
        return self.__endpoint_id

    def create(self, *_, **__):
        pass

    def config(self):
        pass

    def destroy(self):
        pass
