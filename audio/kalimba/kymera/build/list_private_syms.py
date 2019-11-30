############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
import os, sys
import argparse

"""
Given a set of files each listing a number of symbols from a public
source, and another set listing symbols from a private source (each input
file has one symbol per line), output a list of symbols considered 'private',
following the rule that if a symbol appears in a public list, it's public.

(This is a helper script for the Kymera build system. We use it to decide
which .CONST symbols to censor, since the toolchain doesn't track the
provenance of these well enough to do it itself.)

Use 'python list_private_syms.py --help' for usage information.
"""

def parse_args(args):
    """ parse the command line arguments """
    parser = argparse.ArgumentParser(description='Resolve public and private symbols')

    parser.add_argument('-p', '--public', nargs='*',
                        help='Any number of files listing public symbols (any files also in \'--private\' are ignored)')

    parser.add_argument('-s', '--private', nargs='*',
                        help='Any number of files listing private (\'secret\') symbols')

    return parser.parse_args(args)

def main(args):
    parsed_args = parse_args(args)

    private_syms = {}

    # Gather a list of potentially-private symbols, discarding duplicates
    for f in parsed_args.private:
        with open(f, 'r') as fh:
            for line in fh:
                private_syms[line.rstrip()] = 1

    # Remove any that are also in public libraries
    for f in parsed_args.public:
        # TODO: if the filename is also on the 'private' list, skip it
        # (this is just because I was too lazy to prepare a list of
        # just-the-public-libraries in the makefile).
        if f in parsed_args.private:
            continue
        with open(f, 'r') as fh:
            for line in fh:
                sym = line.rstrip()
                if sym in private_syms:
                    del private_syms[sym]

    # Output the net result
    sys.stdout.writelines(map(lambda s: s + '\n', sorted(private_syms)))

if __name__ == '__main__':
    main(sys.argv[1:])
