# Copyright (C) 2016 Qualcomm Technologies International, Ltd.
# All Rights Reserved.
# Qualcomm Technologies International, Ltd. Confidential and Proprietary.

import datetime
import getpass
import sys

extra = ''
config = sys.argv[1]
if len(sys.argv) > 2:
    extra = ' (' + sys.argv[2] + ')'

# Note, FATS expects to find "Kymera" somewhere in this string; its case
# doesn't matter
print "UNRELEASED Kymera", config, getpass.getuser(), datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S") + extra
