############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
# get_kalsim_version.py
# 
# Print the path to kalsim    

import sys

supported_chips = ["gordon", "crescendo_audio", "stre_audio", "napier_audio"]
supported_platforms = ["win32", "linux2"]

if __name__ == '__main__':

    chipname = sys.argv[1]
    if not (chipname in supported_chips):
        print >> sys.stderr, "Unknown chipname:", chipname
        sys.exit(1)

    if not (sys.platform in supported_platforms):
        print >> sys.stderr, "Unknown platform:", sys.platform
        sys.exit(1)

    if chipname == "gordon":
        if sys.platform == "win32":
            kalsim_path = "r:/home/dspsw_tools/kalsim/kalsim_win32/kalsim_gordon.exe"
        else:
            kalsim_path = "/home/devtools/kalsim/external/2016a/bin/kalsim_csr8670"
    else:
        if sys.platform == "win32":
            kalsim_path = "r:/home/devtools/kalsim/20d/win32/kalsim_"+sys.argv[1]+".exe"
        else:
            kalsim_path = "/home/devtools/kalsim/20d/posix/kalsim_"+sys.argv[1]

    print kalsim_path


