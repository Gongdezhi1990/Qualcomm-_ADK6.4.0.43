'''
Generic I2S endpoint class
'''

import logging

from kats.framework.library.docstring import inherit_docstring
from ..endpoint_base import EndpointBase


class EndpointI2sHydra(EndpointBase):
    '''
    Generic I2S Endpoint

    - *endpoint_type* can be:

        - *source*, pushing data to the uut.
        - *sink*, extracting data from the uut.

    - *instance* is the i2s instance, kalsim for hydra supports 4 instances
    - *channel* is the time slot, 0 to 1
    - *i2s_sync_rate* is the sample clock rate in hertzs
    - *i2s_master_clock_rate* is the master clock rate in hertzs
    - *i2s_master_mode* is the master mode enable, 1 for master mode, 0 for slave mode
    - *i2s_justify_mode* is the justify mode, 0 left justified, 1 right justified
    - *i2s_left_justify_delay* is the delay in left justify mode

        - 0 (MSB of SD data occurs in the first SCLK period following WS transition)
        - 1 (MSB of SD data occurs in the second SCLK period)

    - *i2s_channel_polarity* is the channel polarity, 0 data is left channel when WS is high,
      1 data is right channel with WS is high
    - *i2s_audio_attenuation_enable* Audio attenuation enable, 1 for enable, 0 for disable
    - *i2s_audio_attenuation* is the audio attenuation, 0 to 15 in 6dB steps
    - *i2s_justify_resolution* is the justify resolution:

        - 0 16 bit
        - 1 20 bit
        - 2 24 bit

    - *i2s_crop_enable* is crop enable, 1 for enable, 0 for disable
    - *i2s_bits_per_sample* is the number of bit per sample:

        - 0 16 bit
        - 1 20 bit
        - 2 24 bit

    - *i2s_tx_start_sample* 0 during low wclk phase, 1 during high wclk phase
    - *i2s_rx_start_sample* 0 during low wclk phase, 1 during high wclk phase
    - *audio_mute_enable* is mute endpoint enable, 1 for enable, 0 for disable
    - *audio_sample_size* selects the size (width or resolution) of the audio sample on an
      audio interface.

        This setting controls the width of the internal data path, not just the number of bits
        per slot (on digital interfaces).
        All interfaces support the following settings:

            - 16: 16-bit sample size
            - 24: 24-bit sample size

        For the I2S interface, the following extra settings are supported for backward
        compatibility:

            - 0: 13 bits in a 16 bit slot
            - 1: 16 bits in a 16 bit slot (same as 16)
            - 2: 8 bits in a 16 bit slot
            - 3: 8 bits in an 8 bit slot

    Args:
        kymera (kats.kalimba.kymera.kymera_base.KymeraBase): Instance of class Kymera
        endpoint_type (str): Type of endpoint source or sink
        instance (int): I2S instance
        channel (int): Time slot 0 to 1
        i2s_sync_rate (int): Sample clock rate in hertzs
        i2s_master_clock_rate (int): Master clock rate in hertzs
        i2s_master_mode (int): 1 for Master mode, 0 for Slave mode
        i2s_justify_mode (int): 0 left justified, 1 right justified
        i2s_left_justify_delay (int): If using Left Justified format
            - 0 (MSB of SD data occurs in the first SCLK period following WS transition)
            - 1 (MSB of SD data occurs in the second SCLK period)
        i2s_channel_polarity (int): 0 data is left channel when WS is high,
            1 data is right channel with WS is high
        i2s_audio_attenuation_enable (int): Audio attenuation enable, 1 for enable, 0 for disable
        i2s_audio_attenuation (int): Audio attenuation, 0 to 15 in 6dB steps
        i2s_justify_resolution (int): Justify resolution:

            - 0 16 bit
            - 1 20 bit
            - 2 24 bit

        i2s_crop_enable (int): Crop enable, 1 for enable, 0 for disable
        i2s_bits_per_sample (int): Bits per sample:

            - 0 16 bit
            - 1 20 bit
            - 2 24 bit

        i2s_tx_start_sample (int): 0 during low wclk phase, 1 during high wclk phase
        i2s_rx_start_sample (int): 0 during low wclk phase, 1 during high wclk phase
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
    interface = 'i2s'

    def __init__(self, kymera, endpoint_type, *args, **kwargs):
        self._log = logging.getLogger(__name__) if not hasattr(self, '_log') else self._log
        inherit_docstring(self)
        self._instance = kwargs.pop('instance', 0)
        self._channel = kwargs.pop('channel', 0)

        # initialise default values
        self.__args = []
        for entry in args:
            if not isinstance(entry, list):
                raise RuntimeError('arg %s invalid should be a list' % (entry))
            elif len(entry) != 2:
                raise RuntimeError('arg %s invalid should be list of 2 elements' % (entry))
            self.__args.append(entry)

        self.__args += list(kwargs.items())

        super(EndpointI2sHydra, self).__init__(kymera, endpoint_type)

    def create(self, *_, **__):
        self._create('i2s', [self._instance, self._channel])

    def config(self):

        for entry in self.__args:
            self.config_param(entry[0], entry[1])

        super(EndpointI2sHydra, self).config()
