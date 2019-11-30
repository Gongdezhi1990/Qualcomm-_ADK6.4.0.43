#!/usr/bin/env python
# Copyright (c) 2018 Qualcomm Technologies International, Ltd.

import os
import sys
import argparse
from workspace_parse.workspace import Workspace
from workspace_builders.ubuild_builder import UbuildBuilder


def main(args):
    known_args, ubuild_args = parse_args(args)
    workspace_file = os.path.normpath(known_args.workspace_file)
    workspace = Workspace(workspace_file, UbuildBuilder(ubuild_args))
    return workspace.build()


def parse_args(args):
    """ Parse the command line arguments relevant to this script only
        the rest of the arguments will be passed through untouched to ubuild
    """
    parser = argparse.ArgumentParser(description='Build a workspace')

    parser.add_argument('-w', '--workspace_file',
                        required=True,
                        help='Specifies the workspace file')

    parser.add_argument('-k ', '--devkit_root',
                        default=os.environ.get('ADK_ROOT'),
                        help='Specifies path to the root folder of the devkit to use')

    known_args, passthru_args = parser.parse_known_args(args)
    ubuild_args = []

    for k, v in vars(known_args).iteritems():
        ubuild_args += ['--' + k, v]
    ubuild_args += passthru_args

    return known_args, ubuild_args


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
