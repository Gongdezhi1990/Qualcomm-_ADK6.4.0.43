############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
import os, sys, re, subprocess
import argparse

"""
Given a library (.a or .pa file) as input, extract some symbols from
it to files each containing one symbol per line. Currently it can
extract
 - a list of .PUBLIC symbols
 - a list of .CONST symbols
(This is a helper script for the Kymera build system. The idea is that
we need to keep the .PUBLIC symbols in a sanitised '_external' .elf
file, and make sure the .CONST symbols are *not* in that file. See
makerules_src.mkf.)

Use 'python extract_syms_from_lib.py --help' for usage information.
"""

def parse_args(args):
    """ parse the command line arguments """
    parser = argparse.ArgumentParser(description='Extract lists of symbols from a compiled library')

    parser.add_argument('libfile',
                        help='The input library file')

    parser.add_argument('-k', '--kcc_directory', required=True,
                        help='The directory containing kobjdump')

    parser.add_argument('-p', '--public',
                        help='A file to receive .PUBLIC symbols, if desired')

    parser.add_argument('-P', '--public_extra',
                        help='An input file specifying additional symbols to make public (added to --public output)')

    parser.add_argument('-c', '--const',
                        help='A file to receive .CONST symbols, if desired')

    return parser.parse_args(args)

def extract_symbols_from_lib_file(libfile, publicout, publicin, constout, kcc_path):
    OBJDUMP = os.path.join(kcc_path, 'bin', 'kobjdump')

    # Match lines containing .PUBLIC symbols.
    # These are distinguished by having bit 0x80 (STO_KALIMBA_PUBLIC, a
    # CSR/Qualcomm invention) set in the ELF st_other field.
    # (k)objdump prints a representation of this field if nonzero (in
    # _bfd_elf_print_symbol()). It doesn't currently understand
    # STO_KALIMBA_PUBLIC, so prints its numeric value. (There doesn't 
    # seem to be any other tool, e.g. (k)nm, which will show us this
    # information.)
    # It's possible that some future version of kobjdump will print it
    # symbolically (as '.public' or similar), in which case this regex
    # will break and need updating.
    #
    # Example of an input line we're trying to match:
    # 00000000 g     F CVC_INIT_PM?$M.cvclib.table_alloc__maxim    00000000 0x80 $_cvclib_table_alloc
    #          ^ (global symbol)                    (hard tab)--^ .PUBLIC --^^^^ (   symbol name    )
    publicre = re.compile(r"[0-9A-Fa-f]{8} g...... \S+\s+[0-9A-Fa-f]{8}\s+0x([0-9A-Fa-f]{2})\s+(.*)")

    # Match lines containing .CONST symbols.
    # These are anything defined in the *ABS* section.
    # (They could probably be extracted from (k)nm output, but we're
    # already looking at kobjdump output so we may as well get it from
    # here.)
    # (FIXME: there is no mechanism here to deal with symbols that are
    # both .PUBLIC and .CONST. It's not clear that such a thing can
    # exist, and there doesn't seem to be any demand.)
    #
    # Example of an input line we're trying to match:
    # 00000033 g       *ABS*  00000000 $dmss.STRUC_SIZE
    #  (global)^             ^-- (hard tab)
    constre = re.compile(r"[0-9A-Fa-f]{8} g...... \*ABS\*\s+[0-9A-Fa-f]{8}\s+(.*)")

    # Store as maps for de-duplication
    publicsyms = {}
    constsyms = {}

    # '(k)objdump -t' lists symbols in an object or library
    objdump = subprocess.Popen([OBJDUMP, '-t', libfile], stdout=subprocess.PIPE)

    for fullline in objdump.stdout:
        line = fullline.rstrip()
        # Look for .PUBLIC symbols
        p = publicre.match(line)
        if p and (int(p.group(1), base=16) & 0x80):
            publicsyms[p.group(2)] = 1   # value doesn't matter
        # Look for .CONST symbols
        c = constre.match(line)
        if c:
            constsyms[c.group(1)] = 1    # value doesn't matter

    # Make additional symbols public. This is designed to take a
    # .symbols file of the kind the AudioCPU/Kymera build systems
    # pass to 'kalscramble -i'.
    if publicin is not None:
        with open(publicin, 'r') as fh:
            for line in fh:
                publicsyms[line.rstrip()] = 1      # value doesn't matter

    if publicout is not None:
        with open(publicout, 'w') as fh:
            # TODO: kobjdump --keep-symbols= doesn't seem to like an
            # entirely empty file, but is content with a single blank line.
            if len(publicsyms) > 0:
                fh.writelines(map(lambda s: s + '\n', sorted(publicsyms)))
            else:
                fh.write('\n')

    if constout is not None:
        with open(constout, 'w') as fh:
            if len(constsyms) > 0:
                fh.writelines(map(lambda s: s + '\n', sorted(constsyms)))
            else:
                fh.write('\n')

def main(args):
    parsed_args = parse_args(args)

    libfile, publicout, publicin, constout, kcc_path = \
            map(lambda f: None if f is None else os.path.normpath(f),
                    (parsed_args.libfile,
                     parsed_args.public,
                     parsed_args.public_extra,
                     parsed_args.const,
                     parsed_args.kcc_directory))
    extract_symbols_from_lib_file(libfile, publicout, publicin, constout, kcc_path)

if __name__ == '__main__':
    main(sys.argv[1:])
