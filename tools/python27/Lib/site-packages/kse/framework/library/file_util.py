'''
File utilities.
'''

import os

from .constant import WORKSPACE_ENV, WORKSPACE_CONFIG, WORKSPACE_RESULT, WORKSPACE_LOG

YAML_EXTENSION = '.yaml'
MSGPACK_EXTENSION = '.mpack'


def load(filename, strict=True):
    '''
    Load file

    It does support files ending in:

        .yaml (yaml files)
        .mpack (messagepack files)
        .json (json files, default)

    Args:
        filename (str): Path to file to load
        strict (bool): Load in strict mode, this mode is slower but dicts will be returned
            as OrderedDict hence ordering is kept (python 3.6 will keep order even in dicts)

    Returns:
        any: File contents
    '''
    if filename.endswith(YAML_EXTENSION):
        from .file_yaml import load as yaml_load
        ret = yaml_load(filename)
    elif filename.endswith(MSGPACK_EXTENSION):
        from .file_mpack import load as mpack_load
        ret = mpack_load(filename, strict=strict)
    else:
        from .file_json import load as json_load
        ret = json_load(filename)
    return ret


def dump(filename, data):
    '''
    Write file

    It does support files ending in:

        .yaml (yaml files)
        .mpack (messagepack files)
        .json (json files, default)

    Args:
        filename (str): Path to file to dump
        data (dict or list): Data to be dumped
    '''
    if filename.endswith(YAML_EXTENSION):
        from .file_yaml import dump as yaml_dump
        yaml_dump(filename, data)
    elif filename.endswith(MSGPACK_EXTENSION):
        from .file_mpack import dump as mpack_dump
        mpack_dump(filename, data)
    else:
        from .file_json import dump as json_dump
        json_dump(filename, data)


def get_config_filename(filename):
    '''
    Get filename of a configuration file

    Absolute path are left unmodified, relative paths with KATS_WORKSPACE environment
    variable defined are made relative to config directory

    Args:
        filename (str): Path to file

    Returns:
        str: Path to file
    '''
    workspace = os.environ.get(WORKSPACE_ENV)
    if workspace and not os.path.isabs(filename):
        return os.path.join(workspace, WORKSPACE_CONFIG, filename)
    return filename


def get_result_filename(filename):
    '''
    Get filename of a result file

    Absolute path are left unmodified, relative paths with KATS_WORKSPACE environment
    variable defined are made relative to result directory

    Args:
        filename (str): Path to file

    Returns:
        str: Path to file
    '''
    workspace = os.environ.get(WORKSPACE_ENV)
    if workspace and not os.path.isabs(filename):
        return os.path.join(workspace, WORKSPACE_RESULT, filename)
    return filename


def get_log_filename(filename):
    '''
    Get filename of a log file

    Absolute path are left unmodified, relative paths with KATS_WORKSPACE environment
    variable defined are made relative to log directory

    Args:
        filename (str): Path to file

    Returns:
        str: Path to file
    '''
    workspace = os.environ.get(WORKSPACE_ENV)
    if workspace and not os.path.isabs(filename):
        return os.path.join(workspace, WORKSPACE_LOG, filename)
    return filename
