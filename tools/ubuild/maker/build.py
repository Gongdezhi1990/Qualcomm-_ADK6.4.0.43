"""
Copyright (c) 2019 Qualcomm Technologies International, Ltd 

Provides the command line interface for the build utility
"""
# Python 2 and 3
from __future__ import print_function

import collections
import os
import pprint
import sys
import time
import glob
import shutil
import tempfile
import platform
import re
import random

import multiprocessing as MP

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))
import maker.exceptions as bdex
import maker.proclauncher as pl
import maker.parse_proj_file as proj_parser
import maker.util as util

from workspace_parse.workspace import Workspace
import quickChargeConfig.quickChargeHexGen


def build_runner(cmd_line, env, pipeout, working_dir):
    """ Runs a command in a separate process and waits for it to terminate
    cmd_line:  full command line with arguments
    env: a dict of varname: values e.g. {AAA":"1", "BBB":"2"}
    pipeout: an open pipe for the merged stdout+stderr of the lauched process
    working_dir: working dir for the subprocess

    NOTE: this is a separate process. Therefore sys.exit() is used to send a
    return value to the parent
    """
    launcher = pl.ProcLauncher(output_pipe=pipeout,
                               cwd=working_dir,
                               cmd=cmd_line,
                               env_vars=env)
    return_status = launcher.run()

    sys.exit(return_status)

def quote_list_items(list_of_strings):
    """ Takes a list of strings and add double quote to any item that
    contains spaces.
    """
    return ["'s'" if ' ' in s else s for s in list_of_strings]

def launch_cmd(cmd_line, env, tgt_config, working_dir, elf_path, is_audio = False):
    """ Spawns a separate process to launch the build and wait for it to end.
    The stdout of the build is read here until the pipe is closed
    This process then waits for the build process to terminate
    """
    attribs = collections.OrderedDict([
        ('type', 'info'),
        ('config', 'tgt_config'),
        ('core', 'audio/p0' if is_audio else 'app/p1'),
        ('module', 'build')
    ])

    bdex.log_buildproc_output('buildstart', attribs)

    sys.stdout.flush()
    sys.stdout.write('>>> [%s]\n' % ' '.join(quote_list_items(cmd_line)))

    pipein, pipeout = MP.Pipe()

    build_proc = MP.Process(target=build_runner, args=[cmd_line,
                                                       env,
                                                       pipeout,
                                                       working_dir])

    build_proc.start()
    pipeout.close()
    cache_first_line = ''

    while True:
        try:
            line = pipein.recv()

            # Supress warnings about public and private sections being mixed
            private_warning_libs_pattern_first_line = re.compile(r'.*In function.*')
            private_warning_libs_pattern_second_line = re.compile(r'.*Warning: Public.*The contents will not be viewable or debuggable')
            if private_warning_libs_pattern_first_line.search(line, re.MULTILINE):
                cache_first_line = line
                continue
            elif cache_first_line:
                if private_warning_libs_pattern_second_line.search(line, re.MULTILINE):
                    cache_first_line = ''
                    continue
                else:
                    bdex.log_buildproc_output('buildoutput', attribs, cache_first_line.rstrip())
                    cache_first_line = ''

            bdex.log_buildproc_output('buildoutput', attribs, line.rstrip())
        except EOFError:
            break

    build_proc.join()

    pipein.close()

    sys.stdout.flush()
    sys.stdout.write('\n')

    bdex.log_buildproc_output('elfpath', attribs, elf_path)

    attribs['exit_status'] = build_proc.exitcode
    bdex.log_buildproc_output('buildend', attribs)
    return build_proc.exitcode == 0

def launch_simple_cmd(cmd_lines, env=None, cwd=None):
    if env is None:
        env = os.environ.copy()
    if cwd is None:
        cwd = os.getcwd()

    return launch_cmd(cmd_lines, env, None, cwd, None)

def print_section(section, indent=0, title=None):
    """ Print the contents of a section of a devkit file using the specified
        indentation and title.
    """
    if title != None:
        print("%s%s" % (' ' * indent, ''.join(['-', title, '-'])))

    indent += 2
    if isinstance(section, dict):
        mwid = len(max(section, key=len))
        for key, val in section.items():
            print("%s%s:%s" % (' ' * indent, key.ljust(mwid), val))
    elif isinstance(section, list):
        for i in section:
            print("%s%s" % (' ' * indent, i))
    elif isinstance(section, str):
        print("%s%s" % (' ' * indent, section))
    else:
        print("UNKNOWN TYPE", type(section))
        print("VAL", section)
        sys.exit(-1)

def print_info(p_inst, dk_inst, script_args):
    """ print summary of the project and devkit file contents """
    print("================================")
    print("PROJECT INFO ")
    print("================================")
    print("  project_file = %s" % script_args.project_file)
    print("   devkit_root = %s" % script_args.devkit_root)
    print(" configuration = %s" % script_args.configuration)

    print_section(p_inst.get_properties(), title="PROJECT PROPERTIES")

    print("\nSOURCE FILES")
    for src_file in p_inst.get_source_files():
        print(src_file)

    config_list = p_inst.get_configurations()
    for config in config_list:
        print("\nCONFIG NAME =", config)
        props = p_inst.get_properties_from_config(config)
        print("PROPS")
        pprint.pprint(props)

    print("------------------------------------")
    print("DEVKIT INFO ")
    print("------------------------------------")

    dk_xml_file = util.get_sdk_xml_file(script_args.devkit_root)
    print("  Param -d = ", script_args.devkit_root)
    print("  Script = ", sys.argv[0])
    print("  XML FILE =", dk_xml_file)

    print_section(dk_inst.get_identity(), title="KITIDENT", indent=0)

    level = 0
    print_section(dk_inst.get_top_elements(), title="TOP LEVEL ELEMENTS", indent=0)

    level += 4
    print_section(dk_inst.get_commands(), title="COMMANDS", indent=level)

    print_section(dk_inst.get_plugins(), title="TOOLS", indent=0)

