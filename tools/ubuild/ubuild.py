"""
Copyright (c) 2016 - 2018 Qualcomm Technologies International, Ltd

Provides the command line interface for the build utility
    params are:
    build.py <project file> <devkit root> [<config | all>]

    -p --project_file (source, files, configurations (per devkit))

    -d --devkit_root (devkit to use for this build path to folder
                      containing a sdk.xml)
    -c --configuration (all, debug, release, ota, etc.
                        use 'all' if no config specified)
"""
import sys
import argparse
import os

import maker.parse_proj_file as pproj
import maker.parse_devkit as pdk
import maker.build as bld
import maker.exceptions as bdex
import maker.sim_launch as sim
import maker.mergexml as mxml
import maker.util as util

def check_no_spaces(value):
    if ' ' in value:
        raise argparse.ArgumentTypeError("Invalid path. Space characters in file/folder paths are not supported: '{}'".format(value))
    return value

def parse_args(args):
    """ parse the command line arguments """
    parser = argparse.ArgumentParser(description='Build a project using the \
                                                   supplied devkit')

    parser.add_argument('-w', '--workspace_file',
                        required=True, type=check_no_spaces,
                        help='Specifies the workspace file')

    parser.add_argument('-p', '--project_file',
                        required=True, type=check_no_spaces,
                        help='Specifies the project file')

    parser.add_argument('-k', '--devkit_root',
                        required=True, type=check_no_spaces,
                        help='Specifies path to the root folder of the devkit to use \
                              for this build. Which must contain a sdk.xml \
                              describing the devkit contents')

    parser.add_argument('-c', '--configuration',
                        default='all',
                        help='Specifies name of the configuration to build. Defaults to \
                              "all".')

    parser.add_argument('-b', '--build_style',
                        default='rebuild',
                        help='Specifies either "build", "rebuild" or "clean".')

    parser.add_argument('-x', '--xml_output',
                        action="store_true",
                        default=False,
                        help='Wrap output in XML tags.')

    parser.add_argument('-t', '--trace_enabled',
                        action="store_true",
                        default=False,
                        help='Enable debug trace')

    parser.add_argument('--verbose',
                        action="store_true",
                        help=' Provide more verbose output')

    parser.add_argument('-d', '--deploy_target',
                        default=None,
                        help=' Deploy built binary to target device')

    parser.add_argument('-o', '--build_output_folder',
                        default=None, type=check_no_spaces,
                        help=' Specify location for object files and other build artefacts.')

    parser.add_argument('-a', '--appsFs',
                        action="store_true",
                        help=' Build a filesystem for apps using appropriate endian format')

    parser.add_argument('-n', '--nvscmd',
                        action="store_true",
                        help=' Use NvsCmd tool for deploying')

    parser.add_argument('-s', '--special',
                        default=None,
                        help=' Special purpose argument. The value has meaning to \
                        the build and deploy routines in a given Devkit and will \
                        vary across Devkits')

    return parser.parse_args(args)

def main(args):
    """ main entry point.
        - Processes cmd line args.
        - Finds and parses devkit sdk.xml and project file.
        - Calls builder with the target configuration. """

    parsed_args = parse_args(args)
    bdex.Logger(parsed_args)
    sys.path.append(os.path.join(parsed_args.devkit_root, r'tools\pythontools'))
    sys.path.append(os.path.join(parsed_args.devkit_root, r'tools\trbtrans'))
    import maker.deploy as dep

    try:
        os.environ['PATH'] = os.environ['SystemRoot'] + ';' + os.environ['SystemRoot'] + '\system32'

        dk_xml_file = util.get_sdk_xml_file(parsed_args.devkit_root)

        proj_parser = pproj.Project(parsed_args.project_file, parsed_args.devkit_root, parsed_args.workspace_file)
        dk_parser = pdk.Devkit(dk_xml_file)

        if parsed_args.deploy_target:
            return dep.deploy(proj_parser, dk_parser, parsed_args)
        else:
            if parsed_args.verbose:
                bld.print_info(proj_parser, dk_parser, parsed_args)

            return bld.build_configs(proj_parser, dk_parser, parsed_args)

    except bdex.BDError as excep:
        bdex.err_handler(excep)
        return False
    except BaseException:
        bdex.err_handler(None)
        return False
    else:
        bdex.trace("+++ EXIT BUILD %s ++++" % os.path.abspath(sys.argv[0]))
        return True

if __name__ == '__main__':
    if not main(sys.argv[1:]):
        sys.exit(1)
    sys.exit(0)
