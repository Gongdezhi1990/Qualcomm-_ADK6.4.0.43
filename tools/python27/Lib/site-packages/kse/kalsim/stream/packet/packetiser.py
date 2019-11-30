'''
Packetiser class
'''

import argparse
import logging

from kats.framework.library.log import log_input
from kats.library.registry import get_instance

STATE_STARTED = 'stream_state_started'
STATE_STOPPED = 'stream_state_stopped'
STATE_EOF = 'stream_state_eof'

STREAM_TYPE_SOURCE = 'source'
STREAM_TYPE_SINK = 'sink'


class PacketInfo(object):
    '''
    Structure describing a packet info

    Args:
        data (list[list[int,int,int]]): List of packet infos
    '''

    def __init__(self, data):
        self.timestamp = data[0]
        self.offset = data[1]
        self.size = data[2]

    def get_timestamp(self):
        '''
        Get info packet timestamp

        Returns:
            int: Timestamp in microseconds relative to previous packet timestamp
        '''
        return self.timestamp

    def get_offset(self):
        '''
        Get info packet stream data offset.

        This is the offset in the stream data for this packet

        Returns:
            int: Offset
        '''
        return self.offset

    def get_size(self):
        '''
        Get info packet stream data size.

        This is the size  in the stream data for this packet

        Returns:
            int: Offset
        '''
        return self.size


class Packetiser(object):
    '''
    Packet streamer

    Args:
        stream (kats.system.stream.stream_base.StreamBase): Stream instance
        data (list[int]): Audio stream samples
        packet (list[PacketInfo]): Audio packet info data
    '''

    def __init__(self, stream, data, packet):
        self._log = logging.getLogger(__name__) if not hasattr(self, '_log') else self._log

        self._stream = argparse.Namespace()
        self._stream.stream = stream
        # self._stream.type = stream.get_type()  # should be source
        self._stream.data = data
        self._stream.packet = packet
        self._uut = get_instance('uut')
        self._state = STATE_STOPPED
        self._timer_id = None
        self._packet_index = 0

    def _reset(self):
        self._packet_index = 0

    @log_input(logging.DEBUG)
    def _timer_handler(self, timer_id):
        _ = timer_id

        if self._state == STATE_STARTED:
            # send current packet
            packet = PacketInfo(self._stream.packet[self._packet_index])
            self._stream.stream.insert(
                self._stream.data[packet.offset:packet.offset + packet.size])
            self._packet_index += 1

        # check when next packet has to be sent
        while self._state == STATE_STARTED:
            if self._packet_index >= len(self._stream.packet):
                self._log.info('stream_id:%s eof', self._stream.stream.get_id())
                # self._reset()
                self._state = STATE_EOF
                self._stream.stream.eof()
                return

            packet = PacketInfo(self._stream.packet[self._packet_index])
            if packet.timestamp == 0:  # send immediately next packet
                self._log.warning('inserting %s samples',
                                  len(self._stream.data[packet.offset:packet.offset + packet.size]))
                self._stream.stream.insert(
                    self._stream.data[packet.offset:packet.offset + packet.size])
                self._packet_index += 1
            else:
                self._timer_id = \
                    self._uut.timer_add_relative(packet.timestamp / 1000000.0, self._timer_handler)
                break

    @log_input(logging.INFO)
    def start(self):
        '''
        Start stream
        '''
        if self._state != STATE_STOPPED:
            raise RuntimeError('cannot start stream')

        self._reset()
        self._state = STATE_STARTED

        if self._packet_index >= len(self._stream.packet):
            # self._reset()
            self._log.info('stream_id:%s eof', self._stream.stream.get_id())
            self._state = STATE_EOF
            self._stream.stream.eof()
        else:
            packet = PacketInfo(self._stream.packet[self._packet_index])
            if packet.timestamp == 0:
                self._timer_handler(0)
            else:
                self._timer_id = \
                    self._uut.timer_add_relative(packet.timestamp / 1000000.0, self._timer_handler)

    @log_input(logging.INFO)
    def stop(self):
        '''
        Stop stream
        '''
        self._reset()
        self._state = STATE_STOPPED

    def check_active(self):
        '''
        Check if the end of file has been reached.

        Returns:
            bool: End of file
        '''
        return self._state != STATE_EOF