def construct_makefile_frag(proj, per_config_depend, config_name, build_output_folder=None, is_audio=False, capability=None):
    """ Creates a makefile stub suitable for the subordinate build system """
    source_files = proj.get_source_files()
    make_vars = proj.get_make_vars(config_name)
    proj_name = proj.proj_projname
    if is_audio:
        makefile_name = make_vars["MAKEFILE_TO_CALL"]
        cap_path = os.path.abspath(os.path.join(proj.proj_dirname, "..", "..", "capabilities", capability))
        makefile_path = os.path.join(cap_path, makefile_name)
        cap_prop_list = proj.get_cap_props_from_config(config_name,capability)
    else:
        makefile_name = '%s.%s.mak' % (proj.proj_projname, config_name.lower())
        makefile_path = os.path.join(proj.proj_dirname, makefile_name)

    # Prepare Makefile boilerplate text
    time_string = time.strftime("%a %d. %b %H:%M:%S %Y")
    output = ''
    output += '###########################################################\n'
    output += '# Makefile generated by QMDE for CSRA68100                 \n'
    output += '#                                                          \n'
    output += '# Project: %s\n'                           % (proj_name)
    output += '# Configuration: %s\n'                     % (config_name)
    output += '# Generated: %s\n'                         % (time_string)
    output += '#                                                          \n'
    output += '# WARNING: Do not edit this file. Any changes will be lost \n'
    output += '#          when the project is rebuilt.                    \n'
    output += '#                                                          \n'
    output += '###########################################################\n'
    output += '\n'
    if is_audio:
        output += """#########################################################################
# Define root directory (relative so we can be installed anywhere)
#########################################################################

BUILD_ROOT = ../../build
include $(BUILD_ROOT)/roots.mkf
"""
        if 'MAKEFILE_INCLUDE' in cap_prop_list:
            items = cap_prop_list["MAKEFILE_INCLUDE"].split(",")
            for include in items:
                output += "\ninclude %s\n" % include

        output += """
#########################################################################
# Enter source files and directories and header directories here.
#
# Makerules will add the standard interface paths
#########################################################################
"""
        # List source files
        for src_file in [x for x in source_files if (os.path.splitext(x)[1]==".c") and (x.startswith(cap_path + os.sep))]:
            output += '\nC_SRC+=%s' % os.path.basename(src_file)

        for src_file in [x for x in source_files if (os.path.splitext(x)[1]==".asm") and (x.startswith(cap_path + os.sep))]:
            output += '\nS_SRC+=%s' % os.path.basename(src_file)
        output += '\n'

        if 'GEN_ASM_HDRS' in cap_prop_list:
            items = cap_prop_list["GEN_ASM_HDRS"].split(",")
            for file in items:
                output += "\nGEN_ASM_HDRS += %s" % file

        if 'GEN_ASM_DEFS' in cap_prop_list:
            items = cap_prop_list["GEN_ASM_DEFS"].split(",")
            for struct in items:
                output += "\nGEN_ASM_DEFS += %s" % struct

        # Add an H_PATH, C_PATH and S_PATH for each source_file located in a sub-directory so we can find them
        source_paths = set([os.path.dirname(x) for x in source_files])
        source_paths.discard(cap_path)
        source_paths = [os.path.join(os.path.curdir,os.path.relpath(x,cap_path)) for x in source_paths]

        for path in source_paths:
            output += "\nH_PATH += %s" % path
            output += "\nC_PATH += %s" % path
            output += "\nS_PATH += %s" % path

        if 'H_PATH' in cap_prop_list:
            items = cap_prop_list["H_PATH"].split(",")
            for file in items:
                output += "\nH_PATH += %s" % file

        if 'C_PATH' in cap_prop_list:
            items = cap_prop_list["C_PATH"].split(",")
            for file in items:
                output += "\nC_PATH += %s" % file

        if 'S_PATH' in cap_prop_list:
            items = cap_prop_list["S_PATH"].split(",")
            for file in items:
                output += "\nS_PATH += %s" % file

        # Include other makefiles
        output += """\n#########################################################################
# Enter final target file here (only 1 target should be specified)
#########################################################################

TARGET_EXE_ROOT =
TARGET_LIB_ROOT = %s

""" % capability
        output += """#########################################################################
# Include the standard definitions and rules
#########################################################################

include $(BUILD_ROOT)/makerules.mkf
"""
    else:
        # Prepare Makefile variables
        if 'OUTDIR' not in make_vars or make_vars['OUTDIR'] == '':
            make_vars['OUTDIR'] = proj.proj_dirname

        if 'OUTPUT' not in make_vars or make_vars['OUTPUT'] == '':
            make_vars['OUTPUT'] = proj_name

        if 'BUILDOUTPUT_PATH' not in make_vars or make_vars['BUILDOUTPUT_PATH'] == '':
            make_vars['BUILDOUTPUT_PATH'] = per_config_depend
        # adjust libpaths if output location specified
        if build_output_folder is not None:
            if make_vars.has_key('LIBPATHS'):
                libpaths = make_vars['LIBPATHS']
                paths = ""
                BUILDOUTPUT_PATH = ""
                prefix = libpaths.split('installed_libs')[0]
                suffix = proj.proj_projname
                for path in libpaths.split():
                    if path.startswith(prefix):
                        path = path[len(prefix):]
                    if per_config_depend.endswith(suffix):
                        BUILDOUTPUT_PATH = per_config_depend[:len(per_config_depend)-len(suffix)]
                    path = BUILDOUTPUT_PATH + path
                    paths += " " + path
                make_vars['LIBPATHS'] = paths + " " + libpaths
            if make_vars.has_key('INCPATHS'):
                incpaths = make_vars['INCPATHS']
                paths = ""
                BUILDOUTPUT_PATH = ""
                prefix = incpaths.split('installed_libs')[0]
                suffix = proj.proj_projname
                for path in incpaths.split():
                    if path.startswith(prefix):
                        path = path[len(prefix):]
                    if per_config_depend.endswith(suffix):
                        BUILDOUTPUT_PATH = per_config_depend[:len(per_config_depend)-len(suffix)]
                    path = BUILDOUTPUT_PATH + path
                    paths += " " + path
                make_vars['INCPATHS'] = incpaths + paths 

        for var, val in sorted(make_vars.items()):
            if len(var) > 0:
                output += "%s=%s\n" % (var, val)

        # List source files
        output += '\n\nINPUTS=\\\n'
        for src_file in source_files:
            output += '    %s \\\n' % os.path.relpath(src_file, start=proj.proj_dirname)
        output += '    %s \\\n' % os.path.relpath(os.path.join(proj.proj_dirname, 'build_id_str.c'), start=proj.proj_dirname)
        output += '$(DBS)\n'
        output += '\n'

        # Include other makefiles
        output += '-include %s\n' % (proj_name + '.mak')

        output += """# Check required variables have been defined
ifdef MAKEFILE_RULES_DIR
  $(info Using $(MAKEFILE_RULES_DIR)/Makefile.rules)
  include $(MAKEFILE_RULES_DIR)/Makefile.rules
else
  ifdef SDK
    include $(SDK)/Makefile.rules
  else
    $(error Variable SDK has not been defined. It should be set to the location of the Devkit tools folder.)
  endif
endif
"""

    # Is there an existing makefile? If so, write a temporary file and check whether contents are the same
    # If they are the same, do not overwrite existing makefile to avoid make detecting file has been touched
    # and rebuild everything
    if os.path.isfile(makefile_path):
        same = True
        # Write temporary makefile
        with open(makefile_path + "_tmp", 'w') as make_file:
            make_file.write(output)
        #Get differences
        with open(makefile_path, 'r') as file1:
            with open(makefile_path + "_tmp", 'r') as file2:
                same = set(file1).symmetric_difference(file2)
        for line in same:
            if not line.startswith("#"):
                same= False
        # If not the same, write the new makefile
        if not same:
            with open(makefile_path, 'w') as make_file:
                make_file.write(output)
        # Remove temporary makefile
        os.remove(makefile_path + "_tmp")
    else:
        with open(makefile_path, 'w') as make_file:
                make_file.write(output)
    return makefile_path

