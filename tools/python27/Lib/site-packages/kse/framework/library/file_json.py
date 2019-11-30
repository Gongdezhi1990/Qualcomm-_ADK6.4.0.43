'''
JSON file utilities.
'''

import json

from collections import OrderedDict

def load(filename):
    '''
    Load file

    Args:
        filename (str): Path to file to load

    Returns:
        any: File contents
    '''
    with open(filename) as handler:
        return json.load(handler, object_pairs_hook=OrderedDict)


def dump(filename, data):
    '''
    Write file

    Args:
        filename (str): Path to file to dump
        data (dict or list): Data to be dumped
    '''
    with open(filename, 'w') as handler:
        json.dump(data, handler, indent=4, separators=(',', ': '))
