#!/usr/bin/env python2
# Copyright (c) 2018 Qualcomm Technologies International, Ltd.
#   
'''
Installs an ADK header file from the hydra_os source code.

Usage:
  python install_adk_header.py <source> <target>
'''
import argparse
import os

def main():
    '''Main entry point when this script is executed rather than imported.'''

    parser = argparse.ArgumentParser()
    parser.add_argument("source", help="path to the source file.")
    parser.add_argument("target", help="path to the target file.")
    args = parser.parse_args()

    print "Installing " + args.source + " to " + args.target

    # Create the install directory tree if it doesn't already exist.
    try:
        os.makedirs(os.path.dirname(args.target))
    except OSError as error:
        if error.errno != os.errno.EEXIST:
            raise

    # Copy the source to the target, replacing the necessary strings.
    # Avoid creating a temporary file in case another process opens it
    # (e.g. a virus scanner) and we are unable to delete it.
    with open(args.source) as in_file, open(args.target, "w") as out_file:
        for line in in_file:
            for orig, new in [("hydra/hydra_types.h", "hydra_types.h"),
                              ("hydra/hydra_macros.h", "hydra_macros.h"),
                              ("pmalloc/pmalloc_trace.h", "pmalloc_trace.h"),
                              ("hydra_log/hydra_log.h", "hydra_log.h"),
                              ("hydra_log/hydra_log_firm.h", "hydra_log_firm.h"),
                              ("hydra_log/hydra_log_soft.h", "hydra_log_soft.h"),
                              ("hydra_log/hydra_log_disabled.h", "hydra_log_disabled.h"),
                              ("hydra_log/hydra_log_subsystems.h", "hydra_log_subsystems.h"),
                              ('"bluestack/types.h"', '"types.h"'),
                              ("BITFIELD(", "NOT_A_BITFIELD(")]:
                line = line.replace(orig, new)
            out_file.write(line)

if __name__ == "__main__":
    main()
