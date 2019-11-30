'''
Capability support library
'''

from kats.framework.library.file_util import load

ID = 'id'
MSG = 'messages'


def load_capability_file(filename):
    '''
    Get capability data from capability file

    The capability file should have the following format

    .. code-block:: python

        {
            "BASIC_PASS_THROUGH": {
                "id": 1,
                "messages": {
                    "CAPABILITY_VERSION_REQUEST": 4096,
                    "ENABLE_FADE_OUT": 8192,
                    "DISABLE_FADE_OUT": 8193,
                    "SET_DATA_STREAM_BASED": 8207,
                    "CHANGE_INPUT_DATA_TYPE": 10,
                    "CHANGE_OUTPUT_DATA_TYPE": 11
                }
            },
            "SCO_SEND":  {
                "id": 3,
                "messages": {
                }
            },
        }


    Args:
        filename (str): File name

    Returns:
        dict: Capability file contents
    '''

    return load(filename)


def get_capability_names(cap_data):
    '''
    Get all capabilities defined in a capability data object

    Args:
        cap_data (dict): Capability data object

    Returns:
        dict: Capability file contents
    '''

    return [cap for cap in cap_data]


def get_capability_id(cap_data, cap):
    '''
    Get capability id for a given capability id or name.

    Args:
        cap_data (dict): Capability data object
        cap (int or str): Capability id or name to search

    Returns:
        int: Capability id

    Raises:
        KeyError: If the capability name does not exist
    '''
    if isinstance(cap, int):
        return cap
    return cap_data[cap][ID]


def get_capability_name(cap_data, cap):
    '''
    Get capability name for a given capability id or name.

    Args:
        cap_data (dict): Capability data object
        cap (int or str): Capability id or name to search

    Returns:
        str: Capability name

    Raises:
        KeyError: If the capability name does not exist
    '''
    if not isinstance(cap, int):
        return cap
    for cap_name in cap_data:
        if cap_data[cap_name][ID] == cap:
            return cap_name
    raise ValueError('unable to find cap_name cap:%s' % (cap))


def get_capability_msgs(cap_data, cap_name):
    '''
    Get list of messages handled by a capability

    Args:
        cap_data (dict): Capability data object
        cap_name (str): Capability name to search

    Returns:
        dict: Capability messages is the format 'MSG': int

    Raises:
        KeyError: If the capability name does not exist
    '''
    if cap_name in cap_data:
        return cap_data[cap_name].get(MSG, {})
    return {}
