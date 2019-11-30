'''
Generic PCM endpoint class
'''

import logging

from kats.framework.library.docstring import inherit_docstring
from ..endpoint_base import EndpointBase


class EndpointPcmHydra(EndpointBase):
    '''
    Kymera PCM Endpoint

    - *endpoint_type* can be:

        - *source*, pushing data to the uut.
        - *sink*, extracting data from the uut.

    - *instance* is the pcm instance, kalsim for hydra supports 4 instances
    - *channel* is the time slot, 0 to pcm_slot_count-1
    - *pcm_sync_rate* is the sample clock rate in hertzs
    - *pcm_master_clock_rate* is the master clock rate in hertzs
    - *pcm_master_mode* is the master mode enable, 1 for master mode, 0 for slave mode
    - *pcm_slot_count* is the number of slots, 0 for automatic from pcm_sync_rate and
      pcm_master_clock_rate, 1 to 4 for explicit value
    - *pcm_manchester_mode* is Manchester mode enable, 1 for enable, 0 for disable
    - *pcm_short_sync_mode* is short sync enable, 1 for enable, 0 for disable
    - *pcm_manchester_slave_mode* is Manchester slave enable, 1 for enable, 0 for disable
    - *pcm_sign_extend_mode* is sign extend, 1 for enable, 0 for disable
    - *pcm_lsb_first_mode* indicates LSB first, 1 for enable, 0 for disable
    - *pcm_tx_tristate_mode* is tx tristate enable, 1 for enable, 0 for disable
    - *pcm_tx_tristate_rising_edge_mode* is tx tristate rising edge enable, 1 for enable, 0 for disable
    - *pcm_sync_suppress_enable* is sync suppress enable, 1 for enable, 0 for disable
    - *pcm_gci_mode* is GCI mode enable, 1 for enable, 0 for disable
    - *pcm_mute_enable* is mute enable, 1 for enable, 0 for disable
    - *pcm_long_length_sync* is long length sync enable, 1 for enable, 0 for disable
    - *pcm_sample_rising_edge* is sample rising edge enable, 1 for enable, 0 for disable
    - *pcm_rx_rate_delay* is rx rate delay, 0 to 7
    - *pcm_sample_format* is sample format.

        - 0 for 13 bits in 16-bit slot
        - 1 for 16 bits in 16-bit slot
        - 2 for 8 bits in 16-bit slot
        - 3 for 8 bits in 8-bit slot

    - *pcm_manchester_mode_rx_offset* is Manchester mode rx offset, 0 to 3
    - *pcm_audio_gain* is the audio gain, 0 to 7
    - *audio_mute_enable* is mute endpoint enable, 1 for enable, 0 for disable
    - *audio_sample_size* selects the size (width or resolution) of the audio sample on an
      audio interface.

        This setting controls the width of the internal data path, not just the number of bits
        per slot (on digital interfaces).
        All interfaces support the following settings:

            - 16: 16-bit sample size
            - 24: 24-bit sample size

        For the PCM interface, the following extra settings are supported for backward
        compatibility:

            - 0: 13 bits in a 16 bit slot
            - 1: 16 bits in a 16 bit slot (same as 16)
            - 2: 8 bits in a 16 bit slot
            - 3: 8 bits in an 8 bit slot

    Args:
        kymera (kats.kalimba.kymera.kymera_base.KymeraBase): Instance of class Kymera
        endpoint_type (str): Type of endpoint source or sink
        instance (int): PCM instance
        channel (int): Time slot 0 to 3
        pcm_sync_rate (int): Sample clock rate in hertzs
        pcm_master_clock_rate (int): Master clock rate in hertzs
        pcm_master_mode (int): 1 for Master mode, 0 for Slave mode
        pcm_slot_count (int): Number of slots, 0 for automatic from pcm_sync_rate and
            pcm_master_clock_rate, 1-4 for explicit value
        pcm_manchester_mode (int): Manchester mode, 1 for enable, 0 for disable
        pcm_short_sync_mode (int): Short sync, 1 for enable, 0 for disable
        pcm_manchester_slave_mode (int): Manchester slave, 1 for enable, 0 for disable
        pcm_sign_extend_mode (int): Sign extend, 1 for enable, 0 for disable
        pcm_lsb_first_mode (int): LSB first, 1 for enable, 0 for disable
        pcm_tx_tristate_mode (int): Tx tristate, 1 for enable, 0 for disable
        pcm_tx_tristate_rising_edge_mode (int): Tx tristate rising edge, 1 for enable, 0 for disable
        pcm_sync_suppress_enable (int): Sync suppress, 1 for enable, 0 for disable
        pcm_gci_mode (int): GCI mode, 1 for enable, 0 for disable
        pcm_mute_enable (int): Mute, 1 for enable, 0 for disable
        pcm_long_length_sync (int): Long length sync, 1 for enable, 0 for disable
        pcm_sample_rising_edge (int): Sample rising, 1 for enable, 0 for disable
        pcm_rx_rate_delay (int): Rx rate delay, 0 to 7
        pcm_sample_format (int): Sample format:

            - 0 for 13 bits in 16-bit slot
            - 1 for 16 bits in 16-bit slot
            - 2 for 8 bits in 16-bit slot
            - 3 for 8 bits in 8-bit slot

        pcm_manchester_mode_rx_offset (int): Manchester mode rx offset, 0 to 3
        pcm_audio_gain (int): Audio gain, 0 to 7
        audio_mute_enable (int): Mute endpoint, 1 for enable, 0 for disable
        audio_sample_size (int): Selects the size (width or resolution) of the audio sample on an
            audio interface.

            This setting controls the width of the internal data path, not just the number of bits
            per slot (on digital interfaces).
            All interfaces support the following settings:

                - 16: 16-bit sample size
                - 24: 24-bit sample size

            For the PCM interface, the following extra settings are supported for backward
            compatibility:

                - 0: 13 bits in a 16 bit slot
                - 1: 16 bits in a 16 bit slot (same as 16)
                - 2: 8 bits in a 16 bit slot
                - 3: 8 bits in an 8 bit slot
    '''

    platform = ['crescendo', 'stre', 'streplus']
    interface = 'pcm'

    def __init__(self, kymera, endpoint_type, *args, **kwargs):
        self._log = logging.getLogger(__name__) if not hasattr(self, '_log') else self._log
        inherit_docstring(self)
        self._instance = kwargs.pop('instance', 0)
        self._channel = kwargs.pop('channel', 0)

        # initialise values
        self.__args = []
        for entry in args:
            if not isinstance(entry, list):
                raise RuntimeError('arg %s invalid should be a list' % (entry))
            elif len(entry) != 2:
                raise RuntimeError('arg %s invalid should be list of 2 elements' % (entry))
            self.__args.append(entry)

        self.__args += list(kwargs.items())

        super(EndpointPcmHydra, self).__init__(kymera, endpoint_type)

    def create(self, *_, **__):
        self._create('pcm', [self._instance, self._channel])

    def config(self):

        for entry in self.__args:
            self.config_param(entry[0], entry[1])

        super(EndpointPcmHydra, self).config()
