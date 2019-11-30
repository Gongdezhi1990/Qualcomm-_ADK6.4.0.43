'''
Stream Base class
'''

import logging
from collections import OrderedDict

import os
from abc import ABCMeta, abstractproperty
from six import add_metaclass

from kats.framework.library.log import log_input
from kats.library.registry import get_instance
from .stream_base import StreamBase, STREAM_TYPE_SOURCE, STREAM_TYPE_SINK, STREAM_NAME, \
    STREAM_RATE, STREAM_DATA_WIDTH, STATE_EOF

PARAM_DEFAULT = 'default'

STREAM_BACKING = 'stream_backing'
STREAM_BACKING_FILE = 'file'
STREAM_BACKING_KALCMD = 'kalcmd'

STREAM_FLOW_CONTROL_DRIVE = 'stream_flow_control_drive'
STREAM_FLOW_CONTROL_DRIVE_KALSIM = 'kalsim'
STREAM_FLOW_CONTROL_DRIVE_KALCMD = 'kalcmd'

STREAM_FILENAME = 'stream_filename'

STREAM_FORMAT = 'stream_format'

STREAM_FLOW_CONTROL_RATE = 'stream_flow_control_rate'

STREAM_FLOW_CONTROL_BLOCK_SIZE = 'stream_flow_control_block_size'

STREAM_DIRECTION = 'stream_direction'
STREAM_DIRECTION_READ = 'read'
STREAM_DIRECTION_WRITE = 'write'

STREAM_PINCH = 'stream_pinch'

STREAM_BLOCK_PROGRAM_CHANGES = 'stream_block_program_changes'
STREAM_BLOCK_PROGRAM_CHANGES_NONE = 'none'
STREAM_BLOCK_PROGRAM_CHANGES_RATE = 'rate'
STREAM_BLOCK_PROGRAM_CHANGES_ALL = 'all'

CALLBACK_EOF = 'eof'
CALLBACK_DATA_EVENT = 'data_event'


