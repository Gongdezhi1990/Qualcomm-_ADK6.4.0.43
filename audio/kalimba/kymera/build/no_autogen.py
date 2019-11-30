############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
# winpath.py
#
# Given a list of directories, checks for a makefile in the directory and
# determines whether it contains autogeneration rules.
#
# Typically needed when passing make targets etc to Windows tools
# The Kymera build system invokes this via the "ospath" function
#

import os
import re
import sys

autogen_line = re.compile(r"^\s*(GEN|PATCH)_.*=")

if __name__ == '__main__':
    params = sys.argv[1:]
    output = ""
    for path in params:
        makefile = os.path.join(path,"makefile")
        if not os.path.exists(makefile):
            print >>sys.stderr, "Could not find a makefile at : %s"%makefile
        else:
            mf = open(makefile)
            for line in mf:
                if autogen_line.match(line):
                    break
            else:
                output += path + " "
            mf.close()
    print output.rstrip()
    