def get_tool_root(devkit):
    """ Get the path to the tools directory """
    tool_root = devkit.get_toolchain()['root']

    if not os.path.isabs(tool_root):
        tool_root = os.path.join(devkit.devkit_dirname, tool_root)
        tool_root = os.path.normpath(tool_root)

    return tool_root

def is_audio_build(proj, tgt_config):
    config = proj.get_make_vars(tgt_config)
    return (config['SUBSYSTEM_NAME'] == "audio")

def build(proj, devkit, tgt_config, build_style, build_output_folder=None):
    """ determine build style and act accordingly """
    if build_style not in ('build', 'clean'):
        bdex.raise_bd_err('INVALID_BUILD_STYLE', build_style)

    """ Audio and Apps build systems will need different makefiles"""
    is_audio = is_audio_build(proj, tgt_config)
    return do_make(proj, devkit, tgt_config, build_style, is_audio, build_output_folder)

def get_makefile_rules_path(proj, default_makefile_rules_path):
    working_dir = os.path.normpath(proj.proj_dirname)

    head = working_dir.split(os.path.sep+"adk"+os.path.sep)
    # working_dir is divided(by spliting "/adk/") into two parts, 
    # first part is <path_to_adk> which we are interested in.
    path_to_head = head[0]
    path_to_makefile_rules = os.path.sep.join((path_to_head, "adk", "tools"))

    if not os.path.isfile(os.path.join(path_to_makefile_rules, "Makefile.rules")):
        path_to_makefile_rules = default_makefile_rules_path

    return path_to_makefile_rules
    
