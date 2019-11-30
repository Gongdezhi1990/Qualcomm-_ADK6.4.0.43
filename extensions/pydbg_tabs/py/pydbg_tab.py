############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2016 - 2018 Qualcomm Technologies International, Ltd 
#
############################################################################
"""
Forms a wrapper around pydbug allowing pydbg to be started by
passing a heracles workspace where the paths to the  various
images are obtained.
This is primarily intended for use in the heracles tabs.

Usage:
> python pydbg_tab.py -p <path_to_pydbg> -d <debug_interface> \
      [-w <workspace_file>] [-t <type>]
where
<path_to_pydbg> is full path to pydbg.
<debug_interface> trb:scar or equivalent.
<workspace_file> Workspace file for current workspace.
<type> Leave blank for pydbg interactice session, 'prim_live_log'
for primitive logging session and 'trap_live_log' for trap logging
session.
"""

# Python 2 and 3
from __future__ import print_function

import os
import sys
from xml.etree import ElementTree
import argparse
import glob
from workspace_parse.workspace import Workspace

def validate_args(args):
    """
    Check the passed args are sensible
    """
    if not args.pydbg_path:
        print("Please specify a path to pylib")
        return False

    if args.tab_type:
        if not (args.tab_type in ["trap_live_log", "prim_live_log", "fw_live_log"]) and not os.path.isfile(args.tab_type):
            print("Incorrect tab type")
            return False

    if not args.workspace:
        print("Please enter a valid workspace")
        return False

    if not args.kit:
        print("Please enter a valid devkit")
        return False


    return True

def get_projects_from_workspace(workspace_file):
    """
    Given a workspace file (x2w) will return the 
    Path to the apps0, main project and kymera_audio
    projects contained within
    """
    workspace_path = os.path.dirname(workspace_file)
    project_dict = {}
    workspace_name = os.path.basename(workspace_file)
    workspace_name, dont_care = os.path.splitext(workspace_name)
    main_proj_name = workspace_name.rstrip("_a04") + ".x2p"
    ws_projects = Workspace(workspace_file).parse()
    for child in ws_projects.keys():
        project = ws_projects[child].filename
        if "apps0_firmware.x2p" in project:
            project_dict["apps0"] = project
        if "audio_firmware.x2p" in project:
            project_dict["audio_image"] = project
        if "kymera_audio.x2p" in project:
            project_dict["audio_package"] = project
        if main_proj_name in project:
            project_dict["apps1"] = project
    return project_dict


def get_elf_path_from_image_project(project_file, devkit):
    """
    Given a prebuilt image project file (x2v) will return the 
    Path to the image directory
    """
    project_tree = ElementTree.parse(project_file)
    project_root = project_tree.getroot()

    for file in project_root.iter('file'):
        file_path =  file.get('path')
        dont_care, file_extension = os.path.splitext(file_path)
        if '.elf' in file_extension:
            image_path = os.path.join(os.path.dirname(project_file), file_path)
            image_path = os.path.abspath(image_path)
            return image_path

    for file in project_root.iter('sdkfile'):
        file_path = os.path.join(devkit, file.get('path'))
        file_path = os.path.normpath(file_path)
        for f in glob.glob(file_path):
            file_extension = os.path.splitext(f)[1]
            if '.elf' in file_extension:
                return f

    return None

def get_elf_path_from_audio_project(project_file):
    """
    Given an audio project file (x2v) will return the 
    Path to the image directory
    """

    image_path = get_output_value_from_proj(project_file)
    if image_path:
        project_path = os.path.dirname(project_file)
        image_path = os.path.join(project_path, image_path)
        image_path = os.path.abspath(image_path)
    return image_path

def get_output_value_from_proj(project_file):
    """
    Given an audio project file (x2v) will return the 
    value of the property 'OUTPUT' if that field exists
    """
    project_tree = ElementTree.parse(project_file)
    project_root = project_tree.getroot()
    for configuration in project_root.iter('configuration'):
        if 'default' in configuration.get('options'):
            for property in configuration.iter('property'):
                if 'OUTPUT' in property.get('name'):
                    return property.text
    return None

