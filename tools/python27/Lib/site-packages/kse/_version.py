'''
KSE module version definition
'''

__version__ = '0.2.1'  # pylint: disable=invalid-name

version_info = [__version__.split('.')[ind]  # pylint: disable=invalid-name
                if len(__version__.split('.')) > ind
                else 0 if ind != 3 else ''
                for ind in range(5)]
