'''
Hydra a2dp streams
'''

from kats.kalimba.hydra_service.constants import DEVICE_TYPE_A2DP
from .audio_data import StreamHydraAudioData, DEVICE_TYPE


class StreamHydraA2dp(StreamHydraAudioData):
    '''
    Hydra a2dp streams

    Args:
        stream_type (str): Type of stream source or sink
    '''

    platform = ['crescendo', 'stre', 'streplus']
    interface = 'a2dp'

    def __init__(self, stream_type, **kwargs):
        kwargs.setdefault(DEVICE_TYPE, DEVICE_TYPE_A2DP)
        super(StreamHydraA2dp, self).__init__(stream_type, **kwargs)
