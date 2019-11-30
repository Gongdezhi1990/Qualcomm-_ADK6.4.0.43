'''
Capability handler class
'''

import logging

from kats.framework.library.factory import find_subclass
from kats.framework.library.log import log_input, log_output, log_exception
from kats.library.module import get_parent_module
from kats.library.registry import register_instance, unregister_instance
from .capability_base import CapabilityBase
from .generic.capability_generic import CapabilityGeneric, CapabilityGenericInfo

PLUGIN_PLATFORM = 'platform'
PLUGIN_CAP_ID = 'cap_id'
PLUGIN_CLASS = 'class'

CAPABILITY_UNKNOWN = ''


class CapabilityFactory(object):
    '''
    Kalimba Capability factory

    This class handles remote representation of capabilities and operators inside kalimba.
    It supports a factory interface, where capabilities are classified in platforms
    When this class starts it autodiscovers all the capability types available for
    a set of platforms.
    Discovered capabilities should subclass CapabilityBase in a subpackage.

    .. code-block:: python

        capability = CapabilityFactory(['common'], kymera)
        capability.enum_interface()
        op = capability.get_instance('BASIC_PASS_THROUGH',
                                    ['CHANGE_INPUT_DATA_TYPE', [1]],
                                    ['CHANGE_OUTPUT_DATA_TYPE', [1]])
        op.create() # create operator in uut
        op.config() # send all configuration messages
        op.start()
        ...
        op.stop()
        op.destroy()
        del op

    Args:
        platform (list[str]): Platforms available
        kymera (kats.kalimba.kymera.kymera_base.KymeraBase): Instance of class Kymera
    '''

    def __init__(self, platform, kymera):
        self._log = logging.getLogger(__name__) if not hasattr(self, '_log') else self._log
        self._kymera = kymera  # we depend on kymera to message the uut
        self._plugins = {}  # registered plugins

        mod_name = get_parent_module(__name__, __file__)  # get parent module
        try:
            class_list = find_subclass(mod_name, CapabilityBase)  # get all available interfaces
            for entry in class_list:
                if entry.platform in platform and entry != CapabilityGeneric:
                    self._register(entry.platform, entry.interface, entry)
        except ImportError:
            self._log.info('no capabilities found')

        try:
            gen_cap = CapabilityGenericInfo()
            gen_cap_list = gen_cap.enum_interface()
            for entry in gen_cap_list:
                self._register_multiple(entry[3], entry[0], entry[1], entry[2])
        except Exception:  # pylint: disable=broad-except
            self._log.warning('generic capabilities not available')

    def _register(self, platform, interface, plugin):
        self._log.info('registering local capability interface:%s platform:%s class:%s',
                       plugin.interface, platform, plugin.__name__)
        self._plugins[interface] = {
            PLUGIN_PLATFORM: platform,
            PLUGIN_CAP_ID: plugin.cap_id,
            PLUGIN_CLASS: plugin
        }

    def _register_multiple(self, platform, interface, cap_id, class_obj):
        self._log.info('registering local capability interface:%s platform:%s cap_id:%s class:%s',
                       interface, platform, cap_id, class_obj)
        self._plugins[interface] = {
            PLUGIN_PLATFORM: platform,
            PLUGIN_CAP_ID: cap_id,
            PLUGIN_CLASS: class_obj
        }

    @log_input(logging.INFO)
    @log_exception
    def get_class(self, capability):
        '''
        Search in the discovered plugins for a capability and return its class

        Args:
            capability (str or int): Capability name or id to search

        Returns:
            any: Capability class

        Raises:
            ValueError: If unable to find capability
        '''
        if capability in self._plugins:  # search by name (interface)
            operator = self._plugins[capability][PLUGIN_CLASS]
            return operator
        else:  # search by cap_id
            for cap in self._plugins:
                if self._plugins[cap][PLUGIN_CAP_ID] == capability:
                    operator = self._plugins[cap][PLUGIN_CLASS]
                    return operator
        raise ValueError('unable to find capability:%s' % (capability))

    @log_input(logging.INFO)
    @log_exception
    def get_instance(self, capability, *args, **kwargs):
        '''
        Search in the discovered plugins for a capability and return an instance to it

        Args:
            capability (str or int): Capability name or id to search

        Returns:
            any: Capability instance

        Raises:
            ValueError: If unable to find capability
        '''
        if capability in self._plugins:  # search by name (interface)
            operator = self._plugins[capability][PLUGIN_CLASS](
                capability, self._kymera, *args, **kwargs)
        else:  # search by cap_id
            for cap in self._plugins:
                if self._plugins[cap][PLUGIN_CAP_ID] == capability:
                    operator = self._plugins[cap][PLUGIN_CLASS](
                        cap, self._kymera, *args, **kwargs)
                    break
            else:  # not in registered caps, use generic
                operator = CapabilityGeneric(capability, self._kymera, *args, **kwargs)

        register_instance('operator', operator)
        register_instance('operator_' + str(operator.cap_id), operator)
        return operator

    @log_input(logging.INFO)
    @log_exception
    def put_instance(self, capability):
        '''
        Destroy operator instance

        Args:
            capability (any): Capability instance
        '''
        unregister_instance('operator_' + str(capability.cap_id), capability)
        unregister_instance('operator', capability)
        del capability

    @log_output(logging.INFO)
    @log_exception
    def enum_interface(self):
        '''
        Get a list of registered capabilities

        Returns:
            dict:
                str: Capability name/interface
                    int: cap_id
                    any: class
                    str: platform
        '''
        return self._plugins

    @log_output(logging.INFO)
    @log_exception
    def enum_uut(self):
        '''
        Get a list of registered capabilities in the uut

        Returns:
            dict: Capability id and names (or empty string if not locally registered)

            {1: ['pass_through'],
            2: [''],
            19: ['splitter']}
        '''
        cap_list = self._kymera.opmgr_get_capid_list()
        ret = {}
        for cap_id in cap_list:
            ret.setdefault(cap_id, [])
            for interface in self._plugins:
                if self._plugins[interface][PLUGIN_CAP_ID] == cap_id:
                    ret[cap_id].append(interface)
            if not ret[cap_id]:
                ret[cap_id].append(CAPABILITY_UNKNOWN)
        return ret
