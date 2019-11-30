'''
KATS framework Test Base Instrument class
'''

from abc import ABCMeta, abstractproperty

from six import add_metaclass


@add_metaclass(ABCMeta)  # pylint: disable=too-few-public-methods
class Instrument(object):
    '''
    Base Class that every instrument must subclass
    '''

    @abstractproperty
    def interface(self):
        '''
        str: Instrument name/interface
        '''
        pass

    @abstractproperty
    def schema(self):
        '''
        dict: Instrument schema
        '''
        pass