def do_make(proj, devkit, tgt_config, build_style, is_audio, build_output_folder=None):
    """ build a project using a given configuration and devkit
        Expects that proj, devkit and tgt_config have already been validated
    """
    subproc_env=os.environ.copy()
    if is_audio:
        try:
            del subproc_env["PYTHONPATH"]
        except KeyError:
            pass
    fs_prefix = devkit.get_identity()['fsprefix']

    if build_output_folder is not None:
        per_config_depend = build_output_folder
    else:
        per_config_depend = 'depend_%s_%s' % (tgt_config,fs_prefix)

    make_vars = proj.get_make_vars(tgt_config)
    makefile_path = ""
    if is_audio:
        cap_list = proj.get_capabilities_from_config(tgt_config).keys()
        if len(cap_list) == 0:
            raise Exception("Error. Audio project without CAPABILITIES found")
        for cap in cap_list:
            construct_makefile_frag(proj, per_config_depend, tgt_config, build_output_folder, is_audio, cap)
    else:
        makefile_path = construct_makefile_frag(proj, per_config_depend, tgt_config, build_output_folder)

    tool_root = get_tool_root(devkit)
    build_cmd = devkit.get_commands()['build']['run']
    make_cmd = devkit.get_commands()['make']['run']
    sdk_tools = devkit.get_toolchain()['properties']['sdktools']['argument']

    # Note: the variable SDK (sdk_var) is just used by the Makefile fragment
    # to find Makefile.rules.  Within Makefile.rules, DEVKIT_ROOT and
    # VM_LIBS_INSTALL_DIR are used to determine
    # where to find tools and where to find the installed libs
    tools_path = os.path.join(tool_root, sdk_tools)
    make_exe_path = os.path.join(tools_path, make_cmd)
    sdk_var = "SDK=%s" % tools_path

    devkit_var = "DEVKIT_ROOT=%s" % devkit.devkit_dirname

    working_dir = proj.proj_dirname

    path_to_makefile_rules = get_makefile_rules_path(proj, tools_path)

    makefile_rules_path = "MAKEFILE_RULES_DIR=%s" % path_to_makefile_rules

    # work out what the elf file is called.
    config = proj.get_make_vars(tgt_config)
    output_filename = config['OUTPUT']
    if is_audio:
        # Audio projects provide the .elf extension in the OUTPUT tag
        elf_path = os.path.abspath(os.path.join(working_dir, output_filename))
    else:
        if output_filename != '':
            elf_path = os.path.join(working_dir, per_config_depend, output_filename + '.elf')
        else:
            elf_path = os.path.join(working_dir, per_config_depend, proj.proj_projname + '.elf')

    # Ensure paths use a '/' separator
    make_exe_path = make_exe_path.replace('\\', '/')
    sdk_var = sdk_var.replace('\\', '/')
    makefile_path = makefile_path.replace('\\', '/')
    working_dir = working_dir.replace('\\', '/')
    devkit_var = devkit_var.replace('\\', '/')

    make_jobs = make_vars.get('MAKE_JOBS', "-j{:d}".format(MP.cpu_count()))

    cmd_line = [
        make_exe_path,
        sdk_var,
        devkit_var,
        "-s",
        make_jobs,
        build_style]

    if is_audio:
        if make_vars['KALSIM_MODE'] == "true":
            config_make = make_vars['KALSIM_CONFIG_MAKE']
        else:
            config_make = make_vars['CONFIG_MAKE']

        cmd_line += [
            '-f', make_vars['MAKEFILE_TO_CALL'],
            config_make,
            make_vars['BUNDLE_NAME'],
            make_vars['BUILD_NAME'],
            make_vars["KYMERA_SRC_PATH"],
            make_vars["BUILD_PATH"],
            make_vars["BUILD_ROOT"],
            make_vars['OSTYPE']
        ]
    else:
        cmd_line += [
            '-f', makefile_path,
            makefile_rules_path
        ]

    bdex.trace("////////////// BUILD //////////////")
    bdex.trace("+++ PROJECT  '%s'" % proj.proj_fname)
    bdex.trace("+++ DEVKIT   '%s'" % devkit.devkit_fname)
    bdex.trace("+++ CONFIG   '%s'" % tgt_config)
    bdex.trace("+++ MAKEFILE '%s'" % makefile_path)
    bdex.trace("!!! TOOL_ROOT=%s" % tool_root)
    bdex.trace("!!! BUILD_CMD=%s" % build_cmd)
    bdex.trace("!!! SDK_TOOLS=%s" % sdk_tools)
    bdex.trace("!!! MAKE_EXE_PATH=%s " % make_exe_path)
    bdex.trace("!!! SDK VAR=%s" % sdk_var)
    bdex.trace("!!! MAKEFILE_RULES_DIR=%s" % makefile_rules_path)
    bdex.trace("+++ MAKE COMMAND '%s'" % make_exe_path)
    bdex.trace("+++ MAKE COMMAND CMD_ARGS '%s'" % cmd_line)

    # exec the build command and capture output for streaming to caller
    return launch_cmd(cmd_line, subproc_env, tgt_config, working_dir, elf_path, is_audio)

def register_pylib_path(devkit_path):
    """
    Get the parent path to the pylib in the devkit and add it to sys.path if
    it's not already there

    TODO This is duplicated in deploy.py- needs to be available in
    """
    path = os.path.normpath(os.path.join(devkit_path,"apps", "fw", "tools"))
    if not path in sys.path:
        sys.path += [path, os.path.join(path, "pylib")]

def _run_prepare_fs(tool_root, root_dir, xuv_path, offset=0, appsFs=False):
    # Import prepare_fs.py and run it
    packfile_command = os.path.join(tool_root, "tools", "bin", "Packfile.exe")
    if not os.path.isfile(packfile_command):
        packfile_command = os.path.join(tool_root, "tools", "hydra_dev_tools",
                                        "Packfile.exe")
    from prepare_fs import pack_dir
    with open(xuv_path, "w") as out:
        pack_dir(root_dir, out, packfile=packfile_command, offset=offset,
            appsfs=appsFs)

def copydir(src, dst):
    # create the directory in the destination folder if sorce directory doesn't exists
    if not os.path.exists(dst):
      os.makedirs(dst)
      shutil.copystat(src, dst)
    dirlist = os.listdir(src)
    for item in dirlist:
      s = os.path.join(src, item)
      d = os.path.join(dst, item)

      # if s is directory then copy the dir
      if os.path.isdir(s):
        copydir(s, d)
      else:
        shutil.copy2(s, d)

