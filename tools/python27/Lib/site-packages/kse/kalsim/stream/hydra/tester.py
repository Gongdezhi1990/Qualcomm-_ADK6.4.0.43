'''
Hydra tester streams
'''

from kats.kalimba.hydra_service.constants import DEVICE_TYPE_TESTER
from .audio_data import StreamHydraAudioData, DEVICE_TYPE


class StreamHydraTester(StreamHydraAudioData):
    '''
    Hydra tester streams

    Args:
        stream_type (str): Type of stream source or sink
    '''

    platform = ['crescendo', 'stre', 'streplus']
    interface = 'tester'

    def __init__(self, stream_type, **kwargs):
        kwargs.setdefault(DEVICE_TYPE, DEVICE_TYPE_TESTER)
        super(StreamHydraTester, self).__init__(stream_type, **kwargs)
