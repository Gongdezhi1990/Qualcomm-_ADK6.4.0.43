#! /usr/bin/env python
'''
Kalsim shell
'''

from __future__ import print_function

import argparse
import copy
import logging
import sys
import time

import os

from kats.framework.library.collection import json_substitute_env
from kats.framework.library.config import set_config_param, DEBUG_SESSION
from kats.framework.library.constant import WORKSPACE_ENV
from kats.framework.library.file_util import load
from kats.framework.library.log import setup_logging, log_input
from kats.framework.library.repl import launch_repl
from kats.instrument.kalcmd.kalcmd import Kalcmd
from kats.instrument.kalsim.kalsim import Kalsim
from kats.kalimba.capability.capability_factory import CapabilityFactory
from kats.kalimba.endpoint.endpoint_factory import EndpointFactory
from kats.kalimba.kymera.kymera import kymera_get_instance
from kats.kalimba.uut.uut import uut_get_instance
from kats.kalsim.graph.graph import Graph
from kats.kalsim.stream.kalcmd_stream import KalcmdStream
from kats.kalsim.stream.stream_factory import StreamFactory
from kats.library.capability import load_capability_file
from kats.library.registry import register_instance

DESCRIPTION = '''Kalsim Shell.
This in an utility to debug, prototype and test interactively kalimba code running in
kalsim.
It does coordinate three different pieces of software, kalcmd2, kalsim and kalaccess
placing a nice REPL on top of it
'''

LOG_FILE_DEFAULT = 'config/log.kalsim_shell.cfg.json'
LOG_LEVEL_DEFAULT = logging.WARN

PLATFORM = 'platform'
PLATFORM_CRESCENDO = 'crescendo'
PLATFORM_STRE = 'stre'
PLATFORM_STREPLUS = 'streplus'
PLATFORM_DEFAULT = PLATFORM_CRESCENDO

CAPABILITY_DESCRIPTION = 'capability_description'
CAPABILITY_DESCRIPTION_DEFAULT = 'config/capability.cfg.json'

REPL = 'repl'
REPL_DEFAULT = 'ipython'

SCRIPT = 'script'
SCRIPT_DEFAULT = ''

EXIT = 'exit'
EXIT_DEFAULT = False

KALCMD = 'kc'
KALCMD_CLIENT = KALCMD + '_' + 'client'
KALCMD_PORT = KALCMD + '_' + 'port'

KALACCESS = 'ka'
KALACCESS_PATH = KALACCESS + '_' + 'path'
KALACCESS_SKIP = KALACCESS + '_' + 'skip'
KALACCESS_SUBSYSTEM = KALACCESS + '_' + 'subsystem'

KALSIM = 'ks'
KALSIM_SKIP = KALSIM + '_' + 'skip'
KALSIM_PATH = KALSIM + '_' + 'path'
if sys.platform == 'win32':
    KALSIM_PATH_DEFAULT = 'tool/kalsim/kalsim_crescendo_audio.exe'
else:
    KALSIM_PATH_DEFAULT = 'tool/kalsim/kalsim_crescendo_audio'
KALSIM_FIRMWARE = KALSIM + '_' + 'firmware'
KALSIM_PORT = KALSIM + '_' + 'port'
KALSIM_CLIENT = KALSIM + '_' + 'client'
KALSIM_VERBOSE = KALSIM + '_' + 'verbose'
KALSIM_QUIET = KALSIM + '_' + 'quiet'
KALSIM_DEBUG = KALSIM + '_' + 'debug'
KALSIM_DEBUG_PORT = KALSIM + '_' + 'debug_port'
KALSIM_CMDLINE_ARGS = KALSIM + '_' + 'cmdline_args'

DEFAULT_KALSIM_DEBUG_PORT = 31400

ACAT = 'acat'
ACAT_PATH = ACAT + '_' + 'path'
ACAT_USE = ACAT + '_' + 'use'
ACAT_BUNDLE = ACAT + '_' + 'bundle'
ACAT_FIRMWARE_LOG_ENABLE = ACAT + '_' + 'firmware_log_enable'
ACAT_FIRMWARE_LOG_LEVEL = ACAT + '_' + 'firmware_log_level'
ACAT_FIRMWARE_LOG_TIMER_UPDATE_PERIOD = ACAT + '_' + 'firmware_log_timer_update_period'

