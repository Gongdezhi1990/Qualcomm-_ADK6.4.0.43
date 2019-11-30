#!/usr/bin/env python
# Copyright (c) 2016-2018 Qualcomm Technologies International, Ltd.
#   
"""
Provides an interface between Heracles and building a DFU file
    The parameters are:
    buildDFU.py <devkit root> <workspace>

    -k --devkit_root    Specifies path to the root folder of the devkit to use.
    -w --workspace      Specifies workspace file to use.
"""
# Python 2 and 3
from __future__ import print_function

import sys
import argparse
import os
from workspace_parse.workspace import Workspace
from xml.etree.cElementTree import parse as xml_parse
import subprocess
from datetime import datetime
import shutil

try:
    import Tkinter
    import tkFileDialog
except ImportError:
    import tkinter as Tkinter
    import tkinter.filedialog as tkFileDialog
    

class BuildRunner(object):
    """
    Wrapper for invoking build programmatically for a given devkit and chip on
    a specified project.
    Operates on the definitions of a known set of projects returned by 
    "create_workspace_project_definitions()"; projects are referred to by 
    name in BuildRunner's interface, where the name is the same as the project
    name on disk (i.e. the x2p filename without the extension)
    """
    def __init__(self, devkit_root, workspace_file, build_configs, build_output_folder):
        self._devkit = devkit_root
        self._workspace_file = workspace_file
        self._build_configs = build_configs
        self._build_output_folder = build_output_folder
        
    def parse_args(self, args):
        """ parse the command line arguments """
        parser = argparse.ArgumentParser(description='Build a project using the \
                                                       supplied devkit')

        parser.add_argument('-w', '--workspace_file',
                            required=True,
                            help='Specifies the workspace file')

        parser.add_argument('-p', '--project_file',
                            required=True,
                            help='Specifies the project file')

        parser.add_argument('-k', '--devkit_root',
                            required=True,
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

        parser.add_argument('-o', '--build_output_folder',
                            default=None,
                            help='Specify location for object files and other build artefacts.')

        parser.add_argument('-a', '--appsFs',
                            action="store_true",
                            default=False,
                            help='Build a filesystem for apps.')

        return parser.parse_args(args)

    def run_build(self, project_name, config_name, apps_filesystem_build = False):
        """
        Run the build step on a specific project by building up the
        appropriate argument string for build and then invoking it.  Assumes
        that the build_step is appropriate for the project.
        """
        output = "Project: %s" % (project_name)
        banner = "*"*len(output)
        print(banner)
        print(output)
        print(banner)

        def format_arg(name, value):
            return ["--%s"%name, value]
                
        build_main_args = []
        build_main_args += format_arg("workspace_file", self._workspace_file)
        build_main_args += format_arg("devkit_root", self._devkit)
        build_main_args += format_arg("project_file", project_name)
        build_main_args += format_arg("configuration", config_name)
        build_main_args += format_arg("build_style", "build")
        build_main_args += format_arg("build_output_folder", self._build_output_folder)
        if apps_filesystem_build:
            build_main_args += ["--appsFs"]

        main_parsed_args = self.parse_args(build_main_args)
        import maker.util as util
        import maker.parse_devkit as pdk
        import maker.parse_proj_file as pproj
        import maker.exceptions as bdex

        bdex.Logger(main_parsed_args)
        try:
            dk_xml_file = util.get_sdk_xml_file(self._devkit)
            dk_parser = pdk.Devkit(dk_xml_file)
            proj_parser = pproj.Project(project_name, self._devkit, self._workspace_file)
            return self._build_configs(proj_parser, dk_parser, main_parsed_args)
        except bdex.BDError as excep:
            bdex.err_handler(excep)
            return False
        except BaseException:
            bdex.err_handler(None)
            return False
        else:
            bdex.trace("+++ EXIT BUILD %s ++++" % project_name)
            return True


def do_elf2xuv(parsed_args, source):
    """
    Convert an elf file to xuv format.
    """
    import elf2xuv as e2v
    e2v.elf2xuv(0, source)

def process_apps0_project(parsed_args, project, proj_parser, outpath):
    """
    Process an apps p0 project to find the name of any file path ending with
    .xuv, and - if found, form the path and filename to the source file
    indicated and copy that file (as it is a pre-built image) to the required
    destination. If copied successfully, return the filename of the destination
    file, else return None.
    """
    source = None
    destination = None
    files = proj_parser.get_source_files()
    for filepath in files:
        if filepath.endswith(".xuv"):
            source = os.path.abspath(
                os.path.join(
                    os.path.dirname(project),
                        filepath))
            destination = os.path.join(outpath, 
                os.path.basename(filepath))
            # Only take the first xuv filename found
            break

    if source != None:
        try:
            shutil.copy(source, destination)
            print("Copied %s to %s" % (source, destination))
        except IOError as err:
            print("Unable to copy %s to %s: %s" % (source, destination, err))
            return None
    else:
        print("Unable to find file path ending in xuv")
        return None

    return destination

def process_audio_project(parsed_args, project, proj_parser, outpath):

    source = None
    destination = None
    files = proj_parser.get_source_files()
    for filepath in files:
        if filepath.endswith(".xuv"):
            source = os.path.abspath(
                os.path.join(
                    os.path.dirname(project),
                        filepath))
            destination = os.path.join(outpath, 
                os.path.basename(filepath))
            # Only take the first xuv filename found
            break

    if source != None:
        try:
            shutil.copy(source, destination)
            print("Copied %s to %s" % (source, destination))
        except IOError as err:
            print("Unable to copy %s to %s: %s" % (source, destination, err))
            return None
    else:
        print("Unable to find file path ending in xuv")
        return None

    return destination

def process_curator_config_project(build_runner, parsed_args, project,
    outpath, projName):
    """
    Process a curator configuration filesystem project. First attempt to build
    the curation configuration filesystem xuv file. If successful, copy that
    xuv file to the required destination. If the copy was succcesful, return the
    filename of the destination file, else return None.
    """
    destination = None
    path = None
    build_output_folder = parsed_args.build_output_folder

    if build_runner.run_build(project, "filesystem") == True:
        filesystem_name = "curator_config_filesystem.xuv"
        if build_output_folder is None:
            path = os.path.dirname(project)
        else:
            path = os.path.join(build_output_folder, projName)

        source = os.path.abspath(os.path.join(path, filesystem_name))
        destination = os.path.join(outpath, filesystem_name)
        try:
            shutil.copy(source, destination)
            print("Copied %s to %s" % (source, destination))
        except IOError as err:
            print("Unable to copy %s to %s: %s" % (source, destination, err))
            return None
    else:
        print("Build of curator_config_filesystem.xuv failed")
        return None

    return destination

def process_apps_filesystem_project(build_runner, parsed_args, project,
    project_type, outpath, projName):
    """
    Process an apps filesystem project. First attempt to build the apps
    filesystem xuv file. If successful, copy that xuv file to the required
    destination. If the copy was succcesful, return the filename of the
    destination file, else return None.
    """
    destination = None
    path = None
    build_output_folder = parsed_args.build_output_folder

    if build_runner.run_build(project, "filesystem", True) == True:
        filesystem_name = "%s_filesystem_dfu.xuv" %(project_type)
        if build_output_folder is None:
            path = os.path.dirname(project)
        else:
            path = os.path.join(build_output_folder, projName)

        source = os.path.abspath(os.path.join(path, filesystem_name))
        destination = os.path.join(outpath, filesystem_name)
        try:
            shutil.copy(source, destination)
            print("Copied %s to %s" % (source, destination))
        except IOError as err:
            print("Unable to copy %s to %s: %s" % (source, destination, err))
            return None
    else:
        print("Build of %s_filesystem_dfu.xuv failed" % project_type)
        return None

    return destination

def process_apps1_project(build_runner, parsed_args, project, output, outpath, config, projName):
    """
    Process an apps p1 project. First attempt to find the name of the output
    folder that the build output elf file will be put in from the SDK XML file.
    Then attempt to find the name of the output file from the configuration.
    If the output filename is found then attempt to build the output elf file.
    If the output elf file is built, convert it to an xuv file, then copy that
    xuv file  to the required destination filename and return that filename.
    If there are any errors, return None instead.
    """
    try:
        import maker.util as util
        fs_prefix = util.get_fsprefix(parsed_args.devkit_root)
        if parsed_args.build_output_folder is None:          
            per_config_depend = 'depend_%s_%s' % (config, fs_prefix)
        else:    
            per_config_depend = os.path.join(parsed_args.build_output_folder, projName)

    except:
        print("Unable to find fs_prefix in the SDK xml file")
        return None

    elf_filename = output + ".elf"
    if build_runner.run_build(project, config) == True:
        if parsed_args.build_output_folder is None: 
            path = os.path.dirname(project)
        else:
            path = parsed_args.build_output_folder

        source = os.path.abspath(os.path.join(path, per_config_depend, elf_filename))
        do_elf2xuv(parsed_args, source)
    else:
        print("Build of %s failed" % elf_filename)
        return None

    xuv_filename = output + ".xuv"
    if parsed_args.build_output_folder is None: 
        path = os.path.dirname(project)
    else:
        path = parsed_args.build_output_folder

    source = os.path.abspath(os.path.join(path, per_config_depend, xuv_filename))
    destination = os.path.join(outpath, xuv_filename)
    try:
        shutil.copy(source, destination)
        print("Copied %s to %s" % (source, destination))
    except IOError as err:
        print("Unable to copy %s to %s: %s" % (source, destination, err))
        return None

    return destination
    
def apps1_project_is_default(project, config_name):
    """
    Determine if the config_name build configuration is the default build
    configuration in the .x2p apps1 project.
    """
    proj_xml = xml_parse(project).getroot()
    configurations = proj_xml.findall("configurations")
    if len(configurations) != 1:
        raise RuntimeError("Multiple configurations elements in %s" % project)
    configuration = configurations[0].findall("configuration")

    for config in configuration:
        if config.attrib["name"] == config_name:
            if "default" in config.attrib["options"]:
                return True

    return False

def find_flash_config_and_chip_type(parsed_args):
    """
    Determine the flash configuration and chip type.
    The "FLASH_CONFIG" in the default configuration of the "app/p1" project.
    The first part of the "CHIP_TYPE" in the default configuration of the
    "app/p1" project before any '_' character, converted to uppercase.
    """
    import maker.parse_proj_file as pproj
    flash_config = None
    chip_type = None
    ws_projects = Workspace(parsed_args.workspace).parse()
    for child in ws_projects.keys():
        project = ws_projects[child].filename
        proj_parser = pproj.Project(project, parsed_args.devkit_root, parsed_args.workspace)
        config_list = proj_parser.get_configurations()
        for config in config_list:
            project_type = None
            config_keys = proj_parser.get_properties_from_config(config)
            try:
                project_type = config_keys["DBG_CORE"]
                if project_type == "app/p1":
                    if apps1_project_is_default(project, config):
                        if "FLASH_CONFIG" in config_keys:
                            flash_config_file = config_keys["FLASH_CONFIG"]
                            if not os.path.isfile(flash_config_file):
                                flash_config_file = os.path.normpath(
                                    os.path.join(os.path.dirname(project), flash_config_file))
                            if os.path.isfile(flash_config_file):
                                try:
                                    with open(flash_config_file, 'r') as fd:
                                        contents = fd.read()
                                        flash_config = eval(contents)
                                    print("Using FLASH_CONFIG %s" % \
                                        (flash_config_file))
                                    try:
                                        chip_type = config_keys["CHIP_TYPE"].split('_')[0].upper()
                                        print("Using CHIP_TYPE %s" % chip_type)
                                    except KeyError:
                                        print("No CHIP_TYPE found")
                                    sys.stdout.flush()
                                    return flash_config, chip_type
                                except IOError as err:
                                    print("Unable to open and read %s: %s" % \
                                        (flash_config_file, err))
                                    sys.stdout.flush()
                                    return flash_config, chip_type
                            else:
                                print("FLASH_CONFIG file %s not found" % \
                                    (flash_config_file))
                                sys.stdout.flush()
                                return flash_config, chip_type
            except KeyError:
                pass
    return flash_config, chip_type
    
def process_project(build_runner, parsed_args, project, projects, x2p, outpath,
    flash_config):
    """
    Process a project supplied as an x2p xml file. Find all configurations in
    the x2p file given. Look for the project types that are of interest for DFU.
    If the configuration is not for a project type we are interested in, or if
    we have already processed a project of that type, then ignore the
    configuration. If a configuration is for a project thype that is of interest
    and we don't already have a project of that type, then process the project
    and, if processed successfully, add its project type and target filename to
    the dictionary of projects. If a project cannot be processed successfully,
    print an error message and exit.
    """
    print("Processing project %s" % project)
    import maker.parse_proj_file as pproj
    proj_parser = pproj.Project(project, parsed_args.devkit_root, parsed_args.workspace)
    config_list = proj_parser.get_configurations()
    for config in config_list:
        project_type = None
        config_keys = proj_parser.get_properties_from_config(config)
        if config == "prebuilt_image":
            try:
                project_type = config_keys["SUBSYSTEM_NAME"]
            except KeyError as excep:
                print("ERROR! Build Setting %s missing." % excep)
                return None, x2p
        elif config == "filesystem":
            try:
                project_type = config_keys["TYPE"]
            except KeyError as excep:
                print("ERROR! Build Setting %s missing." % excep)
                return None, x2p
        elif config == "makefile_project":
            # The projects with makefile_project configuration are unhandled
            continue
        else:
            # User defined configuration (should be the P1 application project)
            try:
                project_type = config_keys["DBG_CORE"]
            except KeyError as excep:
                print("ERROR! Build Setting %s missing." % excep)
                return None, x2p

        # Make sure we only handle one of each project_type supported
        if project_type != None:
            if not project_type in projects.keys():
                xuv_filename = None
                projName = proj_parser.proj_projname
                if project_type == "apps0":
                    xuv_filename = process_apps0_project(parsed_args, project,
                        proj_parser, outpath)
                elif project_type == "audio":
                    for i in range(len(flash_config["layout"])):
                        if flash_config["layout"][i][0] == "audio":
                            xuv_filename = process_audio_project(parsed_args, project,
                                proj_parser, outpath)
                            break
                    else:
                        continue
                elif project_type == "curator_config":
                    xuv_filename = process_curator_config_project(build_runner, parsed_args,
                        project, outpath, projName)
                elif project_type in ("firmware_config", "customer_ro"):
                    xuv_filename = process_apps_filesystem_project(build_runner, parsed_args,
                        project, project_type, outpath, projName)
                elif project_type == "app/p1":
                    try:
                        output = config_keys["OUTPUT"]
                    except KeyError as excep:
                        print("ERROR! Build Setting %s missing." % excep)
                        return None, x2p
                    # Make sure this is the default project to build
                    if apps1_project_is_default(project, config):
                        xuv_filename = process_apps1_project(build_runner, parsed_args, project,
                        output, outpath, config, projName)
                    else:
                        # Not the default build we are interested in
                        continue
                else:
                    # Not a project_type we are interested in
                    continue

                if xuv_filename:
                    projects[project_type] = xuv_filename
                    x2p[project_type] = os.path.basename(project)
                else:
                    print("failed to process %s; exiting" % project_type)
                    return None, x2p
                    
            else:
                print("Already have a %s" % project_type)

        sys.stdout.flush()
        
    return projects, x2p


def add_layout(lines, file_type, xuv_filename, params, encrypted):
    """
    Add a partition layout to a dfu_file_generator.py config file. If the
    authenticate parameter is None, leave out the authenticate line. If the 
    authenticate parameter is True then an "autheticate:True" line is added.
    Otherwise an "authenticate:False" line is added.
    """
    line = '        ("%s", {\n' % file_type
    lines.append(line)
    if xuv_filename != None:
        line = '           "src_file" : r"%s",\n' % xuv_filename
        lines.append(line)
    for item, value in params.iteritems():
        if item == "authenticate":
            if xuv_filename != None and encrypted == True:
                # If encrypted then all files supplied are authenticated
                value = True
            if file_type == "audio":
                # Apart from audio which is never authenticated
                value = False
            if file_type == "device_ro_fs" and encrypted == True:
                # This method is called for "device_ro_fs" with xuv_filename of None
                # so there is no possibility of getting two "authenticate" lines
                value = True
                line = '            "inline_auth_hash" : True,\n'
        line = '            "{}": {},\n'.format(item, value)
        lines.append(line)
    line = '            }),\n'
    lines.append(line)
    return lines


def prepare_dfu_file_generator_config(devkit_root, projects, dfg_filepath,
    upd_filepath, output_path, encrypted, hardware_encrypted, software_encryption_file,
    folder_for_rsa_files, signing_mode, chip_type, flash_config):
    """
    Create a dfu_file_generator.py config file. The various fixed areas are
    written from the "parts" lists provided. The variable areas are written to
    the file between the fixed areas using the projects information.
    """
    part1 = [
        '#!/usr/bin/env python\n',
        '# Automatically generated input script for dfu_file_generator.py\n\n',
        'dfu_image_parameters = {\n',
        '    "gen_flash_image": "True",\n',
        '    "bank": "bank1"\n',
        '}\n\n',
        'flash_device_parameters = {\n',
    ]

    part2 = [
        '}\n\n',
        'host_tools_parameters = {\n',
    ]

    part3 = [
        '}\n\n',
        'flash0 = {\n',
        '    "flash_device": flash_device_parameters,\n',
        '    "dfu_image": dfu_image_parameters,\n',
        '    "host_tools": host_tools_parameters,\n',
    ]

    part4 = [
        '    "layout": [\n',
    ]

    part5 = [
        '    ]\n',
        '}\n\n',
        'flash1 = {\n',
        '    "flash_device": flash_device_parameters,\n',
        '    "layout": []\n',
        '}\n'
    ]

    from csr.dev.tools.flash_image_builder import ImageBuilder

    section = {}
    for name, params in flash_config["layout"]:
        section[name] = params

    lines = []

    for line in part1:
        lines.append(line)

    line = '    "block_size": {},\n'.format(flash_config["flash_device"]["block_size"])
    lines.append(line)
    line = '    "boot_block_size": {},\n'.format(flash_config["flash_device"]["boot_block_size"])
    lines.append(line)
    line = '    "alt_image_offset": {}\n'.format(flash_config["flash_device"]["alt_image_offset"])
    lines.append(line)

    for line in part2:
        lines.append(line)

    line = '    "devkit": r"%s",' % devkit_root
    lines.append(line)
    lines.append('\n')

    # Use the nvscmd.exe when supplied in the SDK, or use the latest
    fname = os.path.join(devkit_root, "tools", "bin", "nvscmd.exe")
    if os.path.isfile(fname):
        line = '    "NvsCmd": r"%s",' % fname
    else:
        # The tool is not found where it should be
        print("%s not found" % fname)
        sys.stdout.flush()
        return False
    lines.append(line)
    lines.append('\n')
    fname = os.path.join(devkit_root, "tools", "bin", "SecurityCmd.exe")
    if os.path.isfile(fname):
        line = '    "SecurityCmd": r"%s",' % fname
    else:
        # The tool is not found where it should be
        print("%s not found" % fname)
        sys.stdout.flush()
        return False
    lines.append(line)
    lines.append('\n')
    # Use the UpgradeFileGen.exe when supplied in the SDK, or use the latest
    fname = os.path.join(devkit_root, "tools", "bin", "UpgradeFileGen.exe")
    if os.path.isfile(fname):
        line = '    "UpgradeFileGen": r"%s",' % fname
    else:
        # The tool is not found where it should be
        print("%s not found" % fname)
        sys.stdout.flush()
        return False
    lines.append(line)
    lines.append('\n')

    line = '    "crescendo_upd_config": r"%s",\n' % upd_filepath
    lines.append(line)

    line = '    "dfu_dir": r"%s",\n' % output_path
    lines.append(line)

    if folder_for_rsa_files != None:
        line = '    "folder_for_rsa_files": r"%s",\n' % folder_for_rsa_files
        lines.append(line)

    for line in part3:
        lines.append(line)

    line = '    "chip_type": "{}",\n'.format(chip_type)
    lines.append(line)

    line = '    "encrypt": {},\n'.format(encrypted)
    lines.append(line)

    if encrypted == True:
        line = '    "hardware_encrypted": {},\n'.format(hardware_encrypted)
        lines.append(line)
        if hardware_encrypted == False:
            line = '    "encryption_file": r"{}",\n'.format(software_encryption_file)
            lines.append(line)
        
    line = '    "signing_mode": "{}",\n'.format(signing_mode)
    lines.append(line)

    for line in part4:
        lines.append(line)

    map = []
    for i in range(len(flash_config["layout"])):
        map.append(flash_config["layout"][i][0])
    authenticate = None
    if encrypted == True:
        authenticate = True

    for i in range(len(map)):
        if map[i] =="curator_fs":
            xuv_filename = None
            if "curator_config" in projects:
                xuv_filename = projects["curator_config"]
            lines = add_layout(lines, "curator_fs", xuv_filename, section["curator_fs"], encrypted)
        elif map[i] == "apps_p0":
            xuv_filename = None
            if "apps0" in projects:
                xuv_filename = projects["apps0"]
            lines = add_layout(lines, "apps_p0", xuv_filename, section["apps_p0"], encrypted)
        elif map[i] == "apps_p1":
            xuv_filename = None
            if "app/p1" in projects:
                xuv_filename = projects["app/p1"]
            lines = add_layout(lines, "apps_p1", xuv_filename, section["apps_p1"], encrypted)
        elif map[i] == "audio":
            xuv_filename = None
            if "audio" in projects:
                xuv_filename = projects["audio"]
            lines = add_layout(lines, "audio", xuv_filename, section["audio"], False)
        elif map[i] == "device_ro_fs":
            lines = add_layout(lines, "device_ro_fs", None, section["device_ro_fs"], encrypted)
        elif map[i] == "rw_config":
            lines = add_layout(lines, "rw_config", None, section["rw_config"], None)
        elif map[i] == "rw_fs":
            lines = add_layout(lines, "rw_fs", None, section["rw_fs"], None)
        elif map[i] == "ro_cfg_fs":
            xuv_filename = None
            if "firmware_config" in projects:
                xuv_filename = projects["firmware_config"]
            lines = add_layout(lines, "ro_cfg_fs", xuv_filename, section["ro_cfg_fs"], encrypted)
        elif map[i] == "ro_fs":
            xuv_filename = None
            if "customer_ro" in projects:
                xuv_filename = projects["customer_ro"]
            lines = add_layout(lines, "ro_fs", xuv_filename, section["ro_fs"], encrypted)

    for line in part5:
        lines.append(line)
    
    result = False

    try:
        with open(dfg_filepath, "w") as f:

            for line in lines:
                f.write(line)

            result = True

    except IOError as exception:
        print("Error {} on file {}\n".format(exception.errno, dfg_filepath))

    return result

def uint16to2xuint8(uint16):
    """
    Convert a uint16 value into two uint8 values, one for the most significant
    byte and the other for the least significant byte.
    """
    msuint8 = uint16 / 256
    lsuint8 = uint16 % 256
    return msuint8, lsuint8

def addLinesUint16(lines, uint16):
    msuint8, lsuint8 = uint16to2xuint8(uint16)
    line = 'user_header_value_8bit {}\n'.format(msuint8)
    lines.append(line)
    line = 'user_header_value_8bit {}\n'.format(lsuint8)
    lines.append(line)

def addVersionInfo(lines, version_info):
    upgrade_version = version_info['upgrade_version']
    line = '# Upgrade version: major {}, minor {}\n'.format(
        upgrade_version[0], upgrade_version[1])
    lines.append(line)
    addLinesUint16(lines, upgrade_version[0])
    addLinesUint16(lines, upgrade_version[1])
    num_prior_versions = version_info['num_prior_versions']
    line = '# Compatible upgrades: {}\n'.format(num_prior_versions)
    lines.append(line)
    addLinesUint16(lines, num_prior_versions)
    prior_versions = version_info['prior_versions']
    for count in range(num_prior_versions):
        line = '# Compatible upgrade {} version: major {}, minor {}\n'.format(
            count + 1, prior_versions[count][0], prior_versions[count][1])
        lines.append(line)
        addLinesUint16(lines, prior_versions[count][0])
        addLinesUint16(lines, prior_versions[count][1])
    ps_config_version = version_info['ps_config_version'] 
    line = '# PS config version: {}\n'.format(ps_config_version)
    lines.append(line)
    addLinesUint16(lines, ps_config_version)
    num_prior_ps_versions = version_info['num_prior_ps_versions']
    line = '# Prev PS config num: {}\n'.format(num_prior_ps_versions)
    lines.append(line)
    addLinesUint16(lines, num_prior_ps_versions)
    prior_ps_versions = version_info['prior_ps_versions']
    for count in range(num_prior_ps_versions):
        line = '# Prev PS config {} version: {}\n'.format(
            count + 1, prior_ps_versions[count])
        lines.append(line)
        addLinesUint16(lines, prior_ps_versions[count])
    lines.append('\n')

def prepare_UpgradeFileGen_config(projects_selected, upd_filepath, output_path,
    version_info, signing_mode, chip_type, flash_config):
    """
    Create an UpgradeFileGen config file. The fixed area is written from the
    "part" list provided. The variable area is then written to the file after
    the fixed area, using the projects information. The projects are written
    to the file in partition number sequence.
    """
    part1 = [
        '# Upgrade header identifier\n',
        'header_identifier APPUHDR5\n\n'
    ]

    # The device variant string must be ID_FIELD_SIZE 8 characters long
    product_filename_prefix = 'notset_'
    device_variant = "xNOTSETx"
    if chip_type == "CSRA68100":
        product_filename_prefix = ''
        device_variant = "CSRA681x"
    else:
        product_filename_prefix = ''
        device_variant = chip_type
        if len(device_variant) > 8:
            device_variant = device_variant[0:8]
        elif len(device_variant) < 8:
            device_variant = device_variant.ljust(8, 'x')

    part2 = [
        '# Device Variant\n',
        'user_header_value_str {}\n\n'.format(device_variant)
        ]

    part3 = [
        '# Upgrade version major: 1, minor 1\n',
        'user_header_value_8bit 0\n',
        'user_header_value_8bit 1\n',
        'user_header_value_8bit 0\n',
        'user_header_value_8bit 1\n',
        '# Compatible upgrades: 3\n',
        'user_header_value_8bit 0\n',
        'user_header_value_8bit 3\n',
        '# Compatible upgrade version major: 0, minor 65535\n',
        'user_header_value_8bit 0\n',
        'user_header_value_8bit 0\n',
        'user_header_value_8bit 255\n',
        'user_header_value_8bit 255\n',
        '# Compatible upgrade version major: 1, minor 65535\n',
        'user_header_value_8bit 0\n',
        'user_header_value_8bit 1\n',
        'user_header_value_8bit 255\n',
        'user_header_value_8bit 255\n',
        '# Compatible upgrade version major: 2, minor 65535\n',
        'user_header_value_8bit 0\n',
        'user_header_value_8bit 2\n',
        'user_header_value_8bit 255\n',
        'user_header_value_8bit 255\n',
        '# PS config version: 2\n',
        'user_header_value_8bit 0\n',
        'user_header_value_8bit 2\n',
        '# Prev PS config num: 2\n',
        'user_header_value_8bit 0\n',
        'user_header_value_8bit 2\n',
        '# Prev PS config version: 0\n',
        'user_header_value_8bit 0\n',
        'user_header_value_8bit 0\n',
        '# Prev PS config version: 1\n',
        'user_header_value_8bit 0\n',
        'user_header_value_8bit 1\n\n',
    ]

    lines = []

    # Write the fixed part of the file
    for line in part1:
        lines.append(line)
    line = '# Footer\n'
    lines.append(line)
    output_filepath = os.path.join(output_path, 'footer.xuv')
    line = 'add_custom_footer %s\n\n' % output_filepath
    lines.append(line)
    for line in part2:
        lines.append(line)

    if version_info == None:
        # Use the default version information
        for line in part3:
            lines.append(line)
    else:
        addVersionInfo(lines, version_info)

    line = '# Signing mode\n'
    lines.append(line)
    if signing_mode == 'header':
        # Put 0 in the last byte of the header data array. It puts in a word
        # (16-bits) if only one byte specified, so add two bytes of zero.
        line = 'user_header_value_8bit 0\n\n'
        lines.append(line)
        line = 'user_header_value_8bit 0\n\n'
        lines.append(line)
    else:
        # Put 1 in the last byte of the header data array. It puts in a word
        # (16-bits) if only one byte specified, so add one byte of zero and
        # a final byte of one.
        line = 'user_header_value_8bit 0\n\n'
        lines.append(line)
        line = 'user_header_value_8bit 1\n\n'
        lines.append(line)

    # Then write the nonce and image header partitions that are always given
    line = '# Nonce\n'
    lines.append(line)
    output_filepath = os.path.join(output_path, 'partition_0_nonce.xuv')
    line = 'partition 0 0 1 %s\n' % output_filepath
    lines.append(line)

    line = '# Image Header P0\n'
    lines.append(line)
    output_filepath = os.path.join(output_path,
        'partition_1_image_header.xuv')
    line = 'partition 0 1 1 %s\n' % output_filepath
    lines.append(line)

    # Then write the other partitions that have been given in the projects
    if "curator_config" in projects_selected:
        line = '# Curator Filesystem\n'
        lines.append(line)
        output_filepath = os.path.join(output_path,
            'curator_config_filesystem.xuv')
        line = 'partition 0 4 1 %s\n' % output_filepath
        lines.append(line)

    if "apps0" in projects_selected:
        line = '# Apps P0 Image\n'
        lines.append(line)
        output_filepath = os.path.join(output_path, product_filename_prefix + 'app_p0.xuv')
        line = 'partition 0 5 1 %s\n' % output_filepath
        lines.append(line)

    if "firmware_config" in projects_selected:
        line = '# Apps read-only config filesystem\n'
        lines.append(line)
        output_filepath = os.path.join(output_path,
            'firmware_config_filesystem.xuv')
        line = 'partition 0 6 1 %s\n' % output_filepath
        lines.append(line)

    if "customer_ro" in projects_selected:
        line = '# Apps read-only filesystem\n'
        lines.append(line)
        output_filepath = os.path.join(output_path,
            'customer_ro_filesystem.xuv')
        line = 'partition 0 7 1 %s\n' % output_filepath
        lines.append(line)

    if "app/p1" in projects_selected:
        line = '# Apps P1 Image\n'
        lines.append(line)
        output_filepath = os.path.join(output_path, product_filename_prefix + 'app_p1.xuv')
        line = 'partition 0 9 1 %s\n' % output_filepath
        lines.append(line)

    if "audio" in projects_selected:
        line = '# Audio Image\n'
        lines.append(line)
        output_filepath = os.path.join(output_path, 'dfu_audio.xuv')
        line = 'partition 0 12 1 %s\n' % output_filepath
        lines.append(line)

    result = False

    try:
        with open(upd_filepath, "w") as f:

            for line in lines:
                f.write(line)

            result = True

    except IOError as exception:
        print("Error {} on file {}\n".format(exception.errno, upd_filepath))

    return result

def prepare_and_run_dfu_file_generator(devkit_root, projects, projects_selected,
    outpath, nowstring, version_info, encrypted, hardware_encrypted, software_encryption_file,
    folder_for_rsa_files, signing_mode, chip_type, flash_config):
    """
    Create the folders for the scripts and output to be created.
    Create the input script for the dfu_file_generator.py script and the input
    file for the UpgradeFileGen utility. (The input script for the
    dfu_file_generator.py script lists the filename of the input file for the
    UpgradeFileGen utility in the "crescendo_upd_config".)
    Invoke the dfu_file_generator.py script with the input script and return the
    result. If an error occurs prior to running the dfu_file_generator.py script
    an error value of -1 is returned.
    """
    scripts_path = os.path.join(outpath, "scripts")
    output_path = os.path.join(outpath, "output")
    try:
        os.makedirs(scripts_path)
    except (OSError, IOError) as exception:
        print("Unable to create path {}; error {}. Exit!\n".format(scripts_path,
                                                                exception.errno))
        return -1

    print("Created folder %s" % scripts_path)

    try:
        os.makedirs(output_path)
    except (OSError, IOError) as exception:
        print("Unable to create path {}; error {}. Exit!\n".format(output_path,
                                                                exception.errno))
        return -1

    print("Created folder %s" % output_path)

    dfg_filepath = os.path.abspath(os.path.join(scripts_path,
        "%s.py" % nowstring))
    upd_filepath = os.path.abspath(os.path.join(scripts_path,
        "%s.upd" % nowstring))

    if prepare_dfu_file_generator_config(devkit_root, projects, dfg_filepath,
            upd_filepath, output_path, encrypted, hardware_encrypted,
            software_encryption_file, folder_for_rsa_files, signing_mode,
            chip_type, flash_config) == False:
        print("Generation of the %s file failed" % dfg_filepath)
        return -1

    if prepare_UpgradeFileGen_config(projects_selected, upd_filepath,
            output_path, version_info, signing_mode, chip_type,
            flash_config) == False:
        print("Generation of the %s file failed" % upd_filepath)
        return -1

    python_exe = os.path.join(devkit_root,'tools','python27', 'python.exe')
    dfu_file_generator_path = os.path.join(devkit_root, 'apps', 'fw', 'tools',
        'pylib', 'csr', 'tools')
    dfu_file_generator_py = os.path.join(dfu_file_generator_path, 'dfu_file_generator.py')
    if not os.path.isfile(dfu_file_generator_py):
        dfu_file_generator_py = os.path.join(dfu_file_generator_path, 'dfu_file_generator.pyc')
        if not os.path.isfile(dfu_file_generator_py):
            print("Unable to find dfu_file_generator script.pyc (or py) ")
            print("in %s\n" % dfu_file_generator_path)
            return -1
    cmd_line = [python_exe, dfu_file_generator_py, dfg_filepath]
    print("Invoking '%s'" % " ".join(cmd_line))
    sys.stdout.flush()
    return subprocess.call(cmd_line)

class show_partition_selection_ui(object):
    def __init__(self, root, x2p_files):
        """
        Create a Tkinter checkbutton widget for the user to:
        - select the all or none of the partitions
        - select each individual partition
        - proceed, or
        - cancel
        """
        self.root = root
        self._projects = []
        self._x2p = []
        self._projects.append("All")
        self._x2p.append("All")
        for key, value in x2p_files.iteritems():
            self._projects.append(key)
            self._x2p.append(value)
        self.value = 0
        # Create the widget
        self.top_level = Tkinter.Toplevel(self.root)
        # Associate this window with a parent window
        self.top_level.transient(self.root)
        # Make sure no mouse or keyboard events are sent to the wrong window
        self.top_level.grab_set()
        # Set the Return key equivalent to the proceed button being clicked
        self.top_level.bind("<Return>", self._proceed)
        # Set the Escape key equivalent to the cancel button being clicked
        self.top_level.bind("<Escape>", self._cancel)
        # Gived the window a title and a label
        self.top_level.title("Select projects")
        label = Tkinter.Label(self.top_level,
            text="Please select the projects required in the DFU file") \
            .pack(padx=10, pady=10)
        # Privide an area at the bottom of the widget with "Proceed"
        # and "Cancel" buttons
        self.button_frame = Tkinter.Frame(self.top_level)
        self.button_frame.pack(fill=Tkinter.X)
        self.button_number = 0
        self.button_dictionary = {}
        self.button_variable = []
        self.checkbutton_handle = []
        self.buttons()
        select_button = Tkinter.Button(self.button_frame, text="Proceed",
            command=self._proceed).pack(side=Tkinter.LEFT)
        cancel_button = Tkinter.Button(self.button_frame, text="Cancel",
            command=self._cancel).pack(side=Tkinter.RIGHT)

    def buttons(self):
        """ one checkbutton for each item in the list, plus one for "all"
        """
        for proj in self._x2p:
            self.button_dictionary[self.button_number] = proj
            var = Tkinter.IntVar()
            var.set(1)
            self.button_variable.append(var)

            b = Tkinter.Checkbutton(self.button_frame, text = proj, variable=var)
            b.pack(anchor=Tkinter.W, expand=Tkinter.YES)
            b.select()
            self.checkbutton_handle.append ( b )
            def handler ( event, self=self, button_number=self.button_number ):
                return self.cb_handler( event, button_number )
            b.bind ( "<Button-1>", handler )
            self.button_number += 1

    def cb_handler( self, event, cb_number ):
        """ Handle the callback for a button having changed state.
            If Button 0 ("All") changes state then the others are all changed
            to match. For some reason the "get" funtion is returning 0 when
            checked (selected) and 1 when unchecked (deselected).
        """
        if cb_number == 0:
            for i in range (1, len(self.button_variable)):
                if self.button_variable[cb_number].get():
                    self.checkbutton_handle[i].deselect()
                else:
                    self.checkbutton_handle[i].select()

    def getSelectedProjects(self):
        """ Retrieve the values in the selected projects set.
            For some reason the "get" funtion is returning 1 when
            checked (selected) and 0 when unchecked (deselected),
            unlike the situation in the cb_handler method.
        """
        projects_selected = []
        for i in range (1, len(self.button_variable)):
            if self.button_variable[i].get() == 1:
                projects_selected.append(self._projects[i])
        return projects_selected

    def _proceed(self, event=None):
        """
        Actions to perform when the user has selected to proceed.
        """
        self.value = 1
        self.top_level.destroy()

    def _cancel(self, event=None):
        """
        Actions to perform when the user has cancelled the button box.
        """
        self.value = -1
        self.top_level.destroy()

    def returnValue(self):
        """
        Provides the mechanism for the user of the instance to determine whether
        the user has elected to proceed or cancel.
        """
        self.root.wait_window(self.top_level)
        return self.value

class show_encryption_selection_ui(object):
    def __init__(self, root):
        """
        Create a Tkinter radiobutton widget for the user to:
        - select unencrypted or encrypted
        - proceed, or
        - cancel
        """
        self.root = root
        self.value = 0
        self.var = Tkinter.IntVar()
        self.var.set(1)
        # Create the widget
        self.top_level = Tkinter.Toplevel(self.root)
        # Associate this window with a parent window
        self.top_level.transient(self.root)
        # Make sure no mouse or keyboard events are sent to the wrong window
        self.top_level.grab_set()
        # Set the Return key equivalent to the proceed button being clicked
        self.top_level.bind("<Return>", self._proceed)
        # Set the Escape key equivalent to the cancel button being clicked
        self.top_level.bind("<Escape>", self._cancel)
        # Gived the window a title and a label
        self.top_level.title("Select whether encrypted")
        label = Tkinter.Label(self.top_level,
            text="Please select whether the DFU file is to be encrypted") \
            .pack(padx=10, pady=10)
        # Privide an area at the bottom of the widget with "Proceed"
        # and "Cancel" buttons
        self.button_frame = Tkinter.Frame(self.top_level)
        self.button_frame.pack(fill=Tkinter.X)
        self.unencrypted_button = Tkinter.Radiobutton(self.button_frame,
            text = "Unencrypted", variable = self.var, value = 1)
        self.unencrypted_button.pack( anchor = Tkinter.W )
        self.encrypted_button = Tkinter.Radiobutton(self.button_frame,
            text = "Encrypted", variable = self.var, value = 2)
        self.encrypted_button.pack( anchor = Tkinter.W )
        select_button = Tkinter.Button(self.button_frame, text="Proceed",
            command=self._proceed).pack(side=Tkinter.LEFT)
        cancel_button = Tkinter.Button(self.button_frame, text="Cancel",
            command=self._cancel).pack(side=Tkinter.RIGHT)

    def getSelection(self):
        """ Retrieve the values in the selected projects set.
            For some reason the "get" funtion is returning 1 when
            checked (selected) and 0 when unchecked (deselected),
            unlike the situation in the cb_handler method.
        """
        if 1 == self.var.get():
            return False
        else:
            return True

    def _proceed(self, event=None):
        """
        Actions to perform when the user has selected to proceed.
        """
        self.value = 1
        self.top_level.destroy()

    def _cancel(self, event=None):
        """
        Actions to perform when the user has cancelled the button box.
        """
        self.value = -1
        self.top_level.destroy()

    def returnValue(self):
        """
        Provides the mechanism for the user of the instance to determine whether
        the user has elected to proceed or cancel.
        """
        self.root.wait_window(self.top_level)
        return self.value

class show_signing_mode_selection_ui(object):
    def __init__(self, root):
        """
        Create a Tkinter radiobutton widget for the user to:
        - select 'all' or 'header'
        - proceed, or
        - cancel
        """
        self.root = root
        self.value = 0
        self.var = Tkinter.IntVar()
        self.var.set(1)
        # Create the widget
        self.top_level = Tkinter.Toplevel(self.root)
        # Associate this window with a parent window
        self.top_level.transient(self.root)
        # Make sure no mouse or keyboard events are sent to the wrong window
        self.top_level.grab_set()
        # Set the Return key equivalent to the proceed button being clicked
        self.top_level.bind("<Return>", self._proceed)
        # Set the Escape key equivalent to the cancel button being clicked
        self.top_level.bind("<Escape>", self._cancel)
        # Gived the window a title and a label
        self.top_level.title("Select signing mode")
        label = Tkinter.Label(self.top_level,
            text="Please select how the DFU file is to be signed.\n"
            "If the version to be updated is ADK6.1 or greater, all projects should be signed.\n"
            "If the version to be updated is ADK6.0, only the DFU file header should be signed.\n"
            ) \
            .pack(padx=10, pady=10)
        # Privide an area at the bottom of the widget with "Proceed"
        # and "Cancel" buttons
        self.button_frame = Tkinter.Frame(self.top_level)
        self.button_frame.pack(fill=Tkinter.X)
        self.unencrypted_button = Tkinter.Radiobutton(self.button_frame,
            text = "Sign all projects", variable = self.var, value = 1)
        self.unencrypted_button.pack( anchor = Tkinter.W )
        self.encrypted_button = Tkinter.Radiobutton(self.button_frame,
            text = "Only sign the DFU file header", variable = self.var, value = 2)
        self.encrypted_button.pack( anchor = Tkinter.W )
        select_button = Tkinter.Button(self.button_frame, text="Proceed",
            command=self._proceed).pack(side=Tkinter.LEFT)
        cancel_button = Tkinter.Button(self.button_frame, text="Cancel",
            command=self._cancel).pack(side=Tkinter.RIGHT)

    def getSelection(self):
        """ Retrieve the values in the selected projects set.
            For some reason the "get" funtion is returning 1 when
            checked (selected) and 0 when unchecked (deselected),
            unlike the situation in the cb_handler method.
        """
        if 1 == self.var.get():
            return False
        else:
            return True

    def _proceed(self, event=None):
        """
        Actions to perform when the user has selected to proceed.
        """
        self.value = 1
        self.top_level.destroy()

    def _cancel(self, event=None):
        """
        Actions to perform when the user has cancelled the button box.
        """
        self.value = -1
        self.top_level.destroy()

    def returnValue(self):
        """
        Provides the mechanism for the user of the instance to determine whether
        the user has elected to proceed or cancel.
        """
        self.root.wait_window(self.top_level)
        return self.value

class show_encryption_type_selection_ui(object):
    def __init__(self, root, chip_type):
        """
        Create a Tkinter radiobutton widget for the user to:
        - select unencrypted or encrypted
        - proceed, or
        - cancel
        """
        self.root = root
        self.value = 0
        self.var = Tkinter.IntVar()
        self.var.set(1)
        # Create the widget
        self.top_level = Tkinter.Toplevel(self.root)
        # Associate this window with a parent window
        self.top_level.transient(self.root)
        # Make sure no mouse or keyboard events are sent to the wrong window
        self.top_level.grab_set()
        # Set the Return key equivalent to the proceed button being clicked
        self.top_level.bind("<Return>", self._proceed)
        # Set the Escape key equivalent to the cancel button being clicked
        self.top_level.bind("<Escape>", self._cancel)
        # Gived the window a title and a label
        self.top_level.title("Select whether encrypted by hardware or software")
        line1 = "Please select whether the DFU file is to be encrypted by hardware or software.\n"
        line2 = "Hardware encryption requires a {} device attached by usb2trb.\n".format(chip_type)
        line3 = "Software encryption requires the use of an AES-128 key file.\n"
        label = Tkinter.Label(self.top_level, text = line1 + line2 + line3) \
            .pack(padx=10, pady=10)
        # Privide an area at the bottom of the widget with "Proceed"
        # and "Cancel" buttons
        self.button_frame = Tkinter.Frame(self.top_level)
        self.button_frame.pack(fill=Tkinter.X)
        self.unencrypted_button = Tkinter.Radiobutton(self.button_frame,
            text = "Hardware", variable = self.var, value = 1)
        self.unencrypted_button.pack( anchor = Tkinter.W )
        self.encrypted_button = Tkinter.Radiobutton(self.button_frame,
            text = "Software", variable = self.var, value = 2)
        self.encrypted_button.pack( anchor = Tkinter.W )
        select_button = Tkinter.Button(self.button_frame, text="Proceed",
            command=self._proceed).pack(side=Tkinter.LEFT)
        cancel_button = Tkinter.Button(self.button_frame, text="Cancel",
            command=self._cancel).pack(side=Tkinter.RIGHT)

    def getSelection(self):
        """ Retrieve the values in the selected projects set.
            For some reason the "get" funtion is returning 1 when
            checked (selected) and 0 when unchecked (deselected),
            unlike the situation in the cb_handler method.
        """
        if 1 == self.var.get():
            return True
        else:
            return False

    def _proceed(self, event=None):
        """
        Actions to perform when the user has selected to proceed.
        """
        self.value = 1
        self.top_level.destroy()

    def _cancel(self, event=None):
        """
        Actions to perform when the user has cancelled the button box.
        """
        self.value = -1
        self.top_level.destroy()

    def returnValue(self):
        """
        Provides the mechanism for the user of the instance to determine whether
        the user has elected to proceed or cancel.
        """
        self.root.wait_window(self.top_level)
        return self.value

class TCL_LIBRARY_handler():
    def __init__(self, devkit_root):
        """
        If there is already a TCL_LIBRARY environment variable, save it so can
        restore it later when done with Tkinter, or note that there wasn't one.
        Set the TCL_LIBRARY environment variable to what is needed by Tkinter.
        """
        # The TCL_LIBRARY environment variable needs to be set to the
        # tools\python27\tcl\tcl8.5 folder of the devkit for Tkinter to use Tcl.
        if os.environ.get('TCL_LIBRARY'):
            self.had_TCL_LIBRARY = True
            self.old_TCL_LIBRARY = os.environ['TCL_LIBRARY']
        else:
            self.had_TCL_LIBRARY = False

        # Set the TCL_LIBRARY environment variable to what we need it to be.
        tcl_path = os.path.join(devkit_root, "tools", "python27",
            "tcl", "tcl8.5")
        os.environ['TCL_LIBRARY'] = tcl_path

    def close(self):
        """
        Restore the TCL_LIBRARY environment variable to what it was.
        """
        if self.had_TCL_LIBRARY:
            os.environ['TCL_LIBRARY'] = self.old_TCL_LIBRARY
        else:
            os.environ['TCL_LIBRARY'] = ""

def select_projects(top, x2p):
    # Use Tkinter to present the user interface. While still ongoing, the
    # returnValue is 0, -1 indicates cancellation, and 1 indicates proceed
    bb = show_partition_selection_ui(top, x2p)
    returnValue = 0
    while returnValue is 0:
        returnValue = bb.returnValue()
    
    if returnValue == 1:
        # We are proceeding
        return bb.getSelectedProjects()
    else:
        return None

def select_signing_mode(top):
    # Use Tkinter to present the user interface. While still ongoing, the
    # returnValue is 0, -1 indicates cancellation, and 1 indicates proceed
    cb = show_signing_mode_selection_ui(top)
    returnValue = 0
    while returnValue is 0:
        returnValue = cb.returnValue()
    
    if returnValue == 1:
        # We are proceeding
        return cb.getSelection()
    else:
        return None

def select_encrypted(top):
    # Use Tkinter to present the user interface. While still ongoing, the
    # returnValue is 0, -1 indicates cancellation, and 1 indicates proceed
    cb = show_encryption_selection_ui(top)
    returnValue = 0
    while returnValue is 0:
        returnValue = cb.returnValue()
    
    if returnValue == 1:
        # We are proceeding
        return cb.getSelection()
    else:
        return None

def select_encryption_type(top, chip_type):
    # Use Tkinter to present the user interface. While still ongoing, the
    # returnValue is 0, -1 indicates cancellation, and 1 indicates proceed
    cb = show_encryption_type_selection_ui(top, chip_type)
    returnValue = 0
    while returnValue is 0:
        returnValue = cb.returnValue()

    if returnValue == 1:
        # We are proceeding
        return cb.getSelection()
    else:
        return None

def select_encryption_file(top, workspace):
    file_opt = {}
    file_opt['initialdir'] = os.path.dirname(workspace)
    file_opt['title'] = 'Please select an AES-128 key file'
    return tkFileDialog.askopenfilename(**file_opt)
    
def parse_version_file(version_file):
    """
    Parse the contents of the version file. The expected contents are:
    - This upgrade version (major.minor, both uint16)
    - The number of compatible prior versions (uint16)
    - For each compatible prior version:
        - Compatible update version (major.minor, both uint16)
            - a minor value of * is taken as 65535 (0xFFFF) and means "all"
    - This PS config version (uint16)
    - The number of compatible prior PS config versions (uint16)
    - For each compatible prior PS config version:
        - Compatible PS config version (uint16)
    Any blank lines are ignored
    Leading and trailing whitespace on a line is stripped.
    Any line where the first non-whitespace character is a '#' is ignored.
    Any characters after the version (major.minor) or value are ignored.
    If anything in the file is invalid then the method returns None.
    Otherwise a dictionary is returned.
    """
    STATE_THIS_UPGRADE_VERSION          = 0
    STATE_NUMBER_OF_PRIOR_VERSIONS      = 1
    STATE_COMPATIBLE_PRIOR_VERSIONS     = 2
    STATE_THIS_PS_CONFIG_VERSION        = 3
    STATE_NUMBER_OF_PRIOR_PS_VERSIONS   = 4
    STATE_COMPATIBLE_PRIOR_PS_VERSIONS  = 5
    state = STATE_THIS_UPGRADE_VERSION
    num_prior_versions = 0
    prior_versions = []
    prior_ps_config_versions = []
    count = 0
    version_info = {}
    lines = []
    try:
        with open(version_file, "r") as f:
            lines = f.readlines()
    except IOError as exception:
        print("Error {} on file {}\n".format(exception.errno, version_file))
        return None

    line_num = 0
    for full_line in lines:
        # Remove leading and trailing whitespace characters
        line_num += 1
        line = full_line.strip()
        if len(line) == 0:
            # Ignore an empty line
            continue
        if line[0] is '#':
            # A line where the first non-whitespace character is a '#'
            # This is a comment, so ignore
            continue

        if (state is STATE_THIS_UPGRADE_VERSION) or \
        (state is STATE_COMPATIBLE_PRIOR_VERSIONS):
            # Expecting major.minor information on this line
            minor_index = line.find('.')
            if minor_index == -1:
                print("No '.' for major.minor version in line {}:\n {}\n").\
                    format(line_num, full_line)
                return None
            minor_str = line[minor_index+1:]
            minor_str = minor_str.split()[0]
            major_str = line[0:minor_index]
            major_str = major_str.split()[0]
            if not major_str.isdigit():
                print("Major version {} not all digits in line {}:\n {}\n").\
                    format(major_str, line_num, full_line)
                return None
            major_value = int(major_str)
            if major_value >= 65536:
                print("Major value {} is too large (> 65535) line {}:\n {}\n").\
                    format(major_value, line_num, full_line)
                return None
            if (state is STATE_COMPATIBLE_PRIOR_VERSIONS) and (minor_str == '*'):
                # The wildcard character '*' for the minor in compatible prior
                # versions is translated to 65535 (0xFFFF) and means "all minor
                # versions"
                minor_value = 65535
            elif not minor_str.isdigit():
                print("Minor version {} not all digits in line {}:\n {}\n").\
                    format(minor_str, line_num, full_line)
                return None
            else:
                minor_value = int(minor_str)
                if minor_value >= 65536:
                    print("Minor value {} is too large (> 65535) "
                        "line {}:\n {}\n").format(minor_value, line_num,
                            full_line)
                    return None

            upgrade_version = [major_value, minor_value]
            if state is STATE_THIS_UPGRADE_VERSION:
                version_info['upgrade_version'] = upgrade_version
                num_prior_versions = 0
                state += 1
            else:
                # STATE_COMPATIBLE_PRIOR_VERSIONS
                prior_versions.append(upgrade_version)
                count += 1
                if count == num_prior_versions:
                    version_info['prior_versions'] = prior_versions
                    state += 1
        else:
            # Expecting a single uint16 value on this line
            value_str = line.split()[0]
            if not value_str.isdigit():
                print("Value {} not all digits in line {}:\n {}\n").format(
                    value_str, line_num, full_line)
                return None
            value = int(value_str)
            if value >= 65536:
                print("Value {} is too large (> 65535) line {}:\n {}\n").format(
                    value, line_num, full_line)
                return None

            if state == STATE_NUMBER_OF_PRIOR_VERSIONS:
                version_info['num_prior_versions'] = value
                num_prior_versions = value
                count = 0
                state += 1
            elif state == STATE_THIS_PS_CONFIG_VERSION:
                version_info['ps_config_version'] = value
                state += 1
            elif state == STATE_NUMBER_OF_PRIOR_PS_VERSIONS:
                version_info['num_prior_ps_versions'] = value
                num_prior_versions = value
                count = 0
                state += 1
            elif state == STATE_COMPATIBLE_PRIOR_PS_VERSIONS:
                prior_ps_config_versions.append(value)
                count += 1
                if count == num_prior_versions:
                    version_info['prior_ps_versions'] = prior_ps_config_versions
                    break;

    if (state == STATE_COMPATIBLE_PRIOR_PS_VERSIONS) and \
        (count == num_prior_versions):
        return version_info

    print("Insufficient information in the {} file\n").format(version_file)
    return None
        
def askdirectory(root, folder):
    options = {}
    options['initialdir'] = folder
    options['mustexist'] = False
    options['parent'] = root
    options['title'] = 'Please select the location of the DFU security files\n\
The current location is %s' % folder
    return tkFileDialog.askdirectory(**options)

def parse_args(args):
    """ parse the command line arguments """
    parser = argparse.ArgumentParser(description =
        'Build a DFU file')

    parser.add_argument('-k', '--devkit_root',
                        required=True,
                        help='specifies the path to the root folder of the \
                              devkit to use')
    parser.add_argument('-w', '--workspace',
                        required=True,
                        help='specifies the workspace file to use')
    parser.add_argument('-p', '--projects_selected',
                        default=None,
                        help='specifies the projects to select')
    # For example: 
    # -p "['firmware_config', 'customer_ro', 'app/p1', 'curator_config', 'apps0']"
    # -p "['app/p1', 'apps0']"
    # The -p option prevents the partition selection UI being shown
    parser.add_argument('-n', '--nowstring',
                        help='Optional folder name to use instead of nowstring')
    # E.g. -n "Fred"
    parser.add_argument('-v', '--version_file',
                        default=None,
                        help='specifies the file containing version information')
    # E.g. -v "C:\version\version_file.txt"
    parser.add_argument('-e', '--encrypted',
                        default=None,
                        help='specifies whether DFU file is to be encrypted or not')
    # E.g. -e "encrypted"
    # Strings (case insensitive) of "encrypted" or "true" indicate encrypted
    # Any other strings indicate unencrypted
    # The -e option prevents the encryption selection UI being shown
    parser.add_argument('-a', '--aes_128_file',
                        default=None,
                        help='specifies the file containing the AES-128 software encryption key')
    # E.g. -a "C:\encryption\aes_128.key"
    # Only significant if -e or the UI has been used to select encryption
    # The -a option prevents the hardware/software encryption selection UI being 
    # shown, and prevents the software encryption file selection UI being shown
    # The -e option is not needed if the -a option is given as with -a encrypted
    # is assumed
    parser.add_argument('-f', '--folder_for_rsa_files',
                        default=None,
                        help='specifies the folder containing the RSA-2048 security files')
    # E.g. -f "C:\rsa_security\"
    # If not supplied then the default file of host_tools_parameters
    # "dfu_dir", "..", ".." will be used. The "private.pem" file is used whether
    # the -f parameter is given or not.
    parser.add_argument('-s', '--signing_mode',
                        default=None,
                        help='specifies whether "all" projects are signed or just the "header"')
    # E.g. -s "header"
    # If not supplied then the user is presented with a dialog to select
    # The valid options are 'all' or 'header', case insensitive
    # If 'all' then sign all projects
    # If 'header' then only sign the DFU file's image header

    # optional
    parser.add_argument('-o', '--build_output_folder',
                        default=None,
                        help='Specify location for object files and other build artefacts.')

    return parser.parse_args(args)

def chip_supports_hardware_encryption(chip_type):
    _CHIPS_WITH_HARDWARE_ENCRYPTION = ["csra68100"]
    for chip in _CHIPS_WITH_HARDWARE_ENCRYPTION:
        length = len(chip)
        if chip_type[0:length] == chip:
            return True
    return False

def main(args):
    """ main entry point.
        - Processes command line arguments;
        - obtain the input projects' xuv files by processing the projects listed
          in the workspace
        - create the script needed by the dfu_file_generator.py script and the
          input file for the UpgradeFileGen utility that the
          dfu_file_generator.py script runs, and run the dfu_file_generator.py
          script
        - if successful, show the filename of the DFU file created
    """
    UISTATE_LOCATION_SELECTION        = 0
    UISTATE_SIGNING_MODE_SELECTION    = 1
    UISTATE_PARTITION_SELECTION       = 2
    UISTATE_ENCRYPTION_SELECTION      = 3
    UISTATE_ENCRYPTION_TYPE_SELECTION = 4
    UISTATE_ENCRYPTION_FILE_SELECTION = 5
    UISTATE_PROCEED                   = 6
    UISTATE_EXIT                      = -1
    uistate = UISTATE_LOCATION_SELECTION

    parsed_args = parse_args(args)
    # Display whatever arguments have been given
    print("devkit_root: %s" % (parsed_args.devkit_root))
    print("workspace: %s" % (parsed_args.workspace))
    build_output_folder = None
    if parsed_args.projects_selected != None:
        print("projects_selected: %s" % (parsed_args.projects_selected))
    if parsed_args.nowstring != None:
        print("nowstring: %s" % (parsed_args.nowstring))
    if parsed_args.version_file != None:
        print("version_file: %s" % (parsed_args.version_file))
    if parsed_args.encrypted != None:
        print("encrypted: %s" % (parsed_args.encrypted))
    if parsed_args.aes_128_file != None:
        print("aes_128_file: %s" % (parsed_args.aes_128_file))
    if parsed_args.folder_for_rsa_files != None:
        print("folder_for_rsa_files: %s" % (parsed_args.folder_for_rsa_files))
    if parsed_args.signing_mode != None:
        print("signing_mode: %s" % (parsed_args.signing_mode))
    if parsed_args.build_output_folder != None:
        build_output_folder = parsed_args.build_output_folder
        print("build_output_folder %s" % (parsed_args.build_output_folder))
    sys.stdout.flush()

    signing_mode = None
    if parsed_args.signing_mode != None:
        if parsed_args.signing_mode.lower() == 'all':
            signing_mode = 'all'
        elif parsed_args.signing_mode.lower() == 'header':
            signing_mode = 'header'
        else:
            print("Invalid signing_mode value {}.\n" \
                "Valid values are 'all' or 'header'. Exit!\n".format(
                parsed_args.signing_mode))
            sys.stdout.flush()
            return False

    # Add required paths to sys.path if not there already
    path = os.path.join(parsed_args.devkit_root, "tools","ubuild")
    if not path in sys.path:
        sys.path.append(path)
    path = os.path.join(parsed_args.devkit_root,
        "apps","fw","tools","pylib","csr","tools")
    if not path in sys.path:
        sys.path.append(path)
    path = os.path.join(parsed_args.devkit_root,
        "apps","fw","tools","pylib")
    if not path in sys.path:
        sys.path.append(path)

    path = os.path.join(parsed_args.devkit_root,
        "apps","fw","tools","make")
    if not path in sys.path:
        sys.path.append(path)

    if build_output_folder is None:
        build_output_folder = os.getenv("BUILD_OUTPUT_FOLDER")
        parsed_args.build_output_folder = build_output_folder

    # Set up the use of build.py main
    from maker.build import build_configs as build_configs
    build_runner = BuildRunner(parsed_args.devkit_root, parsed_args.workspace,
        build_configs, build_output_folder)

    # Obtain a string with the current date and time to use for folder
    # and filenames.
    if (parsed_args.nowstring != None):
        # The user has provided their own folder name
        nowstring = parsed_args.nowstring
    else:
        nowstring = datetime.strftime(datetime.now(), '%Y%m%d%H%M%S')
    # Create a workspace folder dfu\<nowstring> folder
    if build_output_folder is not None:
        outpath = os.path.join(build_output_folder, "dfu",
                               nowstring)
    else:
        outpath = os.path.join(os.path.dirname(parsed_args.workspace), "dfu",
                               nowstring)
    if os.path.isdir(outpath):
        # If exists then must be using -n
        try:
            shutil.rmtree(outpath)
            sys.stdout.flush()
        except (OSError, IOError) as exception:
            print("Unable to delete path {}; error {}. Exit!\n".format(outpath,
                                                            exception.errno))
            sys.stdout.flush()
            return False
    try:
        os.makedirs(outpath)
    except (OSError, IOError) as exception:
        print("Unable to create path {}; error {}. Exit!\n".format(outpath,
                                                                exception.errno))
        sys.stdout.flush()
        return False

    print("Created folder %s" % outpath)

    # Create a workspace folder dfu\<nowstring>\input folder
    inputpath = os.path.join(outpath, "input")
    try:
        os.makedirs(inputpath)
    except (OSError, IOError) as exception:
        print("Unable to create path {}; error {}. Exit!\n".format(inputpath,
                                                                exception.errno))
        sys.stdout.flush()
        return False

    print("Created folder %s" % inputpath)

    version_file = None
    if (parsed_args.version_file != None):
        if os.path.isfile(parsed_args.version_file):
            version_file = parsed_args.version_file
        else:
            # The tool is not found where it should be
            print("%s not found" % parsed_args.version_file)
            sys.stdout.flush()
            return False
    else:
        version_file = os.path.join(os.path.dirname(parsed_args.workspace),
            "dfu", "version.txt")
        if not os.path.isfile(version_file):
            version_file = None

    version_info = None
    if version_file == None:
        print("Using default version information\n")
    else:
        print("Using version information from {}\n".format(version_file))
        version_info = parse_version_file(version_file)
        if version_info is None:
            # The contents of the version_file is not valid
            sys.stdout.flush()
            return False

    sys.stdout.flush()

    projects = {}
    x2p = {}
    projects_selected = None
    encrypted = None
    hardware_encrypted = None
    software_encryption_file = None
    folder_for_rsa_files = os.path.join(os.path.dirname(parsed_args.workspace),
        "dfu")

    # Build and collect the input xuv files from the projects in the workspace
    flash_config, chip_type = find_flash_config_and_chip_type(parsed_args)
    if flash_config == None:
        # An error has occured
        print ("Failed to locate a valid flash configuration")
        sys.stdout.flush()
        return False

    if chip_type == None:
        # An error has occured
        print ("Failed to locate a valid chip type")
        sys.stdout.flush()
        return False

    ws_projects = Workspace(parsed_args.workspace).parse()
    for child in ws_projects.keys():
        project = ws_projects[child].filename
        projects, x2p = process_project(build_runner, parsed_args, project,
            projects, x2p, inputpath, flash_config)

        if projects == None:
            # An error has occured
            print("Failed to process %s" % project)
            sys.stdout.flush()
            return False
        sys.stdout.flush()

    hardware_encrypt_supported = chip_supports_hardware_encryption(chip_type=chip_type)

    section = {}
    for name, params in flash_config["layout"]:
        section[name] = params

    if "audio" not in section:
        # The flash_config doesn't support having the audio partition
        if "audio" in projects:
            del projects["audio"]
        if "audio" in x2p:
            del x2p["audio"]

    if chip_type is None:
        # An error has occured
        print("Chip type not found")
        sys.stdout.flush()
        return False

    if projects != None:
        if len(projects) == 0:
            print("Nothing for DFU found in workspace. Exit!\n")
            uistate = UISTATE_EXIT
        # Put together the input configuration files and run the
        # dfu_file_generator.py script
        else:
            if parsed_args.projects_selected == None or \
            parsed_args.encrypted == None or \
            parsed_args.aes_128_file == None:
                # A TkInter UI is needed
                tlh = TCL_LIBRARY_handler(parsed_args.devkit_root)
                top = Tkinter.Tk(className='buildDFU')
            else:
                tlh = None

    while uistate != UISTATE_EXIT and uistate != UISTATE_PROCEED:

        if uistate == UISTATE_LOCATION_SELECTION:
            if parsed_args.folder_for_rsa_files == None:
                folder_for_rsa_files = askdirectory(top, folder_for_rsa_files)
            else:
                folder_for_rsa_files = parsed_args.folder_for_rsa_files

            if folder_for_rsa_files == "":
                uistate = UISTATE_EXIT
            else:
                uistate = UISTATE_SIGNING_MODE_SELECTION

        elif uistate == UISTATE_SIGNING_MODE_SELECTION:
            if parsed_args.signing_mode == None:
                signing_mode = select_signing_mode(top)
                if signing_mode is not None:
                    if signing_mode is True:
                        signing_mode = 'header'
                    else:
                        signing_mode = 'all'
            else:
                signing_mode = parsed_args.signing_mode

            if signing_mode != None:
                uistate = UISTATE_PARTITION_SELECTION
            elif parsed_args.folder_for_rsa_files == None:
                uistate = UISTATE_EXIT
            else:
                uistate = UISTATE_LOCATION_SELECTION

        elif uistate == UISTATE_PARTITION_SELECTION:
            if parsed_args.projects_selected == None:
                projects_selected = select_projects(top, x2p)
            else:
                projects_selected = parsed_args.projects_selected

            if projects_selected != None:
                uistate = UISTATE_ENCRYPTION_SELECTION
            else:
                if parsed_args.signing_mode == None:
                    uistate = UISTATE_SIGNING_MODE_SELECTION
                elif parsed_args.folder_for_rsa_files == None:
                    uistate = UISTATE_LOCATION_SELECTION
                else:
                    uistate = UISTATE_EXIT

        elif uistate == UISTATE_ENCRYPTION_SELECTION:
            if parsed_args.encrypted == None:
                if parsed_args.aes_128_file == None:
                    encrypted = select_encrypted(top)
                else:
                    encrypted = True
            else:
                lower_enc = parsed_args.encrypted.lower()
                if lower_enc == 'encrypted' or lower_enc == 'true':
                    encrypted = True
                else:
                    encrypted = False

            if encrypted != None:
                if encrypted == True:
                    uistate = UISTATE_ENCRYPTION_TYPE_SELECTION
                else:
                    uistate = UISTATE_PROCEED
            else:
                if parsed_args.projects_selected == None:
                    uistate = UISTATE_PARTITION_SELECTION
                elif parsed_args.signing_mode == None:
                    uistate = UISTATE_SIGNING_MODE_SELECTION
                elif parsed_args.folder_for_rsa_files == None:
                    uistate = UISTATE_LOCATION_SELECTION
                else:
                    uistate = UISTATE_EXIT

        elif uistate == UISTATE_ENCRYPTION_TYPE_SELECTION:
            if encrypted == True:
                if (parsed_args.aes_128_file == None) and hardware_encrypt_supported:
                    if parsed_args.encrypted == None:
                        hardware_encrypted = select_encryption_type(top,
                                                                chip_type)
                    else:
                        hardware_encrypted = True
                else:
                    hardware_encrypted = False

                if hardware_encrypted != None:
                    if hardware_encrypted == True:
                        uistate = UISTATE_PROCEED
                    else:
                        uistate = UISTATE_ENCRYPTION_FILE_SELECTION
                else:
                    if parsed_args.encrypted == None:
                        uistate = UISTATE_ENCRYPTION_SELECTION
                    elif parsed_args.projects_selected == None:
                        uistate = UISTATE_PARTITION_SELECTION
                    elif parsed_args.signing_mode == None:
                        uistate = UISTATE_SIGNING_MODE_SELECTION
                    elif parsed_args.folder_for_rsa_files == None:
                        uistate = UISTATE_LOCATION_SELECTION
                    else:
                        uistate = UISTATE_EXIT

        elif uistate == UISTATE_ENCRYPTION_FILE_SELECTION:
            if encrypted == True and hardware_encrypted == False:
                if parsed_args.aes_128_file == None:
                    software_encryption_file = select_encryption_file(top, parsed_args.workspace)
                else:
                    software_encryption_file = parsed_args.aes_128_file

                if software_encryption_file != None and software_encryption_file != '':
                    if os.path.isfile(software_encryption_file):
                        uistate = UISTATE_PROCEED
                    else:
                        print("{} not found\n").format(software_encryption_file)
                        uistate = UISTATE_EXIT
                    sys.stdout.flush()
                else:
                    if parsed_args.encrypted == None:
                        uistate = UISTATE_ENCRYPTION_SELECTION
                    elif parsed_args.projects_selected == None:
                        uistate = UISTATE_PARTITION_SELECTION
                    elif parsed_args.signing_mode == None:
                        uistate = UISTATE_SIGNING_MODE_SELECTION
                    elif parsed_args.folder_for_rsa_files == None:
                        uistate = UISTATE_LOCATION_SELECTION
                    else:
                        uistate = UISTATE_EXIT
            else:
                # Belt and braces as should never happen
                uistate = UISTATE_ENCRYPTION_TYPE_SELECTION

    if uistate == UISTATE_PROCEED:
        print("projects_selected: {}".format(projects_selected))
        if encrypted == True:
            if hardware_encrypted == True:
                print("Hardware encrypted\n")
            else:
                print("Software encrypted\n")
                print("Software encryption file is {}\n"
                    .format(software_encryption_file))
        else:
            print("Unencrypted\n")

        if parsed_args.signing_mode is None:
            print("signing_mode: {}".format(signing_mode))

        sys.stdout.flush()

        if folder_for_rsa_files is not None:
            parsed_args.folder_for_rsa_files = folder_for_rsa_files

        if prepare_and_run_dfu_file_generator(parsed_args.devkit_root, projects,
                projects_selected, outpath, nowstring, version_info, encrypted,
                hardware_encrypted, software_encryption_file,
                folder_for_rsa_files, signing_mode, chip_type,
                flash_config) == 0:
            # The DFU file generation has been successful. Find and print
            # the name of the DFU file generated.
            dfu_file = None
            for root, dirs, files in os.walk(os.path.join(outpath, "output")):
                for file in files:
                    if file.endswith(".bin"):
                        dfu_file = file
                        break
            if dfu_file:
                print("The DFU file generated is %s" % (os.path.join(root, file)))
            else:
                print("Unable to find the DFU file generated")
        else:
            print("Failed to generate DFU file")
    else:   # uistate == UISTATE_EXIT
        print("Cancelled")
        print("Deleting {}".format(outpath))
        shutil.rmtree(outpath)

    sys.stdout.flush()

    if tlh:
        tlh.close()

    if uistate == UISTATE_PROCEED:
        return True
    else:
        return False

 
if __name__ == '__main__':
    if not main(sys.argv[1:]):
        sys.exit(1)
    sys.exit(0)
