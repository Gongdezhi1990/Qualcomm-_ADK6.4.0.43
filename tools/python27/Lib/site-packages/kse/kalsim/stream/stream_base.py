'''
Stream Base class
'''

import argparse
import logging

from abc import ABCMeta, abstractmethod, abstractproperty
from six import add_metaclass

from kats.framework.library.log import log_input

STATE_CREATED = 'stream_state_created'
STATE_STARTED = 'stream_state_started'
STATE_STOPPED = 'stream_state_stopped'
STATE_EOF = 'stream_state_eof'

STREAM_TYPE_SOURCE = 'source'
STREAM_TYPE_SINK = 'sink'

STREAM_NAME = 'stream_name'
STREAM_RATE = 'stream_rate'
STREAM_DATA_WIDTH = 'stream_data_width'
STREAM_DATA = 'stream_data'
CALLBACK_EOF = 'callback_eof'


@add_metaclass(ABCMeta)
class StreamBase(object):
    '''
    Basic stream handler class

    Args:
        stream_type (str): Type of stream 'source' or 'sink'
        stream_name (str): Stream name
        stream_rate (int): Stream frame rate in hertzs
        stream_data_width (int): Stream data width in hertzs
        stream_data (list[int]): Stream frame data
        callback_eof (func()): Callback for End of File
    '''

    def __init__(self, stream_type, *args, **kwargs):
        self._log = logging.getLogger(__name__) if not hasattr(self, '_log') else self._log
        self._stream_type = stream_type
        self._stream_data = argparse.Namespace()
        self._stream_data.name = kwargs.pop(STREAM_NAME, '')
        self._stream_data.rate = kwargs.pop(STREAM_RATE, None)
        self._stream_data.data_width = kwargs.pop(STREAM_DATA_WIDTH, None)
        self._stream_data.data = kwargs.pop(STREAM_DATA, None)

        self._state = None
        self._callback = kwargs.pop(CALLBACK_EOF, None)

        if args:
            self._log.warning('unknown args:%s', str(args))

        if kwargs:
            self._log.warning('unknown kwargs:%s', str(kwargs))

    @abstractproperty
    def platform(self):
        '''
        list[str]: Platform name
        '''
        pass

    @abstractproperty
    def interface(self):
        '''
        str: Interface name
        '''
        pass

    def get_type(self):
        '''
        Get stream type

        Returns:
            str: Stream type 'source' or 'sink'
        '''
        return self._stream_type

    @log_input(logging.INFO)
    @abstractmethod
    def create(self):
        '''
        Create stream
        '''
        if self._state is not None:
            raise RuntimeError('stream already created')
        else:
            self._state = STATE_CREATED

    @log_input(logging.INFO)
    @abstractmethod
    def config(self, **kwargs):
        '''
        Configure stream
        '''
        pass

    @log_input(logging.INFO)
    @abstractmethod
    def start(self):
        '''
        Start stream
        '''
        if self._state != STATE_CREATED and self._state != STATE_STOPPED:
            raise RuntimeError('cannot start stream')

        self._state = STATE_STARTED

    @log_input(logging.INFO)
    @abstractmethod
    def stop(self):
        '''
        Stop stream
        '''
        self._state = STATE_STOPPED

    def check_active(self):
        '''
        Check if the end of file has been reached.

        Returns:
            bool: End of file
        '''
        return self._state != STATE_EOF

    @log_input(logging.INFO)
    @abstractmethod
    def destroy(self):
        '''
        Destroy stream
        '''
        if self._state is None:
            raise RuntimeError('stream not created')
        if self._state == STATE_STARTED:
            self.stop()

        self._state = None

    def get_state(self):
        '''
        Get current state

        Returns:
            str: State, one of STATE_CREATED, STATE_STARTED, STATE_STOPPED, STATE_EOF
        '''
        return self._state

    def get_name(self):
        '''
        Get stream name

        Returns:
            str: Stream name
        '''
        return self._stream_data.name

    def get_rate(self):
        '''
        Get stream sample rate

        Returns:
            int or None: Frame rate in Hertzs
        '''
        return self._stream_data.rate

    def get_data_width(self):
        '''
        Get stream data width

        Returns:
            int or None: Sample width in bits
        '''
        return self._stream_data.data_width

    def get_data(self):
        '''
        Get stream data

        Returns:
            list[int]: Stream data
        '''
        return self._stream_data.data

    def set_data(self, data):
        '''
        Set stream data

        Args:
            data (list[int]): Stream data
        '''
        self._stream_data.data = data

    @log_input(logging.INFO)
    def eof(self):
        '''
        End of File event received on stream
        '''
        # if self._stream[stream_id].get_state() != STATE_EOF:
        #    self._stream[stream_id].stop()
        self._state = STATE_EOF
        if self._callback:
            self._callback()