HYDRA_FTP_SERVER = 'hydra_ftp_server'
HYDRA_FTP_SERVER_SKIP = HYDRA_FTP_SERVER + '_' + 'skip'
HYDRA_FTP_SERVER_DIRECTORY = HYDRA_FTP_SERVER + '_' + 'directory'
HYDRA_FTP_SERVER_PREFIX = HYDRA_FTP_SERVER + '_' + 'prefix'

PSTORE = 'pstore'
PSTORE_USE = PSTORE + '_' + 'use'
PSTORE_FILENAME = PSTORE + '_' + 'filename'

OPTIONS = {
    # type, default value, argparse action, multiple parameters description
    KALCMD_CLIENT: (bool, False, 'store_true', False,
                    """if set kalcmd acts as client in reference to kalsim,
                    the parameter %s should be set to non priviledged port (above 1023)
                    otherwise kalcmd is a server""" % (KALCMD_PORT)),
    KALCMD_PORT: (int, 0, 'store', False,
                  """kalcmd port for kalsim connections,
                  if %s is not set, 0 is a valid value which means any free port,
                  if kc_client is set then an available port should be set""" % (KALCMD_CLIENT)),

    KALACCESS_PATH: (str, None, 'store', False,
                     """if set kalaccess will be used from here, if not it should be installed as a
                     package"""),
    KALACCESS_SKIP: (bool, False, 'store_true', False,
                     """for kalaccess to be connected, kalsim has to be started in debug mode
                     (see below) and this flag should not be set,
                     if kalsim is started in debug mode and kalaccess is skipped then an external
                     kalaccess should be connected and a run issued for the initialisation phase
                     to complete"""),
    KALACCESS_SUBSYSTEM: (int, 3, 'store', False, 'kalaccess subsystem for connect'),

    KALSIM_SKIP: (bool, False, 'store_true', False,
                  """if set kalsim binary will not be started and no other kalsim parameter
                    will be used, kalcmd will try to connect to an existing kalsim process,
                    for this to work %s should not be 0
                    otherwise kalsim will be executed"""),
    KALSIM_PATH: (str, KALSIM_PATH_DEFAULT, 'store', False,
                  'kalsim binary path'),
    KALSIM_FIRMWARE: (str, 'firmware/crescendo/kymera_crescendo_audio', 'store', False,
                      'kymera binary path without extension, it is used in acat as well'),
    KALSIM_VERBOSE: (bool, False, 'store_true', False, 'kalsim enable verbose mode'),
    KALSIM_QUIET: (bool, False, 'store_true', False, 'kalsim enable quiet mode'),
    KALSIM_DEBUG: (bool, False, 'store_true', False,
                   'kalsim enable debug, valid kalaccess parameters should be provided'),
    KALSIM_DEBUG_PORT: (int, DEFAULT_KALSIM_DEBUG_PORT, 'store', False,
                        """kalsim will open debugger using this port, valid port and valid kalaccess
                        parameters must be provided"""),
    KALSIM_CMDLINE_ARGS: (str, ['--disable-dots'], 'store', True,
                          """kalsim extra command line arguments,
                          please surround every parameter with '' and add a leading space
                          as in ' --param'"""),
    ACAT_PATH: (str, None, 'store', False,
                """if set ACAT will be used from here, if not it should be installed as a
                package"""),
    ACAT_USE: (bool, False, 'store_true', False,
               """if set ACAT will be started, for this to work %s should be set""" % (
                   KALSIM_DEBUG)),
    ACAT_BUNDLE: (str, [], 'store', True,
                  """ACAT bundle file/s, additional firmware files to be loaded besides main
                  firmware"""),
    ACAT_FIRMWARE_LOG_ENABLE: (
        bool, False, 'store_true', False, """if set ACAT firmware log is enabled"""),
    ACAT_FIRMWARE_LOG_LEVEL: (int, 2, 'store', False, """firmware log level"""),
    ACAT_FIRMWARE_LOG_TIMER_UPDATE_PERIOD: (
        float, 0.5, 'store', False, """firmware log polling time in seconds"""),
    HYDRA_FTP_SERVER_SKIP: (bool, False, 'store_true', False,
                            """if set Hydra FTP server will not be started."""),
    HYDRA_FTP_SERVER_DIRECTORY: (str, 'firmware/crescendo', 'store', False,
                                 """hydra ftp server directory."""),
    HYDRA_FTP_SERVER_PREFIX: (str, '', 'store', False,
                              """hydra ftp server filename prefix."""),
    PSTORE_USE: (bool, False, 'store_true', False,
                 """if set PStore will be started."""),
    PSTORE_FILENAME: (str, 'config/pstore.cfg.json', 'store', False,
                      """PStore filename."""),
}

