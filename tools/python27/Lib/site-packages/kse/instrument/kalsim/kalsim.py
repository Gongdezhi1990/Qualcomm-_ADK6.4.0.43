'''
Kalimba Simulator (kalsim) instrument
'''

import logging
import os
import subprocess
import sys
import threading

from kats.framework.library.instrument import Instrument
from kats.framework.library.log import log_input, log_exception

INSTRUMENT = 'kalsim'
PATH = 'path'
FIRMWARE = 'firmware'
PORT = 'port'
CLIENT = 'client'
VERBOSE = 'verbose'
QUIET = 'quiet'
DEBUG = 'debug'
DEBUG_PORT = 'debug_port'
CMDLINE_ARGS = 'cmdline_args'

DEFAULT_DEBUG_PORT = 31400


class Kalsim(Instrument):
    '''
    kalimba simulator instrument.

    This instrument creates a kalimba simulator process, with control over the command line
    options. By default it starts kalsim as server for kalcmd2 and the debugger.

    Args:
        path (str): Full path to kalsim binary
        firmware (str): Full path to kalsim firmware file without file extension
            (as kalsim expects it)
        port (int): Port to use to connect to kalsim
        client (bool): Connect to kalcmd2 as client with port
        verbose (bool): Enable kalsim output of several messages to console
        quiet (bool): Disable banner and progress information
        debug (bool): Enable kalsim to connect to debugger. kalsim will not run until the debugger
            connects and issues a run
        debug_port (int): port to be used by debugger
        cmdline_args (list[str]): Addtional command-line arguments for kalsim
    '''

    interface = 'kalsim'
    schema = {
        'type': 'array',
        'minItems': 1,
        'uniqueItems': True,
        'items': {
            'type': 'object',
            'required': [PATH, FIRMWARE, PORT],
            'properties': {
                PATH: {'type': 'string'},
                FIRMWARE: {'type': 'string'},
                PORT: {'type': 'integer'},
                CLIENT: {'type': 'boolean'},
                VERBOSE: {'type': 'boolean'},
                QUIET: {'type': 'boolean'},
                DEBUG: {'type': 'boolean'},
                DEBUG_PORT: {'type': 'integer', 'minimum': 0, 'maximum': 65535},
                CMDLINE_ARGS: {
                    'type': 'array',
                    'minItems': 1,
                    'uniqueItems': True,
                    'items': {
                        'type': 'string',
                    },
                },
            }
        }
    }

    def __init__(self, path, firmware, port, **kwargs):
        self._log = logging.getLogger(__name__) if not hasattr(self, '_log') else self._log
        self._log.info('init path:%s firmware:%s port:%s params:%s',
                       path, firmware, port, str(kwargs))

        self._path = path
        self._firmware = os.path.splitext(firmware)[0]
        self._port = port
        self._params = {}
        self._params[CLIENT] = kwargs.pop(CLIENT, False)
        self._params[VERBOSE] = kwargs.pop(VERBOSE, False)
        self._params[QUIET] = kwargs.pop(QUIET, False)
        self._params[DEBUG] = kwargs.pop(DEBUG, False)
        self._params[DEBUG_PORT] = kwargs.pop(DEBUG_PORT, DEFAULT_DEBUG_PORT)
        self._params[CMDLINE_ARGS] = kwargs.pop(CMDLINE_ARGS, [])
        self._proc = None  # kalsim process handler

        if kwargs:
            self._log.warning('parameters:%s unknown', str(kwargs))

    @log_input(logging.INFO)
    @log_exception
    def start(self):
        '''
        Start kalsim

        This will start the kalsim binary

        Raises:
            RuntimeError: If the instrument is already started
        '''
        if self._proc:
            raise RuntimeError('kalsim already started')

        # build command line arguments for kalsim binary
        args = [self._path, self._firmware]

        if self._params[CLIENT]:
            args += ['--kalcmd2-client', 'localhost', str(self._port)]  # connect as client
        else:
            args += ['--kalcmd2', str(self._port)]  # wait for connection as server

        args += ['--non-interactive']  # disable stdin commands as otherwise it would capture stdin

        if self._params[DEBUG]:
            args.append('-d')
            if self._params[DEBUG_PORT]:
                args += ['--listenport', str(self._params[DEBUG_PORT])]

        if self._params[VERBOSE]:
            args += [
                '--kalcmd2-perf',  # enable performance monitoring
                '--kalcmd2-perf-print',  # enable performance printing per second
                '--visible-watchdog'  # print kalcmd2 watchdog to console
            ]

        args += self._params[CMDLINE_ARGS]

        self._log.info('starting kalsim %s', args)
        try:
            self._proc = subprocess.Popen(args, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                          stderr=sys.stderr)
        except Exception:
            self.stop()
            raise RuntimeError('unable to execute kalsim binary:%s' % (args))

        # create kalsim stdout polling thread
        self._log.info('creating kalsim stdout thread')
        thread_hnd = threading.Thread(target=self._stdout_thread, args=())
        thread_hnd.daemon = True
        thread_hnd.setName('kalsim background')
        thread_hnd.start()

    def check_started(self):
        '''
        Check if instrument is connected to kalsim and ready for operation

        For it to be ready, a start should have been issued previously and kalsim binary should not
        have exited

        Returns:
            bool: kalsim started
        '''
        return self._proc is not None

    @log_input(logging.INFO)
    def stop(self):
        '''
        Close instrument.
        '''
        if self._proc:
            self._proc.terminate()
            self._proc.wait()
            self._proc = None

    @log_input(logging.INFO)
    @log_exception
    def set_port(self, port):
        '''
        Change the port value

        This method can only be called if the instrument is not started

        Args:
            port (int): New port to use to connect to kalsim

        Raises:
            RuntimeError: If the instrument is started
        '''
        if not self._proc:
            self._port = port
        else:
            raise RuntimeError('unable to change port while running')

    def get_debug(self):
        '''
        Get debug parameter value

        Returns:
            bool: Debug enabled
        '''
        return self._params[DEBUG]

    def _stdout_thread(self):
        '''
        kalsim stdout monitor thread

        kalsim stdout is connected to a pipe. This thread is monitoring that pipe
        and generating log messages for everything that kalsim is outputing
        '''
        self._log.debug('kalsim stdout thread starting')
        for line in iter(self._proc.stdout.readline, b''):
            self._log.info('kalsim stdout:%s', str(line.decode('utf-8')).rstrip('\r\n'))
        self.stop()
        self._log.info('kalsim died, stdout thread exiting')