def build_filesystem(proj, devkit, script_args, crypto_key, build_output_folder):
    """
    Build any of the possible flavours of the filesystem project
    """

    devkit_root = devkit.devkit_dirname
    register_pylib_path(devkit_root)
    working_dir = proj.proj_dirname
    try:
        workspace_file = script_args.workspace_file
    except AttributeError:
        # For a time the workspace path was auto-inserted into the project file
        # so fall back to that
        workspace_file = proj.get_workspace_path()
    # The option to use NvsCmd for deploying is passed in the Ubuild --special option.
    # If using NvsCmd to deploy, the filesystems must be built using the correct endian format.
    appsFs = False
    try:
        if use_nvscmd(script_args):
            appsFs = True
    except AttributeError:
        appsFs = False
    project_files = proj.get_source_files()
    tool_root = get_tool_root(devkit)
    config = proj.get_properties_from_config('filesystem')
    try:
        output_filename = config['OUTPUT']
    except KeyError:
        output_filename = None
    try:
        filesystem_type = config['TYPE']
    except KeyError as excep:
        print("ERROR! Build Setting %s missing." % excep)
        return False
    if filesystem_type == "curator_config":
        # Curator config filesystem should not use the appsFs argument
        appsFs = False
    if not output_filename:
        output_filename = filesystem_type + "_filesystem"
        if appsFs:
            # This is a build for an apps filesystem that is needed in the form
            # required by DFU, rather than in the form required for deploy
            output_filename = output_filename + "_dfu"

    if build_output_folder is not None:               
        xuv_path = os.path.join(build_output_folder, output_filename + '.xuv')
    else:
        xuv_path = os.path.join(working_dir, output_filename + '.xuv')



    def __get_hydracode_sdbfile_path(proj):
        """
        Lets see if the project has specified a specific hydracore sdb file to use
        If so then then there will be a property HYDRACORE_CONFIG_SDB_FILE defined in the project.x2p file and set to the path to use
        examples
              <property name="HYDRACORE_CONFIG_SDB_FILE">sdk://tools/config/hydracore_config_ALTERNATIVE1.sdb</property>
              <property name="HYDRACORE_CONFIG_SDB_FILE">../../MY_hydracore_config.sdb</property>
              <property name="HYDRACORE_CONFIG_SDB_FILE">C:\TEST_CONFIGS\hydracore_config.sdb</property>
        If this field is defined then that is what we are going to use and checks are made to ensure present.
        If the field is NOT defined or empty then will continue with the existing logic of
            Does the  directory  ../../../../../devkit/config/<chip_name> exist
                 Yes - then assume file is at ../../../../../devkit/config/<chip_name>/hydracore_config.sdb
            Not exist
                 Assume file is    sdk://tools/config/hydracore_config.sdb
        """
        sdb_file = None
        config = proj.get_properties_from_config('filesystem')
        sdb_file_override =  config.get('HYDRACORE_CONFIG_SDB_FILE')
        # Check to see if project override exists
        if sdb_file_override is not None and len(sdb_file_override) > 0 :
            sdb_override_full_path = os.path.realpath(sdb_file_override)
            if not os.path.isfile(sdb_override_full_path) :
                print("WARNING - Can not find HYDRACORE_CONFIG_SDB_FILE defined file = {}".format(sdb_override_full_path))
                if sdb_file_override != sdb_override_full_path:
                    print("Property HYDRACORE_CONFIG_SDB_FILE is defined as = {}".format(sdb_file_override))
                return None
            else:
                sdb_file = sdb_override_full_path

        # No override specified - see if there is a chip folder with hydracore_config.sdb to use
        # If not then just use file sdk://tools/config/hydracore_config.sdb
        if sdb_file is None:
            chip_name = os.path.basename(working_dir)
            if chip_name == 'common':
                chip_name = os.path.basename(os.path.dirname(working_dir))
            # Try to pick the sdb file up from the local branch if that's where the
            # project file is.  Otherwise fall back to the associated devkit.
            branch_config_dir = os.path.realpath(
                os.path.join(
                    working_dir,
                    "..", "..", "..", "..", "..",
                    "devkit",
                    "config",
                    chip_name
                )
            )


            sdb_file = os.path.join(tool_root, "tools", "config", 'hydracore_config.sdb')
            if os.path.isfile(sdb_file) :
                return sdb_file
            
            sdb_file = os.path.join(branch_config_dir, 'hydracore_config.sdb')
            if os.path.isfile(sdb_file) :
                return sdb_file
            
                        
#            if os.path.isdir(branch_config_dir):
#                sdb_path = branch_config_dir
#            else:
#                devkit_config_dir = os.path.join(tool_root, "tools", "config")
#                sdb_path = devkit_config_dir

