############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
# winpath.py
#
# Convert a list of filenames to local format
#
# Typically needed when passing make targets etc to Windows tools
# The Kymera build system invokes this via the "ospath" function
#

import os
import re
import sys

if __name__ == '__main__':
    params = sys.argv[1:]
    output = ""
    for path_string in params:
        output += os.path.normpath(path_string) + " "
    print output.rstrip()
