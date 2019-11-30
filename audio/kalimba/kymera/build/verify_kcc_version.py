############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
# verify_kcc_version.py
#
# Verify the kcc version is the one we expect, by running kcc -v

import sys
import get_kcc_version

if __name__ == '__main__':
    if len(sys.argv) < 2:
        sys.stderr.write(sys.argv[0] + ": Error, OSTYPE was not provided.\n");
        sys.exit(1)
    ostype = sys.argv[1]
    print (get_kcc_version.kcc_version()).verify_kcc(ostype)