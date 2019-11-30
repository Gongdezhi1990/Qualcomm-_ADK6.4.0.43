############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
# get_dynmem_version.py
# 
# Print the path to dynmem    

from sys import platform as _platform
import os

if __name__ == '__main__':
    dynmem_base = os.path.join(os.sep,'home','devtools','kdynmem','B225329')
    if 'linux' in _platform:
        # Linux path and executable
        csr_dynmem_path = os.path.join(dynmem_base, 'linux', 'kdynmem')
    elif _platform == 'win32':
        # Windows path and Windows executable
        csr_dynmem_path = os.path.join(os.environ['DEVKIT_ROOT'], 'tools', 'kdynmem.exe')
    else:
        print 'platform',_platform, 'is not allowed'
        sys.exit(1)

    print csr_dynmem_path


