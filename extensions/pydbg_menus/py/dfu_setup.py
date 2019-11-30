#!/usr/bin/env python
# Copyright (c) 2017 Qualcomm Technologies International, Ltd.
#   
"""
A script to support the QMDE "Tools|Setup DFU security files" menu option.
If the private/public key pair files and the corresponding rsa_pss_constants.c
module do not yet exist in the workspace folder dfu folder, they are created.
The workspace folder dfu folder is also created if necessary.

If they do already exist the user is queried as to whether the existing ones are
to be used or replaced.

If they are to be replaced, the existing files will be archived into a ZIP file
in the workspace folder dfu folder with a filename generated from the current
date and time. A new set of private/public key pair files and the corresponding
rsa_pss_constants.c module are then created.

If the existing files are to be used, or they have just been created, the
rsa_pss_constants.c module will replace that in the workspace supplied in the
-w parameter for the makefile project x2p file that contains:

<file path="rsa_pss_constants/rsa_pss_constants.c"/>

The original "rsa_pss_constants/rsa_pss_constants.c" file in the
workspace is first renamed to "rsa_pss_constants/<YYMMDDHHMMSS>" where
<YYMMDDHHMMSS> is a filename generated from the current date and time, as used
for the ZIP file if existing workspace folder dfu files are replaced.
"""

# Python 2 and 3
from __future__ import print_function

import sys
import argparse
import subprocess
import os
import stat
from datetime import datetime
import shutil
import zipfile
from workspace_parse.workspace import Workspace

try:
    import Tkinter
    import tkFileDialog
except ImportError:
    import tkinter as Tkinter
    import tkinter.filedialog as tkFileDialog

class BuildRunner(object):
    """
    Wrapper for accessing configurations in the workspace.
    """
    def __init__(self, devkit_root, build_configs):
        self._devkit = devkit_root
        self._build_configs = build_configs


