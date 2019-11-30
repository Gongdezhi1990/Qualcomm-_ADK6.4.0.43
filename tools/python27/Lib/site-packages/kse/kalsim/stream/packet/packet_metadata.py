'''
Kalsim Packet Metadata streams
'''

import logging
import types

from kats.library.audio_file.audio import audio_get_instance
from kats.library.registry import get_instance

from .packetiser import Packetiser, STREAM_TYPE_SOURCE
from ..kalsim_stream import KalsimStream, STREAM_FLOW_CONTROL_DRIVE, \
    STREAM_FLOW_CONTROL_DRIVE_KALCMD, STREAM_BACKING, STREAM_BACKING_KALCMD, STREAM_FORMAT, \
    STREAM_RATE
from ..stream_base import STREAM_NAME, STREAM_DATA_WIDTH, STREAM_DATA

FILENAME = 'filename'
CHANNEL = 'channel'

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


class StreamPacketMetadata(KalsimStream):
    '''
    Kalsim Packet Stream

    Args:
        stream_type (str): Type of stream source or sink
        subinterface (str): Interface to the end stream
        filename (str): Path to audio filename.
        channel (int): Channel containing metadata in the file
    '''

    platform = ['common']
    interface = 'packet_metadata'

    def __new__(cls, stream_type, *args, **kwargs):

        filename = kwargs.pop(FILENAME)
        subinterface = kwargs.pop(SUBINTERFACE)

        channel = kwargs.pop(CHANNEL, 0)

        if stream_type != STREAM_TYPE_SOURCE:
            raise RuntimeError('stream_type:%s not supported in packet_metadata streams')

        audio_file = audio_get_instance(filename)

        # metadata data
        metadata = audio_file.get_metadata_stream_data(channel)

        # metadata packet based info
        packet_info = []
        if (hasattr(audio_file, 'get_packet_data_size') and
                audio_file.get_packet_data_size('metadata', channel)):
            packet_info = audio_file.get_packet_data('metadata', channel)
        else:
            logging.getLogger(__name__).warning('filename %s metadata channel %s not found',
                                                filename, channel)

        logging.getLogger(__name__).info('metadata packets:%s', len(packet_info))

        # StreamBase parameters
        kwargs[STREAM_NAME] = filename
        kwargs[STREAM_RATE] = 0
        kwargs[STREAM_DATA_WIDTH] = 8
        kwargs[STREAM_DATA] = metadata

        # the packetiser will control the streaming of data
        # here we configure kalsim for insert/extract to stream
        kwargs[STREAM_BACKING] = STREAM_BACKING_KALCMD
        kwargs[STREAM_FLOW_CONTROL_DRIVE] = STREAM_FLOW_CONTROL_DRIVE_KALCMD
        kwargs[EXTERNAL_PACKETISER] = True

        # in the case of audioslot streams this has to be set in spite of
        # being kalcmd driven and backed and that stream_insert includes the data width
        kwargs[STREAM_FORMAT] = 8

        # instantiate the requested subinterface stream
        stream_factory = get_instance('stream')
        instance = stream_factory.get_instance(subinterface, stream_type, *args, **kwargs)

        # add methods to the stream that will handle the packet based streaming
        # pylint: disable=protected-access
        instance._audio_file = audio_file
        instance._packetiser = Packetiser(instance, metadata, packet_info)
        instance.old_start = instance.start
        instance.start = types.MethodType(start, instance)
        instance.old_stop = instance.stop
        instance.stop = types.MethodType(stop, instance)
        return instance
