'''
kalsim stream helper
'''

from collections import OrderedDict

from kats.kalsim.stream.kalsim_stream import STREAM_BACKING, STREAM_BACKING_FILE, \
    STREAM_BACKING_KALCMD, STREAM_FILENAME, STREAM_FLOW_CONTROL_DRIVE, \
    STREAM_FLOW_CONTROL_DRIVE_KALSIM, STREAM_FLOW_CONTROL_DRIVE_KALCMD, STREAM_FLOW_CONTROL_RATE, \
    STREAM_FLOW_CONTROL_BLOCK_SIZE, STREAM_FORMAT
from kats.kalsim.stream.stream_base import STREAM_DATA_WIDTH


def get_file_kalsim_stream_config(filename, frame_size, sample_rate, sample_width):
    '''
    Get kalsim stream configuration for a file backed and kalsim controlled stream.

    This stream could be a source or sink stream and will be fully controlled by kalsim.
    The file should be supported by kalsim, currently it treats the file as a raw file.

    Source streams do not require attention to send data. EOF generated when appropriate.
    Sink streams do not require attention. No EOF is generated.

    Args:
        filename (str): Filename
        frame_size (int): Number of samples per transaction
        sample_rate (int): Sampling frequency in hertzs
        sample_width (int): Sample data width in bits

    Returns:
        dict: Kalsim stream parameters
    '''
    return OrderedDict([
        [STREAM_BACKING, STREAM_BACKING_FILE],
        [STREAM_FILENAME, filename],
        [STREAM_FORMAT, sample_width],
        [STREAM_FLOW_CONTROL_RATE, sample_rate],
        [STREAM_FLOW_CONTROL_DRIVE, STREAM_FLOW_CONTROL_DRIVE_KALSIM],
        [STREAM_FLOW_CONTROL_BLOCK_SIZE, frame_size],
    ])


def get_file_user_stream_config(filename, sample_rate, sample_width):
    '''
    Get kalsim stream configuration for a file backed and user controlled stream.

    This stream could be a source or sink stream and will require calls to stream_induce to
    actually read or write data. The user is responsible to generate the appropriate timings
    to time stream_induce calls.
    Tbe file should be supported by kalsim, currently it treats the file as a raw file.

    Source streams require stream_induce calls to send data. EOF generated when appropriate.
    Sinks streams require stream_induce calls to receive data. No EOF is generated.

    Args:
        filename (str): Filename
        sample_rate (int): Sampling frequency in hertzs, only used in wav sink streams
        sample_width (int): Sample data width in bits

    Returns:
        dict: Kalsim stream parameters
    '''
    return OrderedDict([
        [STREAM_BACKING, STREAM_BACKING_FILE],
        [STREAM_FILENAME, filename],
        [STREAM_FORMAT, sample_width],
        [STREAM_FLOW_CONTROL_RATE, sample_rate],
        [STREAM_FLOW_CONTROL_DRIVE, STREAM_FLOW_CONTROL_DRIVE_KALCMD],
        [STREAM_DATA_WIDTH, sample_width],  # needed for StreamBase
    ])


def get_user_stream_config(sample_width):
    '''
    Get kalsim stream configuration for a fully user controlled stream.

    This stream could be a source or sink stream and will require calls to stream_insert or
    stream_extract to actually read or write data.
    The user is responsible to generate the appropriate timings to time
    stream_insert/stream_extract calls.

    Source streams require stream_insert calls to send data. No EOF is generated.
    Sinks streams require stream_extract calls to receive data. No EOF is generated.

    Args:
        filename (str): Filename
        sample_width (int): Sample data width in bits

    Returns:
        dict: Kalsim stream parameters
    '''
    return OrderedDict([
        [STREAM_BACKING, STREAM_BACKING_KALCMD],
        [STREAM_FORMAT, sample_width],
        [STREAM_FLOW_CONTROL_DRIVE, STREAM_FLOW_CONTROL_DRIVE_KALCMD],
        [STREAM_DATA_WIDTH, sample_width],  # needed for StreamBase
    ])
