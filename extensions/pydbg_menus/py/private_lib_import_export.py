#!/usr/bin/env python
# Copyright (c) 2018 Qualcomm Technologies International, Ltd.
#   

# Python 2 and 3
from __future__ import print_function

import os
import sys
import errno
import shutil as sh
import argparse
from six.moves import tkinter_tkfiledialog as tkFileDialog
from six.moves import tkinter_messagebox as tkMessageBox
from six.moves.tkinter import Tk, TclError

class UI(object):
    def __init__(self, args):
        self.root = Tk()

        # Make it invisible
        # - 0 size, top left corner, transparent.
        self.root.attributes('-alpha', 0.0)
        self.root.geometry('0x0+0+0')

        try:
            qmde_icon = os.path.join(args.devkit, 'res', 'q_logo.ico')
            self.root.iconbitmap(bitmap=qmde_icon)
        except TclError:
            pass

        # "Show" window again and lift it to top so it can get focus,
        # otherwise dialogs will end up behind other windows
        self.root.deiconify()
        self.root.lift()
        self.root.focus_force()

    def selectFolder(self, **options):
        """
        Show a folder selection window for the user to import/export
        private pre-built libraries
        """
        return tkFileDialog.askdirectory(parent=self.root, **options)

    def selectFile(self, **options):
        """
        Show a folder selection window for the user to import/export
        private pre-built libraries
        """
        return tkFileDialog.askopenfilename(parent=self.root, **options)

    def success(self, action):
        message = 'Library {} operation succesful'.format(action.lower())
        print(message)
        tkMessageBox.showinfo(message=message)

    def cancel(self, action):
        message = 'Library {} operation cancelled'.format(action.lower())
        print(message)
        tkMessageBox.showinfo(message=message)
        sys.exit(0)

    def fail(self, msg):
        print(msg)
        tkMessageBox.showerror(message=msg)
        sys.exit(1)

    def confirmAction(self, action, header, archive):
        proceed = True
        header_base = os.path.basename(header)
        archive_base = os.path.basename(archive)
        info = "About to {} files:\n--{}\n--{}\n\n" \
            .format(action.lower(), header_base, archive_base)
        question = "Proceed?"
        print(info + question)
        proceed = tkMessageBox.askokcancel('Attention!', info + question)
        print(proceed)
        return proceed

    def proceedDstFileExists(self, dst_dir, dst_file):
        dst_file = os.path.join(dst_dir, dst_file)
        proceed = True
        if os.path.isfile(dst_file):
            info = 'File: \n--{}\n\nalready exists at destination' \
                .format(dst_file)
            question = "Proceed?"
            proceed = tkMessageBox.askokcancel('Attention!', info + question)
        return proceed


class Importer(object):
    def __init__(self, args):
        self.ui = UI(args)
        self.find = Finder(args)

    def cancel(self):
        self.ui.cancel('import')

    def success(self):
        self.ui.success('import')

    def importLib(self):
        PRIVATE_LIB_EXTENSION = '.pa'
        HEADER_EXTENSION = '.h'

        src_dir = self.ui.selectFolder(title='Select library to import.',
                                       initialdir=os.path.expanduser("~"))
        if not src_dir:
            self.cancel()

        archive = ''
        header = ''
        for file in os.listdir(src_dir):
            if file.endswith(PRIVATE_LIB_EXTENSION):
                archive = os.path.join(src_dir, file)
            elif file.endswith(HEADER_EXTENSION):
                header = os.path.join(src_dir, file)

        if not header:
            self.ui.fail('Library header not found in selected folder!')

        if not archive:
            self.ui.fail('Library not found in selected folder!')

        if self.ui.confirmAction('import', header, archive):
            self.importHeader(header)
            self.importArchive(archive)
            self.success()
        else:
            self.cancel()

    def importArchive(self, file):
        dst = self.find.privateLibsDir()
        if self.ui.proceedDstFileExists(dst, file):
            self.mkdir_p(dst)
            copy(file, dst)

    def mkdir_p(self, dst):
        """ Equivalent to mkdir -p """
        try:
            os.makedirs(dst)
        except OSError as e:
            if e.errno == errno.EEXIST and os.path.isdir(dst):
                pass
            else:
                raise

    def importHeader(self, file):
        dst = self.find.includeLibsDir()
        if self.ui.proceedDstFileExists(dst, file):
            copy(file, dst)


class Exporter(object):
    def __init__(self, args):
        self.ui = UI(args)
        self.find = Finder(args)

    def cancel(self):
        self.ui.cancel('export')

    def success(self):
        self.ui.success('export')

    def exportLib(self):
        archive = self.selectArchive()
        if not archive:
            self.cancel()

        header = self.selectHeader()
        if not header:
            self.cancel()

        if self.ui.confirmAction('export', header, archive):
            outdir = self.ui.selectFolder(title='Select output folder',
                                          initialdir=self.find.appsParent())
            if not outdir:
                self.cancel()
        else:
            self.cancel()

        export_header = \
            self.ui.proceedDstFileExists(outdir, os.path.basename(header))

        export_archive = \
            self.ui.proceedDstFileExists(outdir, os.path.basename(archive))

        if export_header and export_archive:
            copy(header, outdir)
            copy(archive, outdir)
            self.success()
        else:
            self.cancel()

    def selectHeader(self):
        return self.ui.selectFile(title='Select library header to export',
                                  initialdir=self.find.includeLibsDir())

    def selectArchive(self):
        return self.ui.selectFile(title='Select library archive to export.',
                                  initialdir=self.find.privateLibsDir())


class Finder(object):
    def __init__(self, args):
        self.apps = args.project.split('libs')[0]
        self.installed_libs = os.path.join(self.apps, 'installed_libs')

    def privateLibsDir(self):
        installed_libs_lib = os.path.join(self.installed_libs, 'lib')
        default_dir = self._defaultDir(installed_libs_lib)
        return os.path.join(default_dir, 'private')

    def includeLibsDir(self):
        installed_libs_include = os.path.join(self.installed_libs, 'include')
        default_dir = self._defaultDir(installed_libs_include)
        return os.path.join(default_dir)

    def appsParent(self):
        """ Returns the parent dir of the apps folder """
        return os.path.dirname(self.apps)

    def _defaultDir(self, path):
        """
        Searches the path passed as argument for a 'profile_<chip_name>' folder
        """
        for dirpath, dirnames, files in os.walk(path):
            for d in dirnames:
                if 'default' in d:
                    return os.path.join(dirpath, d)
        return ''


def copy(file, dst):
    sh.copy(file, dst)
    filename = os.path.basename(file)
    print('{} copied to {}'.format(file, os.path.join(dst, filename)))


def parse_args():
    """ parse the command line arguments """
    parser = argparse.ArgumentParser(
                description='Import or export a private library')

    parser.add_argument('-k', '--devkit',
                        required=True,
                        help='Specifies the devkit to use')

    parser.add_argument('-p', '--project',
                        required=True,
                        help='Specifies the workspace file to use')

    parser.add_argument('-a', '--action',
                        choices=['import', 'export'],
                        help='Import a private library from a specified \
                        location into a workspace')

    return parser.parse_args()


if __name__ == '__main__':
    args = parse_args()

    if 'libs' not in args.project:
        UI(args).fail('Please set the libs project as the "Active Project"')

    if 'import' in args.action:
        Importer(args).importLib()
    else:
        Exporter(args).exportLib()
