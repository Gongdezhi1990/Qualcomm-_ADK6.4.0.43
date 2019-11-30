#!/usr/bin/env python
# Copyright (c) 2017-2018 Qualcomm Technologies International, Ltd.
#   
"""
Build a flash image file from the current workspace.
    The parameters are:
    buildFlashImage.py <devkit root> <workspace>

    -k --devkit_root    Specifies path to the root folder of the devkit to use.
    -w --workspace      Specifies workspace file to use.
    -n --nowstring      Optional folder name to use instead of nowstring.
    -a --aes_128_file   Optional aes file for software encryption.
    -e --encrypted      Optional encryption settings.
                        'False': image is unencrypted.
                        'True': image is encrypted
                            If -a is also set software encryption is used.
                            If -a is not given hardware encryption is used,
                            then -d option is mandatory.
    -d, --device_uri    Specifies the device URI
                        E.g. -d device://trb/usb2trb/152672/csra68100
                        Required for hardware encryption
"""

# Python 2 and 3
from __future__ import print_function

import sys
import argparse
import os
from datetime import datetime
import shutil
import maker.prepare_single_image as psi
from workspace_parse.workspace import Workspace
from maker.build import build_configs as build_configs

try:
    import Tkinter
    import tkFileDialog
except ImportError:
    import tkinter as Tkinter
    import tkinter.filedialog as tkFileDialog


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
                              text="Please select whether the file is to be \
                              encrypted").pack(padx=10, pady=10)
        # Privide an area at the bottom of the widget with "Proceed"
        # and "Cancel" buttons
        self.button_frame = Tkinter.Frame(self.top_level)
        self.button_frame.pack(fill=Tkinter.X)
        self.unencrypted_button = Tkinter.Radiobutton(self.button_frame,
                                                      text="Unencrypted",
                                                      variable=self.var,
                                                      value=1)
        self.unencrypted_button.pack(anchor=Tkinter.W)
        self.encrypted_button = Tkinter.Radiobutton(self.button_frame,
                                                    text="Encrypted",
                                                    variable=self.var,
                                                    value=2)
        self.encrypted_button.pack(anchor=Tkinter.W)
        Tkinter.Button(self.button_frame, text="Proceed", command=self._proceed) \
            .pack(side=Tkinter.LEFT)
        Tkinter.Button(self.button_frame, text="Cancel", command=self._cancel) \
            .pack(side=Tkinter.RIGHT)

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
        Provides the mechanism for the user of the instance to determine
        whether the user has elected to proceed or cancel.
        """
        self.root.wait_window(self.top_level)
        return self.value


class show_encryption_type_selection_ui(object):
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
        self.top_level.title("Select encryption type")
        line1 = "Please select encryption type: hardware or software.\n"
        line2 = "Hardware encryption requires a device attached.\n"
        line3 = "Software encryption requires the use of an AES-128 key file.\n"
        label = Tkinter.Label(self.top_level, text=line1 + line2 + line3) \
            .pack(padx=10, pady=10)
        # Privide an area at the bottom of the widget with "Proceed"
        # and "Cancel" buttons
        self.button_frame = Tkinter.Frame(self.top_level)
        self.button_frame.pack(fill=Tkinter.X)
        self.unencrypted_button = Tkinter.Radiobutton(self.button_frame,
                                                      text="Hardware",
                                                      variable=self.var,
                                                      value=1)
        self.unencrypted_button.pack(anchor=Tkinter.W)
        self.encrypted_button = Tkinter.Radiobutton(self.button_frame,
                                                    text="Software",
                                                    variable=self.var,
                                                    value=2)
        self.encrypted_button.pack(anchor=Tkinter.W)
        Tkinter.Button(self.button_frame, text="Proceed", command=self._proceed) \
            .pack(side=Tkinter.LEFT)
        Tkinter.Button(self.button_frame, text="Cancel", command=self._cancel) \
            .pack(side=Tkinter.RIGHT)

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
        Provides the mechanism for the user of the instance to determine
        whether the user has elected to proceed or cancel.
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
        # tools\python27\tcl\tcl8.5 folder of the devkit for Tkinter to use Tcl
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


def select_encryption_type(top):
    # Use Tkinter to present the user interface. While still ongoing, the
    # returnValue is 0, -1 indicates cancellation, and 1 indicates proceed
    cb = show_encryption_type_selection_ui(top)
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


def search_config_keys_for_chip_type(config_keys):
    """
    Process the configuration keys in what may be the app/p1.
    """
    if "DBG_CORE" in config_keys and \
        config_keys["DBG_CORE"] == "app/p1" and \
        "CHIP_TYPE" in config_keys:
        return config_keys["CHIP_TYPE"]
    return None


def search_project_for_chip_type(devkit_root, project, workspace_file):
    """
    Look through the configurations in a project for what may be the
    app/p1 and, if found, process the configuration keys.
    """
    import maker.parse_proj_file as pproj
    proj_parser = pproj.Project(project, devkit_root, workspace_file)
    config_list = proj_parser.get_configurations()
    for config in config_list:
        # Could be the apps/p1 project
        config_keys = \
            proj_parser.get_properties_from_config(config)
        chip_type = \
            search_config_keys_for_chip_type(config_keys)
        if chip_type:
            return chip_type
    return None


def search_projects_for_chip_type(devkit_root, workspace_file):
    """
    Look for a chip type in the default configuration of the
    app/p1 project file.
    """
    ws_projects = Workspace(workspace_file).parse()
    # Go through the projects in the x2w file
    for project in ws_projects.values():
        # Try and get the chip type from the project
        chip_type = \
            search_project_for_chip_type(devkit_root, project.filename, workspace_file)
        if chip_type:
            # The chip type has been found so return it
            return chip_type
    # The chip type has not been found in the projects
    return None


def chip_supports_hardware_encryption(devkit_root, workspace):
    _CHIPS_WITH_HARDWARE_ENCRYPTION = ["csra68100"]
    chip_type = search_projects_for_chip_type(devkit_root, workspace)
    for chip in _CHIPS_WITH_HARDWARE_ENCRYPTION:
        length = len(chip)
        if chip_type[0:length] == chip:
            return True
    return False


def prompt_user_for_encryption_selection(parsed_args):
    UISTATE_ENCRYPTION_SELECTION      = 0
    UISTATE_ENCRYPTION_TYPE_SELECTION = 1
    UISTATE_ENCRYPTION_FILE_SELECTION = 2
    UISTATE_PROCEED                   = 3
    UISTATE_EXIT                      = -1

    encrypted = None
    hardware_encrypted = None
    software_encryption_file = None
    hardware_encrypt_supported = chip_supports_hardware_encryption(devkit_root=parsed_args.devkit_root, workspace=parsed_args.workspace)
    uistate = UISTATE_ENCRYPTION_SELECTION
    tlh = None

    if not parsed_args.encrypted or not parsed_args.aes_128_file:
        # No encryption options found. A TkInter UI is needed
        # to prompt the user for information about encryption preferences
        tlh = TCL_LIBRARY_handler(parsed_args.devkit_root)
        top = Tkinter.Tk()

    while uistate != UISTATE_EXIT and uistate != UISTATE_PROCEED:
        if uistate == UISTATE_ENCRYPTION_SELECTION:
            if parsed_args.encrypted is None:
                if parsed_args.aes_128_file is None:
                    encrypted = select_encrypted(top)
                else:
                    encrypted = True
            else:
                lower_enc = parsed_args.encrypted.lower()
                if lower_enc == 'encrypted' or lower_enc == 'true':
                    encrypted = True
                else:
                    encrypted = False

            if encrypted is not None:
                if encrypted is True:
                    uistate = UISTATE_ENCRYPTION_TYPE_SELECTION
                else:
                    uistate = UISTATE_PROCEED
            else:
                uistate = UISTATE_EXIT

        elif uistate == UISTATE_ENCRYPTION_TYPE_SELECTION:
            if encrypted is True:
                if (parsed_args.aes_128_file is None) and hardware_encrypt_supported:
                    if parsed_args.encrypted is None:
                        hardware_encrypted = select_encryption_type(top)
                    else:
                        hardware_encrypted = True
                else:
                    hardware_encrypted = False

                if hardware_encrypted is not None:
                    if hardware_encrypted is True:
                        uistate = UISTATE_PROCEED
                    else:
                        uistate = UISTATE_ENCRYPTION_FILE_SELECTION
                else:
                    uistate = UISTATE_ENCRYPTION_SELECTION
            else:
                # Belt and braces as should never happen
                uistate = UISTATE_ENCRYPTION_SELECTION

        elif uistate == UISTATE_ENCRYPTION_FILE_SELECTION:
            if encrypted is True and hardware_encrypted is False:
                if parsed_args.aes_128_file is None:
                    software_encryption_file = \
                        select_encryption_file(top, parsed_args.workspace)
                else:
                    software_encryption_file = parsed_args.aes_128_file

                if software_encryption_file is not None and software_encryption_file != '':
                    if os.path.isfile(software_encryption_file):
                        uistate = UISTATE_PROCEED
                    else:
                        print("{} not found\n").format(software_encryption_file)
                        uistate = UISTATE_EXIT
                    sys.stdout.flush()
                else:
                    uistate = UISTATE_ENCRYPTION_SELECTION
            else:
                # Belt and braces as should never happen
                uistate = UISTATE_ENCRYPTION_SELECTION

    if uistate == UISTATE_PROCEED:
        if encrypted is True:
            if hardware_encrypted is True:
                print("Hardware encrypted\n")
            else:
                print("Software encrypted\n")
                print("Software encryption file is {}\n"
                    .format(software_encryption_file))
        else:
            print("Unencrypted\n")
        sys.stdout.flush()

    if tlh:
        tlh.close()
    if top:
        top.destroy()

    return (uistate == UISTATE_PROCEED, encrypted,
            hardware_encrypted, software_encryption_file)


def parse_args(args):
    """ parse the command line arguments """
    parser = argparse.ArgumentParser(
                description='Build a single flash image file from a previously \
                built workspace')

    parser.add_argument('-k', '--devkit_root',
                        required=True,
                        help='Specifies the path to the root folder of the \
                              devkit to use')

    parser.add_argument('-w', '--workspace',
                        required=True,
                        help='Specifies the workspace file to use')

    parser.add_argument('-n', '--nowstring',
                        help='Optional folder name to use instead of nowstring. \
                            E.g. -n "Fred"')

    parser.add_argument('-e', '--encrypted',
                        default=None,
                        help='Optional, specifies if the file is to be \
                            encrypted or not. Strings (case insensitive) of \
                            "encrypted" or "true" indicate encrypted \
                             Any other strings indicate unencrypted')

    parser.add_argument('-a', '--aes_128_file',
                        default=None,
                        help='Optional, specifies the file containing the \
                            AES-128 software encryption key. \
                            E.g. -a "C:\\encryption\\aes_128.key" \
                            Only significant if -e or the UI has been used to \
                            select encryption. The -a option prevents the \
                            hardware/software encryption selection UI being \
                            shown, and prevents the software encryption file \
                            selection UI being shown. The -e option is not \
                            needed if the -a option is given as with -a \
                            encrypted is assumed')

    parser.add_argument('-d', '--device_uri',
                    default=None,
                    help='specifies the device URI')
    # E.g. -d device://trb/usb2trb/152672/csra68100

    parser.add_argument('-o', '--build_output_folder',
                        default=None,
                        help='Specify location for object files and other build artefacts.')

    return parser.parse_args(args)


def main(args):
    """ main entry point.
        - Processes command line arguments;
        - obtain the input projects' xuv files by processing the projects
          listed in the workspace
        - invoke build_flash_image to create the configuration and build the file
        - if successful, show the filename of the flash image file created
    """
    # workaround remove empoty strings from input when run via QWMDE we get
    #  ['-k', 'C:\\qtil\\ADK_CSRA6810x_WIN_RC5_6.1.67_ext_win_32', '-w', 'C:\\ws\\testTuesday_1\\sinkRC5\\apps\\applications\\sink\\csra68100_d01\\sinkRC5.x2w', '']
    args = list([_f for _f in args if _f])

    parsed_args = parse_args(args)

    build_output_folder = parsed_args.build_output_folder

    if build_output_folder is None:
        build_output_folder = os.getenv("BUILD_OUTPUT_FOLDER")

    # Obtain a string with the current date and time to use
    # for folder and filenames.
    if (parsed_args.nowstring is not None):
        # The user has provided their own folder name
        nowstring = parsed_args.nowstring
    else:
        nowstring = datetime.strftime(datetime.now(), '%Y%m%d%H%M%S')
    # Create a workspace folder image\<nowstring> folder
    if build_output_folder is not None:
        outpath = os.path.join(build_output_folder, "image", nowstring)
    else:
        outpath = os.path.join(os.path.dirname(parsed_args.workspace), "image", nowstring)
    if os.path.isdir(outpath):
        # If exists then must be using -n
        try:
            shutil.rmtree(outpath)
            sys.stdout.flush()
        except OSError as exception:
            print("Unable to delete path {}; error {}. Exit!\n".format(outpath, exception.errno))
            sys.stdout.flush()
            return False
    try:
        os.makedirs(outpath)
    except OSError as exception:
        print("Unable to create path {}; error {}. Exit!\n" \
            .format(outpath, exception.errno))
        sys.stdout.flush()
        return False

    print("Created folder %s" % outpath)

    # Create a workspace folder image\<nowstring>\input folder
    inputpath = os.path.join(outpath, "input")
    try:
        os.makedirs(inputpath)
    except OSError as exception:
        print("Unable to create path {}; error {}. Exit!\n" \
            .format(inputpath, exception.errno))
        sys.stdout.flush()
        return False

    print("Created folder %s" % inputpath)

    # Create a workspace folder image\<nowstring>\output folder
    output_path = os.path.join(outpath, "output")
    try:
        os.makedirs(output_path)
    except OSError as exception:
        print("Unable to create path {}; error {}. Exit!\n" \
            .format(output_path, exception.errno))
        return False

    print("Created folder %s" % output_path)

    print("devkit_root: %s" % (parsed_args.devkit_root))
    print("workspace:   %s" % (parsed_args.workspace))
    sys.stdout.flush()

    # Build and collect the input xuv files from the projects in the workspace
    single_image = psi.PrepareSingleImage(parsed_args.devkit_root,
                                          parsed_args.workspace,
                                          parsed_args.workspace,
                                          build_output_folder=build_output_folder, 
                                          generate_audio_image_xuv=True,
                                          device_uri=parsed_args.device_uri)

    print("Collecting all projects")
    collected_projects = \
        single_image.collect_all_workspace_projects(parsed_args.workspace,
                                                    inputpath, build_output_folder=build_output_folder)

    if not collected_projects or not single_image.projects_exist():
        print("Nothing for the image found in workspace. Exit!\n")
        return False

    success, encrypted, hardware_encrypted, software_encryption_file = \
        prompt_user_for_encryption_selection(parsed_args)

    if success:

        single_image.create_flash_image(devkit=parsed_args.devkit_root,
                                        output_path=output_path,
                                        encrypted=encrypted,
                                        hardware_encrypted=hardware_encrypted,
                                        software_encryption_file=software_encryption_file)

    else:
        print("Cancelled")
        print("Deleting {}".format(outpath))
        shutil.rmtree(outpath)

    sys.stdout.flush()
    return success


def remove_empty_string_args(arguments_array):
    """
    If the arguments array from commandline has an element that is an empty
    string, remove it from the array. ArgumentParser throws an error otherwise.
    """
    for i in range(0, len(arguments_array)):
        if arguments_array[i] == '':
            arguments_array.pop(i)
    return arguments_array


if __name__ == '__main__':
    argArray = remove_empty_string_args(sys.argv[1:])

    if not main(argArray):
        sys.exit(1)
    sys.exit(0)
