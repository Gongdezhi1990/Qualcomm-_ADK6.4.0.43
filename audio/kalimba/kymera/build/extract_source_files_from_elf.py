############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
import os, re, subprocess
import argparse
import glob
import os
import re
import subprocess as SP
import sys

"""
Usually this script is invoked with the following line
/path/to/kalimba/kymera/build/extract_source_files_from_elf.py -d /path/to/kalimba/kymera/output/test_intctrlr/crescendo/test_output/debugbin

usage: extract_source_files_from_elf.py [-h] -e ELF_DIRECTORY -k KTOOLS_PATH [--verbose]

Creates a file list with the source files from a folder of elf files

optional arguments:
  -h, --help            show this help message and exit
  -e ELF_DIRECTORY, --elf_directory ELF_DIRECTORY
                        The directory containing the .elf file.
  -k KTOOLS_DIRECTORY, --kcc-directory KTOOLS_DIRECTORY
                        Path to kcc and other kalimba tools
  --verbose             Provide more verbose output
"""

def parse_args(args):
    """ parse the command line arguments """
    parser = argparse.ArgumentParser(description='Creates a file list with the source files from a folder of elf files')

    parser.add_argument('-e', '--elf_directory',
            required=True,
                        help='The directory containing the .elf file.')

    parser.add_argument('-k', '--kcc_directory',
            required=True,
                        help='The directory containing the kobjdump and kaddr2line')

    parser.add_argument('--verbose',
                        action="store_true",
                        help='Provide more verbose output')

    return parser.parse_args(args)

def build_src_list_from_elf_files(elf_files, output_file, kcc_path):
    """
    Output a sorted, normalised list of source files excluding any from devtools
    """
    filename_map = {}
    DWARFDUMP = os.path.join(kcc_path,'bin','dwarfdump')

    # Use a regular expression to match the lines we are interested in
    # file names ending with .c
    uri = re.compile(r".*uri: \"(.*\.(c|asm))\"")
    # skip devtools source files, and other compiler file
    exclude = re.compile(r"([\\/]tmp[\\/])|(.*[\\/]devtools)")

    for elf_file in elf_files:
        if not os.path.isfile(elf_file):
            sys.stderr.write("Error, elf file %s does not exist" % elf_file)
            sys.exit(1)

        # Dump the line number section from the elf file
        line_listing = SP.Popen([DWARFDUMP, '-l', elf_file], stdout=SP.PIPE).communicate()[0]

        # select addresses from the listing
        for line in line_listing.splitlines():
            m = uri.match(line)
            if m:
              source_file = os.path.normpath(m.groups()[0]) + '\n'
              if exclude.match(source_file):
                  continue
              filename_map[source_file] = 1


    # Write the sorted source file list to disc
    with open(output_file, 'w') as fh:
        fh.writelines(sorted(filename_map))

def main(args):
    """ main entry point.
        - Processes cmd line args.
        - Finds and processes the elf file
        - Writes the output to disc
    """

    # parse the command line arguments
    parsed_args = parse_args(args)

    temp_dir = os.path.normpath(parsed_args.elf_directory)
    kcc_path = os.path.normpath(parsed_args.kcc_directory)
    GDB_SRC = os.path.join(temp_dir, 'gdb.src')
    GDB_SOURCE_LIST = os.path.join(temp_dir, '.gdb_info_sources.txt')

    # Get the first and hopefully only elf file in temp_dir
    elf_files = glob.glob(os.path.join(temp_dir, '*.elf'))

    if not os.path.isdir(temp_dir):
        sys.stderr.write("Error, %s is not a directory" % temp_dir)
        sys.exit(1)

    if parsed_args.verbose:
        print "ARGV %s" % sys.argv
        print "TMP  %s" % temp_dir
        print "GDB_SRC %s" % GDB_SRC
        for elf_file in elf_files:
            print "ELF  %s" % elf_file

    build_src_list_from_elf_files(elf_files, GDB_SOURCE_LIST, kcc_path)


if __name__ == '__main__':
    main(sys.argv[1:])
