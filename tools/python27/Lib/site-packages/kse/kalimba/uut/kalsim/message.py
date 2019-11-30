'''
kalsim message abstraction layer
'''

import logging
import time

from six.moves.queue import Queue

from kats.framework.library.config import get_config_param
from kats.framework.library.log import log_input, log_output, log_input_output

HEADER = 'header'
QUEUE = 'queue'
HANDLER = 'handler'

RECV_TIMEOUT = 5


class Message(object):
    '''
    Module to handle routing messages from framework to kalcmd and back

    Args:
        kalcmd (kats.instrument.Kalcmd.Kalcmd): Kalcmd instrument
    '''

    def __init__(self, kalcmd):
        self._log = logging.getLogger(__name__) if not hasattr(self, '_log') else self._log
        self._log.info('init kalcmd:%s', kalcmd)

        self._kalcmd = kalcmd
        self._handlers = []
        self._timeout = get_config_param('UUT_MESSAGE_RECEIVE_TIMEOUT', RECV_TIMEOUT)
        self._kalcmd.install_message_handler(self._message_received)

    @log_input(logging.INFO, formatters={'header': '0x%04x', 'msg': '0x%04x'})
    def _message_received(self, code, header, msg):
        if code != 0:
            raise ValueError('message received code:%s invalid' % (code))
        for entry in self._handlers:
            if entry[HEADER] == header:
                if entry[QUEUE]:
                    entry[QUEUE].put(msg)
                if entry[HANDLER]:
                    entry[HANDLER](msg)

    @log_input(logging.INFO, formatters={'header': '0x%04x'})
    def register_message_handler(self, header, callback=None):
        '''
        Register handler

        This will return a handler that will be used as a parameter to send_message,
        recv_message and send_recv_message
        If callback is set then it will be invoked whenever a message is received

        .. code-block:: python

            msg = Message(kalcmd)
            handler = msg.register_message_handler([0x01])
            resp = msg.send_recv_message(handler, [1, 2, 3])

        Args:
            header (list[int]): Message header

        Returns:
            dict: Handler
        '''
        self._handlers.append({HEADER: header, QUEUE: Queue(), HANDLER: callback})
        return self._handlers[-1]

    @log_input(logging.INFO)
    def unregister_message_handler(self, handler):
        '''
        Unregister handler

        Args:
            handler (dict): Message handler
        '''
        self._handlers.remove(handler)

    @log_input(logging.INFO, formatters={'msg': '0x%04x'})
    def send_message(self, handler, msg):
        '''
        Send a message

        Args:
            handler (dict): Message handler registered with register_handler
            msg (list[int]): Message to send
        '''
        self._kalcmd.send_message(handler[HEADER], msg)

    @log_output(logging.INFO, formatters={'return': '0x%04x'})
    def recv_message(self, handler, timeout=None, flt=None):
        '''
        Wait for message

        Args:
            handler (dict): Message handler registered with register_handler
            timeout (float): Timeout for response in seconds, None for default
            flt (func): Filtering function receiving one parameter being the message received and
                returning True if the message is accepted in the filter, False otherwise

        Returns:
            list[int]: Reply

        Raises:
            RuntimeError: If there is a response reception timeout
        '''
        timeout = timeout if timeout is not None else self._timeout
        while True:
            try:
                t0 = time.time()
                msg = handler[QUEUE].get(True, timeout)
                if not flt or flt(msg):
                    break
                else:
                    t1 = time.time()
                    if timeout > (t1 - t0):
                        timeout -= (t1 - t0)
                    else:
                        raise RuntimeError('no response received')
            except Exception:
                raise RuntimeError('no response received')
        return msg

    @log_input_output(logging.INFO, formatters={'msg': '0x%04x', 'return': '0x%04x'})
    def send_recv_message(self, handler, msg, timeout=None, flt=None):
        '''
        Send a message and wait for response

        Example code

        .. code-block:: python

            def filter_by_sequence(seq, payload):
                return len(payload) >= 3 and seq == payload[2]

            from functools import partial
            payload = uut.message_send_recv(handler, payload,
                                            flt=partial(filter_by_sequence, sequence_num))


        Args:
            handler (dict): Message handler registered with register_handler
            msg (list[int]): Message to send
            timeout (float): Timeout for response in seconds, None for default
            flt (func): Filtering function receiving one parameter being the message received and
                returning True if the message is accepted in the filter, False otherwise

        Returns:
            list[int]: Reply

        Raises:
            RuntimeError: If there is a response reception timeout
        '''
        timeout = timeout if timeout is not None else self._timeout
        with handler[QUEUE].mutex:
            handler[QUEUE].queue.clear()
        self._kalcmd.send_message(handler[HEADER], msg)
        while True:
            try:
                t0 = time.time()
                msg = handler[QUEUE].get(True, timeout)
                if not flt or flt(msg):
                    break
                else:
                    t1 = time.time()
                    if timeout > (t1 - t0):
                        timeout -= (t1 - t0)
                    else:
                        raise RuntimeError('no response received')
            except Exception:
                raise RuntimeError('no response received')
        return msg
