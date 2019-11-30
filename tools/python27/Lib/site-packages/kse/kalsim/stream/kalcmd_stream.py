'''
Kalcmd Stream class
'''

import logging

from kats.framework.library.log import log_input


class KalcmdStream(object):
    '''
    This class provides a one to many interface to kalcmd streams.

    Args:
        kalcmd (kats.instrument.kalcmd.kalcmd.Kalcmd): Instance of class Kalcmd
    '''

    def __init__(self, kalcmd):
        self._log = logging.getLogger(__name__) if not hasattr(self, '_log') else self._log
        self._kalcmd = kalcmd

        self._eof_handler = {}
        self._data_handler = {}
        self._kalcmd.install_stream_eof_handler(self._stream_eof)
        self._kalcmd.install_stream_data_handler(self._stream_data)

    @log_input(logging.INFO)
    def _stream_eof(self, stream_id):
        if stream_id in self._eof_handler:
            self._eof_handler[stream_id]()

    def _stream_data(self, stream_id, samples):
        if stream_id in self._data_handler:
            self._data_handler[stream_id](samples)

    @log_input(logging.INFO)
    def install_eof_handler(self, stream_id, handler=None):
        '''
        Install handler to be invoked when STREAM_EOF_EVENT is received from kalcmd2

        This handler will be invoked when the event is received as

        Args:
            stream_id (int): Stream id to watch
            handler (func()): Handler to install, None to uninstall
        '''
        if handler:
            self._eof_handler[stream_id] = handler
        elif stream_id in self._eof_handler:
            del self._eof_handler[stream_id]

    @log_input(logging.INFO)
    def install_data_handler(self, stream_id, handler=None):
        '''
        Install handler to be invoked when STREAM_DATA_EVENT is received from kalcmd2

        This handler will be invoked when the event is received as

        Args:
            stream_id (int): Stream id to watch
            handler (func(samples)): Handler to install, None to uninstall
        '''
        if handler:
            self._data_handler[stream_id] = handler
        elif stream_id in self._data_handler:
            del self._data_handler[stream_id]
