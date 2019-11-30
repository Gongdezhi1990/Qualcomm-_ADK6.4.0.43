# Copyright (c) 2016 - 2019 Qualcomm Technologies International, Ltd 
# 
"""
Provides an interface between Heracles and pydbg on a coredump
    The parameters are:
    pydbgCoredump.py <devkit root>

    -k --devkit_root    Specifies path to the root folder of the devkit to use.
"""

# Python 2 and 3
from __future__ import print_function

import sys
import argparse
import subprocess
import os

try:
    import Tkinter
    import tkFileDialog
    import tkMessageBox
except ImportError:
    import tkinter as Tkinter
    import tkinter.filedialog as tkFileDialog
    import tkinter.messagebox as tkMessageBox

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

class show_button_box(object):
    def __init__(self, root, initial_path):
        """
        Create a Tkinter button widget for the user to:
        - select a coredump file (required)
        - select an apps1 elf file (optional)
        - select an audio elf file (optional)
        - proceed, or
        - cancel
        """
        self.root = root
        self.value = 0
        self._initial_path = initial_path
        self._xcd_filename = ''
        self._apps1_filename = ''
        self._audio_filename = ''
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
        self.top_level.title("Load coredump into pydbg")
        label = Tkinter.Label(self.top_level,
            text="Please select a coredump file and subsytem(app and/or audio)elf file(s) needs debugging") \
            .pack(padx=10, pady=10)
        # Privide an area at the bottom of the widget with "Proceed"
        # and "Cancel" buttons
        button_frame = Tkinter.Frame(self.top_level)
        button_frame.pack(fill=Tkinter.X)
        coredump_button = Tkinter.Button(button_frame,
            text="Select the coredump file to load (required)",
            command=self._askXcdFilename).pack(fill=Tkinter.X)
        apps1_button = Tkinter.Button(button_frame,
            text="Select the matching app_p1 elf file (optional)",
            command=self._askApps1ElfFilename).pack(fill=Tkinter.X)
        audio_button = Tkinter.Button(button_frame,
          text="Select the matching audio elf file (optional)",
          command=self._askAudioElfFilename).pack(fill=Tkinter.X)
        select_button = Tkinter.Button(button_frame, text="Proceed",
            command=self._proceed).pack(side=Tkinter.LEFT)
        cancel_button = Tkinter.Button(button_frame, text="Cancel",
            command=self._cancel).pack(side=Tkinter.RIGHT)

    def _askXcdFilename(self, event=None):
        """
        Present the user with a file selection dialog box from which to
        find and select the coredump xcd file.
        """
        file_opt = {}
        file_opt['defaultextension'] = '.xcd'
        file_opt['filetypes'] = \
            [('coredump files', '.xcd'), ('all files', '.*')]
        if self._xcd_filename == '':
            file_opt['initialdir'] = self._initial_path
            file_opt['initialfile'] = self._xcd_filename
        else:
            file_opt['initialdir'] = os.path.dirname(self._xcd_filename)
            file_opt['initialfile'] = os.path.basename(self._xcd_filename)
        file_opt['parent'] = self.root
        file_opt['title'] = 'Please select a coredump file'
        filename = tkFileDialog.askopenfilename(**file_opt)
        if filename:
            self._xcd_filename = filename
            if self._apps1_filename == '' and self._audio_filename == '':
                self._initial_path = os.path.dirname(self._xcd_filename)

    def _askApps1ElfFilename(self, event=None):
        """
        Present the user with a file selection dialog box from which to
        find and select the apps1 elf file, if required.
        """
        file_opt = {}
        file_opt['defaultextension'] = '.elf'
        file_opt['filetypes'] = [('elf files', '.elf'), ('all files', '.*')]
        if self._apps1_filename == '':
            file_opt['initialdir'] = self._initial_path
            file_opt['initialfile'] = self._apps1_filename
        else:
            file_opt['initialdir'] = os.path.dirname(self._apps1_filename)
            file_opt['initialfile'] = os.path.basename(self._apps1_filename)
        file_opt['parent'] = self.root
        file_opt['title'] = \
            'Please select the matching app_p1 elf file'
        filename = tkFileDialog.askopenfilename(**file_opt)
        if filename:
            self._apps1_filename = filename

    def _askAudioElfFilename(self, event=None):
        """
        Present the user with a file selection dialog box from which to
        find and select the audio elf file, if required.
        """
        file_opt = {}
        file_opt['defaultextension'] = '.elf'
        file_opt['filetypes'] = [('elf files', '.elf'), ('all files', '.*')]
        if self._audio_filename == '':
            file_opt['initialdir'] = self._initial_path
            file_opt['initialfile'] = self._audio_filename
        else:
            file_opt['initialdir'] = os.path.dirname(self._audio_filename)
            file_opt['initialfile'] = os.path.basename(self._audio_filename)
        file_opt['parent'] = self.root
        file_opt['title'] = \
            'Please select the matching audio elf file'
        filename = tkFileDialog.askopenfilename(**file_opt)
        if filename:
            self._audio_filename = filename

    def _proceed(self, event=None):
        """
        Actions to perform when the user has selected a coredump file.
        """
        if self._xcd_filename == '':
            tkMessageBox.showwarning("No coredump selected",
                                     "Please select a coredump file")
        else:
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

    def getXcdFilename(self):
        return self._xcd_filename

    def getApps1Filename(self):
        return self._apps1_filename

    def getAudioFilename(self):
        return self._audio_filename

