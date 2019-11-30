# Copyright (c) 2016 - 2018 Qualcomm Technologies International, Ltd.

import sys
import argparse
from maker.nvscmd import NvsCmd


def parse_args(args):
    """ parse the command line arguments """
    parser = argparse.ArgumentParser(description='Erase apps from device and reset it')

    parser.add_argument('-t', '--tools_dir',
                        required=True,
                        help='specifies the path to the tools folder \
                              where NvsCmd lives')
    parser.add_argument('-d', '--device_uri',
                        required=True,
                        help='specifies the device URI')

    parsed_args, _ = parser.parse_known_args(args)

    return parsed_args


def nvscmd_erase(args):
    parsed_args = parse_args(args)

    nvscmd = NvsCmd(parsed_args.tools_dir, parsed_args.device_uri)

    return nvscmd.erase_apps(reset_chip=True)


if __name__ == '__main__':
    sys.exit(nvscmd_erase(sys.argv[1:]))
