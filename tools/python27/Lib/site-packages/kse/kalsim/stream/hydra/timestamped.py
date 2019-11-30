'''
Hydra timestamped streams
'''

from kats.kalimba.hydra_service.constants import DEVICE_TYPE_TIMESTAMPED
from .audio_data import StreamHydraAudioData, DEVICE_TYPE, METADATA_FORMAT, METADATA_ENABLE, \
    METADATA_FORMAT_TIMESTAMPED


class StreamHydraTimestamped(StreamHydraAudioData):
    '''
    Hydra timestamped streams

    Args:
        stream_type (str): Type of stream source or sink
    '''

    platform = ['crescendo', 'stre', 'streplus']
    interface = 'timestamped'

    def __init__(self, stream_type, **kwargs):
        kwargs.setdefault(DEVICE_TYPE, DEVICE_TYPE_TIMESTAMPED)
        kwargs.setdefault(METADATA_ENABLE, True)
        kwargs.setdefault(METADATA_FORMAT, METADATA_FORMAT_TIMESTAMPED)
        super(StreamHydraTimestamped, self).__init__(stream_type, **kwargs)
