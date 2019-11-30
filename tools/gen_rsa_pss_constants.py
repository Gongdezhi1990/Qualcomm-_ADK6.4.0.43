#!/usr/bin/env python
# Copyright (c) 2017 Qualcomm Technologies International, Ltd.
#   
"""
Generates an RSA and PSS constants C source code file from a pemtodfukey DFU
public key file. The pemtodfukey DFU public key file is generated using:

SecurityCmd.exe -product hyd pem2dfukey pub public.pem dfu-public.key

where public.pem is the OpenSSL RSA-2048 public key file that has previously
been generated, along with the private.pem OpenSSL RSA-2048 private key file
using:

SecurityCmd.exe -product hyd creatersakey 2048 F4 private.pem public.pem

The parameters are:
    gen_rsa_pss_constants.py <input> <output>

    -i --input  Specifies path and filename of the DFU public key input file
    -o --output Optionally specifies output file RSA and PSS constants C source
                    code file

If the --output specified a full path and filename, that is the output
file that will generated.

If the --output only specifies a path, a file of the name rsa_pss_constants.c
will be generated at that path.

If the --output only specified a filename, a file of that name will be generated
on the path specified by the --input parameter.

If the --output parameter is omitted, a file of the name rsa_pss_constants.c
will be generated on the path specified by the --input parameter.

Paths supplied may be absolute or relative to the current working directory.
"""

# Python 2 and 3
from __future__ import print_function

import sys
import argparse
import os
from datetime import datetime

def parse_args(args):
    """ parse the command line arguments """
    parser = argparse.ArgumentParser(description =
        'Generate an RSA and PSS constants C source code file from a \
        pemtodfukey DFU public key file')

    parser.add_argument('-i', '--input',
                        required=True,
                        help='specifies the [path and] filename of the \
                        pemtodfukey DFU public key file')
    parser.add_argument('-o', '--output',
                        default=None,
                        help='optionally specifies the path and/or filename of \
                        the RSA and PSS constants C source code file')
    return parser.parse_args(args)


