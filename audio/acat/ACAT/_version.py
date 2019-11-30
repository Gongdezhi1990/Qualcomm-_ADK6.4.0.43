############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2015 - 2018 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
'''
ACAT module version definition
'''

__version__ = '1.8.3'  # pylint: disable=invalid-name

version_info = [  # pylint: disable=invalid-name
    __version__.split('.')[ind]
    if len(__version__.split('.')) > ind else 0 if ind != 3 else ''
    for ind in range(5)
]
