############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2012 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
""" A quick python script that the build system uses to check if a file exists
before it calls a script that wants to use that file. Returns 1 exists, 
0 doesn't exist."""

import os
import sys

if os.path.isfile(sys.argv[1]):
    sys.exit(1)

sys.exit(0)
