'''
Configuration options
'''


def check_debug_session():
    '''
    Check if we are running in a pydev debug session

    Returns:
        bool: pydev session executing
    '''
    try:
        # pylint: disable=unused-variable,import-error
        import pydevd  # @UnresolvedImport @UnusedImport
        return True
    except ImportError:
        return False


#: constant indicating we are running inside a pydev debug session
DEBUG_SESSION = check_debug_session()


_LOCAL_CONFIG = {}


def set_config_param(param, value):
    '''
    Set configuration parameter

    Args:
        param (str): Parameter name
        value (any): Parameter value
    '''
    _LOCAL_CONFIG[param] = value


def get_config_param(param=None, default_value=None):
    '''
    Get configuration parameter

    Args:
        param (str or None): Parameter name or None for all
        default_value (any): Default value if parameter does not exist

    Returns:
        any: Parameter value or all parameters value
    '''
    if param:
        return _LOCAL_CONFIG.get(param, default_value)
    return _LOCAL_CONFIG
