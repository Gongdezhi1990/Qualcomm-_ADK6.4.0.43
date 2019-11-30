'''
Raw audio file utilities
'''

import argparse
import logging
import struct

import wrapt

from kats.framework.library.docstring import inherit_docstring
from .audio_base import AudioBase, MODE_READ, MODE_WRITE, MODE_DEFAULT

CHANNELS_DEFAULT = 1
SAMPLE_RATE_DEFAULT = 8000
SAMPLE_WIDTH_DEFAULT = 16
SAMPLE_WIDTH_LIST = [8, 16, 24, 32]
BIG_ENDIAN_DEFAULT = False
START_OFFSET_DEFAULT = 0


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


class AudioRaw(AudioBase):
    '''
    Raw audio file class handler

    Note 8 bits samples are unsigned, 16 and 32 signed as in a wav file

    Args:
        filename (str): Filename
        mode (str): File open mode 'r' for read, 'w' for write
        channels (int): Number of channels in file (required for read mode)
        sample_rate (int): Sample rate in hertzs (required for read mode)
        sample_width (int): Number of bits per sample (required for read mode)
        big_endian (bool): Data in PCM big endian mode
        start_offset (int): File offset position for pcm audio data location
    '''

    platform = 'all'
    interface = 'raw'
    default_interface = True

    def __init__(self, filename, mode=MODE_DEFAULT, *args, **kwargs):
        inherit_docstring(self)
        self._log = logging.getLogger(__name__) if not hasattr(self, '_log') else self._log
        self._filename = filename
        self._mode = mode
        self._data_read = False
        self._param = argparse.Namespace()
        self._param.channels = kwargs.pop('channels', CHANNELS_DEFAULT)
        self._param.sample_rate = kwargs.pop('sample_rate', SAMPLE_RATE_DEFAULT)
        self._param.sample_width = kwargs.pop('sample_width', SAMPLE_WIDTH_DEFAULT)
        self._param.big_endian = kwargs.pop('big_endian', BIG_ENDIAN_DEFAULT)
        self._param.start_offset = kwargs.pop('start_offset', START_OFFSET_DEFAULT)

        if self._param.channels < 1:
            raise RuntimeError('channels %s invalid' % (self._param.channels))
        if self._param.sample_width not in SAMPLE_WIDTH_LIST:
            raise RuntimeError('sample_width %s invalid' % (self._param.sample_width))

        self._audio = [[] for _ in range(self._param.channels)]

        super(AudioRaw, self).__init__(filename, mode=mode, *args, **kwargs)

    def _sample_width_to_struct_fmt(self, sample_width, num=1):
        mod = '>' if self._param.big_endian else '<'
        mod += '%s' % (num)
        if sample_width == 8:
            mod += 'B'  # TODO this is unsigned data, others signed, to match wav file format
        elif sample_width == 16:
            mod += 'h'
        else:  # here we handle 24 and 32 bits, 24 require some magic
            mod += 'l'
        return mod

    def _read(self):
        with open(self._filename, 'rb') as handler:
            data = bytearray(handler.read())

        bytes_per_sample = int(self._param.sample_width / 8)
        pos = self._param.start_offset

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

        with open(self._filename, 'wb') as handler:
            if self._param.start_offset:
                handler.write(bytearray([0]) * self._param.start_offset)

            fmt = self._sample_width_to_struct_fmt(self._param.sample_width,
                                                   len(self._audio) * len(self._audio[0]))
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
            handler.write(data)
