'''
Kymera API library
'''

import logging

from kats.framework.library.factory import find_subclass
from kats.library.module import get_parent_module

from .kymera_base import KymeraBase


def kymera_get_instance(platform, uut, *args, **kwargs):
    '''
    Kymera class loader

    This function loads the relevant kymera class depending on the platform.
    It supports a factory interface, defined by the platform, where it will try to autodetect
    a class subclassing KymeraBase abstract class

    Args:
        platform (list[str]): Platforms available
        uut (kats.kalimba.uut.UutBase): Instance of class uut
    '''
    log = logging.getLogger(__name__)
    log.debug('init platform:%s uut:%s', platform, uut)
    kymera = None

    for plat in platform:
        mod_name = get_parent_module(__name__, __file__)  # get parent module
        mod_name += '.' + plat
        try:
            class_list = find_subclass(mod_name, KymeraBase)  # get all available interfaces
            for entry in class_list:
                log.info('registering kymera platform:%s interface:%s class:%s',
                         platform, entry.interface, entry.__name__)
                kymera = entry(uut, *args, **kwargs)
                return kymera  # only one interface is used
        except ImportError:
            log.info('platform:%s no kymera interface found', plat)

    raise RuntimeError('no kymera platform:%s found' % (platform))
