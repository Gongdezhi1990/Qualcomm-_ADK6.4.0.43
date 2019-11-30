'''
Hydra Audio Data Services
'''

import argparse
import logging

from kats.framework.library.log import log_input, log_output, log_input_output
from kats.library.registry import get_instance
from .constants import KICK_SERVICE_CONSUMER


class HydraAudioDataService(object):
    '''
    Hydra Audio Data Service

    Args:
        hydra_protocol (kats.kalimba.hydra_service.protocol.HydraProtocol): Protocol handler
        service_type (int): Service type
        device_type (int): Device type
        service_tag (int): Hydra service tag
        data_buffer_size (int): Audio data buffer size in octets
        metadata_buffer_size (int): Meta data buffer size in octets
        kick_required (int): Signal remote system want to receive kicks
        metadata_buffer_size (int): Header length in a metadata buffer record in octets
        connect_endpoint (int): Endpoint id provided by the remote subsystem (the endpoint created
           by the service start will be connected via ACCMD to this endpoint.
    '''

    def __init__(self, hydra_protocol, service_type, device_type, service_tag, **kwargs):
        self._log = logging.getLogger(__name__) if not hasattr(self, '_log') else self._log
        self._hydra_protocol = hydra_protocol
        self._kalcmd = get_instance('kalcmd')
        self._service_type = service_type
        self._device_type = device_type
        self._service_tag = service_tag
        self._buffer_size = None
        self._buffer_width = None
        self._md_buffer_size = None
        self._md_buffer_width = None
        self._kick_required = kwargs.pop('kick_required', 0)
        self._space_handler = kwargs.pop('space_handler', None)
        self._kwargs = kwargs

        self._started = False
        self.param = argparse.Namespace()
        self.param.endpoint_id = None
        self.param.data_rd_handle = None
        self.param.data_wr_handle = None
        self.param.meta_data_rd_handle = None
        self.param.meta_data_wr_handle = None
        self.param.kick_status_bits = None
        self.param.kick_block_id = None

        self._msg_handler = self._hydra_protocol.install_message_handler(self._message_received)

    @log_input(logging.DEBUG)
    def _message_received(self, msg):
        # Temporary solution - support both message formats until changes are in place
        # in crescendo and stre FW to not send the data/space information with the
        # KICK_SERVICE_CONSUMER message. When this happens remove the len(msg) == 6 case.
        if len(msg) >= 2 and msg[0] == KICK_SERVICE_CONSUMER and msg[1] == self._service_tag:
            if self._space_handler:
                avail, space = self.get_buffer_stats()
                self._space_handler(avail, space)

    @log_input(logging.INFO)
    def start(self):
        '''
        Start audio data service

        Raises:
            RuntimeError: If the service has already been started
        '''
        if self._started:
            raise RuntimeError('service already started')

        msg = [
            self._service_tag,
            self._kwargs.get('data_buffer_size', 1024),
            self._kwargs.get('metadata_buffer_size', 0),
            self._kick_required,
            self._device_type,
            self._kwargs.get('metadata_header_length', 0),
            self._kwargs.get('connect_endpoint', 0),
        ]

        service_tag, payload = self._hydra_protocol.start_service(self._service_type, msg)

        if len(payload) < 7:
            raise RuntimeError('start response length:%s invalid' % (len(payload)))
        if service_tag != self._service_tag:
            raise RuntimeError('start response service tag:%s invalid' % (payload[1]))

        self._started = True
        self.param.endpoint_id = payload[0]
        self.param.data_rd_handle = payload[1]
        self.param.data_wr_handle = payload[2]
        self.param.meta_data_rd_handle = payload[3]
        self.param.meta_data_wr_handle = payload[4]
        self.param.kick_status_bits = payload[5]
        self.param.kick_block_id = payload[6]

    @log_input(logging.INFO)
    def stop(self):
        '''
        Stop audio data service

        Raises:
            RuntimeError: If the service as not been previously started
        '''
        if not self._started:
            raise RuntimeError('service not started')

        self._hydra_protocol.stop_service(self._service_type, self._service_tag)
        self._buffer_size = None
        self._md_buffer_size = None
        self._started = False

    def check_started(self):
        '''
        Check if the audio data service is started

        Returns:
            bool: Service already started
        '''
        return self._started

    @log_input_output(logging.DEBUG, logging.DEBUG)
    def kick(self):
        '''
        Kick firmware to alert new data is available
        '''
        self._hydra_protocol.kick(self.param.kick_status_bits)

    def get_endpoint_id(self):
        '''
        Get endpoint id

        Returns:
            int: Endpoint id
        '''
        return self.param.endpoint_id

    def get_data_read_handle(self):
        '''
        Get audio data read BAC handle

        Returns:
            int: Read handle
        '''
        return self.param.data_rd_handle

    def get_data_write_handle(self):
        '''
        Get audio data write BAC handle

        Returns:
            int: Write handle
        '''
        return self.param.data_wr_handle

    def get_meta_data_read_handle(self):
        '''
        Get meta data read BAC handle

        Returns:
            int: Read handle
        '''
        return self.param.meta_data_rd_handle

    def get_meta_data_write_handle(self):
        '''
        Get meta data write BAC handle

        Returns:
            int: Read handle
        '''
        return self.param.meta_data_wr_handle

    @log_output(logging.INFO)
    def get_space(self):
        '''
        Get buffer statistics

        Returns:
            int: Free samples
        '''
        # FIXME apparently the return of this function does not match firmware state
        payload = self._hydra_protocol.recv()
        if len(payload) != 6:
            raise RuntimeError('kick consumer length:%s invalid' % (len(payload)))
        elif payload[0] != KICK_SERVICE_CONSUMER:
            raise RuntimeError('kick consumer code:0x%04x invalid' % (payload[0]))
        elif payload[1] != self._service_tag:
            raise RuntimeError('kick consumer service type:%s invalid' % (payload[1]))

        return payload[3]

    @log_output(logging.INFO)
    def get_buffer_stats(self):
        '''
        Get kalcmd buffer statistics

        Buffer size doesn't change during the lifetime of the service; to speed things
        up we read the buffer size only on the first run and cache it.
        The values returned by Kalsim for handle offsets and buffer sizes are in
        octets and include all the octets in the buffer, useful or wasted. If the BAC sample
        size is configured to 8|16|24_BIT_UNPACKED a part of the buffer is wasted
        (e.g. 16_BIT_UNPACKED, only half of the buffer contains useful data - the lower
        16 bits of each 32-bit word).

        Returns:
            tuple:
                int: Used bytes
                int: Free bytes
        '''
        if self._buffer_size is None:
            self._buffer_size = self._kalcmd.get_buffer_size(self.param.data_rd_handle)
            self._buffer_width = self._kalcmd.get_handle_sample_size(self.param.data_rd_handle)

        buff_size = self._buffer_size
        rd_offset = self._kalcmd.get_handle_offset(self.param.data_rd_handle)
        wr_offset = self._kalcmd.get_handle_offset(self.param.data_wr_handle)
        if wr_offset >= rd_offset:
            used = wr_offset - rd_offset
        else:
            used = buff_size - (rd_offset - wr_offset)
        free = buff_size - used - 1

        used = int((used * self._buffer_width) / 32)
        free = int((free * self._buffer_width) / 32)
        return used, free

    @log_output(logging.INFO)
    def get_metadata_buffer_stats(self):
        '''
        Get kalcmd metadata buffer statistics

        Buffer size doesn't change during the lifetime of the service; to speed things
        up we read the buffer size only on the first run and cache it.
        The values returned by Kalsim for handle offsets and buffer sizes are in
        octets and include all the octets in the buffer, useful or wasted. If the BAC sample
        size is configured to 8|16|24_BIT_UNPACKED a part of the buffer is wasted
        (e.g. 16_BIT_UNPACKED, only half of the buffer contains useful data - the lower
        16 bits of each 32-bit word).

        Returns:
            tuple:
                int: Used bytes
                int: Free bytes
        '''
        if self._md_buffer_size is None:
            self._md_buffer_size = self._kalcmd.get_buffer_size(
                self.param.meta_data_rd_handle)
            self._md_buffer_width = self._kalcmd.get_handle_sample_size(
                self.param.meta_data_rd_handle)

        buff_size = self._md_buffer_size
        rd_offset = self._kalcmd.get_handle_offset(self.param.meta_data_rd_handle)
        wr_offset = self._kalcmd.get_handle_offset(self.param.meta_data_wr_handle)
        if wr_offset >= rd_offset:
            used = wr_offset - rd_offset
        else:
            used = buff_size - (rd_offset - wr_offset)
        free = buff_size - used - 1

        used = int((used * self._md_buffer_width) / 32)
        free = int((free * self._md_buffer_width) / 32)
        return used, free