class show_button_box(object):
    def __init__(self, top, initial_path):
        """
        Create a Tkinter button widget for the user to:
        - Use the existing files
        - Replace the existing files, or
        - cancel
        """
        self.top = top
        self.value = 0
        self._initial_path = initial_path
        # Create the widget
        self.top_level = Tkinter.Toplevel(self.top)
        # Associate this window with a parent window
        self.top_level.transient(self.top)
        # Make sure no mouse or keyboard events are sent to the wrong window
        self.top_level.grab_set()
        # Set the Return key equivalent to the "use" button being clicked
        self.top_level.bind("<Return>", self._use)
        # Set the Escape key equivalent to the cancel button being clicked
        self.top_level.bind("<Escape>", self._cancel)
        # Gived the window a title and a label
        label_str = "{} already contains DFU security files\n" \
            "Please select whether to use or replace them".format(initial_path)
        self.top_level.title("Setup DFU security")
        label = Tkinter.Label(self.top_level,
            text=label_str) \
            .pack(padx=10, pady=10)
        # Privide an area at the bottom of the widget with "Use", "Replace"
        # and "Cancel" buttons
        button_frame = Tkinter.Frame(self.top_level)
        button_frame.pack(fill=Tkinter.X)
        use_button = Tkinter.Button(button_frame, text="Use",
            command=self._use).pack(side=Tkinter.LEFT, padx=10, pady=10)
        replace_button = Tkinter.Button(button_frame, text="Replace",
            command=self._replace).pack(side=Tkinter.LEFT, padx=10, pady=10)
        cancel_button = Tkinter.Button(button_frame, text="Cancel",
            command=self._cancel).pack(side=Tkinter.RIGHT, padx=10, pady=10)

    def _use(self, event=None):
        """
        Actions to perform when the user has selected to use existing files.
        """
        self.value = 1
        self.top_level.destroy()

    def _replace(self, event=None):
        """
        Actions to perform when the user has selected a replace existing files.
        """
        self.value = 2
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
        the user has elected to use, replace or cancel.
        """
        self.top.wait_window(self.top_level)
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

def rename_and_replace_file(list_file, filespec, nowstring):
    """
    The list_file is renamed to the same folder with a filename from nowstring.
    The list_file is made writable before it is renamed.
    If the list_file cannot be made writable or renamed, the method returns -1.
    The filespec is copied to the list_file.
    If the filespec cannot be copied to the list_file, the nowstring is renamed
    back to the list_file (if possible) and -1 is returned.
    If no errors occur, the method returns 1.
    """
    print("\nFound {}\n".format(list_file))
    list_path, _ = os.path.split(list_file)
    rename_file = os.path.join(list_path, nowstring)
    try:
        # Ensure the file is not read-only before trying to rename it
        os.chmod(list_file, stat.S_IWRITE)
        os.rename(list_file, rename_file)
        print("\nRenamed {}\n to {}\n".format(list_file, rename_file))
    except (OSError, IOError) as exception:
        print("\nError renaming {}\n to {}; error {}, {}\n" \
            .format(list_file, rename_file, exception.errno,
                os.strerror(exception.errno)))
        if exception.errno == 17:
            # If file already exists then -n option must have been used
            try:
                # Ensure the file is not read-only before trying to delete it
                os.chmod(rename_file, stat.S_IWRITE)
                os.remove(rename_file)
                print("\nDeleted original {}\n".format(rename_file))
            except (OSError, IOError) as exception:
                print("\nError deleting original {}; error {}, {}\n" \
                    .format(rename_file, exception.errno,
                        os.strerror(exception.errno)))
                return -1
            try:
                os.rename(list_file, rename_file)
                print("\nRenamed {}\n to {}\n".format(list_file, rename_file))
            except (OSError, IOError) as exception:
                print("\nError renaming {}\n to {}; error {}, {}\n" \
                    .format(list_file, rename_file, exception.errno,
                        os.strerror(exception.errno)))
                return -1
        else:
            return -1

    try:
        shutil.copyfile(filespec, list_file)
        print("\nCopied {}\n to {}\n".format(filespec, list_file))
    except (OSError, IOError) as exception:
        print("\nError copying {}\n to {}; error {}, {}\n" \
            .format(filespec, list_file, exception.errno,
                os.strerror(exception.errno)))
        try:
            os.rename(rename_file, list_file)
            print("\n Renamed {}\n back to {}\n".format(rename_file, list_file))
        except (OSError, IOError) as exception:
            print("\nError renaming {}\n back to {}; error {}, {}\n" \
                .format(rename_file, list_file, exception.errno,
                    os.strerror(exception.errno)))
        return -1

    return 1

def process_project(build_runner, parsed_args, project, filespec, nowstring):
    """
    Process a project supplied as an x2p xml file. Find all configurations in
    the x2p file given. Look for makefile_project project types.
    If a makefile_project is found, get the list of source files.
    If the file we are looking for is found in the makefile_project, rename
    and replace the file found with that in the filespec, returning whatever
    the rename_and_replace_file method returns. If not a makefile_project, or
    the file we are looking for is not found in the makefile_project, return 0.
    """
    import maker.parse_proj_file as pproj
    proj_parser = pproj.Project(project, parsed_args.devkit_root, parsed_args.workspace)
    config_list = proj_parser.get_configurations()
    test_string = "rsa_pss_constants" + os.sep + \
        os.path.basename(filespec)
    list_file = ""
    for config in config_list:
        if config == "makefile_project":
            file_list = proj_parser.get_source_files()
            for list_file in file_list:
                if list_file.endswith(test_string):
                    return rename_and_replace_file(list_file, filespec,
                        nowstring)

    return 0


def restore_archive(nowstring, outpath, filelist):
    """
    If a ZIP archive of the expected name is found, extract the files in the
    list from it, restoring the files to what they were before an error
    occurred.
    """
    zip_filename = nowstring + ".zip"
    zip_filespec = os.path.join(outpath, zip_filename)
    if os.path.isfile(zip_filespec):
        print("Restoring from archive:")
        # An archive exists so restore from it
        try:
            with zipfile.ZipFile(zip_filespec, mode="r",
                compression=zipfile.ZIP_DEFLATED) as zip:
                for listfile in filelist:
                    print("Extracting {} from archive {}".format(listfile,
                        zip_filespec))
                    zip.extract(os.path.basename(listfile), outpath)
        except (OSError, IOError) as exception:
            print("Error with zip file {}; error {}, {}\n" \
                .format(zip_filespec, exception.errno,
                    os.strerror(exception.errno)))

def askdirectory(root, folder):
    options = {}
    options['initialdir'] = folder
    options['mustexist'] = False
    options['parent'] = root
    options['title'] = 'Please select the location of the DFU security files \
or the location where the DFU security files are to be created\n\
The current location is %s' % folder
    return tkFileDialog.askdirectory(**options)

def parse_args(args):
    """ parse the command line arguments """
    parser = argparse.ArgumentParser(description =
        'Setup DFU security files')

    parser.add_argument('-k', '--devkit_root',
                        required=True,
                        help='specifies the path to the root folder of the \
                              devkit to use')
    parser.add_argument('-w', '--workspace',
                        required=True,
                        help='specifies the workspace file to use')
    parser.add_argument('-n', '--nowstring',
                        help='Optional folder name to use instead of nowstring')
    # E.g. -n "Fred"
    parser.add_argument('-r', '--response',
                        help='Optional response "Use" or "Replace"')
    # Case is ignored
    # E.g. -r "replace"
    parser.add_argument('-f', '--folder_for_rsa_files',
                        default=None,
                        help='specifies the folder containing (or to contain) \
                            the RSA-2048 security files')
    # E.g. -a "C:\rsa_security\"
    # If not supplied then the default file of host_tools_parameters
    # "dfu_dir", "..", ".." will be used.
    return parser.parse_args(args)

def main(args):
    UISTATE_LOCATION_SELECTION  = 0
    UISTATE_RESPONSE_SELECTION  = 1
    UISTATE_PROCEED             = 2
    UISTATE_EXIT                = -1
    uistate = UISTATE_LOCATION_SELECTION

    parsed_args = parse_args(args)

    # Display whatever arguments have been given
    print("devkit root: %s" % (parsed_args.devkit_root))
    print("workspace: %s" % (parsed_args.workspace))
    if (parsed_args.nowstring != None):
        print("nowstring: %s" % (parsed_args.nowstring))
    if (parsed_args.response != None):
        print("response: %s" % (parsed_args.response))
    if parsed_args.folder_for_rsa_files != None:
        print("folder_for_rsa_files: %s" % (parsed_args.folder_for_rsa_files))
    sys.stdout.flush()

    # Add required paths to sys.path if not there already
    path = os.path.join(parsed_args.devkit_root, "tools","ubuild")
    if not path in sys.path:
        sys.path.append(path)

    # Set up the use of build.py main
    from maker.build import build_configs as build_configs
    build_runner = BuildRunner(parsed_args.devkit_root, build_configs)

    if (parsed_args.nowstring != None):
        # The user has provided their own folder name
        nowstring = parsed_args.nowstring
    else:
        nowstring = datetime.strftime(datetime.now(), '%Y%m%d%H%M%S')

    if parsed_args.folder_for_rsa_files == None:
        # The -f option has not been given so use the default location
        # Present the UI to the user to confirm or change that location
        outpath = os.path.join(os.path.dirname(parsed_args.workspace), "dfu")
    else:
        # The -f option has been given so use that location
        outpath = os.path.abspath(parsed_args.folder_for_rsa_files)
        uistate = UISTATE_RESPONSE_SELECTION

    returnValue = 3
    if (parsed_args.response != None):
        # The user has provided a response on the command line
        # to avoid having to present a UI, for test automation
        response = str.lower(parsed_args.response)
        if response == "use":
            returnValue = 1
        elif response == "replace":
            returnValue = 2
        else:
            print("Invalid -r option {}".format(parsed_args.response))
            print('Valid options are "Use" or "Replace"')
            sys.stdout.flush()
            return False

        if uistate is UISTATE_RESPONSE_SELECTION:
            uistate = UISTATE_PROCEED

    sys.stdout.flush()

    tlh = None
    if uistate is UISTATE_LOCATION_SELECTION or \
    uistate is UISTATE_RESPONSE_SELECTION:
        tlh = TCL_LIBRARY_handler(parsed_args.devkit_root)
        top = Tkinter.Tk()

    private_pem_file = os.path.join(outpath, "private.pem")
    rsa_pss_constants_c_file = os.path.join(outpath, "rsa_pss_constants.c")

    while uistate != UISTATE_EXIT and uistate != UISTATE_PROCEED:
        if uistate is UISTATE_LOCATION_SELECTION:
            if not os.path.isdir(outpath):
                try:
                    os.makedirs(outpath)
                    print("Created folder %s" % outpath)
                    sys.stdout.flush()
                    returnValue = 3
                except (OSError, IOError) as exception:
                    print("Unable to create path {}; error {}. Exit!\n".format(outpath,
                                                                        exception.errno))
                    sys.stdout.flush()
                    return False
            newpath = askdirectory(top, outpath)
            if newpath is "":
                # The directory selection has been cancelled
                # Cancel is exit
                print("Cancelled\n")
                sys.stdout.flush()
                return False
                
            if not os.path.isdir(newpath):
                try:
                    os.makedirs(newpath)
                    print("Created folder %s" % newpath)
                    sys.stdout.flush()
                    returnValue = 3
                except (OSError, IOError) as exception:
                    print("Unable to create path {}; error {}. Exit!\n".format(newpath,
                                                                        exception.errno))
                    sys.stdout.flush()
                    return False

            outpath = newpath
            sys.stdout.flush()
            private_pem_file = os.path.join(outpath, "private.pem")
            rsa_pss_constants_c_file = os.path.join(outpath,
                "rsa_pss_constants.c")

            if (parsed_args.response is None):
                uistate = UISTATE_RESPONSE_SELECTION
            else:
                uistate = UISTATE_PROCEED

        elif uistate is UISTATE_RESPONSE_SELECTION:
            if os.path.isfile(private_pem_file) and \
                os.path.isfile(rsa_pss_constants_c_file):
                # There are existing files to use or replace.
                # The above is the minimum set.
                # Ask the user what to do via a dialog
                bb = show_button_box(top, outpath)
                returnValue = 0
                while returnValue is 0:
                    returnValue = bb.returnValue()
                if returnValue == -1:
                    # Cancelling
                    if parsed_args.folder_for_rsa_files == None:
                        # Back to location selection UI
                        uistate = UISTATE_LOCATION_SELECTION
                    else:
                        # Folder given so no location  selectio UI
                        # Cancel is exit
                        print("Cancelled\n")
                        sys.stdout.flush()
                        return False
                else:
                    uistate = UISTATE_PROCEED
            else:
                returnValue = 3
                uistate = UISTATE_PROCEED

    if tlh is not None:
        tlh.close()

    if not os.path.isdir(outpath):
        try:
            os.makedirs(outpath)
            print("Created folder %s" % outpath)
            returnValue = 3
        except (OSError, IOError) as exception:
            print("Unable to create path {}; error {}. Exit!\n".format(outpath,
                                                                exception.errno))
            return False

    # Set up the file specs for the files to of interest for archive/create
    filelist = []
    private_pem_file = os.path.join(outpath, "private.pem")
    filelist.append(private_pem_file)
    public_pem_file = os.path.join(outpath, "public.pem")
    filelist.append(public_pem_file)
    dfu_private_key_file = os.path.join(outpath, "dfu-private.key")
    filelist.append(dfu_private_key_file)
    dfu_public_key_file = os.path.join(outpath, "dfu-public.key")
    filelist.append(dfu_public_key_file)
    rsa_pss_constants_c_file = os.path.join(outpath, "rsa_pss_constants.c")
    filelist.append(rsa_pss_constants_c_file)
    
    # At this point the returnValue should be:
    # 1 to use (known to already exist),
    # 2 to replace (known to already exist), or
    # 3 to create

    if returnValue >= 2:
        # If we are to replace or create we need SecurityCmd.exe,
        # python.exe and gen_rsa_pss_constants.py
        scexe = os.path.join(parsed_args.devkit_root, "tools", "bin",
            "SecurityCmd.exe")
        if not os.path.isfile(scexe):
            print("{} does not exist\n".format(scexe))
            print("Exiting!\n")
            sys.stdout.flush()
            return False
        python_exe = os.path.join(parsed_args.devkit_root,'tools','python27',
            'python.exe')
        if not os.path.isfile(python_exe):
            print("{} does not exist\n".format(python_exe))
            print("Exiting!\n")
            sys.stdout.flush()
            return False
        gen_rsa_pss_constants_py = os.path.join(parsed_args.devkit_root,
            'tools', 'gen_rsa_pss_constants.py')
        if not os.path.isfile(gen_rsa_pss_constants_py):
            print("{} does not exist\n".format(gen_rsa_pss_constants_py))
            print("Exiting!\n")
            sys.stdout.flush()
            return False
    
    if returnValue == 2:
        # Going to replace so archive what we already have
        zip_filename = nowstring + ".zip"
        zip_filespec = os.path.join(outpath, zip_filename)
        # Create the archive for writing "deflated" (compressed) files to
        try:
            with zipfile.ZipFile(zip_filespec, mode="w",
                compression=zipfile.ZIP_DEFLATED) as zip:
                for listfile in filelist:
                    if os.path.isfile(listfile):
                        print("Adding {} to archive {}".format(listfile,
                            zip_filespec))
                        zip.write(listfile, os.path.basename(listfile))
        except (OSError, IOError) as exception:
            print("Error with zip file {}; error {}, {}\n" \
                .format(zip_filespec, exception.errno,
                    os.strerror(exception.errno)))
            print("Exiting!\n")
            sys.stdout.flush()
            return False

        try:
            # Having archived all the files without an error, delete them
            for listfile in filelist:
                # Ensure the file is not read-only before trying to delete it
                os.chmod(listfile, stat.S_IWRITE)
                os.remove(listfile)
                print("Deleted {}".format(listfile))
        except (OSError, IOError) as exception:
            print("Error deleting file; error {}, {}\n" \
                .format(exception.errno, os.strerror(exception.errno)))
            restore_archive(nowstring, outpath, filelist)
            print("Exiting!\n")
            sys.stdout.flush()
            return False
        
        sys.stdout.flush()
        # Having archived and deleted the original files, now as for create
        returnValue = 3

    if returnValue == 3:
        # Create the required files
        cmd_line = [scexe, "-product", "hyd", "creatersakey", "2048", "F4", \
            private_pem_file, public_pem_file]
        print("Invoking '%s'" % " ".join(cmd_line))
        sys.stdout.flush()
        result = subprocess.call(cmd_line)
        if result != 0:
            print("'%s' failed" % " ".join(cmd_line))
            restore_archive(nowstring, outpath, filelist)
            print("Exiting!\n")
            sys.stdout.flush()
            return False
        if not os.path.isfile(private_pem_file):
            print("Failed to create '%s'" % private_pem_file)
            restore_archive(nowstring, outpath, filelist)
            print("Exiting!\n")
            sys.stdout.flush()
            return False
        if not os.path.isfile(public_pem_file):
            print("Failed to create '%s'" % public_pem_file)
            restore_archive(nowstring, outpath, filelist)
            print("Exiting!\n")
            sys.stdout.flush()
            return False

        print("'%s' created" % private_pem_file)
        print("'%s' created" % public_pem_file)

        cmd_line = [scexe, "-product", "hyd", "pem2dfukey", "prv", \
            private_pem_file, dfu_private_key_file]
        print("Invoking '%s'" % " ".join(cmd_line))
        sys.stdout.flush()
        result = subprocess.call(cmd_line)

        # We don't care whether that failed or not as the dfu_private_key_file
        # is not used for any other processing

        if os.path.isfile(dfu_private_key_file):
            print("'%s' created" % dfu_private_key_file)

        cmd_line = [scexe, "-product", "hyd", "pem2dfukey", "pub", \
            public_pem_file, dfu_public_key_file]
        print("Invoking '%s'" % " ".join(cmd_line))
        sys.stdout.flush()
        result = subprocess.call(cmd_line)
        if result != 0:
            print("'%s' failed" % " ".join(cmd_line))
            restore_archive(nowstring, outpath, filelist)
            print("Exiting!\n")
            sys.stdout.flush()
            return False
        if not os.path.isfile(dfu_public_key_file):
            print("Failed to create '%s'" % dfu_public_key_file)
            restore_archive(nowstring, outpath, filelist)
            print("Exiting!\n")
            sys.stdout.flush()
            return False

        print("'%s' created" % dfu_public_key_file)

        cmd_line = [python_exe, gen_rsa_pss_constants_py, "-i", \
            dfu_public_key_file]
        print("Invoking '%s'" % " ".join(cmd_line))
        sys.stdout.flush()
        result = subprocess.call(cmd_line)
        if result != 0:
            print("'%s' failed" % " ".join(cmd_line))
            restore_archive(nowstring, outpath, filelist)
            print("Exiting!\n")
            sys.stdout.flush()
            return False
        if not os.path.isfile(rsa_pss_constants_c_file):
            print("Failed to create '%s'" % rsa_pss_constants_c_file)
            restore_archive(nowstring, outpath, filelist)
            print("Exiting!\n")
            sys.stdout.flush()
            return False

        print("'%s' created" % rsa_pss_constants_c_file)

    # All the required files are now in place
    # Find the rsa_pss_constants.c in the workspace and replace it
    ws_projects = Workspace(parsed_args.workspace).parse()
    for project in ws_projects.keys():
        result = process_project(build_runner, parsed_args,
            ws_projects[project].filename,
            rsa_pss_constants_c_file, nowstring)
        if result < 0:
            # An error has occured
            restore_archive(nowstring, outpath, filelist)
            sys.stdout.flush()
            return False
        elif result > 0:
            print("\nPlease rebuild library project %s and your application" \
                % ws_projects[project].filename)
            sys.stdout.flush()
            break
    
    if result == 0:
        print("\nUnable to find {} in any of the projects".format(
            rsa_pss_constants_c_file))
        sys.stdout.flush()
        return False

    sys.stdout.flush()
    return True

if __name__ == '__main__':
    if not main(sys.argv[1:]):
        sys.exit(1)
    sys.exit(0)

