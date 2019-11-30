'''
Audio file helper
'''

import logging

import os

from kats.framework.library.factory import find_subclass
from kats.library.module import get_parent_module
from .audio_base import AudioBase, MODE_DEFAULT

DEFAULT_INTERFACE = '_default_interface_'
REGISTERED_AUDIO_FILE = {DEFAULT_INTERFACE: None}

MOD_NAME = get_parent_module(__name__, __file__)  # get parent module
try:
    CLASS_LIST = find_subclass(MOD_NAME, AudioBase)  # get all available interfaces
    for entry in CLASS_LIST:
        log = logging.getLogger(__name__)
        log.info('registering audio file interface:%s class:%s', entry.interface, entry.__name__)
        REGISTERED_AUDIO_FILE[entry.interface] = entry
        if entry.default_interface:
            REGISTERED_AUDIO_FILE[DEFAULT_INTERFACE] = entry
except ImportError:
    log = logging.getLogger(__name__)
    log.info('no audio file interfaces found')


def audio_get_instance(filename, mode=MODE_DEFAULT, *args, **kwargs):
    '''
    Audio file class loader

    This function loads the relevant audio file class.
    It supports a factory interface, defined by the filename extensnio, where it will try to
    autodetect a class subclassing AudioBase abstract class

    Args:
        filename (str): Filename
        mode (str): File open mode 'r' for read, 'w' for write
    '''

    ext = os.path.splitext(filename)[1][1:]
    if ext not in REGISTERED_AUDIO_FILE:
        if REGISTERED_AUDIO_FILE[DEFAULT_INTERFACE]:
            return REGISTERED_AUDIO_FILE[DEFAULT_INTERFACE](filename, mode, *args, **kwargs)
        raise RuntimeError('filename %s unable to find registered handler' % (filename))

    return REGISTERED_AUDIO_FILE[ext](filename, mode, *args, **kwargs)
