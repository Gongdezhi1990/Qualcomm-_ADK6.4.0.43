#!/usr/bin/env python
############################################################################
# CONFIDENTIAL
#
# Copyright 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
import os
import re
import sys

########################### Main script ###############################################
if __name__ == "__main__":
    import argparse 
    usage = """This script parses a string of definitions of the form -DFOO -DBAR and
writes them into a file of the format
#define FOO
#define BAR    """
    parser = argparse.ArgumentParser(prefix_chars='+', add_help=False)

    parser.add_argument("+o", "++output", help="The target build directory")
    parser.add_argument("+i", "++input", nargs='*', help="String of definitions to process")

    args = parser.parse_args()

    if args.input == ""  or  args.output == "":
        raise Exception("Wrong number of arguments")

    destFile=open(args.output,"w")
    for elem in args.input:
        if elem[0:2] == "-D" :
            destFile.write("#define " + elem[2:].replace("="," ")+"\n")
        elif elem[0:2] == "-U" :
            destFile.write("#undef " + elem[2:]+"\n")
        else:
            raise Exception(elem + " does not start with -D or -U")
    destFile.flush()
    os.fsync(destFile.fileno())
    destFile.close()