#            sdb_file = os.path.join(sdb_path, 'hydracore_config.sdb')
            sdb_file = None

            if not os.path.isfile(sdb_file) :
                print("WARNING - Can not find determined HYDRACORE_CONFIG_SDB_FILE  file = {}".format(sdb_file))
        return sdb_file



    def gather_files(proj, sdb_system_name, image_directory):
        """
        Helper function that grabs all htfs and compiles them into image_directory,
        and copies all hcfs into the same directory
        """
        supported_fs_ext = [".hcf", ".dkcs"]
        sdb_file = __get_hydracode_sdbfile_path(proj)
        if sdb_file is None:
            return False
        print('SDB File: {}'.format(sdb_file))
        print('tool_root = {}'.format(tool_root))
        config_command = os.path.join(tool_root, "tools", "bin", "ConfigCmd.exe")
        if not os.path.isfile(config_command):
            config_command = os.path.join(tool_root, "tools","hydra_core_tools",
                                          "ConfigCmd.exe")

        for cfg_file in project_files:
            if os.path.splitext(cfg_file)[1] == ".htf":
                cmd_line = [config_command, "-noprefix", "binary", cfg_file,
                            image_directory, "-system", sdb_system_name, "-quiet",
                            "-database", sdb_file]
                if not launch_simple_cmd(cmd_line):
                    print("ConfigCmd failed: invoked as '%s'" % " ".join(cmd_line))
                    return False
            elif os.path.splitext(cfg_file)[1] in supported_fs_ext:
                # Precompiled - just copy into place
                shutil.copy(cfg_file, image_directory)

        return True

    def get_patch_files(image_project_path):
        image_project = proj_parser.Project(image_project_path,
                                      devkit.devkit_dirname, script_args.workspace_file)
        return [file for file in image_project.get_source_files() if file.endswith(".hcf")]

    if filesystem_type in ("firmware_config", "curator_config", "device_config"):
        # Grab the firmware htfs and the sdbs from the image projects and run
        # configcmd to produce the contents of a local images directory.  Then
        # run packfile on it.
        sdb_system_name = config["system_label"]
        image_directory = tempfile.mkdtemp()
        if not gather_files(proj, sdb_system_name, image_directory):
            return False

        # Get the project paths from the workspace because we need to be able
        # to find information from other projects
        # Read the workspace into a dictionary of projects
        ws_projects = Workspace(workspace_file).parse()
        project_paths = [p.filename for p in ws_projects.values()]
        audio_project_path = None
        btss_project_path = None
        curator_project_path = None
        for proj_path in project_paths:
            if "kymera_audio" in proj_path:
                audio_project_path = proj_path
            if "audio_firmware" in proj_path:
                audio_project_path = proj_path
            if "btss_firmware" in proj_path:
                btss_project_path = proj_path
            if "curator_firmware" in proj_path:
                curator_project_path = proj_path

        if filesystem_type == "firmware_config":
            # Do magic things to grab audio constants from the audio package
            if audio_project_path is not None:
                patch_file_list = get_patch_files(audio_project_path)

                for patch_fname in patch_file_list:
                    print("Copying patch: {}".format(patch_fname))
                    shutil.copy(patch_fname, image_directory)

                image_project = proj_parser.Project(audio_project_path,
                                                    devkit.devkit_dirname, workspace_file)
                audio_xuv_file = None
                for file in image_project.get_source_files():
                    if file.endswith(".xuv"):
                        audio_xuv_file = os.path.join(os.path.dirname(audio_project_path), file)
                        break

                if not audio_xuv_file and not patch_file_list:
                    print("WARNING: Couldn't find an XUV or patch files in %s" % audio_project_path)

                props = image_project.get_properties_from_config('prebuilt_image')

                extconst_key = 'EXTCONST_PATH'
                ext_const_path = props[extconst_key]
                if ext_const_path:
                    if os.path.isabs(ext_const_path):
                        audio_constants_path = os.path.normpath(ext_const_path)
                    else:
                        audio_constants_path = os.path.join(image_project.proj_dirname, ext_const_path)
                    for fname in glob.glob(os.path.join(audio_constants_path, "*.hcf")):
                        print("Copying constants: {}".format(fname))
                        shutil.copy(fname, image_directory)
                else:
                    print("No {} property in project: Assuming no audio constants required".format(extconst_key))
            else:
                print("WARNING: no audio project in workspace: assuming no constants files required")

            if btss_project_path is not None:
                btss_wildcard_pattern = re.compile('.*(subsys1_(patch|config)[0-9]*_?.*\.hcf)')
                for patch_fname in get_patch_files(btss_project_path):
                    # The BTSS patch names have the wildcard prefix present, so get
                    # rid of it.
                    dest_patch_fname = os.path.join(
                        image_directory,
                        (btss_wildcard_pattern.match(os.path.basename(patch_fname)).groups())[0]
                    )
                    print("Copying %s to %s and removing wildcard" % (patch_fname, dest_patch_fname))
                    shutil.copy(patch_fname, dest_patch_fname)
            else:
                print("WARNING: no btss_firmware project: assuming no BT patch file required")

        elif filesystem_type == "curator_config":
            if curator_project_path is not None:
                for patch_fname in get_patch_files(curator_project_path):
                    print("Copying %s" % patch_fname)
                    shutil.copy(patch_fname, image_directory)
            else:
                print("WARNING: no curator_firmware project: assuming no curator patch file required")

        _run_prepare_fs(tool_root, image_directory, xuv_path, appsFs=appsFs)

        def make_writeable(func, path, exc_info):
            if func is os.remove:
                os.chmod(path, 0o640)
                func(path)
        shutil.rmtree(image_directory, onerror=make_writeable)

    elif filesystem_type == "user_ps":
        # Grab the user ps htf and convert it to XUV using the psflash_converter
        # module

        htf_files = [f for f in project_files
                        if os.path.splitext(f.lower())[1] == '.htf']
        if len(htf_files) > 0:
            print("Building user key persistent store image")
            from csr.dev.fw.psflash import PSFLASH_DEFAULT_STORE_SIZE
            from csr.dev.fw.psflash_converter import PsflashConverter
            try:
                psfs = PsflashConverter(crypto_key, stores_offset=0,
                                    store_size=PSFLASH_DEFAULT_STORE_SIZE)
            except TypeError:
                # TODO: Older API needed a default crypto_key to be passed to
                # PsflashConverter. This can be removed once all builds use
                # the PsflashConverter implementation which comes with its own default.
                crypto_key = (0, 0, 0, 0) 
                psfs = PsflashConverter(crypto_key, stores_offset=0,
                                    store_size=PSFLASH_DEFAULT_STORE_SIZE)
            # We also need to push the PS keys into the SQIF
            # 1. Load the htf
            psfs.convert(htf_files, xuv_path)
        else:
            print("No PS keys to flash")
            # Better delete any xuv file that might have been hanging around
            # so we don't accidentally flash it later
            if os.path.isfile(xuv_path):
                os.remove(xuv_path)

    elif filesystem_type == "customer_ro":
        # Point packfile at the customer-supplied filesystem root to produce an
        # XUV
        # Temporary: this filesystem needs to contain the Apps config
        # as well as any customer RO filesystem

        try:
            fs_root = config["FS_ROOT"]
            no_setting = False if fs_root else True
        except KeyError:
            no_setting = True

        quick_charge_config_exists = bool('QUICKCHARGE_CONFIG' in config)
        sdb_system_name = config["system_label"]

        # Create a temporary directory to gather everything into
        image_directory = tempfile.mkdtemp()

        ws_projects = Workspace(workspace_file).parse()
        audio_project = ws_projects.get('kymera_audio', ws_projects.get('audio_firmware', None))
        if audio_project:
            project_files += get_capabilities_files_from_props(config, audio_project)

        bundle_files = [x for x in project_files if x.endswith("dkcs")]

        for bundle in bundle_files:
            # Get the associated ELF
            bundle_elf = os.path.splitext(bundle)[0] + ".elf"
            if os.path.isfile(bundle_elf):
                # Now report the ELF to the IDE for loading when debugging
                attribs = collections.OrderedDict()
                attribs['type'] = 'info'
                attribs['config'] = script_args.configuration
                # bundles are only for audio SS
                attribs["core"] = "audio/p0"
                attribs['module'] = 'deploy'
                bdex.log_buildproc_output('elfpath', attribs, bundle_elf)

        print("\nCopying files to %s filesystem...\n" % filesystem_type)

        # Firstly, copy any files that are added to the customer RO filesystem project
        if project_files:
            print("\nCopying files added to %s filesystem project...\n" % filesystem_type)
            for ro_file in project_files:
                print("Copying file %s" % ro_file)
                shutil.copy(ro_file, image_directory)

        sys.stdout.flush()

        # Then, if there is a FS_ROOT directory specified in the customer RO filesystem project properties,
        # copy all the files under this root directory
        if not no_setting:
            if not os.path.isdir(fs_root):
                # Assume it's relative to the project root
                fs_root = os.path.join(working_dir, fs_root)
                if not os.path.isdir(fs_root):
                    print("FS_ROOT directory does not exist")
                    return False

            # Generate the quick charge configuration file
            if quick_charge_config_exists:
                quick_charge_config_file = os.path.join(
                    working_dir,
                    config["QUICKCHARGE_CONFIG"]
                )
                quickChargeConfig.quickChargeHexGen.xml_to_hex(
                    quick_charge_config_file,
                    os.path.join(fs_root, "quick_charge_config")
                )

            print("\nCopying files under FS_ROOT...")
            print("FS_ROOT (%s) with working dir (%s):" % (fs_root, working_dir))
            for root, dirs, files in os.walk(fs_root):
                for file in files:
                    print("Copying file %s\%s" % (root,file))
            sys.stdout.flush()
            copydir(fs_root, image_directory)

        _run_prepare_fs(tool_root, image_directory, xuv_path, appsFs=appsFs)

    return True


