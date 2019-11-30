'''
Hydra Capability Download Service
'''

import logging
import os
import time

from kats.framework.library.log import log_input, log_input_output
from kats.kalsim.stream.hydra.hydra import HYDRA_TYPE, HYDRA_TYPE_SUBSYSTEM, HYDRA_BAC_HANDLE
from kats.kalsim.stream.kalsim_stream import STREAM_FORMAT, STREAM_FLOW_CONTROL_DRIVE, \
    STREAM_FLOW_CONTROL_DRIVE_KALCMD, STREAM_FILENAME, STREAM_FLOW_CONTROL_BLOCK_SIZE, \
    STREAM_BACKING, STREAM_BACKING_FILE
from .audio_data_service import HydraAudioDataService
from .constants import SERVICE_TYPE_AUDIO_DATA_CAP_DOWNLOAD_SERVICE, \
    DEVICE_TYPE_CAP_DOWNLOAD, AUDIO_DATA_SERVICE_AUX_MSG

MAX_BLOCK_SIZE = 128

AUDIO_DATA_SERVICE_AUX_CAP_DOWNLOAD_KCS_ID = 0x00
AUDIO_DATA_SERVICE_AUX_CAP_DOWNLOAD_ERROR = 0x01

SAMPLE_WIDTH = 32


class HydraCapabilityDownloadService(HydraAudioDataService):
    '''
    Hydra Capability Download Service

    A service used to download and remove hydra capability files to and from kalsim.
    '''

    def __init__(self, stream, accmd, hydra_protocol, service_tag=1, **kwargs):
        self._stream = stream
        self._accmd = accmd
        self._total_samples = 0
        self._sent_samples = 0
        self._cur_stream = None

        kwargs.setdefault('data_buffer_size', 1024)
        kwargs.setdefault('metadata_buffer_size', 0)
        kwargs.setdefault('kick_required', 1)
        kwargs.setdefault('metadata_header_length', 0)
        kwargs.setdefault('connect_endpoint', 0)
        kwargs['space_handler'] = self._space_available

        super(HydraCapabilityDownloadService, self).__init__(
            hydra_protocol,
            SERVICE_TYPE_AUDIO_DATA_CAP_DOWNLOAD_SERVICE,
            DEVICE_TYPE_CAP_DOWNLOAD,
            service_tag,
            **kwargs)

    @log_input(logging.INFO)
    def _space_available(self, avail, space):
        _ = avail
        space = int((space * 8) / SAMPLE_WIDTH)

        if self._sent_samples >= self._total_samples:
            pass
        elif space >= MAX_BLOCK_SIZE:
            send = self._total_samples - self._sent_samples
            send = send if send < MAX_BLOCK_SIZE else MAX_BLOCK_SIZE

            self._cur_stream.induce(send)
            self.kick()
            self._sent_samples += send
        else:
            self.kick()

    @log_input_output(logging.INFO, logging.INFO)
    def download(self, filename, timeout=15):
        '''
        Download dkcs file with capabilities

        Args:
            filename (str): Full path to dkcs file
            timeout (float): Timeout in seconds

        Returns:
            int: kcs identifier
        '''
        super(HydraCapabilityDownloadService, self).start()

        kwargs = {
            HYDRA_TYPE: HYDRA_TYPE_SUBSYSTEM,
            HYDRA_BAC_HANDLE: self.param.data_wr_handle,

            STREAM_FORMAT: SAMPLE_WIDTH,
            STREAM_FLOW_CONTROL_DRIVE: STREAM_FLOW_CONTROL_DRIVE_KALCMD,
            STREAM_FILENAME: filename,
            STREAM_FLOW_CONTROL_BLOCK_SIZE: 1,
            STREAM_BACKING: STREAM_BACKING_FILE
        }

        stream = self._stream.get_instance('hydra', 'source', **kwargs)
        stream.create()
        stream.config()
        stream.start()
        self._cur_stream = stream

        self._total_samples = int(os.path.getsize(filename) / (SAMPLE_WIDTH / 8))
        self._sent_samples = 0
        send = self._total_samples - self._sent_samples
        send = send if send < MAX_BLOCK_SIZE else MAX_BLOCK_SIZE
        self._sent_samples += self._cur_stream.induce(send)
        self.kick()

        for _ in range(int(4 * timeout)):
            if self._sent_samples >= self._total_samples:
                break
            time.sleep(0.25)
        else:
            stream.stop()
            stream.destroy()
            super(HydraCapabilityDownloadService, self).stop()
            raise RuntimeError('timeout downloading capability')

        stream.stop()
        stream.destroy()

        # ==========================================================================================
        # after the last KICK_SERVICE_CONSUMER we will probably receive another one with space set
        # to maximum (255) before receiving the expected AUDIO_DATA_SERVICE_AUX_MSG
        # ==========================================================================================
        start = time.time()
        while True:
            payload = self._hydra_protocol.recv()
            try:
                if len(payload) != 4:
                    raise RuntimeError('audio data service aux message invalid')
                elif payload[0] != AUDIO_DATA_SERVICE_AUX_MSG:
                    raise RuntimeError(
                        'audio data service aux message invalid code:0x%04x' %
                        (payload[0]))
                elif payload[1] != self._service_tag:
                    raise RuntimeError(
                        'audio data service aux message invalid service tag:0x%04x' %
                        (payload[1]))
                elif payload[2] != AUDIO_DATA_SERVICE_AUX_CAP_DOWNLOAD_KCS_ID:
                    raise RuntimeError(
                        'audio data service aux message invalid parameter:0x%04x' %
                        (payload[2]))
                break
            except RuntimeError:
                pass
            if time.time() - start > 2:
                raise RuntimeError('audio data service aux message not received')

        super(HydraCapabilityDownloadService, self).stop()
        return payload[3]

    @log_input(logging.INFO)
    def remove(self, kcs_id):
        '''
        Remove a previously downloaded capability file

        Args:
            kcs_id (int): kcs identifier obtained in download
        '''
        self._accmd.cap_download_remove_kcs(kcs_id)
