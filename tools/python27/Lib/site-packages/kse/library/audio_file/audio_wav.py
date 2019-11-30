'''
Wav audio file utilities
'''

import argparse
import logging
import struct
import wave

import wrapt

from kats.framework.library.docstring import inherit_docstring
from .audio_base import AudioBase, MODE_READ, MODE_WRITE, MODE_DEFAULT

SAMPLE_WIDTH_LIST = [8, 16, 24, 32]


@wrapt.decorator
def read_data(wrapped, instance, args, kwargs):  # @DontTrace
    '''
    log exception decorator, logs an exception if any exception is raised in the decorated function

    Args:
        wrapped (function): Wrapped function which needs to be called by your wrapper function.
        instance (any): Object to which the wrapped function was bound when it was called.
        args (tuple[any]): Positional arguments supplied when the decorated function was called.
        kwargs (dict): Keyword arguments supplied when the decorated function was called.
    '''
    if instance._mode == MODE_READ and not instance._data_read:  # pylint: disable=protected-access
        instance._read()  # pylint: disable=protected-access
        instance._data_read = True  # pylint: disable=protected-access
    return wrapped(*args, **kwargs)


class AudioWav(AudioBase):
    '''
    Wav audio file class handler

    Args:
        filename (str): Filename
        mode (str): File open mode 'r' for read, 'w' for write
    '''

    platform = 'all'
    interface = 'wav'
    default_interface = False

    def __init__(self, filename, mode=MODE_DEFAULT, *args, **kwargs):
        inherit_docstring(self)
        self._log = logging.getLogger(__name__) if not hasattr(self, '_log') else self._log
        self._filename = filename
        self._mode = mode
        self._data_read = False
        self._param = argparse.Namespace()
        self._param.channels = None
        self._param.sample_rate = None
        self._param.sample_width = None
        self._param.frames = 0

        self._audio = [[]]

        if mode == MODE_READ:
            self._read_header()

        super(AudioWav, self).__init__(filename, mode=mode, *args, **kwargs)

    @staticmethod
    def _sample_width_to_struct_fmt(sample_width, num=1):
        mod = '<%s' % (num)
        if sample_width == 8:
            mod += 'B'  # wav format for 8 bits in unsigned
        elif sample_width == 16:
            mod += 'h'
        else:  # here we handle 24 and 32 bits, 24 require some magic
            mod += 'l'
        return mod

    def _read_header(self):
        handler = wave.open(self._filename, 'rb')
        try:
            params = handler.getparams()
            compression_type = params[4]
            # compression_name = params[5]
            if compression_type != 'NONE':
                raise RuntimeError('wav file compression_type %s unsupported' % (compression_type))

            self._param.channels = params[0]
            self._param.sample_rate = params[2]
            self._param.sample_width = params[1] * 8
            self._param.frames = params[3]
            self._audio = [[] for _ in range(self._param.channels)]

        finally:
            handler.close()

    def _read(self):
        handler = wave.open(self._filename, 'rb')
        try:
            data = bytearray(handler.readframes(self._param.frames))

            bytes_per_sample = int(self._param.sample_width / 8)
            pos = 0

            # 24 handled as 32
            # we have to bit extend highest bit (24th) to 4th byte
            # note this only works for little endian (wav format)
            if self._param.sample_width == 24:
                loc = 0
                new_data = []
                for entry in data[pos:]:
                    new_data.append(entry)
                    if loc == 2:
                        new_data.append(255 * (entry >> 7))
                    loc = 0 if loc == 2 else loc + 1
                data = data[:pos] + bytearray(new_data)
                bytes_per_sample = 4

            fmt = self._sample_width_to_struct_fmt(self._param.sample_width,
                                                   int(len(data) / (bytes_per_sample)))
            all_chan = struct.unpack(fmt, data[pos:])
            for chan in range(self._param.channels):
                self._audio[chan] = all_chan[chan::self._param.channels]

        finally:
            handler.close()

    def get_filename(self):
        return self._filename

    def get_audio_stream_num(self):
        return self._param.channels

    def get_audio_stream_sample_rate(self, index=0):
        if index >= len(self._audio):
            raise RuntimeError('audio stream %d not present' % (index))

        return self._param.sample_rate

    def get_audio_stream_sample_width(self, index=0):
        if index >= len(self._audio):
            raise RuntimeError('audio stream %d not present' % (index))

        return self._param.sample_width

    @read_data
    def get_audio_stream_data(self, index=0, start=0, limit=0):
        if index >= len(self._audio):
            raise RuntimeError('audio stream %d not present' % (index))

        if not limit:
            ret = self._audio[index][start:]
        else:
            ret = self._audio[index][start:start + limit]
        return ret

    def add_audio_stream(self, sample_rate, sample_width, data):
        if self._mode != MODE_WRITE:
            raise RuntimeError('file %s not in write mode' % (self._filename))

        if sample_width not in SAMPLE_WIDTH_LIST:
            raise RuntimeError('sample_width:%s not supported' % (sample_width))

        if self._audio[0]:
            if sample_rate != self._param.sample_rate:
                raise RuntimeError('add additional audio stream sample_rate does not match')
            if sample_width != self._param.sample_width:
                raise RuntimeError('add additional audio stream sample_width does not match')
            if len(self._audio[0]) != len(data):
                raise RuntimeError('add additional audio stream number of frames does not match')

        self._param.sample_rate = sample_rate
        self._param.sample_width = sample_width

        if not self._audio[0]:
            self._audio[0] = data
        else:
            self._audio.append(data)

        self._param.channels = len(self._audio)

    def write(self):
        if self._mode != MODE_WRITE:
            raise RuntimeError('file %s not in write mode' % (self._filename))

        handler = wave.open(self._filename, 'wb')
        try:
            handler.setparams(
                (self._param.channels,
                 int(self._param.sample_width / 8),
                 self._param.sample_rate,
                 len(self._audio[0]),
                 'NONE',
                 'not compressed')
            )

            fmt = self._sample_width_to_struct_fmt(
                self._param.sample_width, len(self._audio) * len(self._audio[0]))
            # FIXME this should be more efficient
            multi_data = [int(self._audio[chan][sample]) for sample in range(
                len(self._audio[0])) for chan in range(len(self._audio))]
            data = struct.pack(fmt, *multi_data)
            # 24 handled as 32,
            # we have to remove every 4th byte
            # note this only works for little endian (wav format)
            if self._param.sample_width == 24:
                data = bytearray(data)
                del data[3::4]  # remove every fourth element
            handler.writeframes(data)

        finally:
            handler.close()
