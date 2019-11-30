'''
Factory support library
'''

import glob
import importlib
import inspect
import os
import pkgutil
import sys


def find_subclass(package, base_class):
    '''
    Find all classes subclassing a given class contained in any module located in a given
    package or subpackages.

    With the following structure

    .. code-block:: python

        kats/
            __init__.py
            package1/
                __init__.py
                file1.py
                package2/
                    __init__.py
                    file2.py

        find_subclass('kats.package1', MyBaseClass)

    would return all classes in the files file1.py and file2.py that subclass MyBaseClass

    Args:
        package (str): Package
        base_class (any): Base or abstract class

    Returns:
        list[any]: List of classes found subclassing base_class

    Note:
        In the case of abstract classes, this call will not check if the classes found
        are valid MyBaseClass subclasses, that is comply will all abstractmethod and
        abstract property.
    '''
    class_list = []

    mod = importlib.import_module(package)

    # get all submodules
    submodules = [entry for entry in pkgutil.walk_packages(mod.__path__, package + '.')]
    for (mod, mod_name, is_package) in submodules:
        if not is_package and mod_name != __name__:
            try:
                mod = importlib.import_module(mod_name)
                for _name, obj in inspect.getmembers(mod):
                    if (obj not in class_list and
                            inspect.isclass(obj) and
                            obj != base_class and
                            issubclass(obj, base_class)):
                        class_list.append(obj)
            except Exception:  # pylint: disable=broad-except
                pass

    return class_list


def find_subclass_dir(path, base_class):
    '''
    Find all classes subclassing a given class contained in a specified directory

    With the following structure

    Args:
        path (str): Location to search for classes
        base_class (any): Base or abstract class

    Returns:
        list[any]: List of classes found subclassing base_class

    Note:
        In the case of abstract classes, this call will not check if the classes found
        are valid MyBaseClass subclasses, that is comply will all abstractmethod and
        abstract property.
    '''
    class_list = []

    sys.path.insert(0, path)
    files = glob.glob(os.path.join(path, '*.py'))

    for filename in files:
        mod_name = os.path.splitext(os.path.basename(filename))[0]
        try:
            mod = importlib.import_module(mod_name)
            for _name, obj in inspect.getmembers(mod):
                if (obj not in class_list and
                        inspect.isclass(obj) and
                        obj != base_class and
                        issubclass(obj, base_class)):
                    class_list.append(obj)
        except Exception:  # pylint: disable=broad-except
            pass

    sys.path.pop(0)
    return class_list
