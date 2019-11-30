'''
Docstring extensions
'''

import six


def inherit_docstring(self):
    '''
    Inherit docstring from base class

    When defining base classes or abstract classes, may times the documentation is only written
    once in the base/abstract class and the subclass has empty documentation
    This call copies the docstring for every public callable (methods not starting with _ of an
    instance if its methods have no docstring and they exist and have docstring in the base class

    Args:
        self (any): Instance to derived class
    '''

    methods = [method for method in dir(self) if callable(getattr(self, method))]
    bases = self.__class__.__bases__
    for entry in methods:
        if not entry.startswith('_'):
            if not getattr(self, entry).__doc__:
                for ancestor in bases:
                    if hasattr(ancestor, entry) and getattr(ancestor, entry).__doc__:
                        if six.PY2:
                            getattr(self, entry).im_func.__doc__ = getattr(ancestor, entry).__doc__
                        else:
                            getattr(self, entry).__func__.__doc__ = getattr(ancestor, entry).__doc__
                        break
