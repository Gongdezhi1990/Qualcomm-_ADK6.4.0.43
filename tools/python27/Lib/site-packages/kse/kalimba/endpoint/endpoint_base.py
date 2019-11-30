'''
Endpoint Base class
'''

import logging

from abc import ABCMeta, abstractmethod, abstractproperty
from six import add_metaclass

from kats.framework.library.log import log_input, log_exception

SOURCE = 'source'
SINK = 'sink'

ENDPOINT_TYPES = [SOURCE, SINK]


@add_metaclass(ABCMeta)
class EndpointBase(object):
    '''
    Endpoint base class. Should be subclassed by every endpoint extension.

    This builds an interface on top of kymera to handle endpoints.
    kymera interface is a single interface of multiple endpoints while EndpointBase is
    an instance that handle a single endpoint.

    .. code-block:: python

        # EndpointPcmCrescendo is subclassing EndpointBase
        ep = EndpointPcmCrescendo(kymera, 'source')
        ep.create() # create endpoint in uut
        ep.config() # send all configuration messages
        ...
        ep.destroy()
        del ep

    Args:
        kymera (kats.kalimba.kymera.kymera_base.KymeraBase): Instance of class Kymera
        endpoint_type (str): Type of endpoint source or sink
    '''

    def __init__(self, kymera, endpoint_type, *_, **__):
        self._log = logging.getLogger(__name__) if not hasattr(self, '_log') else self._log
        self._kymera = kymera
        self._endpoint_type = endpoint_type
        self._endpoint_id = None

    @abstractproperty
    def platform(self):
        '''
        str: Platform name
        '''
        pass

    @abstractproperty
    def interface(self):
        '''
        str: Interface name
        '''
        pass

    def get_id(self):
        '''
        Get endpoint id

        Returns:
            int: Endpoint id
        '''
        return self._endpoint_id

    @log_input(logging.INFO)
    @log_exception
    def _create(self, endpoint_type, params):
        '''
        Create new kymera based endpoint
        '''
        if self._endpoint_id:
            raise RuntimeError('endpoint already created')
        elif self._endpoint_type not in ENDPOINT_TYPES:
            raise RuntimeError('endpoint type:%s unknown' % (self._endpoint_type))
        elif self._endpoint_type == SOURCE:
            self._endpoint_id = self._kymera.stream_if_get_source(endpoint_type, params)
        else:
            self._endpoint_id = self._kymera.stream_if_get_sink(endpoint_type, params)

    @abstractmethod
    def create(self, *_, **__):
        '''
        Create endpoint
        '''
        pass

    @log_input(logging.INFO)
    @log_exception
    def destroy(self):
        '''
        Destroy endpoint
        '''
        if not self._endpoint_id:
            raise RuntimeError('endpoint not created')
        elif self._endpoint_type == SOURCE:
            self._kymera.stream_if_close_source(self._endpoint_id)
        else:
            self._kymera.stream_if_close_sink(self._endpoint_id)
        self._endpoint_id = None

    @log_input(logging.INFO)
    @abstractmethod
    def config(self):
        '''
        Configure endpoint
        '''
        pass

    def config_param(self, key, value):
        '''
        Configure single parameter

        Args:
            key (str or int): Key
            value (int): Value
        '''
        if not self._endpoint_id:
            raise RuntimeError('endpoint not created')
        else:
            self._kymera.stream_if_configure_sid(self._endpoint_id, key, value)
