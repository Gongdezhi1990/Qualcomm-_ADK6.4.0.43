############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2018 - 2019 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
"""
Configuring the logger and provide some decorators for logging methods and
functions.

Decorators for functions and methods require `logger` as parameter, this helps
the log user to relate log lines with their corresponding packages. Individual
packages need to define their own loggers and pass it to the suitable
decorator.

See the example below for hypothetical package called `ACAT.Core.foo.py`:

    # foo.py
    import logging

    from ACAT.Core.logger import function_logger

    logger = logging.getLogger(__name__)


    @function_logger(logger)
    def say_hi(name):
        return 'Hi %s' % name

Above code will generate log lines for `name` parameter and record the result
for `ACAT.Core.foo` (or whatever the full package name is). The usage for
`method_logger` decorator is exactly the same but should be used against
methods only, the first parameter will be missed otherwise.
"""
from __future__ import print_function

import functools
import json
import numbers
import os

from logging.config import dictConfig

CURRENT_DIRECTORY = os.path.dirname(os.path.realpath(__file__))
LOG_LOCATION = os.path.join(
    CURRENT_DIRECTORY, '..', 'logs'
)
LOG_CONFIGURATIONS = os.path.join(
    CURRENT_DIRECTORY, '..', 'config', 'logging.json'
)
LOG_CONFIGURATIONS_VERBOSE = os.path.join(
    CURRENT_DIRECTORY, '..', 'config', 'logging_verbose.json'
)


def config_logger(verbose=False):
    """Configure the logger using the configuration json file.

    Args:
        verbose (bool, optional): If logger should run in verbose mode,
            the verbose configuration file will be used. This means log
            lines are going to be more expressive and more lines on the
            console.
    """
    if verbose:
        log_configuration = LOG_CONFIGURATIONS_VERBOSE
        # Sanity checks
        if not os.path.exists(LOG_LOCATION):
            os.makedirs(LOG_LOCATION)

    else:
        log_configuration = LOG_CONFIGURATIONS

    if not os.path.exists(log_configuration):
        # Log Configuration doesn't exist, print the message and move on
        print("Log Configuration file <%s> doesn't exist" % log_configuration)
        return

    # Start the configuration
    with open(log_configuration) as handler:
        try:
            config = json.load(handler)

        except ValueError as error:
            # Invalid json content. Prints the error and move on
            print("Invalid JSON file. Error: %s" % error)
            return

    # Make the logging file names absolute
    for handler_name, handler_attrs in config.get('handlers', {}).items():
        if handler_attrs.get('filename') is not None:
            abs_path = os.path.join(
                LOG_LOCATION,
                handler_attrs.get('filename')
            )
            config['handlers'][handler_name]['filename'] = abs_path

    dictConfig(config)


def function_logger(logger):
    """
    A decorator wraps around functions and record parameters and results.

    If the function raises exception, this will be record it as well
    and re-raise the exception.

    Args:
        logger: A logger instance.

    Returns:
        A wrapped function.
    """
    def decorator(func):
        """Decorates a function."""
        @functools.wraps(func)
        def wrapper(*args, **kwargs):
            """Wraps around the called function with the exact parameters."""
            # pylint: disable=star-args
            logger.debug(_gen_call_repr(func, *args, **kwargs))
            # Call the requested function with the same arguments.
            result = func(*args, **kwargs)
            logger.debug('{} returned: {}'.format(
                func.__name__, str(_convert_to_hex(result)))
            )
            return result

        return wrapper

    return decorator


def method_logger(logger):
    """A Decorator wraps around methods and record parameters and results.

    If the method raises exception, it will record it as well and re-raise
    the exception. Also, it skips the first argument of all the wrapped
    methods, since that argument is reserved for `cls` and `self`.

    Args:
        logger: A logger instance.

    Returns:
        Wrapped method.
    """
    def decorator(method):
        """Decorates a method."""
        @functools.wraps(method)
        def wrapper(*args, **kwargs):
            """Wraps around the called method with the exact parameters."""
            # pylint: disable=star-args
            logger.debug(_gen_call_repr(method, *args[1:], **kwargs))
            # Call the requested method with the same arguments.
            result = method(*args, **kwargs)
            logger.debug('{} returned: {}'.format(
                method.__name__, str(_convert_to_hex(result)))
            )
            return result

        return wrapper

    return decorator


def _convert_to_hex(value):
    """Converts the value to hex.

    Args:
        value (any): The value that needs to be converted into hex.

    Returns:
        Any: The converted value to hex can be in in forms of None, a tuple,
            a list or a dictionary.
    """
    if isinstance(value, (list, tuple)):
        output = []
        for item in value:
            output.append(_convert_to_hex(item))

        return output

    elif isinstance(value, dict):
        output = {}
        for key, val in value.items():
            output[key] = _convert_to_hex(val)

        return output

    elif isinstance(value, bool):
        return value

    elif isinstance(value, numbers.Integral):
        return hex(value)

    else:
        return value

def _gen_call_repr(wrapped, *args, **kwargs):
    """Generate a representative message on how a method/function is called.

    Args:
        wrapped: The function/method which is wrapped by decorators.
        arags: Arbitrary arguments.
        kwargs: Arbitrary keyword arguments.

    Returns:
        str: Method name with its arguments and keyword arguments.
    """
    args = _convert_to_hex(args)
    kwargs = _convert_to_hex(kwargs)

    kwargs_lst = sorted([('{}={}'.format(k, v)) for k, v in kwargs.items()])
    called_parameters = ', '.join(
        [str(param) for param in list(args) + kwargs_lst]
    )
    return '{} called with: ({})'.format(wrapped.__name__, called_parameters)