def get_elf_path_for_std_project(project_file, devkit):
    """
    Given a Heracles project file (x2v) will return the 
    Path to the image directory
    """

    # Add required paths to sys.path if not there already
    import sys
    path = os.path.join(devkit, "tools","ubuild")
    if not path in sys.path:
        sys.path.append(path)
    import maker.util as util

    project_tree = ElementTree.parse(project_file)
    project_root = project_tree.getroot()

    for configuration in project_root.iter('configuration'):
        if 'default' in configuration.get('options'):
            config_name = configuration.get('name')

    project_path = os.path.dirname(os.path.abspath(project_file))
    fsprefix  = util.get_fsprefix(devkit)
    build_dir = "depend_" + config_name + "_" + fsprefix
    elf_dir = os.path.join(project_path, build_dir)
    # Now look for any files with a .elf extension
    elf_files = glob.glob(os.path.join(elf_dir, "*.elf"))
    # There should be exactly one if the project has been built
    if len(elf_files) == 1:
        return elf_files[0]

def get_pylib_target(devkit_target):
    """
    Munge Heracles' syntax for dongles into pydbg's
    """
    # drop uri scheme, convert to list and drop device name from end
    target_list = devkit_target.split('://')[1].split('/')[:-1]

    # join list back together with ':' inbetween
    return ':'.join(target_list)
    

if __name__ == "__main__":
    parser = argparse.ArgumentParser()

    parser.add_argument("-p", "--pydbg", help="Path to pydbg.py",
                        dest="pydbg_path", default=None)

    parser.add_argument("-d", "--debug_interface", help="Debug interface eg. trb:scar",
                        dest="debug", default="trb:scar")

    parser.add_argument("-t", "--tab_type", help="prim_live_log, trap_live_log or none for normal pydbg",
                        dest="tab_type", default=None)

    parser.add_argument("-k", "--kit", help="The devkit this is being used in",
                        dest="kit", default=None)

    parser.add_argument("-w", "--workspace", help="The workspace file that is being debugged",
                        dest="workspace", default=None)

    parser.add_argument("--passOn", help="Options to be passed on to pydbg",
                        dest="passOn", default=None, nargs=argparse.REMAINDER)

    args = parser.parse_args()

    if validate_args(args):
        #Get the paths of the projects
        projects = get_projects_from_workspace(args.workspace)

        # Now get the paths to the various images
        firmware_args = []
        try:
            apps0_path = get_elf_path_from_image_project(projects['apps0'], args.kit)
            if apps0_path:
                firmware_args.append("apps0:" + apps0_path)
        except KeyError:
            pass

        try:
            apps1_path = get_elf_path_for_std_project(projects['apps1'], args.kit)
            if apps1_path:
                firmware_args.append("apps1:" + apps1_path)
        except KeyError:
            pass

        try:
            if 'audio_package' in projects:
                audio_path = get_elf_path_from_audio_project(projects['audio_package'])
            else:
                audio_path = get_elf_path_from_image_project(projects['audio_image'], args.kit)
            if audio_path:
                firmware_args.append("audio:" + audio_path)
        except KeyError:
            pass

        #Setup the calling string for pydbg
        command_line = [args.pydbg_path]
        command_line += ["-d", get_pylib_target(args.debug)]
        if firmware_args:
            command_line += ["-f", ",".join(firmware_args)] 
        if args.tab_type:
            command_line.append(args.tab_type)
        if args.passOn:
            command_line += args.passOn

        sys.argv = command_line
        fw_tools_path = os.path.realpath(os.path.dirname(args.pydbg_path))
        sys.path = [fw_tools_path, os.path.join(fw_tools_path,"pylib")] + sys.path
        os.environ["PYDBG_RUNNING_IN_SUBPROCESS"] = "1"
        sys.stderr = sys.stdout # reassing Python's notion of stderr so everything
        # goes to stdout.  This seems to work better wtih the QMDE process handling
        
        from csr.front_end.pydbg_front_end import PydbgFrontEnd
        PydbgFrontEnd.main_wrapper()
        