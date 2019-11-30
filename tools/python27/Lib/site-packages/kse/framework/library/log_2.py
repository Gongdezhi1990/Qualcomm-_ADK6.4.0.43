'''
Logging extensions
'''

import inspect
import logging
import sys

import wrapt

from .config import DEBUG_SESSION

PARAM_MAX_SIZE = 200


def _get_repr(param_name, value):
    '''
    Get representation of an object name

    Obtain the string representation of an object

    Args:
        param_name (any): Any object name
        value (any): Any object value

    Returns:
        str: Object name representation
    '''
    if isinstance(value, list) or isinstance(value, tuple):
        return '%s[%s]' % (param_name, len(value))
    return str(param_name)


def _get_repr_value(param, max_length=PARAM_MAX_SIZE, formatter='%s'):
    '''
    Get representation of an object value

    Obtain the string representation of an object trimming max length

    Args:
        param (any): Any object value
        max_length (int): Maximum representation string length
        formatter (str): Format specifier for the given `param`. If the param is a tuple or a list
                         this format specifier will be applied to all of their members. When the
                         format specifier fails to apply, e.g. trying to allow decimal point for a
                         string, the whole param will be converted to the string regardless of the
                         given format specifier.

    Returns:
        str: Object value representation
    '''
    try:
        if type(param) in (tuple, list):
            # Apply the format to all the members of the given list/tuple.
            ret = '[' + ', '.join([formatter % item for item in param]) + ']'

        else:
            ret = formatter % param

    except TypeError:
        ret = str(param)

    if len(ret) > max_length:
        ret = ret[:max_length] + '...'
    return ret


def _get_param(instance):
    '''
    Get representation of all parameters of an instance

    Args:
        instance (any): Any object

    Returns:
        str: Instance parameter representation
    '''
    ret = ''
    # pylint: disable=protected-access
    if hasattr(instance, '_log_param'):
        ret += '['
        for arg_name in instance._log_param:
            arg_val = instance._log_param[arg_name]
            ret += _get_repr(arg_name, arg_val) + ':' + _get_repr_value(arg_val) + ' '
        ret = ret[:-1] + '] '
    return ret


@wrapt.decorator
def log_exception(wrapped, instance, args, kwargs):  # @DontTrace
    '''
    log exception decorator, logs an exception if any exception is raised in the decorated function

    Args:
        wrapped (function): Wrapped function which needs to be called by your wrapper function.
        instance (any): Object to which the wrapped function was bound when it was called.
        args (tuple[any]): Positional arguments supplied when the decorated function was called.
        kwargs (dict): Keyword arguments supplied when the decorated function was called.
    '''
    try:
        try:
            return wrapped(*args, **kwargs)
        except Exception:
            exc_type, exc_value, exc_traceback = sys.exc_info()  # @UnusedVariable
            try:
                exc_traceback = exc_traceback.tb_next
            except Exception:  # pylint: disable=broad-except
                pass
            raise exc_type, exc_value, exc_traceback
    except Exception as exc:
        # pylint: disable=protected-access
        instance._log.error('error executing %s (%s)', wrapped.__name__, str(exc))
        raise


def log_input(level=logging.DEBUG, formatters=None):  # @DontTrace
    '''
    Decorator to emit a log message in the logger self._log when the function is invoked

    Args:
        level (int): Log level of the message
        formatters (dict): Specific log format for the given parameter.
                           The parameter name is the key and its format
                           specifier should be its value.
    '''
    if formatters is None:
        formatters = {}

    @wrapt.decorator(enabled=not DEBUG_SESSION)  # @DontTrace
    def wrapper(wrapped, instance, args, kwargs):  # @DontTrace
        '''
        Args:
            wrapped (function): Wrapped function which needs to be called by your wrapper function.
            instance (any): Object to which the wrapped function was bound when it was called.
            args (tuple[any]): Positional arguments supplied when the decorated function was called.
            kwargs (dict): Keyword arguments supplied when the decorated function was called.
        '''
        args_spec = inspect.getargspec(wrapped)
        kwargs_arg_num = len(args_spec.defaults) if args_spec.defaults else 0

        # Since `inspect.getargspec` always picks up the `self` argument
        # of the method as well as its other arguments, hence the magic
        # starting slice index of 1.
        arg_names = args_spec.args[1:len(args_spec.args) - kwargs_arg_num]
        args_key_vals = [(_get_repr(k, v), _get_repr_value(v, formatter=formatters.get(k, '%s')))
                         for k, v in zip(arg_names, args)]
        args_str = ' '.join('{k}:{v}'.format(k=k, v=v) for k, v in args_key_vals)

        kwargs_key_vals = [(_get_repr(k, v), _get_repr_value(v, formatter=formatters.get(k, '%s')))
                           for k, v in kwargs.items()]
        kwargs_str = ' '.join('{k}:{v}'.format(k=k, v=v) for k, v in kwargs_key_vals)

        all_arguments = ' '.join((args_str, kwargs_str)).strip()
        param_str = _get_param(instance)
        # pylint: disable=protected-access
        instance._log.log(level, '%s%s %s', param_str, wrapped.__name__, all_arguments)

        try:
            return wrapped(*args, **kwargs)
        except Exception:
            exc_type, exc_value, exc_traceback = sys.exc_info()  # @UnusedVariable
            try:
                exc_traceback = exc_traceback.tb_next
            except Exception:  # pylint: disable=broad-except
                pass
            raise exc_type, exc_value, exc_traceback

    return wrapper