def get_capabilities_files_from_props(config, audio_project):
    audio_props = audio_project["prebuilt_image"].properties
    try:
        caps_path = audio_props['DKCS_PATH']
    except KeyError:
        return []

    if not os.path.isabs(caps_path):
        caps_path = os.path.normpath(os.path.join(audio_project.dirname, caps_path))

    if os.path.exists(caps_path):
        if not os.path.isdir(caps_path):
            raise ValueError("DKCS_PATH should point to a folder not a file")
    else:
        raise ValueError("DKCS_PATH points to non-existent location")

    return [os.path.join(caps_path, cap) for cap in config.get('DOWNLOADABLE_CAPABILITIES', '').split()]


def finalize_prebuilt_image(proj, devkit, tgt_config, build_output_folder):
    working_dir = proj.proj_dirname

    config = proj.get_make_vars(tgt_config)
    output_filename = config['OUTPUT']
    is_audio = is_audio_build(proj, tgt_config)
    if is_audio:
        elf_path = os.path.abspath(os.path.join(working_dir, output_filename))
    else:
        if build_output_folder is not None:
            per_config_depend = build_output_folder
        else:
            fs_prefix = devkit.get_identity()['fsprefix']
            per_config_depend = 'depend_%s_%s' % (tgt_config,fs_prefix)
    
        if output_filename != '':
            elf_path = os.path.join(working_dir, per_config_depend, output_filename + '.elf')
        else:
            elf_path = os.path.join(working_dir, per_config_depend, proj.proj_projname + '.elf')
    attribs = collections.OrderedDict([
        ('type', 'info'),
        ('config', 'tgt_config'),
        ('core', 'audio/p0' if is_audio else 'app/p1'),
        ('module', 'build')
    ])    
    bdex.log_buildproc_output('elfpath', attribs, elf_path)

def build_with_makefile(proj, devkit, build_style, devkit_root, build_output_folder=None):
    config = proj.get_make_vars('makefile_project')
    makefile = config['MAKEFILE_TO_CALL']

    arguments_to_pass = (
        config['BUILD_ARGUMENTS'] if build_style == "build" else config['CLEAN_ARGUMENTS']
    ).split()

    if 'CHIP_TYPE' in config:
        arguments_to_pass += ["CHIP_TYPE=%s" % config['CHIP_TYPE']]

    if 'BUILD_TYPE' in config:
        arguments_to_pass += ["BUILD_TYPE=%s" % config['BUILD_TYPE']]

    if 'BUILD_LIBS_AS_PRIVATE' in config:
        arguments_to_pass += [
            "PRIVATE_LIBS=%s" % config['BUILD_LIBS_AS_PRIVATE'],
            "PRIVATE_LIBS_SEED=%d" % random.getrandbits(16)
        ]

    if 'INCPATHS' in config:
        arguments_to_pass += ["INCPATHS=%s" % config['INCPATHS']]

    basename = os.path.dirname(proj.proj_fname)
    output_suffix = " "
    if build_output_folder is not None:
        output_suffix = os.path.relpath(build_output_folder, basename)
        output_suffix = output_suffix.replace('\\', '/')
        arguments_to_pass += [
            "BUILDOUTPUT_PATH=%s" % output_suffix
        ]

    devkit_tool_root = get_tool_root(devkit)

    makefile = makefile.replace('\\', '/')

    # Pick up make.exe from inside the devkit
    make_cmd = devkit.get_commands()['make']['run']
    sdk_tools = devkit.get_toolchain()['properties']['sdktools']['argument']
    sdk_path = os.path.join(devkit_tool_root, sdk_tools)
    make_exe_path = os.path.join(sdk_path, make_cmd).replace('\\', '/')
    devkit_root = (devkit_root).replace('/', '\\')

    is_audio = is_audio_build(proj, "makefile_project")

    cmd_line = [
        make_exe_path,
        "-s",
        "-f", makefile,
        "-R",
        "DEVKIT_ROOT=%s" % devkit_root,
        "-j%d" % MP.cpu_count()
    ]
    if not is_audio:
        # Using -Otarget in the Audio build system makes it unnervingly silent,
        # possibly because it is recursive.  But it's handy the rest of the time.
        cmd_line.append("-Otarget")
    cmd_line += arguments_to_pass

    path_to_makefile_rules = get_makefile_rules_path(proj, sdk_path)

    cmd_line.append("MAKEFILE_RULES_DIR=%s" % path_to_makefile_rules)

    # The audio firmware makefile uses DEVKIT_ROOT but get_kcc_version.py expects
    # it in an environment variable, so supply it there as well as in the command line
    # The Audio build also needs PATH not to contain MSYS, so we reset it to the
    # minimum we can get away with
    subproc_env = os.environ.copy()
    subproc_env["DEVKIT_ROOT"] = devkit_root
    print("Added DEVKIT_ROOT=%s to the environment" % devkit_root)
    if is_audio and platform.system() == "Windows":
        print("Adjusting PATH for Audio build")
        subproc_env["PATH"] = "%s;%s" % (os.getenv("SYSTEMROOT"),
                                         os.path.join(os.getenv("SYSTEMROOT"),
                                                      "system32"))
        try:
            del subproc_env["PYTHONPATH"]
        except KeyError:
            pass
    print ("Running %s in %s" % (" ".join(cmd_line), proj.proj_dirname))
    subproc_env["PYTHON"] = os.path.join(sdk_path, "python27", "python.exe")
    if not launch_simple_cmd(cmd_line, cwd=proj.proj_dirname, env=subproc_env):
        print("%s in %s failed!" % (" ".join(cmd_line), proj.proj_dirname))
        return False
    return True

