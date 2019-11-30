'''
Kalaccess instrument
'''

import logging
import os
import sys

import six

from kats.framework.library.instrument import Instrument
from kats.framework.library.log import log_input

INSTRUMENT = 'kalaccess'
SUBSYSTEM = 'subsystem'
PORT = 'port'
PATH = 'path'


class Kalaccess(Instrument):
    '''
    kalimba python tools kalacess instrument.

    This instrument instantiates a kalaccess class instance from kalimba python tools.

    Args:
        subsystem (int): Subsystem to connect to
        port (int): Debugger port
        path (str): Path to kalimba python tools package directory, if not set then the module
            will be imported as kal_python_tools
    '''

    interface = 'kalaccess'
    schema = {
        'type': 'array',
        'minItems': 1,
        'uniqueItems': True,
        'items': {
            'type': 'object',
            # 'required': [],
            'properties': {
                SUBSYSTEM: {'type': 'integer', 'default': 3},
                PORT: {'type': 'integer', 'minimum': 0, 'maximum': 65535, 'default': 31400},
                PATH: {'type': 'string'},
            }
        }
    }

    def __init__(self, subsystem=3, port=31400, path=None):
        self._log = logging.getLogger(__name__) if not hasattr(self, '_log') else self._log
        self._log.info('init subsystem:%s port:%s', subsystem, port)

        self._subsystem = subsystem
        self._port = port
        self._path = path

        if not six.PY2:
            raise RuntimeError('kalaccess is currently only supported for python 2')

        # specify new kalsim binding
        os.environ["KALSIM_HOSTS"] = "localhost=" + str(self._port)

        if path:
            if not self._path is sys.path:
                sys.path.insert(1, os.path.abspath(self._path))
            kal_path = os.path.join(self._path, 'kalaccess.py')
            import imp
            mod = imp.load_source('_kalaccess_', kal_path)
            Kaccess = mod.Kalaccess
            # this does not work as our modules is called kalaccess as well
            # from kalaccess import Kalaccess as Kaccess  # @UnresolvedImport
        else:
            # pylint: disable=import-error
            from kal_python_tools.kalaccess import Kalaccess as Kaccess  # @UnresolvedImport
        self._kal = Kaccess()

    @log_input(logging.INFO)
    def connect(self):
        '''
        Connect kalaccess to kalsim

        Raises:
            RuntimeError: If the instrument is already started
        '''
        self._kal.connect(trans='kalsim SPIPORT=2', subsys=self._subsystem)
        self._kal.run()

    def check_connected(self):
        '''
        Check if instrument is connected to kalsim and ready for operation

        Returns:
            bool: kalaccess connected
        '''
        return self._kal.is_connected()

    @log_input(logging.INFO)
    def disconnect(self):
        '''
        Disconnect instrument.
        '''
        self._kal.disconnect()

    def get_kal_access(self):
        '''
        Get Kalaccess instance

        Returns
            kalaccess.Kalaccess: kalaccess instance
        '''
        return self._kal
