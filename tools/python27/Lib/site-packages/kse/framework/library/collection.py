'''
Collections library utilities

All function in this module handle json objects
A json object is the representation of valid json in python.
The object supports dict, OrderedDict and list containers and any values.
Subelements could be containers as well.
'''

import copy
from string import Template

import os
from six import string_types

from .config import get_config_param

OUT_MODE_INPLACE = 'inplace'
OUT_MODE_SHALLOW_COPY = 'shallow_copy'
OUT_MODE_DEEP_COPY = 'deep_copy'
OUT_MODES = [OUT_MODE_INPLACE, OUT_MODE_SHALLOW_COPY, OUT_MODE_DEEP_COPY]

OUT_MODE_FUNC = {
    OUT_MODE_INPLACE: lambda x: x,
    OUT_MODE_SHALLOW_COPY: copy.copy,
    OUT_MODE_DEEP_COPY: copy.deepcopy

}

DICT_MODE_OVERWRITE = 'overwrite'
DICT_MODE_APPEND = 'append'
DICT_MODES = [DICT_MODE_OVERWRITE, DICT_MODE_APPEND]

LIST_MODE_OVERWRITE = 'overwrite'
LIST_MODE_APPEND = 'append'
LIST_MODE_OVERWRITE_APPEND = 'overwrite_append'
LIST_MODE_APPEND_ALWAYS = 'append_always'
LIST_MODES = [
    LIST_MODE_OVERWRITE,
    LIST_MODE_APPEND,
    LIST_MODE_OVERWRITE_APPEND,
    LIST_MODE_APPEND_ALWAYS]


def json_merge(data1, data2, **kwargs):
    '''
    Merge two json objects.

    This function will return data1 data (base) with whatever modifications included in data2.
    The decision to add more data in data1 that was not originally present but present in data2 is
    defined by dict_mode and list_mode

    Args:
        data1 (dict, OrderedDict or list): Base input data
        data2 (dict, OrderedDict or list): New data
        out_mode (str). How to return data, set it to
            - OUT_MODE_INPLACE all operation will be done in data1
            - OUT_MODE_SHALLOW_COPY all operations will be done in a shallow copy of data1
            - OUT_MODE_DEEP_COPY all operations will be done in a deep copy of data1
        dict_mode (str). How to handle dict and OrderedDict
            - DICT_MODE_OVERWRITE keys that are already present in data1 will be overwritten with
              data2 if they exist, but new keys in data2 will be omitted.
            - DICT_MODE_APPEND keys only in data1 will be left intact, both in data1 and data2 will
              return data2 values and only in data2 will return data2 values
        list_mode (str). How to handle list
            - LIST_MODE_OVERWRITE only indexes in data1 will be overwritten with data2 if they exist
              in both data1 and data2
            - LIST_MODE_APPEND indexes only in data1 will be left intact, both in data1 and data2
              will return data2 values and only in data2 will return data2 values
            - LIST_MODE_OVERWRITE_APPEND values in data2 will overwrite existing indices in data1
              and new indices in data2 will append
            - LIST_MODE_APPEND_ALWAYS values in data2 will be appended to values in data1

    Returns:
        dict OrderedDict or list: Output data
    '''
    out_mode = kwargs.pop('out_mode', OUT_MODE_INPLACE)
    dict_mode = kwargs.pop('dict_mode', DICT_MODE_APPEND)
    list_mode = kwargs.pop('list_mode', LIST_MODE_OVERWRITE_APPEND)
    rec_kwargs = {
        'out_mode': out_mode,
        'dict_mode': dict_mode,
        'list_mode': list_mode,
    }

    if out_mode not in OUT_MODES:
        raise RuntimeError('out_mode:%s invalid')
    if dict_mode not in DICT_MODES:
        raise RuntimeError('dict_mode:%s invalid')
    if list_mode not in LIST_MODES:
        raise RuntimeError('list_mode:%s invalid')

    data = OUT_MODE_FUNC.get(out_mode)(data1)

    if isinstance(data2, dict):
        if isinstance(data, dict):
            for key in data2:
                if key in data:
                    data[key] = json_merge(data[key], data2[key], **rec_kwargs)
                elif dict_mode == DICT_MODE_APPEND:
                    data[key] = data2[key]
        else:
            return OUT_MODE_FUNC.get(out_mode)(data2)
    elif isinstance(data2, list):
        if isinstance(data, list):
            for index, value in enumerate(data2):
                if list_mode == LIST_MODE_APPEND_ALWAYS:
                    data.append(value)
                elif len(data) > index:
                    if list_mode == LIST_MODE_OVERWRITE or list_mode == LIST_MODE_OVERWRITE_APPEND:
                        data[index] = json_merge(data[index], value, **rec_kwargs)
                elif list_mode == LIST_MODE_OVERWRITE_APPEND or list_mode == LIST_MODE_APPEND:
                    data.append(value)
        else:
            return OUT_MODE_FUNC.get(out_mode)(data2)
    else:
        return OUT_MODE_FUNC.get(out_mode)(data2)
    return data


def json_substitute_env(data):
    '''
    Substitute string values that contain $ENV or $(ENV) with the value of the environmental
    variable

    .. code-block:: python

        json_substitute_env({'a': '$TMP/trash'})
        {'a': '/tmp/trash' }

    Args:
        data (dict, OrderedDict or list): Input data

    Returns:
        dict OrderedDict or list: Output data
    '''
    if isinstance(data, dict):
        for key in data:
            data[key] = json_substitute_env(data[key])
    elif isinstance(data, list):
        for index, value in enumerate(data):
            data[index] = json_substitute_env(value)
    elif isinstance(data, string_types):
        str_template = Template(data)
        env_data = copy.deepcopy(dict(os.environ))
        env_data = json_merge(env_data, get_config_param())
        return str_template.safe_substitute(**env_data)
    return data
