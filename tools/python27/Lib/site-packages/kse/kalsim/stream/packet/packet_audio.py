'''
Kalsim Packet streams
'''

import logging
import types

from kats.library.audio_file.audio import audio_get_instance
from kats.library.registry import get_instance

from .packetiser import Packetiser, STREAM_TYPE_SOURCE
from ..kalsim_stream import KalsimStream, STREAM_FLOW_CONTROL_DRIVE, \
    STREAM_FLOW_CONTROL_DRIVE_KALCMD, STREAM_BACKING, STREAM_BACKING_KALCMD, STREAM_FORMAT
from ..stream_base import STREAM_NAME, STREAM_RATE, STREAM_DATA_WIDTH, STREAM_DATA

FILENAME = 'filename'
CHANNEL = 'channel'
CHANNELS = 'channels'
SAMPLE_RATE = 'sample_rate'
SAMPLE_WIDTH = 'sample_width'

SUBINTERFACE = 'subinterface'
EXTERNAL_PACKETISER = 'external_packetiser'


def start(self):
    '''
    Start packet based streaming
    '''
    self._log.info('start')  # pylint: disable=protected-access
    self.old_start()
    self._packetiser.start()  # pylint: disable=protected-access


def stop(self):
    '''
    Sop packet based streaming
    '''
    self._log.info('stop')  # pylint: disable=protected-access
    self._packetiser.stop()  # pylint: disable=protected-access
    self.old_stop()


class StreamPacketAudio(KalsimStream):
    '''
    Kalsim Packet Stream

    Args:
        stream_type (str): Type of stream source or sink
        subinterface (str): Interface to the end stream
        filename (str): Path to audio filename.
        channels (int): Number of audio channels in file (only need for some file formats)
        sample_rate (int): Audio sampling rate in Herts (only need for some file formats)
        sample_width (int): Audio sampling size in bits (only need for some file formats)
        channel (int): Channel containing audio in the file
    '''

    platform = ['common']
    interface = 'packet_audio'

    def __new__(cls, stream_type, *args, **kwargs):

        filename = kwargs.pop(FILENAME)
        subinterface = kwargs.pop(SUBINTERFACE)
        stream_kwargs = {}
        stream_kwargs[CHANNELS] = kwargs.pop(CHANNELS, 1)
        stream_kwargs[SAMPLE_RATE] = kwargs.pop(SAMPLE_RATE, 8000)
        stream_kwargs[SAMPLE_WIDTH] = kwargs.pop(SAMPLE_WIDTH, 16)

        channel = kwargs.pop(CHANNEL, 0)

        if stream_type != STREAM_TYPE_SOURCE:
            raise RuntimeError('stream_type:%s not supported in packet_audio streams')

        audio_file = audio_get_instance(filename, **stream_kwargs)
        stream_kwargs[CHANNELS] = audio_file.get_audio_stream_num()
        stream_kwargs[SAMPLE_RATE] = audio_file.get_audio_stream_sample_rate(channel)
        stream_kwargs[SAMPLE_WIDTH] = audio_file.get_audio_stream_sample_width(channel)

        # audio data
        audio_data = audio_file.get_audio_stream_data(channel)

        # audio packet based data
        if (hasattr(audio_file, 'get_packet_data_size') and
                audio_file.get_packet_data_size('audio', channel)):
            packet_info = audio_file.get_packet_data('audio', channel)
        else:
            # we do not have packet based information so we will stream the file
            # at the right rate in packets of 1 msec
            # FIXME this does not work for 44.1 KHz
            sample_rate_khz = int(stream_kwargs[SAMPLE_RATE] / 1000)
            packet_info = [[1000, sample_rate_khz * pos, sample_rate_khz]
                           for pos in range(int(len(audio_data) / sample_rate_khz))]
            # FIXME include any pending bytes at the end
            # do not delay first packet
            if len(packet_info):
                packet_info[0][0] = 0

        logging.getLogger(__name__).info('metadata packets:%s', len(packet_info))

        # StreamBase parameters
        kwargs[STREAM_NAME] = filename
        kwargs[STREAM_RATE] = stream_kwargs[SAMPLE_RATE]
        kwargs[STREAM_DATA_WIDTH] = stream_kwargs[SAMPLE_WIDTH]
        kwargs[STREAM_DATA] = audio_data

        # the packetiser will control the streaming of data
        # here we configure kalsim for insert/extract to stream
        kwargs[STREAM_BACKING] = STREAM_BACKING_KALCMD
        kwargs[STREAM_FLOW_CONTROL_DRIVE] = STREAM_FLOW_CONTROL_DRIVE_KALCMD
        kwargs[EXTERNAL_PACKETISER] = True

        # in the case of audioslot streams this has to be set in spite of
        # being kalcmd driven and backed and that stream_insert includes the data width
        kwargs[STREAM_FORMAT] = audio_file.get_audio_stream_sample_width(channel)

        # instantiate the requested subinterface stream
        stream_factory = get_instance('stream')
        instance = stream_factory.get_instance(subinterface, stream_type, *args, **kwargs)

        # add methods to the stream that will handle the packet based streaming
        # pylint: disable=protected-access
        instance._audio_file = audio_file
        instance._packetiser = Packetiser(instance, audio_data, packet_info)
        instance.old_start = instance.start
        instance.start = types.MethodType(start, instance)
        instance.old_stop = instance.stop
        instance.stop = types.MethodType(stop, instance)
        return instance
