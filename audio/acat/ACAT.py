############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2012 - 2019 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
"""
ACAT's main script will import ACAT as a package and run it.
"""
import sys

import ACAT


def main():
    """
    Parse the argument and execute the ACAT
    """
    ACAT.main()


if __name__ == '__main__':
    sys.exit(main())
