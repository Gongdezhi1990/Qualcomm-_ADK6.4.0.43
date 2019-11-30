'''
Kalimba Command (kalcmd) instrument
'''

import argparse
import logging
import socket
import threading
import time

import kalcmd2

from kats.framework.library.config import get_config_param
from kats.framework.library.instrument import Instrument
from kats.framework.library.log import log_exception, log_input, log_output, dump
from kats.framework.library.util import RLockTimeout
from .msg_header import MsgHeader

INSTRUMENT = 'kalcmd'
SERVER = 'server'
PORT = 'port'

KALCMD_EVENT_TERMINATE = 'terminate'
KALCMD_EVENT_TIMER = 'timer'
KALCMD_EVENT_MESSAGE = 'message'
KALCMD_EVENT_STREAM_DATA = 'stream_data'
KALCMD_EVENT_STREAM_EOF = 'stream_eof'
KALCMD_EVENT_KALIMBA_WANTS_ATTENTION = 'kalimba_wants_attention'

KALCMD2_EVENT = 'event'
KALCMD2_RETURN_CODE = 'return_code'
KALCMD2_PAYLOAD = 'payload'
KALCMD2_VALUE = 'value'
KALCMD2_TIME = 'time'
KALCMD2_KAL_ARCH = 'kal_arch'
KALCMD2_NAME = 'name'
KALCMD2_MAJOR = 'major'
KALCMD2_MINOR = 'minor'
KALCMD2_COOKIE = 'cookie'
KALCMD2_STREAM_ID = 'stream_id'
KALCMD2_STREAM_IDS = 'stream_ids'
KALCMD2_SAMPLES = 'samples'
KALCMD2_FORMAT = 'format'
KALCMD2_SAMPLES_INDUCED = 'samples_induced'
KALCMD2_CLOCK_SPEED = 'clock_speed'
KALCMD2_WORDS = 'words'

WAKE_TIMER_USEC = 50000  # this dictates maximum time between sent messages
WAKE_TIMER_COOKIE = 0xFFFFFFFF

SOCKET_SERVER_TIMEOUT = 60  # socket communication timeout in server mode without debugger
SOCKET_SERVER_TIMEOUT_DEBUGGER = 3600  # socket communication timeout in server mode with debugger

LOCK_TIMEOUT_DEFAULT = 3


