'''
Registry
'''

_REGISTERED_INSTANCE = {}


def register_instance(name, instance):
    '''
    Register a instance of a given name

    Args:
        name (str): Name of the instance
        instance (any): Instance to register
    '''
    _REGISTERED_INSTANCE.setdefault(name, [])
    _REGISTERED_INSTANCE[name].append(instance)


def unregister_instance(name, instance):
    '''
    Unregister a instance of a given name

    Args:
        name (str): Name of the instance
        instance (any): Instance to unregister
    '''
    # note this is only removing the first instance found
    _REGISTERED_INSTANCE[name].remove(instance)


def get_instance(name, index=0):
    '''
    Get the instance of a registered name

    Args:
        name (str): Name of the instance
        index (int): Zero based index of the instance

    Returns:
        any: Instance
    '''
    return _REGISTERED_INSTANCE[name][index]


def get_instance_num(name):
    '''
    Get number of instance of a registered name

    Args:
        name (str): Name of the instance

    Returns:
        int: Number of instances
    '''
    return 0 if name not in _REGISTERED_INSTANCE else len(_REGISTERED_INSTANCE[name])