def get_crypto_key(script_args):
    """
    Search all projects in the workspace to find where the crypto key is and extract it.
    :param script_args:
    :return: the crypto key needed to encrypt the PS storage
    """
    crypto_key = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    # Get the project paths from the workspace because we need to be able
    # to find information from other projects
    # Get the list of the workspace proj_path
    ws_projects = Workspace(script_args.workspace_file).parse()


    # Go through each project, find the filesystem and then look for *.htf files under different filesystems
    # and scan them to find the encryption key "PsAesKey"
    for project in ws_projects.values():
        proj = proj_parser.Project(project.filename, script_args.devkit_root, script_args.workspace_file)
        config = proj.get_properties_from_config('filesystem')
        try:
            filesystem_type = config['TYPE']
        except KeyError as excep:
            print("ERROR! Build Setting %s missing." % excep)
            return False
        if filesystem_type in ("firmware_config", "curator_config", "device_config", "user_ps"):
            project_files = proj.get_source_files()
            htf_files = [f for f in project_files
                            if os.path.splitext(f.lower())[1] == '.htf']
            for file in htf_files:
                with open(file,'r') as htf_file:
                    file_content = htf_file.read().splitlines()
                    for i in range(len(file_content)):
                        # Do not consider the key or anything else which is commented out
                        file_content[i] = re.sub("#.*$", "", file_content[i])
                        if "PsAesKey" in file_content[i]:
                            # PsAesKey = [ 00 2f 00 80 00 00 00 00 00 00 00 00 00 00 00 10]
                            # after splitting ['PsAesKey ', ' [ 00 2f 00 80 00 00 00 00 00 00 00 00 00 00 00 10]']
                            crypto_key = file_content[i].split("=")[-1:]

                            # removing  "[ ]" and extra spaces
                            crypto_key = crypto_key[0].replace("[","").replace("]","").replace(" ","")

                            # creating 16 elements, each octet long this is what P0 expects
                            # e.g. [0, 47, 0, 128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16]
                            crypto_key = [int(crypto_key[i:i+2],16) for i in range(0,len(crypto_key),2)]

    return crypto_key

def build_configs(proj, devkit, script_args):
    """ If the configuration named in tgt_config is 'all' then build
        every configuration listed in the project file.
        Otherwise build the one specified
    """

    tgt_config = script_args.configuration
    build_style = script_args.build_style
    build_output_folder = script_args.build_output_folder
    props = proj.get_properties_from_config(tgt_config)

    if build_output_folder is None:
        build_output_folder = props.get('BUILD_OUTPUT_FOLDER')
        
    if build_output_folder is None:
        build_output_folder = os.getenv("BUILD_OUTPUT_FOLDER")
        
    if build_output_folder is not None:
        proj_name = proj.proj_projname
        is_audio =  is_audio_build(proj, tgt_config)
        if not is_audio:
            build_output_folder = os.path.join(build_output_folder, proj_name)

        if not os.path.exists(build_output_folder):
            os.makedirs(build_output_folder)
        build_output_folder = os.path.abspath(build_output_folder)

    config_list = proj.get_configurations()

    if tgt_config.lower() == 'all':
        for config in config_list:
            if not build(proj, devkit, config, build_style, build_output_folder):
                return False

    elif tgt_config.lower() == 'prebuilt_image':
        finalize_prebuilt_image(proj, devkit, tgt_config, build_output_folder)
        print("Nothing to build for 'prebuilt_image' project")

    elif tgt_config.lower() == "filesystem":
        crypto_key = get_crypto_key(script_args);
        return build_filesystem(proj, devkit, script_args, crypto_key, build_output_folder)

    elif tgt_config.lower() == 'makefile_project':
        print("Build using a makefile")
        return build_with_makefile(proj, devkit, build_style, script_args.devkit_root, build_output_folder)

    else:
        if tgt_config in config_list:
            return build(proj, devkit, tgt_config, build_style, build_output_folder)
        else:
            bdex.raise_bd_err('INVALID_CONFIG', proj.proj_fname, tgt_config)

    return True


def use_nvscmd(parsed_args):
    ubuild_special = parsed_args.special
    if ubuild_special is not None:
        if "nvscmd" in ubuild_special:
            return True
    return False
