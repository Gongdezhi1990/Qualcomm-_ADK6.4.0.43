############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2016 - 2018 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
"""
Forms a wrapper around ACAT allowing it to be started by
passing a QMDE workspace where the paths to the Kymera ELF
image is obtained.
This is primarily intended for use in the QMDE tabs.

Usage:
> python acat_tab.py -p <path_to_acat> -d <debug_interface> \
      [-w <workspace_file>]
where
<path_to_acat> is full path to ACAT.
<debug_interface> trb/scar/0 or equivalent.
<workspace_file> Workspace file for current workspace.
"""
from __future__ import print_function

import argparse
import importlib
import os
import subprocess
from xml.etree import ElementTree


class ACATPackages(object):
    """
    Handle dependencies for ACAT
    """

    REQUIRED_PACKAGES = {
        'graphviz': 'graphviz==0.8.4'
    }

    def check_only(self):
        """
        Checks the installation of required packages

        It prints a simple complaint for each missing package
        """
        for pkg, pkg_version in self.REQUIRED_PACKAGES.items():
            if self._is_installed(pkg) is not True:
                print(
                    "Package {} is missing. Not all the ACAT features "
                    "will work!".format(pkg_version)
                )

    @staticmethod
    def _is_installed(package):
        """
        Checks whether the given package is already installed

        Args:
            package (str): The package name that `import` use

        Returns:
            Bool: True if the package is already installed, False otherwise
        """
        try:
            importlib.import_module(package)
            return True
        except ImportError:
            return False


def validate_args(args):
    """
    Check the passed args are sensible
    """
    if not args.acat_path:
        print("Please specify a path to ACAT")
        return False

    if not args.workspace:
        print("Please enter a valid workspace")
        return False

    return True


def get_audio_elf_path_from_workspace(workspace_file, isSimTarget):
    """
    Given a workspace file (x2w) will return the
    Path to the kymera_audio project contained within
    """
    workspace_tree = ElementTree.parse(workspace_file)
    workspace_root = workspace_tree.getroot()
    workspace_path = os.path.dirname(workspace_file)
    project_dict = {}
    workspace_name = os.path.basename(workspace_file)
    workspace_name, dont_care = os.path.splitext(workspace_name)
    main_proj_name = workspace_name + ".x2p"
    kymera_image_path = None
    dkcs_path = None
    bundle_image_list = []
    projects_visited = []

    for project in workspace_root.iter('project'):
        project_path = project.get('path')
        # Ignore absolute paths or paths that use aliases. Kymera projects
        # (audio project and downloadables) use relative paths. Since top-level
        # project items are require to have `path` attribute, ignore those
        # items without path.
        if project_path is None or project_path.startswith("sdk://"):
            continue
        if project_path not in projects_visited:
            projects_visited.append(project_path)
            project_path = os.path.abspath(os.path.join(workspace_path, project_path))
            if "kymera_audio.x2p" in project_path:
                is_audio_proj, kymera_image_path = get_output_value_from_audio_proj(project_path, isSimTarget, False)
                if kymera_image_path:
                    kymera_project_path = os.path.dirname(project_path)
                    kymera_image_path = os.path.join(kymera_project_path, kymera_image_path)
                    kymera_image_path = os.path.abspath(kymera_image_path)
                    dkcs_path = get_dkcs_path_value_from_audio_proj(project_path)
                    if dkcs_path:
                        dkcs_path = os.path.abspath(os.path.join(kymera_project_path, dkcs_path))
                        if os.path.exists(dkcs_path):
                            for bundle_name in os.listdir(dkcs_path):
                                bundle_name = os.path.join(os.path.abspath(dkcs_path), bundle_name)
                                if os.path.splitext(bundle_name)[1] == ".elf":
                                    bundle_image_list.append(bundle_name)
            else:
                is_audio_proj, bundle_image_path = get_output_value_from_audio_proj(project_path, isSimTarget, True)
                if is_audio_proj:
                    kymera_project_path = os.path.dirname(project_path)
                    bundle_image_path = os.path.join(kymera_project_path, bundle_image_path)
                    bundle_image_path = os.path.abspath(bundle_image_path)
                    bundle_image_list.append(bundle_image_path)
    return kymera_image_path, bundle_image_list


