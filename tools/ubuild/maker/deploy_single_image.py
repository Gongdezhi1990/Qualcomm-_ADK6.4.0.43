#!/usr/bin/env python
# Copyright (c) 2017 - 2018 Qualcomm Technologies International, Ltd.
"""
    Build and deploy a flash image file from the current workspace.
"""
# Python 2 and 3
from __future__ import print_function

import sys
import argparse
import os
import time
from xml.etree.cElementTree import parse as xml_parse
import tempfile

import shutil
from .prepare_single_image import PrepareSingleImage
from workspace_parse.workspace import Workspace
from workspace_builders.base_builder import BaseBuilder
import maker.subsystem_numbers as subsys_nos
from maker.nvscmd import NvsCmd
from maker.pydbg_flash_image import Pyflash
from .device_checker import check_device_with_timeout

def parse_args(args):
    """ parse the command line arguments """
    parser = argparse.ArgumentParser(description = 'Build a flash image file')

    parser.add_argument('-k', '--devkit_root',
                        required=True,
                        help='specifies the path to the root folder of the \
                              devkit to use')
    parser.add_argument('-w', '--workspace',
                        required=True,
                        help='specifies the workspace file to use')
    parser.add_argument('-a', '--active_project',
                        action="store_true",
                        required=False,
                        help='specifies the output directory to use')
    parser.add_argument('-t', '--tools_dir',
                        required=True,
                        help='specifies the location of NvsCmd tool directory')
    parser.add_argument('-d', '--device_uri',
                        required=True,
                        help='specifies the transport URI')
    parser.add_argument('-o', '--build_output_folder',
                        default=None,
                        help='Specify location for object files and other build artefacts.')

    return parser.parse_args(args)


def collect_all_workspace_projects(single_image, workspace_file, inputpath):
    """
    Collect all workspace projects and combine them into a single image.
    Build and collect the input xuv files from all the projects in the workspace
    """
    workspace = Workspace(workspace_file).parse()

    for project in workspace.values():
        if not single_image.process_project(project.filename, inputpath):
            # An error has occured
            print("Failed to process %s" % project)
            sys.stdout.flush()
            return False

    sys.stdout.flush()

    return True


class PrepareForImage(BaseBuilder):
    def __init__(self, args='', single_image=None, inputpath=None):
        super(PrepareForImage, self).__init__(args)
        self.single_image = single_image
        self.inputpath = inputpath

    def _is_project_buildable(self, config):
        """ Returns True if the project can be deployed """
        return config.is_deployable()

    def _run_builder(self, project, config):
        super(PrepareForImage, self)._run_builder(project, config)
        print("Adding dependent project {}".format(project.filename))
        self.single_image.process_project(project.filename, self.inputpath)


def collect_default_projects(single_image, workspace_file, inputpath):
    """
    Collect the default project dependencies and combine them into a single image.
    """
    # Build and collect the input xuv files from the default project in the workspace
    workspace = Workspace(workspace_file, PrepareForImage(single_image=single_image, inputpath=inputpath))
    workspace.build_default_project()
    return True


def main(args):
    """ deploy_single_image main entry point.
    """
    success = False

    parsed_args = parse_args(args)

    build_output_folder = parsed_args.build_output_folder

    if build_output_folder is None:
        build_output_folder = os.getenv("BUILD_OUTPUT_FOLDER")

    output_path = tempfile.mkdtemp()
    input_path = tempfile.mkdtemp()

    single_image = PrepareSingleImage(parsed_args.devkit_root, parsed_args.workspace, parsed_args.workspace, build_output_folder, generate_audio_image_xuv = True)

    collected_projects = False
    if parsed_args.active_project:
        print("Deploy active project")
        # Deploy active project and the dependent projects
        collected_projects = collect_default_projects(single_image, parsed_args.workspace, input_path)
    else:
        # Deploy all projects
        print("Deploy all projects")
        collected_projects = collect_all_workspace_projects(single_image, parsed_args.workspace, input_path)

    if collected_projects:
        if single_image.projects_exist():
            success = True
        
            curator_image = single_image.get_prebuilt_image("curator")
            if curator_image:
                # Get known version of Curator in place first to guarantee nvscmd operation
                print("Program the curator SQIF with image " + curator_image)
                sys.stdout.flush()
                pyflash = Pyflash(parsed_args.tools_dir, parsed_args.device_uri)
                result = pyflash.burn("curator", curator_image, reset_device=True)
                if result != True:
                    success = False
                time.sleep(2)

            nvscmd = NvsCmd(parsed_args.tools_dir, parsed_args.device_uri)
            apps_image, audio_image = single_image.create_flash_image(parsed_args.devkit_root, output_path)
            
            # If we have successfully created the files they are in a temporary directory
            # Lets keep a copy
            output_path_for_copy = os.path.dirname(parsed_args.workspace)
            if apps_image:
               output_filename_for_copy = os.path.join(output_path_for_copy, os.path.basename(apps_image))
               print("Keeping reference copy [" + output_filename_for_copy +"]")
               shutil.copyfile(apps_image, output_filename_for_copy);
            if audio_image:
               output_filename_for_copy = os.path.join(output_path_for_copy, os.path.basename(audio_image))
               print("Keeping reference copy [" + output_filename_for_copy +"]")
               shutil.copyfile(audio_image, output_filename_for_copy);

            if apps_image and success == True:
                # Apps image contains Curator config, so needs to be programmed before Audio and Bluetooth
                print("Program the apps SQIF...")
                sys.stdout.flush()
                subsys_id = subsys_nos.SubsystemNumbers.get_subsystem_number_from_name("apps1")
                result = nvscmd.burn(apps_image, subsys_id, reset_chip=True)
                if result != 0:
                    success = False
                time.sleep(2)

            if audio_image and success == True:
                print("Program the audio SQIF...")
                sys.stdout.flush()
                subsys_id = subsys_nos.SubsystemNumbers.get_subsystem_number_from_name("audio")
                result = nvscmd.burn(audio_image, subsys_id, reset_chip=True)
                if result != 0:
                    success = False
                time.sleep(2)

            btss_image = single_image.get_prebuilt_image("btss")
            if btss_image and success == True:
                print("Program the btss SQIF with image " + btss_image)
                sys.stdout.flush()
                subsys_id = subsys_nos.SubsystemNumbers.get_subsystem_number_from_name("btss")
                result = nvscmd.burn(btss_image, subsys_id, reset_chip=True)
                if result != 0:
                    success = False
                time.sleep(2)


    def make_writeable(func, path, exc_info):
        if func is os.remove:
            os.chmod(path, 0o640)
            func(path)
    shutil.rmtree(output_path, onerror=make_writeable)
    shutil.rmtree(input_path, onerror=make_writeable)

    if success == True:
        success = check_device_with_timeout(parsed_args.device_uri,
                                        timeout=10,
                                        devkit_path=parsed_args.devkit_root)

    return success


if __name__ == '__main__':
    if not main(sys.argv[1:]):
        sys.exit(1)
    sys.exit(0)
