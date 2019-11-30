# Copyright (c) 2014 - 2018 Qualcomm Technologies International, Ltd
# Part of the Python bindings for the kalaccess library.
# Python definitions matching C structures we need from kalaccess

from ctypes import c_int, c_char_p, c_uint, Structure


class ka_err(Structure):
    _fields_ = [("err_code", c_int),
                ("err_string", c_char_p)]


class ka_connection_details(Structure):
    _fields_ = [("transport_string", c_char_p),
                ("subsys_id", c_int),
                ("processor_id", c_uint),
                ("dongle_id", c_char_p)]


def ctypes_arr_type(elem_type, length):
    """
    Create a ctypes array type of 'length' elements of type 'elem_type'
    :type elem_type: Type
    :return Type
    """
    return elem_type * length


def to_ctypes_array(enumerable, c_types_type):
    transformed = (c_types_type * len(enumerable))()
    for index, value in enumerate(enumerable):
        transformed[index] = value
    return transformed


class CTypesStrIn(object):
    """
    Python 2/3 compatible wrapper for passing strings into ctypes wrapped functions.
    """
    @classmethod
    def from_param(cls, value):
        """
        Called by ctypes when the given value is going to be passed as a parameter.
        """

        # Already bytes: nothing to do
        if isinstance(value, bytes):
            return value

        # Python 3 (or in principle Python 2 unicode type) => encode to ASCII
        return value.encode('ascii')


def from_cstr(s):
    """
    Python 2 and 3 compatible ASCII-encoded C string to Python-string conversion.
    Used when bringing values back from the C interface.
    """
    if s is None:
        return s

    # Python 2: no-op
    if isinstance(s, str):
        return s

    # Python 3: from bytes
    return s.decode('ascii')
