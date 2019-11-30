# %%fullcopyright(2016,2018)
# 
"""
Provides an interface between Heracles and coredump.exe
    The parameters are:
    coredump.py <devkit root>

    -k --devkit_root    Specifies path to the root folder of the devkit to use.
"""

# Python 2 and 3
from __future__ import print_function

import sys
import argparse
import subprocess
import os
from datetime import datetime
import zipfile
from workspace_parse.workspace import Workspace


def parse_args(args):
    """ parse the command line arguments """
    parser = argparse.ArgumentParser(description =
        'Obtain a coredump from the debugger for the supplied devkit')

    parser.add_argument('-k', '--devkit_root',
                        required=True,
                        help='specifies the path to the root folder of the \
                              devkit to use')
    parser.add_argument('-w', '--workspace',
                        required=True,
                        help='specifies the workspace file to use')
    parser.add_argument('-e', '--extra',
                        help='specifies whether to dump apps sqif')
    parser.add_argument('-d', '--device_uri',
                        required=True,
                        help='specifies the device URI')
    return parser.parse_args(args)

def is_valid_elf_file(filepath, file, core, config):
    # app/p1 can have multiple "depend_Config" subdirs.
    # Make sure we pick the right one
    if file.lower().endswith(".elf"):
        if core == "app/p1":
            # ELF for app/p1, make sure the depend_Config also matches
            if config in filepath:
                return True
            else:
                return False
        else:
            # ELF for some other subsystem, assume it's good
            return True
    else:
        # Not an ELF, return false
        return False

def process_project(project, proj_parser):
    elffile = None
    lstfile = None
    files = proj_parser.get_source_files()
    path, _ = os.path.split(project)
    # Look in the list of the project's source files (if any)
    for file in files:
        if file.lower().endswith(".elf"):
            # An ELF file is listed. Does it exist?
            elffile = os.path.join(path, file)
            if not os.path.isfile(elffile):
                # No, it doesn't. Keep trying
                print("%s does not exist\n" % elffile)
                elffile = None
            else:
                # Yes, it does. Add that file to the zip
                # Is there a corresponding LST file?
                lstfile, _ = os.path.splitext(elffile)
                lstfile = lstfile  + ".lst"
                if not os.path.isfile(lstfile):
                    # No there isn't
                    lstfile = None

            if elffile is not None:
                # Have found an ELF file
                return (elffile, lstfile)

    if elffile is None:
        # Have not found an ELF file yet. Look in the OUTPUT path(s)
        try:
            config_list = proj_parser.get_configurations()
            for config in config_list:
                attribs = proj_parser.get_attributes_from_config(config)
                if "options" in attribs and not "default" in attribs["options"]:
                    continue
                config_keys = proj_parser.get_properties_from_config(config)
                core = config_keys["DBG_CORE"] if "DBG_CORE" in config_keys else ""
                output = config_keys["OUTPUT"]
                if output is not "":
                    outputfile = os.path.abspath(os.path.join(path, output))
                    outputpath, _ = os.path.split(outputfile)
                    if os.path.isdir(outputpath):
                        for filepath, subdirs, files in os.walk(outputpath):
                            for file in files:
                                if is_valid_elf_file(filepath, file, core, config):
                                    # An ELF file has been found in OUTPUT
                                    elffile = os.path.join(filepath, file)
                                    # Is there a corresponding LST file?
                                    lstfile, _ = os.path.splitext(elffile)
                                    lstfile = lstfile + ".lst"
                                    if not os.path.isfile(lstfile):
                                        # No there isn't
                                        lstfile = None
                                    # Have found an ELF file
                                    return (elffile, lstfile)
                    else:
                        print("OUTPUT path %s is not a directory" % outputpath)

        except KeyError as excep:
            pass

    return (elffile, lstfile)

