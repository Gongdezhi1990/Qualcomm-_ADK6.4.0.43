                            * * * * * * * * * * * * * * *
                            *   Kalimba Python Tools    *
                            * * * * * * * * * * * * * * *



Introduction
============

These tools are designed to help developers of Kalimba software. Functionality includes:
 - An interactive debugger:
   - Report Kalimba version and features
   - Read/write memories/registers
   - Start, stop and step the processor
   - Read debug symbol information
   - Set/clear PM/DM breakpoints
   - Report the status of on-chip data structures related to the Kalimba standard library code.
 - A "Kalimba Python shell" environment, which provides command history, and shortcuts for common 
   operations, such as displaying numbers in hexadecimal format.

Installation (Windows)
======================

1. Install the Python 2.7 or 3.6 - http://www.python.org.
   Ensure that the interpreter platform/architecture (32-bit/64-bit) matches the tools installed.
2. Install pyreadline.
   - For Python 2.7, version 1.5 has been tested to work with these tools; the easiest route is to
   use the Windows installer package available from:
   https://pypi.python.org/pypi/pyreadline/1.5
   - For Python 3, version 2.1, available via pip, has been tested to work. Later versions will
   likely also work.
3. Optionally add the directory where the installed tools reside on disk, to your PYTHONPATH 
   environment variable. This allows the tools to be conveniently invoked from any location.


Usage without kalshell environment
==================================

The tools can be used as standard Python modules, without the kalshell environment, as follows:

>> python
>> import kalaccess
>> kal = kalaccess.Kalaccess()

Simple Example
==============

To test the installation and try out the interactive debugger, follow the steps below.
This assumes that a Kalimba device is attached via a debug interface.
For all connection options, consult help(kal.connect).

From a shell command prompt:
    1. cd to the Python tools directory (see step 3 above), containing kalshell.py
    2. Run python kalshell.py
    3. At the >>> prompt type:
        kal.connect()
    4. Choose one of the options presented, to connect to that device.
    5. Type
        kal.is_running()

This will report the name and version of the Kalimba you have connected to and will report whether 
the chip is currently running.

Connecting to the same Kalimba processor can be made quicker next time by passing arguments to 
kal.connect(), so that it does not search for all Kalimbas, but instead directly connects to the one
specified. The transport string and any other arguments needed are shown in the printout produced by
kal.connect().

Kalshell Usage
==============

Kalshell provides a Python object called "kal". Most of the functionality of Kalshell is
accessed via this object. One of the best ways to find what commands are available is to use
the tab completion feature of Kalshell. Just type "kal." then press tab. This will return a
complete list of supported commands.
    
Standard Python syntax applies when invoking these commands, i.e. all function calls require
parentheses. Blocks of data are passed around as Python lists, dictionaries, tuples, or instances
of custom classes defined by these tools.

Help can be found on any function in the usual Python way; type help(functionName). This
will report the built-in doc string, fetched from the Python source code.

Kalshell provides a persistent command history. You can access that history by pressing the up
arrow. The history can be filtered; for example, if you type "kal.sym" and press up, only commands 
in the history relating to the debug symbol module (sym) will be returned.

There are special shortcuts to access PM, DM and registers on the Kalimba. You could type
"kal._read_dm_block(0x123, 1)". The shortcut verison would be "kal.dm[0x123]". This shortcut
mechanism can be used for block accesses as well as reads or writes. Here are a few examples:

    kal.pm[0xabc]         - reads PM location 0xabc
    kal.dm[123]           - reads DM location 123
    kal.pm[0xab8 : 0xabc] - reads PM locations 0xab8 to 0xabb, inclusive (four locations).
                            This means that for word-addressable PM, four words are returned; for 
                            byte-addressable PM, it is one word.
    
There are several shortcut commands embedded into the Kalshell interpreter. These do not follow
normal Python syntax. These commands are:

    ls <dir>   - List the contents of directory "dir".
    pwd        - Prints the current working directory
    cd <dir>   - Change the current directory to "dir"
    hex        - To be used at the end of a Python command line. Takes the result of the Python
                 command and calls hex() on it. eg "kal.pc hex" is equivalent of having typed
                 "hex(kal.pc)"
    help       - To be used at the end of a Python command line. Takes the result of the Python
                 command and calls help() on it. eg "kal.pause help" is equivalent of having typed
                 "help(kal.pause)"
    !          - Use this at start of a line to pass the rest of the line to the system shell,
                 e.g. "!del file.txt"
    lasterr    - Prints the stack trace for the last exception caught by Kalshell.
    who <type> - Lists objects in the global Python dictionary, grouped by type. Accepts an optional
                 "type" argument to show only objects of that type.

Troubleshooting (Windows-specific)
==================================

1. If you see errors relating to the loading of DLLs, please ensure that your Python
installation's platform/architecture matches that of the installed tools: these must both be
either 32-bit or 64-bit. Python cannot load 32-bit DLLs into a 64-bit interpreter, or vice-versa.

2. Many problems are caused by these Python tools running with the wrong version of
pttransport.dll.

These tools are bundled with a version of pttransport.dll. The problem occurs when
a different version is present on the system PATH.

The solution is to find the offending DLL and prevent it being found during the loading process,
either by removing it or removing its directory from the PATH.

The path_check script assists in finding potentially troublesome pttransport DLLs. 
At a command line, type:

    python path_check.py

This will print the locations of any pttransport DLLs on the PATH.
