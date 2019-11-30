#!python

# Copyright (c) 2014 - 2018 Qualcomm Technologies International, Ltd

"""
This module overrides the standard Python command interpreter to add features
like a persistent history and auto completion. It also imports kalaccess and adds
a few handy shortcuts (implemented in the push function below).
"""

from __future__ import print_function
import argparse
import code
import kalaccess
import os
import readline
# It looks like nothing uses this, but it needs to be imported before readline.parse_and_bind()
# is called, in order to make tab completion work.
import rlcompleter
# noinspection PyStatementEffect
rlcompleter  # Silence pyflakes
import sys
import traceback


class HistoryConsole(code.InteractiveConsole):
    def __init__(self, supplied_locals, histfile):
        self.histfile = histfile
        code.InteractiveConsole.__init__(self, locals=supplied_locals)
        readline.parse_and_bind("tab: complete")
        try:
            readline.read_history_file(histfile)
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

        print(exception_type_name + ":", value)

        if tb.tb_next.tb_next:
            print("    type 'lasterr' for stack trace")
            # Store the traceback so the user can inspect it later if they want by calling lasterr()
            self.traceback = traceback.format_exc()

    def lasterr(self):
        """
        Prints the stack trace for the last exception that the console caught.
        """
        if self.traceback is None:
            print("No error traceback is stored")
            return

        print(self.traceback)

    # Splits line into words. Handles quoted strings as one 'word' - and strips the quotes
    @staticmethod
    def _split(line):
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
        return list(words)

    def push(self, line):
        """
        This function is called every time the user enters a line.
        """

        # Save the history every time because there seems to be no other way
        # to ensure that history is kept when the session is Ctrl+C'd.
        readline.write_history_file(self.histfile)

        # 'line' is in unicode. Convert line to ascii. Ignore non-ascii characters.
        line = line.encode("ascii", "ignore")
        # For Python 3, decode from bytes to str
        if not isinstance(line, str):
            line = line.decode()

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

            # If it ends in hex, wrap the line in a call of kalaccess.toHex()
            elif line[-3:] == "hex":
                line = line[:-3]
                line = "kalaccess.toHex(" + line + ")"

            elif line[-4:] == "help":
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
                        pieces.append("var_type='"+s+"'")
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
        data = list(map(lambda x: (get_type(x), x), globals().keys()))
        data.sort()
        # are we showing them all
        if not show_all:
            data = list(filter(lambda x: x[1][:1] != "_", data))
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
                syms[t] = list(filter(lambda x: get_type(x[1]) == t, data[i:]))
                i += len(syms[t])
        else:
            syms[var_type] = list(filter(lambda x: get_type(x[1]) == var_type, data))

        out = ""
        for k in syms.keys():
            out += "%-20s%s" % (k, sep[0])
            out += sep[1].join("  %s" % x[1] for x in syms[k])
            out += "\n"

        print(out)


if __name__ == "__main__":
    # Define command line arguments
    parser = argparse.ArgumentParser(
        description="An interactive Python shell with kalaccess integration",
        epilog="""The -p, -s and -t options work together to allow you to automatically open a 
                  connection to a Kalimba core on startup. Whatever is specified by them will 
                  be passed as arguments to a call of 'kal.connect(...)'. For details of what 
                  values can be specified, please see the help text for kal.connect."""
    )
    parser.add_argument("-p", "--proc", help="Processor ID for default connection")
    parser.add_argument("-s", "--subsys", help="Subsystem ID for default connection")
    parser.add_argument("-t", "--trans", help="Transport string for default connect")
    args = parser.parse_args()

    kal = kalaccess.Kalaccess()
    
    hc = HistoryConsole(locals(), os.path.expanduser("~/.kalaccess.history"))

    print("""
    *** K A L S H E L L ***\n
    Based on Python """ + sys.version[:5] + """
    Type kal.connect() to get started
    Type kal.help() for more info\n
    Press Ctrl+D to quit""")

    sys.ps1 = "\n\001\033[1;36m\002>>> \001\033[0m\002"

    # If command line arguments were specified to create a connection on startup,
    # then act on them now by forming a call of kal.connect() and eval'ing it.
    if args.trans:
        command = "kal.connect('" + args.trans + "'"
        if args.subsys:
            command = command + ", subsys=" + args.subsys
        if args.proc:
            command = command + ", processor=" + args.proc
        command = command + ")"

        print("\n>>> " + command + "\n")
        eval(command)

    # Remove variables from the local namespace that we don't need again
    del argparse, args, parser, rlcompleter

    try:
        # Python 3: suppress default exit message
        hc.interact("", exitmsg='')
    except TypeError:
        # Python 2 doesn't accept the exitmsg keyword arg.
        hc.interact("")
