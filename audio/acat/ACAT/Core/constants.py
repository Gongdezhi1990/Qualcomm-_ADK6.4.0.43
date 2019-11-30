############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2018 Qualcomm Technologies, Inc. and/or its subsidiaries.
# All rights reserved.
#
############################################################################
"""
Constant values in ACAT
"""
import sys
from collections import OrderedDict


COREDUMP_EXTENSION = '.xcd'
IS_PY2 = sys.version_info.major == 2


# The map between an analysis name and its class name in the Analysis
# package. Add new analysis modules here. The order is important. It will
# be used as a running order in automatic mode.
ANALYSES = OrderedDict((
    # Internal streams (graph) and state debug
    ('stream', 'Stream'),
    ('opmgr', 'Opmgr'),
    ('sanitycheck', 'SanityCheck'),
    ('debuglog', 'DebugLog'),
    ('stackinfo', 'StackInfo'),
    ('profiler', 'Profiler'),
    # memory related
    ('poolinfo', 'PoolInfo'),
    ('heapmem', 'HeapMem'),
    ('sharedmem', 'SharedMem'),
    ('scratchmem', 'ScratchMem'),
    ('heappmmem', 'HeapPmMem'),
    # Specific Kymera modules
    ('sched', 'Sched'),
    ('adaptor', 'Adaptor'),
    ('audio', 'Audio'),
    ('fault', 'Fault'),
    ('patches', 'Patches'),
    ('dorm', 'Dorm'),
    ('sco', 'Sco'),
    ('ipc', 'IPC'),
    # helper analyses:
    ('cbops', 'Cbops'),
    ('buffers', 'Buffers'),
    ('fats', 'Fats')
))