class Kalcmd(Instrument):
    '''
    kalimba kalcmd2 instrument

    This instrument handles kalcmd2 but when connecting it is able to coordinate
    kalsim (kalimba simulator) and

    Args:
        server (bool): If True create a server socket where kalsim should connect, if not connect
        port (int): Port number for the socket connection, if server is True then 0 is find free
            port in the system
    '''

    interface = 'kalcmd'
    schema = {
        'type': 'array',
        'minItems': 1,
        'uniqueItems': True,
        'items': {
            'type': 'object',
            'properties': {
                SERVER: {'type': 'boolean', },
                PORT: {'type': 'integer', },
            }
        }
    }

    def __init__(self, server=False, port=0):
        self._log = logging.getLogger(__name__) if not hasattr(self, '_log') else self._log
        self._log.info('init server:%s port:%s', server, port)

        self._server = server
        self._port = port

        self._instr = argparse.Namespace()
        self._instr.kalsim = None
        self._instr.kalaccess = None

        self._kalcmd = None  # kalcmd2 command

        self._data = argparse.Namespace()
        self._data.kalsim_ready = False
        self._data.conn = None  # kalcmd2 connection
        self._data.sock = None  # kalcmd2/kalsim accept socket
        self._data.server_sock = None  # kalcmd2/kalsim bind socket
        lock_timeout = get_config_param('KALCMD_LOCK_TIMEOUT', LOCK_TIMEOUT_DEFAULT)
        self._data.lock = RLockTimeout(lock_timeout)
        self._data.connected = False  # kalcmd2 socket connection has started
        self._data_background_thread = None
        self._data.handlers = {
            KALCMD_EVENT_TERMINATE: None,
            KALCMD_EVENT_TIMER: None,
            KALCMD_EVENT_MESSAGE: None,
            KALCMD_EVENT_STREAM_DATA: None,
            KALCMD_EVENT_STREAM_EOF: None,
            KALCMD_EVENT_KALIMBA_WANTS_ATTENTION: None,
        }
        self._data.pend_msgs = []
        self._data.pend_callbacks = []

        if self._server:
            socket.setdefaulttimeout(5)
        else:
            socket.setdefaulttimeout(120)

    def _start_kalsim(self):
        self._log.info('starting kalsim')
        self._instr.kalsim.start()

    def _disconnect_kalsim(self):
        if self._instr.kalsim:
            try:
                self._instr.kalsim.stop()
            except Exception:  # pylint: disable=broad-except
                pass
            self._instr.kalsim = None

    def _connect_kalaccess(self):
        self._log.info('connecting kalaccess')
        self._instr.kalaccess.connect()

    def _disconnect_kalaccess(self):
        if self._instr.kalaccess:
            try:
                self._instr.kalaccess.disconnect()
            except Exception:  # pylint: disable=broad-except
                pass
            self._instr.kalaccess = None

    def _create_background_thread(self):
        self._log.info('creating kalcmd2 background thread')
        self._data_background_thread = threading.Thread(target=self._background_thread, args=())
        self._data_background_thread.daemon = True
        self._data_background_thread.setName('kalcmd background')
        self._data_background_thread.start()

    def _wait_ready(self):
        while not self._data.kalsim_ready:
            if not self._data.connected:
                raise RuntimeError('unable to connect')
            time.sleep(0.05)

    def _add_callback(self, handler, *args, **kwargs):
        if handler in self._data.handlers and self._data.handlers[handler]:
            self._data.pend_callbacks.append({'handler': handler, 'args': args, 'kwargs': kwargs})

    def _invoke_callbacks(self):
        for callback in self._data.pend_callbacks:
            self._data.handlers[callback['handler']](*(callback['args']), **(callback['kwargs']))
        self._data.pend_callbacks = []

    def _handle_init(self):
        self._log.info('kalsim relative timer time:%ss cookie:%s added',
                       WAKE_TIMER_USEC / 1000000.0, WAKE_TIMER_COOKIE)
        self._kalcmd.set_timer_rel(WAKE_TIMER_USEC, WAKE_TIMER_COOKIE)

    def _handle_connected(self):
        self._data.kalsim_ready = True

    def _process_event(self, event):
        if event == kalcmd2.kalcmd_events.INIT_EVENT:
            self._log.info('kalsim init event')
            self._handle_init()
        elif event == kalcmd2.kalcmd_events.TERMINATE_EVENT:
            self._log.info('kalsim terminate event')
            self._add_callback(KALCMD_EVENT_TERMINATE)
            return True
        elif event == kalcmd2.kalcmd_events.KALSIM_ERROR_EVENT:
            self._log.info('kalsim error event')
            return True
        elif event == kalcmd2.kalcmd_events.MESSAGE_EVENT:
            msg = self._kalcmd.get_received_message()
            self._log.info('kalsim message event msg ret_code:%s header:%s payload:%s',
                           msg[KALCMD2_RETURN_CODE], msg['header'],
                           dump(msg[KALCMD2_PAYLOAD], '0x%04x'))
            self._add_callback(KALCMD_EVENT_MESSAGE, msg[KALCMD2_RETURN_CODE], msg['header'],
                               msg[KALCMD2_PAYLOAD])
        elif event == kalcmd2.kalcmd_events.TIMER_EVENT:
            self._handle_connected()
            cur_time = self.get_current_time()
            cookie = self.get_last_timer_cookie()
            if cookie != WAKE_TIMER_COOKIE:
                self._log.info('kalsim timer event cookie:%s at %.06fs', cookie, cur_time)
                self._add_callback(KALCMD_EVENT_TIMER, cookie, cur_time)
            else:
                self._kalcmd.set_timer_rel(WAKE_TIMER_USEC, WAKE_TIMER_COOKIE)
        elif event == kalcmd2.kalcmd_events.WATCHDOG_EVENT:
            cur_time = self.get_current_time()
            self._log.debug('kalsim watchdog event at %.06fs', cur_time)
        elif event == kalcmd2.kalcmd_events.STREAM_DATA_EVENT:
            stream, samples = self.stream_data_event_query()
            self._log.info('kalsim stream data event stream:%s samples:%s', stream, samples)
            self._add_callback(KALCMD_EVENT_STREAM_DATA, stream, samples)
        elif event == kalcmd2.kalcmd_events.STREAM_EOF_EVENT:
            stream = self.stream_eof_event_query()
            self._log.info('kalsim stream eof event stream:%s', stream)
            self._add_callback(KALCMD_EVENT_STREAM_EOF, stream)
        elif event == kalcmd2.kalcmd_events.KALIMBA_WANTS_ATTENTION_EVENT:
            self._log.info('kalsim kalimba wants attention event')
            self._add_callback(KALCMD_EVENT_KALIMBA_WANTS_ATTENTION)
        # Note NO_EVENT and STREAM_DONE_EVENT not handled
        return False

    def _background_thread(self):
        self._log.debug('background thread starting')

        while True:
            with self._data.lock:
                if not self._data.sock:
                    break

                try:
                    if self._data.pend_msgs:
                        header, payload = self._data.pend_msgs.pop(0)
                        self._send_message(header, payload)

                    cmd = self._kalcmd.get_next_event()
                    ret_code = cmd[KALCMD2_RETURN_CODE]
                    if ret_code != kalcmd2.kalcmd_response.KALCMD_OK:
                        self._log.warning('received ret_code %s', str(ret_code))
                    else:
                        event = cmd[KALCMD2_EVENT]
                        self._log.debug('received event %s', str(cmd))
                        if self._process_event(event):
                            break
                except Exception:  # pylint: disable=broad-except
                    self._log.exception('unable to communicate with kalcmd2')
                    break

            try:
                self._invoke_callbacks()
            except Exception:  # pylint: disable=broad-except
                self._log.exception('unable to invoke callbacks')
                break

        self.disconnect()
        self._log.info('background thread exiting')

    def get_lock_object(self):
        '''
        Get the lock object associated with shared access to kalcmd/kalsim socket
        This can be acquired to block the background thread from issuing a get_next_event
        which is equivalent to stopping firmware

        .. code-block:: python

            kallock = kalcmd.get_lock_object()
            kallock.acquire()
            # do operations with firmware stopped
            kalloc.release()


            # or even better
            kallock = kalcmd.get_lock_object()
            with kallock:
                # do operations with firmware stopped
                print(kalcmd.get_current_time())
                import time
                time.sleep(1)
                print(kalcmd.get_current_time())

        Returns:
            RLockTimeout: Lock object
        '''
        return self._data.lock

    @log_input(logging.INFO)
    @log_exception
    def connect(self, kalsim=None, kalaccess=None):
        '''
        Connect to kalcmd2/kalsim
        If kalsim is None the will try to connect to an externally executed kalsim
        If kalsim is an instrument this will start the kalsim binary and connect to it

        Args:
            kalsim (kats.instrument.kalsim): kalsim instrument or None
            kalaccess (kats.instrument.kalaccess): kalaccess instrment or None
        '''
        if self._data.connected:
            raise RuntimeError('kalcmd already connected')

        self._instr.kalsim = kalsim
        self._instr.kalaccess = kalaccess
        self._data.connected = True

        # ==========================================================================================
        # create server socket
        # configure the socket as blocking. Non blocking sockets will
        # throw an exception if recv is called without data in the buffer.
        # ==========================================================================================
        if self._server:
            self._data.server_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self._data.server_sock.settimeout(5)
            self._data.server_sock.setblocking(1)
            try:
                bind_to = ('localhost', self._port)
                self._data.server_sock.bind(bind_to)
            except socket.error:
                raise RuntimeError('kalcmd socket bind:%s failed' % (bind_to))
            self._data.server_sock.listen(1)
            self._log.info('server socket created %s', str(self._data.server_sock.getsockname()))
            if self._instr.kalsim:
                self._instr.kalsim.set_port(self._data.server_sock.getsockname()[1])

        kalsim_launched = False
        if self._instr.kalsim:
            self._start_kalsim()
            kalsim_launched = True

        if self._server:
            try:
                # Note, there is a problem in the connection when running in the grid,
                # some slow machines can make kalsim run slow and not being able to connect to us
                # in 10 seconds, so lets increase that time
                self._data.server_sock.settimeout(15)
                self._data.sock, addr = self._data.server_sock.accept()
                if not kalsim or not kalsim.get_debug():
                    timeout = SOCKET_SERVER_TIMEOUT
                else:
                    # debugger could freeze kalcmd socket for a while
                    timeout = SOCKET_SERVER_TIMEOUT_DEBUGGER
                self._data.sock.settimeout(timeout)
                _ = addr
            except Exception:
                self.disconnect()
                raise RuntimeError('server socket accept timeout, exiting')

            self._log.info('server socket connected by remote client')
            self._data.conn = kalcmd2.kalmsg_server_connection(self._data.sock)
            self._kalcmd = kalcmd2.kalcmd(self._data.conn)
        else:
            if kalsim_launched:
                time.sleep(1)  # give kalsim time to open server connection
            self._log.info('connecting client socket to localhost:%s', self._port)
            self._data.conn = kalcmd2.kalmsg_client_connection(port=self._port)
            self._kalcmd = kalcmd2.kalcmd(self._data.conn)
            self._data.sock = self._data.conn.socket

        # ==========================================================================================
        # create kalcmd2 background polling thread
        # ==========================================================================================
        self._create_background_thread()

        if self._instr.kalaccess:
            # once kalcmd is connected to kalsim, kalsim still has to load the firmware before
            # enabling the debugger port and that takes a while, we have to wait here because if
            # we try to connect immediately then kalaccess will fail
            print('waiting 10 sec before connecting kalaccess')
            time.sleep(10)  # give kalsim time to start
            self._connect_kalaccess()

        # ==========================================================================================
        # wait for kalsim ready
        # ==========================================================================================
        self._wait_ready()

        self._log.info('connected')

    def check_connected(self):
        '''
        Check if instrument is connected to kalsim and ready for operation

        Returns:
            bool: kalsim ready to be used
        '''
        return self._data.kalsim_ready

    @log_input(logging.INFO)
    def disconnect(self):
        '''
        Disconnect instrument.

        After the call the only available operation would be connect
        '''
        if self._data.connected:
            with self._data.lock:
                self._disconnect_kalaccess()

                if self.check_connected():
                    try:
                        self.terminate(0)
                        time.sleep(0.2)
                    except Exception:  # pylint: disable=broad-except
                        pass

                self._disconnect_kalsim()

                if self._data.sock:
                    try:
                        with self._data.lock:
                            self._data.sock.close()
                    except Exception:  # pylint: disable=broad-except
                        pass
                    del self._data.sock
                    self._data.sock = None

                if self._data.conn:
                    try:
                        self._data.conn.close()
                    except Exception:  # pylint: disable=broad-except
                        pass
                    del self._data.conn
                    self._data.conn = None

                if self._data.server_sock:
                    try:
                        self._data.server_sock.close()
                    except Exception:  # pylint: disable=broad-except
                        pass
                    del self._data.server_sock
                    self._data.server_sock = None

            self._data.connected = False

            # wait for background thread to die
            while self._data_background_thread and self._data_background_thread.is_alive():
                time.sleep(0.01)
            self._data_background_thread = None

            self._data.kalsim_ready = False

    @log_input(logging.INFO)
    def install_terminate_handler(self, handler):
        '''
        Install handler to be invoked when TERMINATE_EVENT is received from kalcmd2

        This handler will be invoked when the event is received as

        .. code-block:: python

            def terminate_handler():
                pass

        Args:
            handler (func()): Handler to install
        '''
        self._data.handlers[KALCMD_EVENT_TERMINATE] = handler

    @log_input(logging.INFO)
    def install_timer_handler(self, handler):
        '''
        Install handler to be invoked when TIMER_EVENT is received from kalcmd2

        This handler will be invoked when the event is received as

        .. code-block:: python

            def timer_handler(cookie, cur_time):
                # cookie (int) same parameter used in add_absolute_timer / add_relative_timer
                # cur_time (float) current time in seconds
                pass

        Args:
            handler (func(cookie, cur_time)): Handler to install
        '''
        self._data.handlers[KALCMD_EVENT_TIMER] = handler

    @log_input(logging.INFO)
    def install_message_handler(self, handler):
        '''
        Install handler to be invoked when MESSAGE_EVENT is received from kalcmd2

        This handler will be invoked when the event is received as

        .. code-block:: python

            def message_handler(stream_id, samples):
                # return_code (int) message return code, 0 for ok
                # header (any) message header
                # payload (list[any]) message received
                pass

        Args:
            handler (func(return_code, header, payload)): Handler to install
        '''
        self._data.handlers[KALCMD_EVENT_MESSAGE] = handler

    @log_input(logging.INFO)
    def install_stream_data_handler(self, handler):
        '''
        Install handler to be invoked when STREAM_DATA_EVENT is received from kalcmd2

        This handler will be invoked when the event is received as

        .. code-block:: python

            def stream_data_handler(stream_id, samples):
                # stream_id (int) same return in stream_create method
                # samples (int) number of samples in/needed by stream
                pass

        Args:
            handler (func(stream_id, samples)): Handler to install
        '''
        self._data.handlers[KALCMD_EVENT_STREAM_DATA] = handler

    @log_input(logging.INFO)
    def install_stream_eof_handler(self, handler):
        '''
        Install handler to be invoked when STREAM_EOF_EVENT is received from kalcmd2

        This handler will be invoked when the event is received as

        .. code-block:: python

            def stream_eof_handler(stream_id):
                # stream_id (int) same return in stream_create method
                pass

        Args:
            handler (func(stream_id)): Handler to install
        '''
        self._data.handlers[KALCMD_EVENT_STREAM_EOF] = handler

    @log_input(logging.INFO)
    def install_kalimba_wants_attention_handler(self, handler):
        '''
        Install handler to be invoked when KALIMBA_WANTS_ATTENTION_EVENT is received from kalcmd2

        This handler will be invoked when the event is received as

        .. code-block:: python

            def kalimba_wants_attention_handler():
                pass

        Args:
            handler (func()): Handler to install
        '''
        self._data.handlers[KALCMD_EVENT_KALIMBA_WANTS_ATTENTION] = handler

    @log_input(logging.INFO)
    @log_exception
    def set_relative_timer(self, cookie, period):
        '''
        Set relative simulated timer, if it already exists then modify

        Args:
            cookie (int): Timer id
            period (flaot): Period in seconds

        Raises:
            ValueError: if the timer is not added correctly
        '''
        if cookie == WAKE_TIMER_COOKIE:
            raise ValueError('cookie:%s invalid' % (cookie))

        with self._data.lock:
            period = int(period * 1000000)
            status = self._kalcmd.set_timer_rel(period, cookie)
            if status[KALCMD2_RETURN_CODE] != kalcmd2.kalcmd_response.KALCMD_OK:
                raise ValueError('%s:%s' % (KALCMD2_RETURN_CODE, status[KALCMD2_RETURN_CODE]))

    @log_input(logging.INFO)
    @log_exception
    def set_absolute_timer(self, cookie, period):
        '''
        Set absolute simulated timer, if it already exists then modify

        Args:
            cookie (int): Timer id
            period (flaot): Period in seconds

        Raises:
            ValueError: if the timer is not added correctly
        '''
        if cookie == WAKE_TIMER_COOKIE:
            raise ValueError('cookie:%s invalid' % (cookie))

        with self._data.lock:
            period = int(period * 1000000)
            status = self._kalcmd.set_timer_abs(period, cookie)
            if status[KALCMD2_RETURN_CODE] != kalcmd2.kalcmd_response.KALCMD_OK:
                raise ValueError('%s:%s' % (KALCMD2_RETURN_CODE, status[KALCMD2_RETURN_CODE]))

    @log_output(logging.INFO, formatters={'address': '0x%08x', 'return': '0x%08x'})
    @log_exception
    def peek(self, memory_space, address, size):
        '''
        Peek memory

        Args:
            memory_space (int): Memory space as specified in kalcmd2.kalmem_spaces
            address (int): Memory address
            size (int): Memory size as specified in kalcmd.kal_widths

        Returns:
            int: Memory value

        Raises:
            ValueError: if memory is not read correctly
        '''
        with self._data.lock:
            status = self._kalcmd.peek(memory_space, address, size)
            if status[KALCMD2_RETURN_CODE] != kalcmd2.kalcmd_response.KALCMD_OK:
                raise ValueError('%s:%s' % (KALCMD2_RETURN_CODE, status[KALCMD2_RETURN_CODE]))
            return status[KALCMD2_VALUE]

    @log_input(logging.INFO, formatters={'address': '0x%08x', 'value': '0x%08x'})
    @log_exception
    def poke(self, memory_space, address, size, value):
        '''
        Poke memory

        Args:
            memory_space (int): Memory space as specified in kalcmd2.kalmem_spaces
            address (int): Memory address
            size (int): Memory size as specified in kalcmd.kal_widths
            value (int): Value to write

        Raises:
            ValueError: if memory is not written correctly
        '''
        with self._data.lock:
            status = self._kalcmd.poke(memory_space, address, size, value)
            if status[KALCMD2_RETURN_CODE] != kalcmd2.kalcmd_response.KALCMD_OK:
                raise ValueError('%s:%s' % (KALCMD2_RETURN_CODE, status[KALCMD2_RETURN_CODE]))

    @log_input(logging.INFO)
    @log_exception
    def interrupt(self, interrupt):
        '''
        Assert interrupt

        Args:
            interrupt (int): Interrupt number

        Raises:
            ValueError: if interrupt is not asserted correctly
        '''
        with self._data.lock:
            status = self._kalcmd.interrupt(interrupt)
            if status[KALCMD2_RETURN_CODE] != kalcmd2.kalcmd_response.KALCMD_OK:
                raise ValueError('%s:%s' % (KALCMD2_RETURN_CODE, status[KALCMD2_RETURN_CODE]))

    @log_input(logging.INFO, formatters={'header': '0x%04x', 'payload': '0x%04x'})
    @log_exception
    def send_message(self, header, payload):
        '''
        Send message to kalimba

        Args:
            header (list[int]): Header to be sent
            payload (list[int]): Message to be sent

        Raises:
            ValueError: if the message is not delivered correctly
        '''
        self._data.pend_msgs.append([header, payload])

    @log_input(logging.DEBUG, formatters={'header': '0x%04x', 'payload': '0x%04x'})
    @log_exception
    def _send_message(self, header, payload):
        '''
        Send message to kalimba (real send)

        Args:
            header (list[int]): Header to be sent
            payload (list[int]): Message to be sent

        Raises:
            ValueError: if the message is not delivered correctly
        '''
        with self._data.lock:
            header = MsgHeader(header)
            status = self._kalcmd.send_message(header, payload)
            if status[KALCMD2_RETURN_CODE] != kalcmd2.kalcmd_response.KALCMD_OK:
                raise ValueError('%s:%s' % (KALCMD2_RETURN_CODE, status[KALCMD2_RETURN_CODE]))

    @log_output(logging.DEBUG)
    @log_exception
    def get_current_time(self):
        '''
        Get current time since simulation started

        Returns:
            float: Time in seconds
        '''
        with self._data.lock:
            status = self._kalcmd.get_time()
            if status[KALCMD2_RETURN_CODE] != kalcmd2.kalcmd_response.KALCMD_OK:
                raise ValueError('%s:%s' % (KALCMD2_RETURN_CODE, status[KALCMD2_RETURN_CODE]))
            return status[KALCMD2_TIME] / 1000000.0

    @log_input(logging.INFO, formatters={'return_code': '0x%04x'})
    @log_exception
    def terminate(self, return_code):
        '''
        Send a terminate command.

        This will make kalsim exit

        Args:
            return_code (int): Termination code
        '''
        with self._data.lock:
            status = self._kalcmd.terminate(return_code)
            if status[KALCMD2_RETURN_CODE] != kalcmd2.kalcmd_response.KALCMD_OK:
                raise ValueError('%s:%s' % (KALCMD2_RETURN_CODE, status[KALCMD2_RETURN_CODE]))

    # stream_change obsolete
    # stream_get_info obsolete

    # get message handled internally

    @log_output(logging.INFO)
    @log_exception
    def get_kalimba_arch(self):
        '''
        Get Kalimba architecture

        Returns:
            int: Kalimba architecture

        Raises:
            ValueError: if kalsim does not responds correctly
        '''
        with self._data.lock:
            status = self._kalcmd.get_kal_arch()
            if status[KALCMD2_RETURN_CODE] != kalcmd2.kalcmd_response.KALCMD_OK:
                raise ValueError('%s:%s' % (KALCMD2_RETURN_CODE, status[KALCMD2_RETURN_CODE]))
            return status[KALCMD2_KAL_ARCH]

    @log_output(logging.INFO)
    @log_exception
    def get_kalimba_name(self):
        '''
        Get Kalimba name

        Returns:
            str: Kalimba name

        Raises:
            ValueError: if kalsim does not responds correctly
        '''
        with self._data.lock:
            status = self._kalcmd.get_kalimba_name()
            if status[KALCMD2_RETURN_CODE] != kalcmd2.kalcmd_response.KALCMD_OK:
                raise ValueError('%s:%s' % (KALCMD2_RETURN_CODE, status[KALCMD2_RETURN_CODE]))
            return str(status[KALCMD2_NAME])

    @log_output(logging.INFO)
    @log_exception
    def get_version(self):
        '''
        Get kalcmd2 version

        Returns:
            float: Version

        Raises:
            ValueError: if kalsim does not responds correctly
        '''
        with self._data.lock:
            status = self._kalcmd.get_version()
            if status[KALCMD2_RETURN_CODE] != kalcmd2.kalcmd_response.KALCMD_OK:
                raise ValueError('%s:%s' % (KALCMD2_RETURN_CODE, status[KALCMD2_RETURN_CODE]))
            return float(str(status[KALCMD2_MAJOR]) + '.' + str(status[KALCMD2_MINOR]))

    @log_output(logging.DEBUG)
    @log_exception
    def get_last_timer_cookie(self):
        '''
        Get cookie of the last timer that reported an event

        Returns:
            int: Timer cookie

        Raises:
            ValueError: if kalsim does not responds correctly
        '''
        with self._data.lock:
            status = self._kalcmd.get_last_timer_cookie()
            if status[KALCMD2_RETURN_CODE] != kalcmd2.kalcmd_response.KALCMD_OK:
                raise ValueError('%s:%s' % (KALCMD2_RETURN_CODE, status[KALCMD2_RETURN_CODE]))
            return status[KALCMD2_COOKIE]

    @log_output(logging.INFO)
    @log_exception
    def stream_create(self):
        '''
        Create kalsim stream

        Returns:
            int: Stream id

        Raises:
            ValueError: if kalsim does not responds correctly
        '''
        with self._data.lock:
            status = self._kalcmd.stream_create()
            if status[KALCMD2_RETURN_CODE] != kalcmd2.kalcmd_response.KALCMD_OK:
                raise ValueError('%s:%s' % (KALCMD2_RETURN_CODE, status[KALCMD2_RETURN_CODE]))
            return status[KALCMD2_STREAM_ID]

    @log_input(logging.INFO)
    @log_exception
    def stream_destroy(self, stream_id):
        '''
        Destroy an existing stream

        Args:
            stream_id (int): Stream id

        Raises:
            ValueError: if kalsim does not responds correctly
        '''
        with self._data.lock:
            status = self._kalcmd.stream_destroy(stream_id)
            if status[KALCMD2_RETURN_CODE] != kalcmd2.kalcmd_response.KALCMD_OK:
                raise ValueError('%s:%s' % (KALCMD2_RETURN_CODE, status[KALCMD2_RETURN_CODE]))

    @log_output(logging.INFO)
    @log_exception
    def stream_list(self):
        '''
        Get all created streams

        Returns:
            list[int]: Stream id

        Raises:
            ValueError: if kalsim does not responds correctly
        '''
        with self._data.lock:
            status = self._kalcmd.streams_list()
            if status[KALCMD2_RETURN_CODE] != kalcmd2.kalcmd_response.KALCMD_OK:
                raise ValueError('%s:%s' % (KALCMD2_RETURN_CODE, status[KALCMD2_RETURN_CODE]))
            return status[KALCMD2_STREAM_IDS]

    @log_output(logging.INFO)
    @log_exception
    def stream_query_property(self, stream_id, key):
        '''
        Query stream property

        Args:
            stream_id (int): Stream id
            key (str): Property

        Returns:
            str: Property value

        Raises:
            ValueError: if kalsim does not responds correctly
        '''
        with self._data.lock:
            status = self._kalcmd.stream_query_property(stream_id, key)
            if status[KALCMD2_RETURN_CODE] != kalcmd2.kalcmd_response.KALCMD_OK:
                raise ValueError('%s:%s' % (KALCMD2_RETURN_CODE, status[KALCMD2_RETURN_CODE]))
            return status[KALCMD2_VALUE]

    @log_input(logging.INFO)
    @log_exception
    def stream_change_property(self, stream_id, key, val):
        '''
        Change stream property

        Args:
            stream_id (int): Stream id
            key (str): Property
            val (str): Property value

        Raises:
            ValueError: if kalsim does not responds correctly
        '''
        with self._data.lock:
            status = self._kalcmd.stream_change_property(stream_id, key, str(val))
            if status[KALCMD2_RETURN_CODE] != kalcmd2.kalcmd_response.KALCMD_OK:
                raise ValueError('%s:%s' % (KALCMD2_RETURN_CODE, status[KALCMD2_RETURN_CODE]))

    @log_input(logging.INFO)
    @log_exception
    def stream_commit_changes(self, stream_id):
        '''
        Apply all property changes in the stream

        Args:
            stream_id (int): Stream id

        Raises:
            ValueError: if kalsim does not responds correctly
        '''
        with self._data.lock:
            status = self._kalcmd.stream_commit_changes(stream_id)
            if status[KALCMD2_RETURN_CODE] != kalcmd2.kalcmd_response.KALCMD_OK:
                raise ValueError('%s:%s' % (KALCMD2_RETURN_CODE, status[KALCMD2_RETURN_CODE]))

    @log_input(logging.INFO, formatters={'data': '0x%04x'})
    @log_exception
    def stream_insert(self, stream_id, data_size, data):
        '''
        Insert data into an stream

        This is used for write streams that are kalcmd backed,
        could be either kalcmd or kalsim driven

        Args:
            stream_id (int): Stream id
            data_size (int): Data size in bits
            data (list[int]): Data elements of size data_size

        Raises:
            ValueError: if kalsim does not responds correctly
        '''
        with self._data.lock:
            status = self._kalcmd.stream_insert(stream_id, str(data_size), data)
            if status[KALCMD2_RETURN_CODE] != kalcmd2.kalcmd_response.KALCMD_OK:
                raise ValueError('%s:%s' % (KALCMD2_RETURN_CODE, status[KALCMD2_RETURN_CODE]))

    @log_output(logging.INFO, formatters={'return': '0x%04x'})
    @log_exception
    def stream_extract(self, stream_id, sample_num, sign_extend=False):
        '''
        Extract data from a stream

        This is used for read streams that are kalcmd backed,
        could be either kalcmd or kalsim driven

        Args:
            stream_id (int): Stream id
            sample_num (int): Number of samples
            sign_extend (bool): Do we want to sign extend the data elements

        Returns:
            list[int]: Data elements
        Raises:
            ValueError: if kalsim does not responds correctly
        '''
        with self._data.lock:
            status = self._kalcmd.stream_extract(stream_id, sample_num, sign_extend)
            if status[KALCMD2_RETURN_CODE] != kalcmd2.kalcmd_response.KALCMD_OK:
                raise ValueError('%s:%s' % (KALCMD2_RETURN_CODE, status[KALCMD2_RETURN_CODE]))
            # TODO not checking format
            status.pop(KALCMD2_RETURN_CODE)
            return status[KALCMD2_SAMPLES]

    @log_input(logging.INFO)
    @log_exception
    def stream_induce(self, stream_id, sample_number):
        '''
        Insert data into an stream

        This is used for write streams that are file backed and kalcmd driven

        Args:
            stream_id (int): Stream id
            data (list[int]): Data elements of size data_size

        Raises:
            ValueError: if kalsim does not responds correctly
        '''
        with self._data.lock:
            status = self._kalcmd.stream_induce(stream_id, sample_number)
            if status[KALCMD2_RETURN_CODE] != kalcmd2.kalcmd_response.KALCMD_OK:
                raise ValueError('%s:%s' % (KALCMD2_RETURN_CODE, status[KALCMD2_RETURN_CODE]))
            return status[KALCMD2_SAMPLES_INDUCED]

    @log_exception
    def stream_eof_event_query(self):
        '''
        Query the last stream with an active EOF event

        Should be called after a STREAM_EOF_EVENT is received

        Returns:
            int: Stream id

        Raises:
            ValueError: if kalsim does not responds correctly
        '''
        with self._data.lock:
            status = self._kalcmd.stream_eof_event_query()
            if status[KALCMD2_RETURN_CODE] != kalcmd2.kalcmd_response.KALCMD_OK:
                raise ValueError('%s:%s' % (KALCMD2_RETURN_CODE, status[KALCMD2_RETURN_CODE]))
            return status[KALCMD2_STREAM_ID]

    @log_exception
    def stream_data_event_query(self):
        '''
        Query stream state

        Should be called after a STREAM_DATA_EVENT is received

        Returns:
            int: Stream id
            int: Number of samples

        Raises:
            ValueError: if kalsim does not responds correctly
        '''
        with self._data.lock:
            status = self._kalcmd.stream_data_event_query()
            if status[KALCMD2_RETURN_CODE] != kalcmd2.kalcmd_response.KALCMD_OK:
                raise ValueError('%s:%s' % (KALCMD2_RETURN_CODE, status[KALCMD2_RETURN_CODE]))
            return status[KALCMD2_STREAM_ID], status['num_of_samples']

    @log_input(logging.INFO)
    @log_exception
    def stream_rewind(self, stream_id):
        '''
        Rewind stream

        Raises:
            ValueError: if kalsim does not responds correctly
        '''
        with self._data.lock:
            status = self._kalcmd.stream_rewind(stream_id)
            if status[KALCMD2_RETURN_CODE] != kalcmd2.kalcmd_response.KALCMD_OK:
                raise ValueError('%s:%s' % (KALCMD2_RETURN_CODE, status[KALCMD2_RETURN_CODE]))

    @log_input(logging.INFO)
    def stream_flush(self, stream_id):
        '''
        Flush the stream designated with the stream_id.

        Accepts all kinds of streams however, it will only actually flush output streams to files.

        Args:
            stream_id:
        '''
        with self._data.lock:
            status = self._kalcmd.stream_flush(stream_id)
            if status[KALCMD2_RETURN_CODE] != kalcmd2.kalcmd_response.KALCMD_OK:
                raise ValueError('%s:%s' % (KALCMD2_RETURN_CODE, status[KALCMD2_RETURN_CODE]))

    # get_flag_state obsolete

    # TODO def handles_to_same_buffer(self, handle):
    # It can query if two BAC handles correspond to the same physical buffer in the system
    # (defined to be if they share same start address),
    # this was designed to be generic between Bluecore and Hydra.

    @log_exception
    def get_buffer_size(self, handle):
        '''
        Get buffer size

        Returns:
            int: size

        Raises:
            ValueError: if kalsim does not responds correctly
        '''
        with self._data.lock:
            status = self._kalcmd.get_buffer_size(handle)
            if status[KALCMD2_RETURN_CODE] != kalcmd2.kalcmd_response.KALCMD_OK:
                raise ValueError('%s:%s' % (KALCMD2_RETURN_CODE, status[KALCMD2_RETURN_CODE]))
            return status['size']

    @log_exception
    def get_handle_offset(self, handle):
        '''
        Get handle offset in buffer

        Args:
            handle (int): BAC handle

        Returns:
            int: offset

        Raises:
            ValueError: if kalsim does not responds correctly
        '''
        with self._data.lock:
            status = self._kalcmd.get_handle_offset(handle)
            if status[KALCMD2_RETURN_CODE] != kalcmd2.kalcmd_response.KALCMD_OK:
                raise ValueError('%s:%s' % (KALCMD2_RETURN_CODE, status[KALCMD2_RETURN_CODE]))
            return status['offset']

    @log_exception
    def get_handle_sample_size(self, handle):
        '''
        Get handle sample size in buffer

        Args:
            handle (int): BAC handle

        Returns:
            int: Sample size in bits

        Raises:
            ValueError: if kalsim does not responds correctly
        '''
        with self._data.lock:
            status = self._kalcmd.get_handle_sample_size(handle)
            if status[KALCMD2_RETURN_CODE] != kalcmd2.kalcmd_response.KALCMD_OK:
                raise ValueError('%s:%s' % (KALCMD2_RETURN_CODE, status[KALCMD2_RETURN_CODE]))
            return status['sample_size']

    def query_stats(self, key, watchdog):
        '''
        It is a dict like performance querying mechanism. (KATS 2 used it, see the
        following example code).
        It is mostly the same information as you can get from Kalsim if -kalcmd2-pref and
        -kalcmd2-perf-print are enabled, however now query able from Kalcmd2 instead of the console.
        (see B-216072 for the Kalsim bug and B-219617 for the KATS2 side)

        print ("Watchdog stats")
        print (kalcmd_inst.query_stats("simulation_load", kalcmd_stat.WATCHDOG));
        print (kalcmd_inst.query_stats("wait_load", kalcmd_stat.WATCHDOG));
        print (kalcmd_inst.query_stats("proc_load", kalcmd_stat.WATCHDOG));
        print (kalcmd_inst.query_stats("event_count", kalcmd_stat.WATCHDOG));
        print (kalcmd_inst.query_stats("command_count", kalcmd_stat.WATCHDOG));
        print (kalcmd_inst.query_stats("timers_fired_count", kalcmd_stat.WATCHDOG));
        print (kalcmd_inst.query_stats("timers_added_count", kalcmd_stat.WATCHDOG));
        print (kalcmd_inst.query_stats("real_time",kalcmd_stat.WATCHDOG));
        print ("Total stats")
        print (kalcmd_inst.query_stats("simulation_load", kalcmd_stat.TOTAL));
        print (kalcmd_inst.query_stats("wait_load", kalcmd_stat.TOTAL));
        print (kalcmd_inst.query_stats("proc_load", kalcmd_stat.TOTAL));
        print (kalcmd_inst.query_stats("event_count", kalcmd_stat.TOTAL));
        print (kalcmd_inst.query_stats("command_count", kalcmd_stat.TOTAL));
        print (kalcmd_inst.query_stats("timers_fired_count", kalcmd_stat.TOTAL));
        print (kalcmd_inst.query_stats("timers_added_count", kalcmd_stat.TOTAL));
        print (kalcmd_inst.query_stats("real_time", kalcmd_stat.TOTAL));

        This only works if kalcmd2-perf is enabled on the command line.
        The first argument is the to be queried key, while the second argument describes the scale
        of the query. Kalsim maintains performance internal counters and timers at two scales.
        (kalcmd_stat.WATCHDOG is per watchdog event, while kalcmd_stat.TOTAL is since start up,
        up to the last watchdog)

        Simulation_load + wait_load + proc_load should equal about 1.0.
        Simulation load represents the fraction of time Kalsim spend simulating during the whole
        time between the two watch dog events.
        Wait_load represent the time spend waiting on the socket, while proc_load represent the time
        Kalcmd2 spend processing the commands.
        The things that end with count are simple counters.
        The names should be descriptive enough if your familiar with kalcmd2.

        Real time represents the amount of time in second it took for Kalsim to simulate between the
        two watchdogs for kalcmd_stat.WATCHDOG.
        (which always come in intervals of 1 simulated sec)
        While in the TOTAL mode it represents the total time spend simulating up to the last
        watchdog.
        For safety reasons it probably best to only use this call directly after WATCHDOG_EVENT.
        '''
        with self._data.lock:
            status = self._kalcmd.query_stats(key, watchdog)
            if status[KALCMD2_RETURN_CODE] != kalcmd2.kalcmd_response.KALCMD_OK:
                raise ValueError('%s:%s' % (KALCMD2_RETURN_CODE, status[KALCMD2_RETURN_CODE]))
            return status['value']

    @log_input(logging.INFO)
    @log_exception
    def change_max_clockspeed(self, clock_speed):
        '''
        Change clock speed

        Args:
            clock_speed (int): Clock speed in MHz

        Raises:
            ValueError: if kalsim does not responds correctly
        '''
        with self._data.lock:
            status = self._kalcmd.change_max_clockspeed(clock_speed)
            if status[KALCMD2_RETURN_CODE] != kalcmd2.kalcmd_response.KALCMD_OK:
                raise ValueError('%s:%s' % (KALCMD2_RETURN_CODE, status[KALCMD2_RETURN_CODE]))

    @log_output(logging.INFO)
    @log_exception
    def query_max_clock_speed(self):
        '''
        Get kalimba maximum clock speed

        Returns:
            int: Maximum clock speed in MHz

        Raises:
            ValueError: if kalsim does not responds correctly
        '''
        with self._data.lock:
            status = self._kalcmd.query_max_clock_speed()
            if status[KALCMD2_RETURN_CODE] != kalcmd2.kalcmd_response.KALCMD_OK:
                raise ValueError('%s:%s' % (KALCMD2_RETURN_CODE, status[KALCMD2_RETURN_CODE]))
            return status[KALCMD2_CLOCK_SPEED]

    @log_output(logging.INFO, formatters={'return': '0x%08x'})
    @log_exception
    def query_dsp_register(self, register):
        '''
        Get kalimba register value

        Args:
            register (int): Register as in kalcmd2.kal_regs

        Returns:
            int: Register value

        Raises:
            ValueError: if kalsim does not responds correctly
        '''
        with self._data.lock:
            status = self._kalcmd.query_dsp_reg(register)
            if status[KALCMD2_RETURN_CODE] != kalcmd2.kalcmd_response.KALCMD_OK:
                raise ValueError('%s:%s' % (KALCMD2_RETURN_CODE, status[KALCMD2_RETURN_CODE]))
            return status[KALCMD2_VALUE]

    @log_output(logging.INFO)
    @log_exception
    def get_kalsim_version(self):
        '''
        Get kalimba simulator version

        Returns:
            dict:
                build_date (str): Build date in human readable format
                major (str): Major version
                minor (str): Minor version
                release_version (bool): Release version

        Raises:
            ValueError: if kalsim does not responds correctly
        '''
        with self._data.lock:
            status = self._kalcmd.get_kalsim_version()
            if status[KALCMD2_RETURN_CODE] != kalcmd2.kalcmd_response.KALCMD_OK:
                raise ValueError('%s:%s' % (KALCMD2_RETURN_CODE, status[KALCMD2_RETURN_CODE]))
            status.pop(KALCMD2_RETURN_CODE)
            return status

    @log_input(logging.INFO, formatters={'address': '0x%08x', 'data': '0x%08x'})
    @log_exception
    def block_mem_write(self, memory_space, address, data):
        '''
        Write kalimba memory in blocks of uint32_t data

        Args:
            memory_space (int): Memory space as specified in kalcmd2.kalmem_spaces
            address (int): Memory address
            data (list[int]): Data to be written

        Raises:
            ValueError: if kalsim does not responds correctly
        '''
        with self._data.lock:
            status = self._kalcmd.block_mem_write(memory_space, address, data)
            if status[KALCMD2_RETURN_CODE] != kalcmd2.kalcmd_response.KALCMD_OK:
                raise ValueError('%s:%s' % (KALCMD2_RETURN_CODE, status[KALCMD2_RETURN_CODE]))

    @log_output(logging.INFO, formatters={'address': '0x%08x', 'return': '0x%08x'})
    @log_exception
    def block_mem_read(self, memory_space, address, num_words):
        '''
        Read kalimba memory in blocks of uint32_t data

        Args:
            memory_space (int): Memory space as specified in kalcmd2.kalmem_spaces
            address (int): Memory address
            num_words (int): Number of uint32_t data blocks to read

        Returns:
            list[int]: Data read

        Raises:
            ValueError: if kalsim does not responds correctly
        '''
        with self._data.lock:
            status = self._kalcmd.block_mem_read(memory_space, address, num_words)
            if status[KALCMD2_RETURN_CODE] != kalcmd2.kalcmd_response.KALCMD_OK:
                raise ValueError('%s:%s' % (KALCMD2_RETURN_CODE, status[KALCMD2_RETURN_CODE]))
            return status[KALCMD2_WORDS]

    @log_input(logging.INFO)
    @log_exception
    def print_message(self, message):
        '''
        Send a message to be displayed in kalsim console

        Args:
            message(str): Message to send

        Raises:
            ValueError: if kalsim does not responds correctly
        '''
        with self._data.lock:
            status = self._kalcmd.print_message(message)
            if status[KALCMD2_RETURN_CODE] != kalcmd2.kalcmd_response.KALCMD_OK:
                raise ValueError('%s:%s' % (KALCMD2_RETURN_CODE, status[KALCMD2_RETURN_CODE]))