class show_error_and_exit(object):
    def __init__(self, root=None, tlh=None, message=None):
        """
        Display an error message in a widget with an "Exit" button until the
        button is clicked.
        """
        self.root = root
        self.tlh = tlh
        # Create the widget
        self.top_level = Tkinter.Toplevel(self.root)
        # Associate this window with a parent window
        self.top_level.transient(self.root)
        # Make sure no mouse or keyboard events are sent to the wrong window
        self.top_level.grab_set()
        # Set the Return key equivalent to the "Exit" button being clicked
        self.top_level.bind("<Return>", self._cancel)
        # Set the Escape key equivalent to the "Exit" button being clicked
        self.top_level.bind("<Escape>", self._cancel)
        # Give our widget a simple title
        self.top_level.title("Error!")
        # Tell the user the cause of the error, if told
        if message:
            Tkinter.Label(self.top_level, text=message).pack(padx=5, pady=5)

        error_frame = Tkinter.Frame(self.top_level)
        error_frame.pack(side=Tkinter.TOP, padx=5, pady=5)
        # Give the widget an area with an "Exit" button in it
        button_frame = Tkinter.Frame(self.top_level)
        button_frame.pack(side=Tkinter.BOTTOM)
        cancel_button = Tkinter.Button(button_frame, text="Exit",
            command=self._cancel)
        cancel_button.pack(side=Tkinter.RIGHT)
        # Sit there until they click the button
        self.root.mainloop()

    def _cancel(self, event=None):
        """
        The button has been clicked. Restore the TCL_LIBRARY environment
        variable to its former glory and exit.
        """
        self.top_level.destroy()
        self.tlh.close()
        sys.exit(-1)

def parse_args(args):
    """ parse the command line arguments """
    parser = argparse.ArgumentParser(description =
        'Run pydbg on a coredump selected from those captured')

    parser.add_argument('-k', '--devkit_root',
                        required=True,
                        help='specifies the path to the root folder of the \
                              devkit to use')
    parser.add_argument('-w', '--workspace',
                        default=None,
                        help='specifies the workspace file to use')
    return parser.parse_args(args)

def main(args):
    """ main entry point.
        - Processes command line arguments
        - Saves and replacves the TCL_LIBRARY envirnment variable
        - Checks that all elements are in the expected places, showing an
            error message widget until acknowledged, then restoring the
            TCL_LIBRARY envirnment variable and exiting, if not
        - Presents the user with a button box UI from which to select:
            - The coredump file to pass to pydbg (required)
            - The apps1 eld file (optional)
            - The audio elf file (optional)
            - and buttons to cancel or proceed
    """
    parsed_args = parse_args(args)
    tlh = TCL_LIBRARY_handler(parsed_args.devkit_root)
    root = Tkinter.Tk()

    # Check that everything is where it is expected to be. Exit if not.
    # Find the  workspace coredump folder
    coredump_path = os.path.join(os.path.dirname(parsed_args.workspace), "coredump")

    if not os.path.isdir(coredump_path):
        # Try the workspace folder instead
        coredump_path = os.path.dirname(parsed_args.workspace)
        
    if not os.path.isdir(coredump_path):
        show_error_and_exit(root, tlh, coredump_path + " directory not found")

    # Find the python executable
    python_spec = os.path.join(parsed_args.devkit_root, "tools", "python27",
        "python.exe")

    if not os.path.isfile(python_spec):
        show_error_and_exit(root, tlh, python_spec + " file not found")

    # Find the pydbg script
    pydbg_spec = os.path.join(parsed_args.devkit_root, "apps", "fw",
        "tools", "pydbg.py")

    if not os.path.isfile(pydbg_spec):
        show_error_and_exit(root, tlh, pydbg_spec + " file not found")

    # Use Tkinter to present the user interface. While still ongoing, the
    # returnValue is 0, -1 indicates cancellation, and 1 indicates proceed
    bb = show_button_box(root, coredump_path)
    returnValue = 0
    while returnValue is 0:
        returnValue = bb.returnValue()

    if returnValue == 1:
        # We have a coredump file to proceed with invoking pydbg with
        xcd_filename = bb.getXcdFilename().replace('/', os.path.sep)
        # See if we also have any elf files selected, too
        apps1_filename = bb.getApps1Filename().replace('/', os.path.sep)
        audio_filename = bb.getAudioFilename().replace('/', os.path.sep)
        elf_specs = ''
        if apps1_filename != '':
            elf_specs += 'apps1:'
            elf_specs += apps1_filename
            if audio_filename != '':
                # We have both elf files, so separate with a comma
                elf_specs +=  ','
        if audio_filename != '':
            elf_specs += 'audio:'
            elf_specs += audio_filename

        # Open a new command window to run the pydbg session in.
        args = ["cmd", "/c", "start", python_spec, "-i", pydbg_spec,
            "-d", "xcd3:" + xcd_filename]
        if elf_specs != '':
            args += ["-f", elf_specs]
        subprocess.call(args)

    # Tidy up and be done
    tlh.close()
    sys.exit(0)
       
if __name__ == '__main__':
    if not main(sys.argv[1:]):
        sys.exit(1)
    sys.exit(0)
