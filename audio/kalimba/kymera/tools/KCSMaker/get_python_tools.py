############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
# get_python_tools.py
#
# Print the path to python tools
# This is different for Linux and Windows environments

import os
import sys

class python_tools_version:
    python_ver = "1.1.6"

    def get_python_tools_path(self, ostype):
        """ Method to return the python tools path depending on the OS. """
        if "linux" in ostype:
            path = os.path.normcase("/home/devtools/kal-python-tools/linux/{ver}/kal_python_tools_linux64_{ver}".format(ver=self.python_ver))
        elif "Windows" in ostype:
            path = os.path.join(os.environ["DEVKIT_ROOT"], 'tools', 'pythontools')
        else:
            sys.stderr.write("Error, Invalid OSTYPE: " + ostype + "\n")
            sys.exit(1)
        return path

if __name__ == '__main__':
    version = python_tools_version()
    print(version.get_python_tools_path("linux"))