KALCMD_LOCK_TIMEOUT_DEBUG = 3600
UUT_MESSAGE_RECEIVE_TIMEOUT_DEBUG = 3600


class KalsimShell(object):
    '''
    Kalsim shell

    This basically initialises a kalsim/kalcmd/kalaccess environment with multiple options,
    then instantiates some middleware libraries and invokes a repl to let the user
    send commands interactively

    Args:
        repl (str): Read Evaluate Print Loop tool
        platform (str): Chip platform
    '''

    def __init__(self, **kwargs):
        self._log = logging.getLogger(__name__) if not hasattr(self, '_log') else self._log

        self._namespace = {}
        self._param = argparse.Namespace()
        self._param.repl = kwargs.pop(REPL, REPL_DEFAULT)
        self._param.platform = kwargs.pop(PLATFORM, PLATFORM_DEFAULT)
        self._param.capability_description = kwargs.pop(CAPABILITY_DESCRIPTION,
                                                        CAPABILITY_DESCRIPTION_DEFAULT)
        self._param.debug = kwargs.pop(KALSIM_DEBUG, OPTIONS[KALSIM_DEBUG][1])
        self._param.debug_port = kwargs.pop(KALSIM_DEBUG_PORT, OPTIONS[KALSIM_DEBUG_PORT][1])
        self._param.server = not kwargs.pop(KALCMD_CLIENT, OPTIONS[KALCMD_CLIENT][1])
        self._param.port = kwargs.pop(KALCMD_PORT, OPTIONS[KALCMD_PORT][1])
        self._param.kalaccess_path = kwargs.pop(KALACCESS_PATH, OPTIONS[KALACCESS_PATH][1])
        self._param.kalaccess_skip = kwargs.pop(KALACCESS_SKIP, OPTIONS[KALACCESS_SKIP][1])
        self._param.kalsim_skip = kwargs.pop(KALSIM_SKIP, OPTIONS[KALSIM_SKIP][1])
        self._param.acat_path = kwargs.pop(ACAT_PATH, OPTIONS[ACAT_PATH][1])
        self._param.acat_use = kwargs.pop(ACAT_USE, OPTIONS[ACAT_USE][1])
        self._param.acat_bundle = kwargs.pop(ACAT_BUNDLE, OPTIONS[ACAT_BUNDLE][1])
        self._param.acat_firmware_log_enable = kwargs.pop(ACAT_FIRMWARE_LOG_ENABLE,
                                                          OPTIONS[ACAT_FIRMWARE_LOG_ENABLE][1])
        self._param.acat_firmware_log_level = kwargs.pop(ACAT_FIRMWARE_LOG_LEVEL,
                                                         OPTIONS[ACAT_FIRMWARE_LOG_LEVEL][1])
        self._param.acat_firmware_log_timer_update_period = \
            kwargs.pop(ACAT_FIRMWARE_LOG_TIMER_UPDATE_PERIOD,
                       OPTIONS[ACAT_FIRMWARE_LOG_TIMER_UPDATE_PERIOD][1])
        self._param.script = kwargs.pop(SCRIPT, SCRIPT_DEFAULT)
        self._param.exit = kwargs.pop(EXIT, EXIT_DEFAULT)

        self._param.hydra_ftp_server_skip = kwargs.pop(HYDRA_FTP_SERVER_SKIP,
                                                       OPTIONS[HYDRA_FTP_SERVER_SKIP][1])
        self._param.hydra_ftp_server_directory = kwargs.pop(HYDRA_FTP_SERVER_DIRECTORY,
                                                            OPTIONS[HYDRA_FTP_SERVER_DIRECTORY][1])
        self._param.hydra_ftp_server_prefix = kwargs.pop(HYDRA_FTP_SERVER_PREFIX,
                                                         OPTIONS[HYDRA_FTP_SERVER_PREFIX][1])
        self._param.pstore_use = kwargs.pop(PSTORE_USE, OPTIONS[PSTORE_USE][1])
        self._param.pstore_filename = kwargs.pop(PSTORE_FILENAME, OPTIONS[PSTORE_FILENAME][1])

        self._config = argparse.Namespace()

        # initialise kalcmd instrument parameters
        self._config.kalcmd = {
            'server': self._param.server,
            'port': self._param.port,
        }

        # initialise kalaccess instrument parameters (if it is enabled)
        self._config.kalaccess = None
        if self._param.debug and not self._param.kalaccess_skip:
            self._config.kalaccess = {
                'subsystem': kwargs.pop(KALACCESS_SUBSYSTEM, OPTIONS[KALACCESS_SUBSYSTEM][1]),
                'port': self._param.debug_port
            }
            if self._param.kalaccess_path:
                self._config.kalaccess['path'] = self._param.kalaccess_path

        # initialise kalsim instrument parameters
        self._config.kalsim = None
        if not self._param.kalsim_skip:
            self._config.kalsim = {
                'path': kwargs.pop(KALSIM_PATH, OPTIONS[KALSIM_PATH][1]),
                'firmware': kwargs.get(KALSIM_FIRMWARE, OPTIONS[KALSIM_FIRMWARE][1]),
                'port': self._param.port,
                'client': self._param.server,
                'verbose': kwargs.pop(KALSIM_VERBOSE, OPTIONS[KALSIM_VERBOSE][1]),
                'quiet': kwargs.pop(KALSIM_QUIET, OPTIONS[KALSIM_QUIET][1]),
                'debug': self._param.debug,
                'debug_port': self._param.debug_port,
                'cmdline_args': kwargs.pop(KALSIM_CMDLINE_ARGS, OPTIONS[KALSIM_CMDLINE_ARGS][1]),
            }

        # initialise acat instrument parameters
        self._config.acat = None
        if self._param.acat_use:
            self._config.acat = {
                'firmware': kwargs.get(KALSIM_FIRMWARE, OPTIONS[KALSIM_FIRMWARE][1]),
                'interactive': True,
            }
            if self._param.acat_path:
                self._config.acat['path'] = self._param.acat_path
            if self._param.kalaccess_path:
                self._config.acat['kal_path'] = self._param.kalaccess_path
            if self._param.acat_bundle:
                self._config.acat['bundle'] = self._param.acat_bundle
            if self._param.acat_firmware_log_enable:
                self._config.acat['firmware_log_enable'] = self._param.acat_firmware_log_enable
            if self._param.acat_firmware_log_enable:
                self._config.acat['firmware_log_level'] = self._param.acat_firmware_log_level
            self._config.acat['firmware_log_timer_update_period'] = \
                self._param.acat_firmware_log_timer_update_period

        self._instr = argparse.Namespace()
        self._instr.kalcmd = None
        self._instr.kalsim = None
        self._instr.kalaccess = None
        self._instr.acat = None

        self.helper = argparse.Namespace()
        self.helper.uut = None
        self.helper.stream = None
        self.helper.kymera = None
        self.helper.endpoint = None
        self.helper.capability = None
        self.helper.graph = None

    @log_input(logging.INFO)
    def initialise(self):
        '''
        Initialise kalsim shell environment

        This includes launching the selected combination of kalcmd, kalsim and kalaccess
        components and starting several middleware components as uut, stream, kymera, endpoint
        and capability handlers
        '''
        print('Initialising')

        if self._param.debug:
            set_config_param('KALCMD_LOCK_TIMEOUT', KALCMD_LOCK_TIMEOUT_DEBUG)
            set_config_param('UUT_MESSAGE_RECEIVE_TIMEOUT', UUT_MESSAGE_RECEIVE_TIMEOUT_DEBUG)
        if DEBUG_SESSION:
            set_config_param('UUT_MESSAGE_RECEIVE_TIMEOUT', UUT_MESSAGE_RECEIVE_TIMEOUT_DEBUG)

        self._instr.kalsim = None
        if self._config.kalsim:
            self._instr.kalsim = Kalsim(**self._config.kalsim)

        self._instr.kalaccess = None
        if self._config.kalaccess:
            from kats.instrument.kalaccess.kalaccess import Kalaccess
            self._instr.kalaccess = Kalaccess(**self._config.kalaccess)

        self._instr.kalcmd = Kalcmd(**self._config.kalcmd)
        register_instance('kalcmd', self._instr.kalcmd)

        if self._param.debug and self._param.kalaccess_skip:
            print(
                'Initialisation will not be completed until an external kalaccess session is connected and run issued')

        # kalcmd is smart enough to spawn kalsim and kalaccess in its connect method
        self._instr.kalcmd.connect(self._instr.kalsim, self._instr.kalaccess)

        print('Kalcmd connected to a %s' % (self._instr.kalcmd.get_kalimba_name()))

        self._instr.acat = None
        if self._config.acat:
            from kats.instrument.acat.acat import Acat
            self._instr.acat = Acat(**self._config.acat)
            self._instr.acat.connect()

        cap = load_capability_file(self._param.capability_description)
        register_instance('capability_description', cap)

        # load all the middleware that will be available in the repl
        platforms = ['common', 'kalsim', self._param.platform]
        self.helper.uut = uut_get_instance(platforms, self._instr.kalcmd)
        register_instance('uut', self.helper.uut)

        self.helper.kalcmd_stream = KalcmdStream(self._instr.kalcmd)
        register_instance('kalcmd_stream', self.helper.kalcmd_stream)
        self.helper.stream = StreamFactory(platforms, self._instr.kalcmd)
        register_instance('stream', self.helper.stream)

        self.helper.kymera = kymera_get_instance(platforms, self.helper.uut)
        self.helper.endpoint = EndpointFactory(platforms, self.helper.kymera)
        self.helper.capability = CapabilityFactory(platforms, self.helper.kymera)
        self.helper.graph = Graph(self.helper.stream,
                                  self.helper.endpoint,
                                  self.helper.capability,
                                  None,
                                  self.helper.kymera,
                                  None)

        if self._param.platform == PLATFORM_CRESCENDO or self._param.platform == PLATFORM_STRE or \
                self._param.platform == PLATFORM_STREPLUS:
            self.helper.accmd = self.helper.kymera._accmd  # pylint: disable = protected-access
            register_instance('accmd', self.helper.accmd)

            from kats.kalimba.hydra_service.protocol import HydraProtocol
            self.helper.hydra_protocol = HydraProtocol(self.helper.uut)
            register_instance('hydra_protocol', self.helper.hydra_protocol)

            from kats.kalimba.hydra_service.download_service import HydraCapabilityDownloadService
            self.helper.hydra_cap_download = HydraCapabilityDownloadService(
                self.helper.stream, self.helper.accmd, self.helper.hydra_protocol,
                service_tag=1000)
            register_instance('hydra_cap_download', self.helper.hydra_cap_download)

            if not self._param.hydra_ftp_server_skip:
                from kats.kalsim.component.hydra_ftp_server import HydraFtpServer
                config = {
                    'directory': self._param.hydra_ftp_server_directory,
                    'prefix': self._param.hydra_ftp_server_prefix,
                }
                self.helper.hydra_ftp_server = HydraFtpServer(self.helper.hydra_protocol, **config)
                register_instance('hydra_ftp_server', self.helper.hydra_ftp_server)

            if self._param.pstore_use:
                from kats.kalsim.component.pstore import PStore
                config = {
                    'filename': self._param.pstore_filename,
                }
                self.helper.ps_store = PStore(**config)
                register_instance('pstore', self.helper.ps_store)

    @log_input(logging.INFO)
    def run(self):
        '''
        Executing shell
        '''
        self._namespace = {
            'uut': self.helper.uut,
            'kymera': self.helper.kymera,
            'endpoint': self.helper.endpoint,
            'capability': self.helper.capability,
            'stream': self.helper.stream,
            'graph': self.helper.graph,
            'kalcmd': self._instr.kalcmd,
            'reset': self.reset,
            'khelp': self.help
        }
        if self._instr.kalaccess:
            self._namespace['kal'] = self._instr.kalaccess.get_kal_access()
        if self._instr.acat:
            self._namespace['acat'] = self._instr.acat.get_session()
        if vars(self.helper).get('hydra_protocol', None):
            self._namespace['hydra_protocol'] = vars(self.helper)['hydra_protocol']
        if vars(self.helper).get('hydra_cap_download', None):
            self._namespace['hydra_cap_download'] = vars(self.helper)['hydra_cap_download']

        if self._param.script:
            for script in self._param.script:
                params = script.split()
                print('Executing %s %s' % (params[0], ' '.join(params[1:])))
                nspace = copy.copy(self._namespace)
                nspace['__name__'] = '__main__'
                sys.argv = params
                with open(params[0]) as handler:
                    exec (handler.read(), nspace)

        if self._param.exit:
            print('Exit requested')
        else:
            print('khelp() to obtain help')
            print('Launching repl %s' % (self._param.repl))
            launch_repl(self._param.repl, self._namespace)

    def help(self):
        '''
        Display a help screen
        '''
        instance_list = [
            ('uut', 'unit under test abstraction'),
            ('kymera', 'kymera internal api'),
            ('endpoint', 'endpoint factory'),
            ('capability', 'capability factory'),
            ('stream', 'stream factory'),
            ('graph', 'graph helper'),
            ('kalcmd', 'kalcmd interface to kalcmd2/kalsim'),
            ('kal', 'kalimba access python tools'),
            ('acat', 'Audio Coredump Analysis Tool'),
            ('hydra_protocol', 'hydra protocol'),
            ('hydra_cap_download', 'hydra capability download'),
        ]

        print('')
        print('This is in interactive session in a REPL')
        print('')
        print('Instances added to the execution environment are:')
        for entry in instance_list:
            if entry[0] in self._namespace:
                print('  %-20s %s' % (entry[0], entry[1]))
        print('')
        print('Commands added to the execution environment are:')
        print('  reset() to restart the session')

    @log_input(logging.INFO)
    def reset(self):
        '''
        Disconnect kalsim, kalcmd and kalaccess and recrete the connetions
        '''
        print('Resetting')

        self.clean_up()
        time.sleep(1)
        self._instr.kalcmd.connect(self._instr.kalsim, self._instr.kalaccess)

        if self._config.acat:
            self._instr.acat.connect()

    @log_input(logging.INFO)
    def clean_up(self):
        '''
        Disconnect kalsim, kalcmd and kalaccess
        '''
        if self._instr.acat:
            try:
                self._instr.acat.disconnect()
            except Exception:  # pylint: disable=broad-except
                pass

        if self._instr.kalcmd:
            try:
                self._instr.kalcmd.disconnect()
            except Exception:  # pylint:disable=broad-except
                pass


