#!/usr/bin/env python
# Copyright (c) 2016 Qualcomm Technologies International, Ltd.
#   
"""
Provides an interface between Heracles and UniversalFrontEnd.exe
    The parameters are:
    run_obpm.py <devkit root>

    -k --devkit_root    Specifies path to the root folder of the devkit to use.

The normal location for the installation of UniversalFrontEnd.exe is
..\OBPM\tools\UFE\UniversalFrontEnd.exe compared to the devkit_root

If the file is found there it is run, otherwise this script looks under
devkit_root for UniversalFrontEnd.exe and, if found, runs that.
"""
import sys
import argparse
import subprocess
import os

def parse_args(args):
    """ parse the command line arguments """
    parser = argparse.ArgumentParser(description =
        'Run the Operator Based Parameter Manager')

    parser.add_argument('-k', '--devkit_root',
                        required=True,
                        help='specifies the path to the root folder of the \
                              devkit to use')
    return parser.parse_args(args)

def find_first_file(root, filename):
    print "Searching below {} for {}\n".format(root, filename)
    filepath = None
    if os.path.isdir(root):
        for filepath, subdirs, files in os.walk(root):
            for file in files:
                if file.lower() == filename.lower():
                    filepath = os.path.join(filepath, file)
                    print "Found {}\n".format(filepath)
                    return filepath
    print "{} not found below {}\n".format(filename, root)
    return None

def main(args):
    """ main entry point.
        - Processes command line arguments;
        - Determines that coredump.exe utility is where it expects to find it;
        - Creates a folder to put the xcd and other files in;
        - Calls coredump.exe for trb 0 to generate the xcd and log files;
        - Calls a function to zip together the xcd and log files, and any elf
            and lst files that are to be found below the devkit root directory.
    """
    parsed_args = parse_args(args)
    ufe_exe = os.path.abspath(os.path.join(parsed_args.devkit_root, '..',
        'OBPM', 'tools', 'UFE', 'UniversalFrontEnd.exe'))

    # Check that it is where it is supposed to be.
    if not os.path.isfile(ufe_exe):
        print "File {} not found\n".format(ufe_exe)
        ufe_exe = find_first_file(os.path.abspath(parsed_args.devkit_root),
            'UniversalFrontEnd.exe')
        if ufe_exe == None:
            ufe_exe = find_first_file(os.path.abspath(os.path.join(
                parsed_args.devkit_root, '..')), 'UniversalFrontEnd.exe')
            if ufe_exe == None:
                print "Unable to find UniversalFrontEnd.exe; is OBPM installed?"
                sys.stdout.flush()
                return False

    sys.stdout.flush()
    ufe_exe = os.path.abspath(ufe_exe)
    if os.path.isfile(ufe_exe):
        try:
            print "Executing {}\n".format(ufe_exe)
            sys.stdout.flush()
            subprocess.Popen([ufe_exe])
            return True
        except OSError:
            print "Failed to execute {}\n".format(ufe_exe)
            sys.stdout.flush()

    return False

if __name__ == '__main__':
    if not main(sys.argv[1:]):
        sys.exit(1)
