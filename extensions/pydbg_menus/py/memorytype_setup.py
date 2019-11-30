# Copyright (c) 2019 Qualcomm Technologies International, Ltd.
"""
Will allow the user to specify a bespoke memory chip type option to be used.
Because the project definition file can not be updated automatically by the script at present
then what we have is a file always added to the project - by default there is no contents,
but when a user selects another memory option then this file is overwritten with the bespoke contents.

    The parameters are:
    memorytype_setup.py <devkit root> <workspace>

    -k --devkit_root    Specifies path to the root folder of the devkit to use.
    -w --workspace      Specifies workspace file to use.
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


try:
    import Tkinter
    import tkFileDialog
except ImportError:
    import tkinter as Tkinter
    import tkinter.filedialog as tkFileDialog

DEFAULT_FILE_LOCATION = "tools"
DEFAULT_FILE_NAME = "default_memory_type_definition.htf"
DEFAULT_FILE_LINE1= "This is the default setting - no specific memory type overrides"

FILE_SPEC_FOR_CURATOR = "file=curator4"

chiptype_lookup_dir = {
            "QCC512x":"1283_qcc512x",
            "CSRA68015":"1390_csra68105", 
            "CSRA68100":"1102_csra68100"
            }




class memory_option_bean(object):
    def __init__(self, folder, filename, fullpath, text_line):
        """
        Holder to contain the memory option available
        """
        self.folder = folder
        self.filename = filename
        self.fullpath = fullpath
        self.text_line = text_line

    def __str__(self):
        return "memory_option_bean:[folder:{}, filename:{}, fullpath:{}, text_line:{}]".format(self.folder, self.filename,
               self.fullpath, self.text_line)


class show_memory_type_selection_ui(object):
    def __init__(self, root, memory_options_list):
        """
        Create a Tkinter radiobutton widget for the user to:
        - select bespoke memory type
        - proceed, or
        - cancel
        """
        self.root = root
        self.memory_options_list = memory_options_list
        self.value = 0
        self.buttons_dict= {}


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
        self.top_level.title("Select memory type")
        line1 = "Please select specific Manufacturer Memory type.\n"
        line2 = "Select default if no specific one listed.\n"
        label = Tkinter.Label(self.top_level, text=line1 + line2) \
            .pack(padx=10, pady=10)
        # Privide an area at the bottom of the widget with "Proceed"
        # and "Cancel" buttons
        self.button_frame = Tkinter.Frame(self.top_level)
        self.button_frame.pack(fill=Tkinter.X)

        # Now we need something to loop around each Memory option

        button_no = 0
        for option in memory_options_list:
            button_no = button_no + 1
            self.buttons_dict[button_no] = option
            filename = option.filename
            if filename.endswith(".htf"):
                filename = filename[:-len(".htf")]
            temp_button = Tkinter.Radiobutton(self.button_frame,
                                                      text=option.folder + " - " + filename,
                                                      variable=self.var,
                                                      value=button_no)
            temp_button.pack(anchor=Tkinter.W)


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
        but_no = self.var.get()
        return self.buttons_dict[but_no]

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



def select_memory_type(top, memory_options_list):
    # Use Tkinter to present the user interface. While still ongoing, the
    # returnValue is None indicates cancellation, and value indicates proceed
    cb = show_memory_type_selection_ui(top, memory_options_list)
    returnValue = 0
    while returnValue is 0:
        returnValue = cb.returnValue()

    if returnValue == 1:
        # We are proceeding
        print("Selected memory type  {}".format(cb.getSelection().text_line))
        return cb.getSelection()
    else:
        return None


def determine_chip_type(devkit_root, project, workspace_file):
    # Do not know how to do this at the moment so assume QCC512x
    return "QCC512x"



def search_project_for_chip_type(devkit_root, project, workspace_file):
    """
    Lets see what memory options are available
    """
    import maker.parse_proj_file as pproj

    chip_type = determine_chip_type (devkit_root, project, workspace_file)

    path_segment_to_memory_configs = chiptype_lookup_dir[chip_type]

    if not path_segment_to_memory_configs:
        print("Do not recognise chip type [{}] - Can not continue".format(chip_type))
        return

    # Lets find any memory options
    #  In dir  <adk install>\tools\bin\qspi_config\1283_qcc512x
    # walk
    path_for_memtypes =  os.path.join(devkit_root, "tools", "bin", "qspi_config", path_segment_to_memory_configs)

    memory_options_list = []
    option = memory_option_bean("", DEFAULT_FILE_NAME, os.path.join(devkit_root, DEFAULT_FILE_LOCATION), DEFAULT_FILE_LINE1)
    memory_options_list.append(option)
    for filepath, subdirs, files in os.walk(path_for_memtypes):
        for sub in subdirs:
            path_for_memtype_specific = os.path.join(filepath,sub)
            for filepathSub, subdirsSub, filesSub in  os.walk(path_for_memtype_specific):
                for  fileSub in filesSub:
                    # Check options
                    if fileSub.endswith(".htf"):
                        option = memory_option_bean(sub, fileSub, filepathSub,"Ref file :" + os.path.join(sub,fileSub))
                        memory_options_list.append(option)
                    else:
                        # Skip this one
                        pass


    return memory_options_list


def search_projects_for_chip_type(devkit_root, workspace_file):
    """
    Look for what memory type options are available
    """
    ws_projects = Workspace(workspace_file).parse()
    # Go through the projects in the x2w file and find dev_cfg_filesystem
    for project in ws_projects.values():
        # Only going to work on project 'dev_cfg_filesystem'
        if "dev_cfg_filesystem" == project.name:
            # Try and get the chip type from the project
            memory_options_list = \
                search_project_for_chip_type(devkit_root, project.filename, workspace_file)
            if memory_options_list:
                # The chip type has been found so return it
                return memory_options_list
    # The chip type has not been found in the projects
    return None



def prompt_user_for_memorytype_selection(parsed_args, devkit_root, workspace):

    memory_options_list = search_projects_for_chip_type(devkit_root, workspace)

    tlh = None

    tlh = TCL_LIBRARY_handler(parsed_args.devkit_root)
    top = Tkinter.Tk()
    mem_option = select_memory_type(top, memory_options_list)

    if tlh:
        tlh.close()
    if top:
        top.destroy()

    return mem_option


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


    sys.stdout.flush()


    mem_option = \
        prompt_user_for_memorytype_selection(parsed_args,devkit_root=parsed_args.devkit_root, workspace=parsed_args.workspace)

    if mem_option:
        # Need to copy chosen file to 1 dir update
        src =  os.path.join(mem_option.fullpath,mem_option.filename)
        dest =  os.path.join(parsed_args.workspace,"..","..","common","user_selected_specific_memory_type.htf")
        print("Reading in details from file {}".format(src))
        input_file = open(src)
        contents = input_file.read()
        out_file=open(dest, mode="w")
        out_file.write(FILE_SPEC_FOR_CURATOR)
        out_file.write("\n")
        out_file.write("# \n")
        out_file.write("# Auto generated from memory type selection memu\n")
        out_file.write("# \n")
        out_file.write("# Reference file listed below\n")
        out_file.write("# \n")
        out_file.write("# ")
        out_file.write(mem_option.text_line)
        out_file.write("\n")
        out_file.write("# \n")
        out_file.write("# \n")
        out_file.write(contents)
        out_file.close()
        input_file.close()
        input_file.close()

        # shutil.copyfile(src, dest)

    else:
        print("Cancelled")

    sys.stdout.flush()
    return 0


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
