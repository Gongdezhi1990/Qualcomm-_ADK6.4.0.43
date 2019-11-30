'''
Module support library
'''

import os

import kats


def get_parent_module(module_name, module_filename):
    '''
    Get the parent module path of a given module path and file

    The module_filename is needed to compute the parent module
    file file1.file2.py inside the module kats would be module kats.file1.file2

    .. code-block:: python

        __name__
        'kats.kalimba.uut.uut'
        __file__
        'C:/Users/martina/project/kats4/kats/kats/kalimba/uut/uut.pyc'
        get_parent_module(__name__, __file__)
        kats.kalimba.uut

    Args:
        param (str): Module name
        module_filename (str): Filename for that module

    Returns:
        str: Parent module
    '''

    module_path = os.path.splitext(os.path.basename(module_filename))[0]
    return module_name.rstrip(module_path).rstrip('.')


def get_module_from_file(filename):
    '''
    Get the module path of a given file

    Args:
        filename (str): Filename to compute the module

    Returns:
        str: kats module name or '' if none found
    '''

    kats_filename = os.path.dirname(kats.__file__)
    filename = os.path.splitext(filename)[0]
    if filename.startswith(kats_filename):
        filename = filename[len(kats_filename) + 1:]
        return kats.__name__ + '.' + '.'.join(filename.split(os.sep))
    return ''