def main(args):

    parsed_args = parse_args(args)

    input_dirname, input_filename = os.path.split(
        os.path.normpath(parsed_args.input))
    if input_dirname is '':
        # No path given so use $CWD
        input_dirname = os.getcwd()
    if not os.path.isdir(input_dirname):
        # The input path given is not a path so notify an error
        print ("Error: Input {} - not a directory\n".format(input_dirname))
        return False
    if not os.path.isfile(os.path.join(input_dirname, input_filename)):
        # The input file given is not a file so notify an error
        print ("Error: Input {} - not a file\n".format(os.path.join(
            input_dirname, input_filename)))
        return False

    if parsed_args.output is None:
        # No output specified so use the input path as the output path
        # and use the default output filename
        output_dirname = input_dirname
        output_filename = "rsa_pss_constants.c"
    else:
        output_dirname, output_filename = os.path.split(
            os.path.normpath(parsed_args.output))
        if output_dirname is '':
            # No output path so use the input path
            output_dirname = input_dirname
        if output_filename is '':
            # No output file so use the default
            output_filename = "rsa_pss_constants.c"
        if os.path.isdir(os.path.join(output_dirname, output_filename)):
            # Split has been on a path with no filename, so re-join the path
            # and use the default filename
            output_dirname = os.path.join(output_dirname, output_filename)
            output_filename = "rsa_pss_constants.c"

    if not os.path.isdir(output_dirname):
        # The output path given is not a path so notify an error
        print ("Output {} - not a directory\n".format(output_dirname))
        return False

    input_dirfile = os.path.abspath(os.path.join(input_dirname, input_filename))
    output_dirfile = os.path.abspath(os.path.join(output_dirname, 
                                                    output_filename))
    print ("Input: {}".format(input_dirfile))

    InputLine = []
    try:
        with open(input_dirfile, 'r') as f:
            for line in f:
                InputLine.append(line)
    except IOError as exception:
        print ("Error {} '{}' on input file {}\n".format(exception.errno, 
                                        exception.strerror, input_dirfile))
        return False

    if len(InputLine) < 4:
        print("Error: Too few lines ({}) read from {}\n").format(len(InputLine,
                                                                input_dirfile))
        return False

    # Each line should begin with '@ ' so remove those
    for i in range(4):
        if InputLine[i].startswith('@ '):
            InputLine[i] = InputLine[i][2:]
        else:
            print("Error: Line {} of {} did not start with the expected '@ '\n"
                .format(i + 1, input_dirfile))
            return False

    Modulus = InputLine[1].split()
    if len(Modulus) is not 128:
        print("Error: Modulus in line 1 does not contains 128 items\n")
        return False
    # The items in the Modulus list are in the reverse order to that required
    Modulus.reverse()
    M_dash = InputLine[2].split()
    if len(M_dash) is not 1:
        print("Error: M_dash in line 2 does not contain only one item\n")
        return False
    R2NmodM = InputLine[3].split()
    if len(R2NmodM) is not 128:
        print("Error: R2NmodM in line 3 does not contains 128 items\n")
        return False
    # The items in the Modulus list are in the reverse order to that required
    R2NmodM.reverse()

    part1 = [
        'FILE NAME\n',
        '    rsa_pss_constants.c\n\n',
        'DESCRIPTION\n'
        '    Constant data needed for the rsa_decrypt and the ce_pkcs1_pss_padding_verify\n',
        '    function.\n\n',
        'NOTES\n',
        '    The constant data for the rsa_decrypt is generated by the host from the\n',
        '    RSA public key and compiled into the rsa_pss_constants library.\n',
        '    It is directly related to the RSA private and public key pair, and if they\n',
        '    change then this file must be regenerated from the public key.\n\n',
        '****************************************************************************/\n\n'
        '#include "rsa_decrypt.h"\n\n',
        '/*\n',
        ' * The const rsa_mod_t *mod parameter for the rsa_decrypt function.\n',
        ' */\n',
        'const rsa_mod_t rsa_decrypt_constant_mod = {\n',
        '    /* uint16 M[RSA_SIGNATURE_SIZE] where RSA_SIGNATURE_SIZE is 128 */\n',
        '    {\n'
    ]

    part2 = [
        '    },\n',
        '    /* uint16 M_dash */\n'
    ]

    part3 = [
        '};\n\n',
        '/* \n',
        ' * The "lump of memory passed in initialised to R^2N mod M" needed for the\n',
        ' * uint16 *A parameter for the rsa_decrypt function. This must be copied into\n',
        ' * a writable array of RSA_SIGNATURE_SIZE uint16 from here before use.\n',
        ' */\n',
        'const  uint16 rsa_decrypt_constant_sign_r2n[RSA_SIGNATURE_SIZE] = {\n'
    ]

    part4 = [
        '};\n\n',
        '/*\n',
        ' * The value to be provided for the saltlen parameter to the \n',
        ' * ce_pkcs1_pss_padding_verify function used to decode the PSS padded SHA-256\n',
        ' * hash. It has to match the value that was used in the encoding process.\n',
        ' */\n',
        'const unsigned long ce_pkcs1_pss_padding_verify_constant_saltlen = 222;\n'
    ]
    
    lines = []

    line = '/****************************************************************************\n'
    lines.append(line)
    line = 'Generated from {}\n'.format(input_dirfile)
    lines.append(line)
    line = 'by {}\n'.format(os.path.abspath(os.path.join(os.getcwd(),
                                                            sys.argv[0])))
    lines.append(line)
    line = 'at {}\n\n'.format(datetime.strftime(datetime.now(), 
                                                    '%H:%M:%S %d/%m/%Y'))
    lines.append(line)

    for line in part1:
        lines.append(line)

    for i in range(len(Modulus) - 1):
        value = int(Modulus[i], 16)
        line = '        0x{:04x},     /* {:02d} */\n'.format(value, i)
        lines.append(line)

    value = int(Modulus[-1], 16)
    line = '        0x{:04x}      /* {:02} */\n'.format(value, len(Modulus) - 1)
    lines.append(line)

    for line in part2:
        lines.append(line)

    line = '    0x{}\n'.format(M_dash[0])
    lines.append(line)

    for line in part3:
        lines.append(line)

    for i in range(len(R2NmodM) - 1):
        value = int(R2NmodM[i], 16)
        line = '    0x{:04x},     /* {:02} */\n'.format(value, i)
        lines.append(line)

    value = int(R2NmodM[-1], 16)
    line = '    0x{:04x}      /* {:02} */\n'.format(value, len(R2NmodM) - 1)
    lines.append(line)

    for line in part4:
        lines.append(line)

    print ("Output: {}".format(output_dirfile))

    try:
        with open(output_dirfile, 'w') as f:
            for line in lines:
                f.write(line)        
    except IOError as exception:
        print ("Error {} '{}' on output file {}\n".format(exception.errno, 
                                        exception.strerror, output_dirfile))
        return False

    return True

if __name__ == '__main__':
    if not main(sys.argv[1:]):
        sys.exit(1)
    sys.exit(0)

