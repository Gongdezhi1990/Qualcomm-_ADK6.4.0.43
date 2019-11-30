#!python

# ***********************************************************************
# * Copyright 2014-2016 Qualcomm Technologies International, Ltd.
# ***********************************************************************

"""
This module overrides the standard Python command interpreter to add features
like a persistent history and auto completion. It imports the trbtrans module,
and also adds a few handy shortcuts (implemented in the push function below).
"""

import code
import os
import readline
# It looks like nothing uses the following, but it needs to be imported before readline.parse_and_bind()
# is called, in order to make tab completion work.
import rlcompleter
import sys
import traceback
import trbtrans
try:
    import trbutils
except ImportError:
    # Treat this as non-fatal. If the utils aren't there, it doesn't prevent the core bindings working.
    pass


def to_hex(data):
    """
    Returns a string representing an array/list of data in hexadecimal.
    """
    try:
        rv = []
        for i in data:
            rv.append("0x%x" % i)
        return " ".join(rv)
    except TypeError:
        return "0x%x" % data


class HistoryConsole(code.InteractiveConsole):
    def __init__(self, supplied_locals, hist_file):
        self.histfile = hist_file
        code.InteractiveConsole.__init__(self, locals=supplied_locals)
        readline.parse_and_bind("tab: complete")
        try:
            readline.read_history_file(hist_file)
        except IOError:
            # On some versions of readline on Linux, a missing history file is treated as an exception.
            pass
        self.traceback = None

    def showtraceback(self):
        # Just print the exception string, not the stack trace
        ex_type, value, tb = sys.exc_info()

        # Remove decoration that Python adds to the front of exceptions.
        exception_type_name = str(ex_type)[:-2]
        # a) Transform built-in exception strings:
        # <type 'exceptions.RuntimeError: to "RuntimeError"
        exception_type_name = exception_type_name.replace("<type 'exceptions.", "")
        # b) Transform custom exceptions: "<class 'module.SomeException" to "module.SomeException"
        exception_type_name = exception_type_name.replace("<class '", "")

        print exception_type_name + ":", value

        if tb.tb_next.tb_next:
            print "    type 'lasterr' for stack trace"
            # Store the traceback so the user can inspect it later if they want by calling lasterr()
            self.traceback = traceback.format_exc()

    def lasterr(self):
        """
        Prints the stack trace for the last exception that the console caught.
        """
        if self.traceback is None:
            print "No error traceback is stored"
            return

        print self.traceback

    @staticmethod
    def _split(line):
        """
        Splits line into words. Handles quoted strings as one 'word' - and strips the quotes.
        """
        words = []
        word = ''
        quote = None
        for c in line:
            word = word + c

            if quote:
                if c == quote:
                    quote = None
                    words.append(word[:-1])
                    word = ""
            else:
                if c == '"' or c == "'":
                    quote = c
                    words.append(word[:-1])
                    word = ''
                if c == ' ' or c == '\t':
                    words.append(word[:-1])
                    word = ''
        words.append(word)
        words = filter(len, words)  # Remove empty strings
        return words

    @staticmethod
    def display_help():
        print """
    *************
    trbshell help
    *************

    You are sitting at a slightly modified interactive Python prompt.

    An object called trb has been created. Most functionality is accessed through it.
    For example, type 'trb.open("usb2trb")' to open a connection to a usb2trb dongle.
    The module trbutils has also been imported if it was found. This provides some extra utility functions.

    Type 'help(trb)' or 'trb help' to see the built in Python docstrings for the trb
    object."""

    def push(self, line):
        """
        This function is called every time the user enters a line.
        """

        # Save the history every time because there seems to be no other way
        # to ensure that history is kept when the session is Ctrl+C'd.
        readline.write_history_file(self.histfile)

        # 'line' is in unicode. Convert line to ascii. Ignore non-ascii characters.
        line = line.encode("ascii", "ignore")
        words = self._split(line)

        if len(words) > 0:
            cmd = words[0]
            if cmd == 'lasterr':
                line = "hc.lasterr()"

            elif cmd == 'ls':
                # Handle ls shortcuts
                arg = '.'
                if len(words) > 1:
                    arg = words[1]
                line = 'os.listdir("' + arg + '")'

            elif cmd == 'pwd':
                line = 'os.getcwd()'

            elif cmd == 'cd':
                if len(words) == 1:
                    line = 'os.getcwd()'
                else:
                    path = " ".join(words[1:]).replace("\\", "/")
                    line = 'os.chdir("' + path + '")'

            # If the line starts with '!', give it to the system shell
            elif line[0] == '!':
                if line[0:3] == '!cd':
                    path = " ".join(words[1:]).replace("\\", "/")
                    line = "os.chdir('" + path + "')"
                else:
                    line = "os.system('" + line[1:] + "')"

            # If it ends in hex, wrap the line in a call to to_hex()
            elif line[-3:] == "hex":
                line = line[:-3]
                line = "to_hex(" + line + ")"

            elif line[-4:] == "help":
                if line == "help":
                    self.display_help()
                    line = ""
                else:
                    line = line[:-4]
                    line = "help(" + line + ")"

            elif line == "who":
                line = "hc.who()"

            elif line[0:4] == "who ":
                sec = line.split()
                pieces = []
                for s in sec[1:]:
                    if s == "all":
                        pieces.append("show_all=True")
                    else:
                        pieces.append("var_type='" + s + "'")
                line = "hc.who(" + ", ".join(pieces) + ")"

            elif line == "whos":
                line = "hc.who(new_line=True)"

            elif line[0:5] == "whos ":
                sec = line.split()
                pieces = ["new_line=True"]
                for s in sec[1:]:
                    if s == "all":
                        pieces.append("show_all=True")
                    else:
                        pieces.append("var_type='" + s + "'")
                line = "hc.who(" + ", ".join(pieces) + ")"

            elif line == "exit":
                sys.exit()

        # Give the line to the Python interpreter
        return code.InteractiveConsole.push(self, line)

    @staticmethod
    def who(var_type = None, new_line=False, show_all=False):
        def get_type(x):
            return repr(type(globals()[x]))[7:-2]
        # get the list of symbols and sort by type
        data = map(lambda x: (get_type(x), x), globals().keys())
        data.sort()
        # are we showing them all
        if not show_all:
            data = filter(lambda x: x[1][:1] != "_", data)
        # one per line?
        sep = (": ", ", ")
        if new_line:
            sep = ("\n", "\n")
        syms = {}
        if var_type is None:
            i = 0
            # sort them
            while i < len(data):
                t = data[i][0]
                syms[t] = filter(lambda x: get_type(x[1]) == t, data[i:])
                i += len(syms[t])
        else:
            syms[var_type] = filter(lambda x: get_type(x[1]) == var_type, data)

        out  = ""
        for k in syms.keys():
            out += "%-20s%s" % (k, sep[0])
            out += sep[1].join("  %s" % x[1] for x in syms[k])
            out += "\n"

        print out


if __name__ == "__main__":
    # Set the buffer size to zero on the stdout stream. This makes it possible
    # to drive this script from another tool/script by piping in commands on
    # stdin and waiting for responses on stdout. With buffering enabled, the
    # external tool can end-up waiting forever for buffered output. Disabling
    # the buffer makes no noticeable difference to the speed of the interactive
    # console (and if speed is important, you're not using it anyway).
    sys.stdout = os.fdopen(sys.stdout.fileno(), 'w', 0)

    trb = trbtrans.Trb()
    
    hc = HistoryConsole(locals(), os.path.expanduser("~/.trbshell.history"))

    banner = """
    *** T R B S H E L L ***\n
    Based on Python """ + sys.version[:5] + """
    Type 'help' to learn more.
    Press Ctrl+D to quit\n"""

    sys.ps1 = "\n\001\033[1;36m\002>>> \001\033[0m\002"

    # Remove variables from the local namespace that we don't need again
    del rlcompleter

    hc.interact(banner)
