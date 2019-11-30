############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2012 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
# get_kcc_version.py
#
# Print the path to Kalimba tools
# This is different for Linux and Windows environments

import os
import re
import sys
import subprocess

class kcc_version:
    kcc_release = 63

    def get_kcc_path(self, ostype):

        if "linux" in ostype:
            # Internal KCC releases live here
            path = "/home/devtools/kcc/kcc-" + str(self.kcc_release) + "-linux"
        elif "Windows" in ostype:
            path = os.path.join(os.environ["DEVKIT_ROOT"], 'tools','kcc')
        elif "win32" in ostype:
            path = os.path.join(os.environ["DEVKIT_ROOT"], 'tools','kcc')
        else:
            sys.stderr.write("Error, Invalid OSTYPE: " + ostype + "\n");
            sys.exit(1)
        return path

    def verify_kcc(self, ostype):
        cmd = self.get_kcc_path(ostype)
        # Check KCC version matches the one we expect
        cmd = os.path.join(cmd, "bin", "kcc") + " -v"
        result = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, )
        result.wait()
        result = result.stderr.readlines()[0]
        # Temporarily disable the version check until KCC56 devkit is available - JN 2017-08-29
        # if result.find("Kalimba C compiler driver, version {}".format(self.kcc_release))==-1:
        #    return "Error, invalid KCC version. Expected: {expected}, found: {found}\n".format(expected=self.kcc_release,found=result);
        return True

if __name__ == '__main__':
    if len(sys.argv) < 2:
        sys.stderr.write(sys.argv[0] + ": Error, OSTYPE was not provided.\n");
        sys.exit(1)
    ostype = sys.argv[1]
    version = kcc_version()
    print version.get_kcc_path(ostype)