def get_files_from_proj(project_file, ext):
    """
    Given a customer_ro_filesystem will return the
    list of files with extension "ext"
    """
    project_tree = ElementTree.parse(project_file)
    project_root = project_tree.getroot()
    for file in project_root.iter('file'):
        if file.get("path").endswith(ext):
            yield file.get("path")

def get_output_value_from_audio_proj(project_file, isSimTarget, isBundle):
    """
    Given an audio project file (x2p) will return the
    value of the property 'OUTPUT' if that field exists
    """
    project_tree = ElementTree.parse(project_file)
    project_root = project_tree.getroot()
    subsys = ""
    for configuration in project_root.iter('configuration'):
        if (configuration.get('name') == "kse" and isSimTarget) or (configuration.get('name') != "kse" and not isSimTarget):
            for property in configuration.iter('property'):
                if 'SUBSYSTEM_NAME' in property.get('name'):
                    subsys = property.text
            if subsys != "audio":
                return False, None
            for property in configuration.iter('property'):
                if isSimTarget and not isBundle:
                    if 'KALSIM_FIRMWARE' in property.get('name'):
                        return True, property.text
                else:
                    if 'OUTPUT' in property.get('name'):
                        return True, property.text
    return False, None

def get_dkcs_path_value_from_audio_proj(project_file):
    """
    Given an audio project file (x2p) will return the
    value of the property 'DKCS_PATH' if that field exists
    """
    project_tree = ElementTree.parse(project_file)
    project_root = project_tree.getroot()
    for configuration in project_root.iter('configuration'):
        for property in configuration.iter('property'):
            if 'DKCS_PATH' in property.get('name'):
                return property.text
    return None

if __name__ == "__main__":
    parser = argparse.ArgumentParser()

    parser.add_argument(
        "-a", "--acat", help="Path to ACAT.py", dest="acat_path", default=None
    )

    parser.add_argument(
        "-d",
        "--debug_interface",
        help="Debug interface eg. trb/scar/0",
        dest="debug",
        default="trb/scar/0"
    )

    parser.add_argument(
        "-t",
        "--python_tools_path",
        help="Path to python tools",
        dest="python_tools_path",
        default="./tools/pythontools"
    )

    parser.add_argument(
        "-w",
        "--workspace",
        help="The workspace file that is being debugged",
        dest="workspace",
        default=None
    )

    args = parser.parse_args()

    # Check ACAT dependencies. If anything missing, a suitable message
    # will appear on the screen.
    ACATPackages().check_only()

    if args.debug.split("/")[-2] == "sim":
        isSimTarget = True
    else:
        isSimTarget = False

    if validate_args(args):
        try:
            audio_path, bundles_list = get_audio_elf_path_from_workspace(args.workspace, isSimTarget)
        except KeyError:
            audio_path = None

        if audio_path is not None:
            # Setup the calling string for ACAT
            command_string = "python -i " + args.acat_path

            env = os.environ.copy()

            command_string += " -i "

            print("Kalaccess transport string is: {}".format(args.debug))

            command_string += " -s " + args.debug

            # Kalsim doesn't support dual-core
            if not isSimTarget:
                # Dual-core
                command_string += " -d "

            # Do not fail to launch if chip is not enabled
            command_string += " -q "

            command_string += " -t " + args.python_tools_path

            command_string += " -b " + os.path.splitext(audio_path)[0]

            for bundle in bundles_list:
                if os.path.isfile(os.path.splitext(bundle)[0] + ".elf"):
                    command_string += " -j " + os.path.splitext(bundle)[0]

            print(command_string)

            subprocess.call(command_string, env=env)