def zip_it(root_dir, workspace, out_dir, base_name):
    """ Archive the xcd file and any elf and lst files found below the root_dir
        - the root directory of the devkit
        - the output directory containing the xcd and log files, and to contain
            the zip file containing those and any elf and lst files found
        - the base string used for the name of the xcd, log and zip files
    """
    zip_filename = base_name + ".zip"
    zip_filespec = os.path.join(out_dir, zip_filename)
    # Create the archive for writing "deflated" (compressed) files to
    try:
        with zipfile.ZipFile(zip_filespec, mode="w",
            compression=zipfile.ZIP_DEFLATED) as zip:
            # So put the xcd file in it...
            xcd_arcname = base_name + ".xcd"
            xcd_filespec = os.path.join(out_dir, xcd_arcname)
            assert os.path.isfile(xcd_filespec)
            print("Adding {} to archive {}\n".format(xcd_arcname, zip_filespec))
            zip.write(xcd_filespec, xcd_arcname)
            # ... and put the log file in it ...
            log_arcname = base_name + ".log"
            log_filespec = os.path.join(out_dir, log_arcname)
            if os.path.isfile(log_filespec):
                print("Adding {} to archive {}\n".format(log_arcname,
                                                         zip_filespec))
                zip.write(log_filespec, log_arcname)
            else:
                print("{} not found\n".format(log_filespec))
            # ... and put any *.elf/*.lst files found in the workspace in it
            print("Looking in {} project locations for *.elf and *.lst files\n"
                .format(workspace))
            sys.stdout.flush()
            ws_projects = Workspace(workspace).parse()
            for proj in ws_projects.keys():
                project = ws_projects[proj].filename
                import maker.parse_proj_file as pproj
                proj_parser = pproj.Project(project, root_dir, workspace)
                print("Processing project %s\n" % project)
                elffile, lstfile = process_project(project, proj_parser)

                if elffile is not None:
                    _, elffilename = os.path.split(elffile)
                    print("Adding %s to archive %s\n" % (elffile, zip_filespec))
                    sys.stdout.flush()
                    zip.write(elffile, elffilename)
                    if lstfile is not None:
                        _, lstfilename = os.path.split(lstfile)
                        print("Adding %s to archive %s\n" % (lstfile,
                            zip_filespec))
                        sys.stdout.flush()
                        zip.write(lstfile, lstfilename)
                else:
                    print("No elf file found for this project\n")
                        
            print("Please send the {} file to your".format(zip_filespec))
            print("Qualcomm Technologies International, Ltd. representative.\n")
    except OSError as exception:
        print("Error with zip file {} failed; error {}, {}\n" \
            .format(zip_filespec, exception.errno, os.strerror(exception.errno)))
    
    sys.stdout.flush()
    

def main(args):
    """ main entry point.
        - Processes command line arguments;
        - Determines that coredump.exe utility is where it expects to find it;
        - Creates a folder to put the xcd and other files in;
        - Calls coredump.exe for trb 1 to generate the xcd and log files;
        - Calls a function to zip together the xcd and log files, and any elf
            and lst files that are to be found below the devkit root directory.
    """
    parsed_args = parse_args(args)

    path = os.path.join(parsed_args.devkit_root, "tools", "ubuild")
    if path not in sys.path:
        sys.path.append(path)

    # Obtain a string with the current date and time to use for folder
    # and xcd, log and zip filenames.
    nowstring = datetime.strftime(datetime.now(), '%Y%m%d%H%M%S')

    # Create a workspace folder coredump\<nowstring> folder
    outpath = os.path.join(os.path.dirname(parsed_args.workspace), "coredump",
                           nowstring)

    try:
        os.makedirs(outpath)
    except OSError as exception:
        print("Unable to create path {}; error {}. Exit!\n"
              .format(outpath, exception.errno))
        return False

    # Work out the file specification of the xcd file to be created.
    dumpfile = nowstring + '.xcd'
    dumpfile = os.path.join(outpath, dumpfile)

    coredump_cmd = build_coredump_cmd(dumpfile, parsed_args)

    try:
        print("Running {} \nPlease wait...".format(' '.join(coredump_cmd)))
        sys.stdout.flush()
        # Work out the file specification of, and open, the log file that will
        # be used to capture the stdout and stderr from coredump.exe
        logfile = nowstring + '.log'
        logfile = os.path.join(outpath, logfile)
        with open(logfile, 'w') as log_fd:
            run_coredump(coredump_cmd, log_fd)
    except IOError:
        # Could not create log file: run without redirecting stdout and stderr.
        run_coredump(coredump_cmd, None)

    print("coredump successful\n")
    # Zip the log and log up along with any *.elf and *.lst files.
    zip_it(parsed_args.devkit_root, parsed_args.workspace, outpath, nowstring)
    sys.stdout.flush()
    return 0


def run_coredump(coredump_cmd, log_fd):
    """ Run coredump to generate a <nowstring>.xcd file in the
    workspace folder coredump\<nowstring> folder """
    returncode = 0
    try:
        try:
            returncode = subprocess.check_call(coredump_cmd,
                                               stdout=log_fd,
                                               stderr=log_fd)
        except subprocess.CalledProcessError as e:
            print("coredump failed with return value: {}\n"
                  .format(e.returncode))
            raise
        except OSError as e:
            print(e.strerror + ": " + coredump_cmd[0])
            raise
    except:
        sys.exit(returncode)


def build_coredump_cmd(dumpfile, parsed_args):
    """ Find the coredump executable and build the command for the specified
        debugger device and output file """
    coredump_exe = os.path.join(parsed_args.devkit_root, 'tools', 'bin',
                                'coredump.exe')

    transport_arg_mapper = {'usb2trb': '-trb',
                            'usb2tc': '-usbdbg'}

    target_list = parsed_args.device_uri.split('://')[1].split('/')[:-1]

    try:
        transport = transport_arg_mapper[target_list[1]]
    except KeyError:
        print('Unknown debug transport: {}').format(transport)
        sys.exit(1)

    port = target_list[2]

    coredump_cmd = [coredump_exe, transport, port]

    if parsed_args.extra:
        coredump_cmd.append('-extra')
        coredump_cmd.append(parsed_args.extra)
    
    coredump_cmd.append(dumpfile)
    return coredump_cmd

if __name__ == '__main__':
    if not main(sys.argv[1:]):
        sys.exit(1)
    sys.exit(0)
