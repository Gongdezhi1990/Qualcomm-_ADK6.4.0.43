'''
Endpoint handler class
'''

import logging

from kats.framework.library.factory import find_subclass
from kats.framework.library.log import log_input, log_output, log_exception
from kats.library.module import get_parent_module
from kats.library.registry import register_instance, unregister_instance
from .endpoint_base import EndpointBase

PLUGIN_PLATFORM = 'platform'
PLUGIN_CLASS = 'class'


class EndpointFactory(object):
    '''
    Kalimba Endpoint factory

    This class handles endpoints inside kalimba.
    It supports a factory interface, where endpoints are classified in platforms
    and there could be multiple endpoints for each platform.
    When this class starts it autodiscovers all the endpoint types available for
    a set of platforms.
    Discovered endpoints should subclass EndpointBase in a subpackage.

    .. code-block:: python

        endpoint = EndpointFactory(['crescendo'], kymera)
        endpoint.enum_interface()
        ep = endpoint.get_instance('pcm', 'source', pcm_sync_rate=8000)
        ep.create() # create endpoint in uut
        ep.config() # send all configuration messages
        ...
        ep.destroy()
        del ep

    Args:
        platform (list[str]): Platforms available
        kymera (kats.kalimba.kymera.kymera_base.KymeraBase): Instance of class Kymera
    '''

    def __init__(self, platform, kymera):
        self._log = logging.getLogger(__name__) if not hasattr(self, '_log') else self._log
        self._kymera = kymera  # we depend on kymera to message the uut
        self._plugins = {}  # registered plugins

        mod_name = get_parent_module(__name__, __file__)
        class_list = find_subclass(mod_name, EndpointBase)
        for entry in class_list:
            try:
                if list(set(entry.platform).intersection(platform)):
                    self._register(entry.platform, entry.interface, entry)
            except Exception:  # pylint: disable=broad-except
                pass

    def _register(self, platform, interface, plugin):
        self._log.info('registering endpoint interface:%s platform:%s class:%s',
                       plugin.interface, platform, plugin.__name__)
        self._plugins[interface] = {
            PLUGIN_PLATFORM: platform,
            PLUGIN_CLASS: plugin
        }

    @log_input(logging.INFO)
    @log_exception
    def get_class(self, interface):
        '''
        Search in the discovered plugins for an endpoint interface and return its class

        Args:
            interface (str): Endpoint interface

        Returns:
            any: Endpoint class

        Raises:
            ValueError: If unable to find endpoint interface
        '''
        if interface in self._plugins:  # search by name (interface)
            operator = self._plugins[interface][PLUGIN_CLASS]
            return operator
        raise ValueError('unable to find endpoint interface:%s' % (interface))

    @log_input(logging.INFO)
    @log_exception
    def get_instance(self, interface, endpoint_type, *args, **kwargs):
        '''
        Search in the discovered plugins for an endpoint interface and return an instance of it

        Args:
            interface (str): Endpoint interface
            endpoint_type (str): Endpoint type name

        Returns:
            any: Endpoint instance

        Raises:
            ValueError: If unable to find endpoint interface
        '''
        if interface in self._plugins:  # search by name (interface)
            endpoint = self._plugins[interface][PLUGIN_CLASS](self._kymera,
                                                              endpoint_type,
                                                              *args,
                                                              **kwargs)
            register_instance('endpoint', endpoint)
            register_instance('endpoint_' + interface, endpoint)
            return endpoint
        raise ValueError('unable to find endpoint interface:%s' % (interface))

    @log_input(logging.INFO)
    @log_exception
    def put_instance(self, endpoint):
        '''
        Destroy endpoint instance

        Args:
            endpoint (any): Endpoint instance

        '''
        unregister_instance('endpoint_' + endpoint.interface, endpoint)
        unregister_instance('endpoint', endpoint)
        del endpoint

    @log_output(logging.INFO)
    @log_exception
    def enum_interface(self):
        '''
        Get a list of registered endpoints

        Returns:
            list[str]: Endpoint id and names
        '''
        return self._plugins