@add_metaclass(ABCMeta)
class KalsimStream(StreamBase):
    '''
    Basic kalsim stream handler class

    Modes supported

    .. code-block:: python

        # kalsim will automatically stream a file
        stream_type='source'/'sink'
        stream_backing='file'
        stream_flow_control_drive='kalsim'
        stream_filename='pathtorawfile'
        stream_format=bits_per_sample
        stream_flow_control_rate=freq
        stream_flow_control_block_size=1

        # kalsim will stream a file that is controlled by the application, with calls to induce
        stream_type='source'
        stream_backing='file'
        stream_flow_control_drive='kalcmd'
        stream_filename='pathtorawfile'
        stream_format=bits_per_sample
        stream_flow_control_rate=freq
        stream_flow_control_block_size=1

        # the application will stream data with calls to insert
        stream_type='source'
        stream_backing='kalcmd'
        stream_flow_control_drive='kalcmd'
        stream_format=bits_per_sample
        stream_flow_control_block_size=1

    Args:
        stream_type (str): Type of stream 'source' or 'sink'
        callback_data_event (func(samples)): Callback for attention required
        stream_backing (str): Who is streaming the data
            - file, kalsim, stream_filename should be provided (raw data file)
            - kalcmd, the application with insert and extract
        stream_flow_control_drive (str). Who is responsible of determining when samples have to be
            induced
            - kalsim
            - kalcmd
            - kalsim_level
        stream_filename (str). Filename for kalsim to use as stream.
            Should be a raw mono file and has to be present when stream_backing is file
        stream_format (int): Number of bits in a sample
        stream_flow_control_rate (int): Rate in samples per second
        stream_flow_control_block_size (int): Minimum size of block to stream
    '''

    __param_schema = {
        STREAM_BACKING: {PARAM_DEFAULT: STREAM_BACKING_FILE},
        STREAM_FLOW_CONTROL_DRIVE: {PARAM_DEFAULT: STREAM_FLOW_CONTROL_DRIVE_KALSIM},
        STREAM_FILENAME: {PARAM_DEFAULT: None},
        STREAM_FORMAT: {PARAM_DEFAULT: 8},
        STREAM_FLOW_CONTROL_RATE: {PARAM_DEFAULT: 8000},
        STREAM_FLOW_CONTROL_BLOCK_SIZE: {PARAM_DEFAULT: 1},
    }

    def __init__(self, stream_type, *_, **kwargs):
        self._log = logging.getLogger(__name__) if not hasattr(self, '_log') else self._log
        self._log_param = OrderedDict() if not hasattr(self, '_log_param') else self._log_param
        self._log_param['id'] = None
        self._kalcmd = get_instance('kalcmd')
        self._kalcmd_stream = get_instance('kalcmd_stream')

        self._stream_type = stream_type
        self._stream_id = None

        self.__callback = {}
        self.__callback[CALLBACK_EOF] = self.eof
        self.__callback[CALLBACK_DATA_EVENT] = kwargs.pop('callback_data_event', None)

        # initialise default values
        self.__parameters = {}
        for entry in self.__param_schema:
            if entry in kwargs or self.__param_schema[entry][PARAM_DEFAULT] is not None:
                self.__parameters[entry] = kwargs.pop(entry,
                                                      self.__param_schema[entry][PARAM_DEFAULT])

        if stream_type == STREAM_TYPE_SOURCE:
            self.__parameters[STREAM_DIRECTION] = STREAM_DIRECTION_READ
        else:
            self.__parameters[STREAM_DIRECTION] = STREAM_DIRECTION_WRITE

        # for kalsim handled stream provide parameters to StreamBase if they are
        # not already provided
        if self.__parameters[STREAM_BACKING] == STREAM_BACKING_FILE:
            if (STREAM_NAME not in kwargs and
                    STREAM_FILENAME in self.__parameters and
                    self.__parameters[STREAM_FILENAME]):
                kwargs[STREAM_NAME] = self.__parameters[STREAM_FILENAME]
            if STREAM_RATE not in kwargs and self.__parameters[STREAM_FLOW_CONTROL_RATE]:
                kwargs[STREAM_RATE] = self.__parameters[STREAM_FLOW_CONTROL_RATE]
            if STREAM_DATA_WIDTH not in kwargs and self.__parameters[STREAM_FORMAT]:
                kwargs[STREAM_DATA_WIDTH] = self.__parameters[STREAM_FORMAT]

        # verify whether the resource file exists or not,if not then raise meaningful
        # trace output, the traceback will provide the info about file status
        # os.stat() is used to find whether file exists or not
        if (stream_type == STREAM_TYPE_SOURCE and
                self.__parameters[STREAM_BACKING] == STREAM_BACKING_FILE):
            os.stat(self.__parameters[STREAM_FILENAME])

        super(KalsimStream, self).__init__(stream_type, **kwargs)

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

    def __config(self, **kwargs):
        '''
        extract values from kwargs that we handle
        '''
        for entry in self.__param_schema:
            if entry in kwargs:
                self.__parameters[entry] = kwargs.pop(entry)
        return kwargs

    def get_id(self):
        '''
        Get stream id

        Returns:
            int: Stream id
        '''
        return self._stream_id

    @log_input(logging.INFO)
    def create(self):
        '''
        Create stream
        '''
        super(KalsimStream, self).create()
        self._stream_id = self._kalcmd.stream_create()
        self._log_param['id'] = self._stream_id

    @log_input(logging.INFO)
    def config(self, **kwargs):
        '''
        Configure stream
        '''
        kwargs = self.__config(**kwargs)

        self._kalcmd_stream.install_eof_handler(self._stream_id, self.eof)
        self._kalcmd_stream.install_data_handler(self._stream_id, self.data_event)

        if kwargs:
            self._log.warning('unknown parameters:%s', str(kwargs))

        for key in self.__parameters:
            self._kalcmd.stream_change_property(self._stream_id, key, str(self.__parameters[key]))

        # as we are always setting the rate of the stream we enforce that rate to be kept by
        # kalsim, disabling any autoupdate of the rate based on kymera assumptions
        # This is only supported in kalsim 21f+
        import kats
        if kats.__name__ == 'kats':
            self._kalcmd.stream_change_property(self._stream_id, STREAM_BLOCK_PROGRAM_CHANGES,
                                                STREAM_BLOCK_PROGRAM_CHANGES_RATE)

        if self.__parameters[STREAM_FLOW_CONTROL_DRIVE] == STREAM_FLOW_CONTROL_DRIVE_KALSIM:
            self._kalcmd.stream_change_property(self._stream_id, STREAM_PINCH, str(True))

        self._kalcmd.stream_commit_changes(self._stream_id)
        for prop in self.__parameters:
            _ = self._kalcmd.stream_query_property(self._stream_id, prop)

        # This is only supported in kalsim 21f+
        if kats.__name__ == 'kats':
            _ = self._kalcmd.stream_query_property(self._stream_id, STREAM_BLOCK_PROGRAM_CHANGES)

    def _config_param(self, key, value):
        if not self._stream_id:
            raise RuntimeError('stream not created')
        else:
            self._kalcmd.stream_change_property(self._stream_id, key, str(value))

    @log_input(logging.INFO)
    def start(self):
        '''
        Start stream
        '''
        super(KalsimStream, self).start()

        if self.__parameters[STREAM_FLOW_CONTROL_DRIVE] == STREAM_FLOW_CONTROL_DRIVE_KALSIM:
            self._kalcmd.stream_rewind(self._stream_id)
            self._kalcmd.stream_change_property(self._stream_id, STREAM_PINCH, str(False))
            self._kalcmd.stream_commit_changes(self._stream_id)

    @log_input(logging.INFO)
    def stop(self):
        '''
        Stop stream
        '''
        super(KalsimStream, self).stop()

        if self.__parameters[STREAM_FLOW_CONTROL_DRIVE] == STREAM_FLOW_CONTROL_DRIVE_KALSIM:
            self._kalcmd.stream_change_property(self._stream_id, STREAM_PINCH, str(True))
            self._kalcmd.stream_commit_changes(self._stream_id)
            if self._stream_type == STREAM_TYPE_SINK:
                # TODO stream_flush is a new command in kalcmd2 2.8 and kalsim 21c+
                # KATS is tied to kalcmd2==2.8 and kalsim==21d
                # KSE is tied to kalcmd2==2.8 but to an older version of kalsim
                import kats
                if kats.__name__ == 'kats':
                    self._kalcmd.stream_flush(self._stream_id)

    def induce(self, samples):
        '''
        Induce stream, force some data to be sent

        Args:
            samples (int): Number of sampes to induce

        Returns:
            int: Number of samples actually induced
        '''
        return self._kalcmd.stream_induce(self._stream_id, samples)

    def insert(self, data):
        '''
        Inserts data with the size specified in stream_format property

        Args:
            data (list[int]): Data to insert
        '''
        self._kalcmd.stream_insert(self._stream_id, self.get_data_width(), data)

    def extract(self, samples, sign_extend=False):
        '''
        Extracts data from the stream

        Args:
            samples (int): Number of samples to extract
            sign_extend (bool): Do we want to sign extend the data elements

        Returns:
            data (list[int]): Data extracted
        '''
        return self._kalcmd.stream_extract(self._stream_id, samples, sign_extend=sign_extend)

    @log_input(logging.INFO)
    def destroy(self):
        '''
        Destroy stream
        '''
        super(KalsimStream, self).destroy()
        self._kalcmd.stream_destroy(self._stream_id)
        self._stream_id = None
        self._log_param['id'] = self._stream_id

    @log_input(logging.INFO)
    def eof(self):
        '''
        End of File event received on stream
        '''
        if self.get_state() != STATE_EOF:
            self.stop()
        super(KalsimStream, self).eof()

    @log_input(logging.INFO)
    def data_event(self, samples):
        '''
        Data event received on stream

        Args:
            samples (int): Number of samples
        '''
        if self.__callback[CALLBACK_DATA_EVENT]:
            self.__callback[CALLBACK_DATA_EVENT](samples)

    def query(self, key):
        '''
        Query stream property

        Args:
            key (str): Property

        Returns:
            str: Property value

        '''
        return self._kalcmd.stream_query_property(self._stream_id, key)