def create_argparser(description):
    '''
    Parse command line arguments

    Args:
        description (str): Parser description

    Returns:
        argparse.ArgumentParser: Argument parser
    '''
    parser = argparse.ArgumentParser(description=description,
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('-l', '--log', default=LOG_FILE_DEFAULT, help='Log configuration file')
    parser.add_argument('--log_level', type=int, metavar='LEVEL', default=LOG_LEVEL_DEFAULT,
                        help='Log level')
    parser.add_argument('--repl', default=REPL_DEFAULT, help='Read Eval Print Loop tool')
    parser.add_argument('--platform', default=PLATFORM_DEFAULT, help='Chip platform')
    parser.add_argument('--capability_description', default=CAPABILITY_DESCRIPTION_DEFAULT,
                        help='Configuration file with description of capabilities')
    parser.add_argument('--script', nargs='+',
                        help='''Script to execute after kalsim connect. For script arguments enclose
                         it with "" as --script "script1.py arg1" "script2.py arg1 arg2"''')
    parser.add_argument('--exit', action='store_true', help='Exit after connection')

    # command line arguments are described in the OPTIONS dict
    for entry in sorted(OPTIONS):
        args = ['--' + entry]
        kwargs = {
            'action': OPTIONS[entry][2],
            'default': OPTIONS[entry][1],
            'help': OPTIONS[entry][4],
        }
        if OPTIONS[entry][2] != 'store_true' and OPTIONS[entry][2] != 'store_false':
            kwargs['type'] = OPTIONS[entry][0]

        if OPTIONS[entry][2] == 'store':
            kwargs['metavar'] = 'VAL'

        if OPTIONS[entry][3]:
            kwargs['nargs'] = '+'

        parser.add_argument(*args, **kwargs)

    return parser


def main():
    '''
    Script entry point
    '''
    args = create_argparser(DESCRIPTION).parse_args()
    for index, entry in enumerate(args.ks_cmdline_args):
        args.ks_cmdline_args[index] = entry.strip("' ")

    # if KATS_WORKSPACE is defined then we change our current directory to point there
    workspace = os.environ.get(WORKSPACE_ENV)
    if not workspace:
        print('%s environmental variable not defined running from local directory' %
              (WORKSPACE_ENV))
    else:
        print('workspace set to %s' % (workspace))
        os.chdir(workspace)

    # load log configuration file
    config = load(args.log)
    config = json_substitute_env(config)
    setup_logging(config, args.log_level)

    # proceed with the shell
    shell = KalsimShell(**vars(args))
    try:
        shell.initialise()
        shell.run()
    except Exception:
        raise
    finally:
        shell.clean_up()


if __name__ == '__main__':
    main()