def log_output(level=logging.DEBUG, formatters=None):  # #@DontTrace
    '''
    Decorator to emit a log message in the logger self._log when the function returns

    Args:
        level (int): Log level of the message
        formatters (dict): Specific log format for the given parameter. The parameter name is the
                           key and its format specifier should be its value. To format the return
                           value, use `return` as the key.
    '''
    if formatters is None:
        formatters = {}

    @wrapt.decorator(enabled=not DEBUG_SESSION)  # @DontTrace
    def wrapper(wrapped, instance, args, kwargs):  # @DontTrace
        '''
        Args:
            wrapped (function): Wrapped function which needs to be called by your wrapper function.
            instance (any): Object to which the wrapped function was bound when it was called.
            args (tuple[any]): Positional arguments supplied when the decorated function was called.
            kwargs (dict): Keyword arguments supplied when the decorated function was called.
        '''
        args_spec = inspect.getargspec(wrapped)
        kwargs_arg_num = len(args_spec.defaults) if args_spec.defaults else 0

        # Since `inspect.getargspec` always picks up the `self` argument
        # of the method as well as its other arguments, hence the magic
        # starting slice index of 1.
        arg_names = args_spec.args[1:len(args_spec.args) - kwargs_arg_num]
        args_key_vals = [(_get_repr(k, v), _get_repr_value(v, formatter=formatters.get(k, '%s')))
                         for k, v in zip(arg_names, args)]
        args_str = ' '.join('{k}:{v}'.format(k=k, v=v) for k, v in args_key_vals)

        kwargs_key_vals = [(_get_repr(k, v), _get_repr_value(v, formatter=formatters.get(k, '%s')))
                           for k, v in kwargs.items()]
        kwargs_str = ' '.join('{k}:{v}'.format(k=k, v=v) for k, v in kwargs_key_vals)

        try:
            ret = wrapped(*args, **kwargs)
        except Exception:
            exc_type, exc_value, exc_traceback = sys.exc_info()  # @UnusedVariable
            try:
                exc_traceback = exc_traceback.tb_next
            except Exception:  # pylint: disable=broad-except
                pass
            raise exc_type, exc_value, exc_traceback

        all_arguments = ' '.join((args_str, kwargs_str)).strip()
        param_str = _get_param(instance)
        # pylint: disable=protected-access
        instance._log.log(level, '%s%s %s ret:%s', param_str, wrapped.__name__, all_arguments,
                          '%s' % (_get_repr_value(ret, formatter=formatters.get('return', '%s')))
                          if ret is not None else '')
        return ret

    return wrapper


def log_input_output(in_level=logging.DEBUG, out_level=logging.DEBUG,
                     formatters=None):  # @DontTrace
    '''
    Decorator to emit a log message in the logger self._log when the function is invoked
    and another one when the function returns

    Args:
        in_level (int): Log level of the message when invoked
        out_level (int): Log level of the message when returns
        formatters (dict): Specific log format for the given parameter. The parameter name is the
                           key and its format specifier should be its value. To format the return
                           value, use `return` as the key.
    '''
    if formatters is None:
        formatters = {}

    @wrapt.decorator(enabled=not DEBUG_SESSION)  # @DontTrace
    def wrapper(wrapped, instance, args, kwargs):  # @DontTrace
        '''
        Args:
            wrapped (function): Wrapped function which needs to be called by your wrapper function.
            instance (any): Object to which the wrapped function was bound when it was called.
            args (tuple[any]): Positional arguments supplied when the decorated function was called.
            kwargs (dict): Keyword arguments supplied when the decorated function was called.
        '''
        args_spec = inspect.getargspec(wrapped)
        kwargs_arg_num = len(args_spec.defaults) if args_spec.defaults else 0

        # Since `inspect.getargspec` always picks up the `self` argument
        # of the method as well as its other arguments, hence the magic
        # starting slice index of 1.
        arg_names = args_spec.args[1:len(args_spec.args) - kwargs_arg_num]
        args_key_vals = [(_get_repr(k, v), _get_repr_value(v, formatter=formatters.get(k, '%s')))
                         for k, v in zip(arg_names, args)]
        args_str = ' '.join('{k}:{v}'.format(k=k, v=v) for k, v in args_key_vals)

        kwargs_key_vals = [(_get_repr(k, v), _get_repr_value(v, formatter=formatters.get(k, '%s')))
                           for k, v in kwargs.items()]
        kwargs_str = ' '.join('{k}:{v}'.format(k=k, v=v) for k, v in kwargs_key_vals)

        all_arguments = ' '.join((args_str, kwargs_str)).strip()
        param_str = _get_param(instance)
        # pylint: disable=protected-access
        instance._log.log(in_level, '%s %s', wrapped.__name__, all_arguments)

        try:
            ret = wrapped(*args, **kwargs)
        except Exception:
            exc_type, exc_value, exc_traceback = sys.exc_info()  # @UnusedVariable
            try:
                exc_traceback = exc_traceback.tb_next
            except Exception:  # pylint: disable=broad-except
                pass
            raise exc_type, exc_value, exc_traceback

        instance._log.log(out_level, '%s%s %s ret:%s', param_str, wrapped.__name__, all_arguments,
                          '%s' % (_get_repr_value(ret, formatter=formatters.get('return', '%s')))
                          if ret is not None else '')
        return ret

    return wrapper
