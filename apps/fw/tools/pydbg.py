#!/usr/bin/env python2
############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2013 - 2016 Qualcomm Technologies International, Ltd.
#   
#
############################################################################
"""\
Device Debugging python shell.

All things to all folk ...but in all cases the shell installs a handy
pylib Chip (local or proxy) into the global name space as "chip". 

For usage:-

  > pydbg.py -h

For an interactive debugging session with a chip connected via the  
default pttransport connection:-

  > pydbg.py

For the same but with the shell pre-poluted with those xap2emu-like
commands:-

  > pydbg.py -x

To run python debugging & report scripts (non-interactively):-

  > pydbg.py script1 script2 ...

To connect to a local chip via non default pttransport connection:-

  > pydbg.py -d pttrans:"SPITRANS=USB SPIPORT=12345 SPIMAXCLOCK=172"

...Note the quotes. 

To connect to an xcd3 coredump:-

  > pydbg.py -d xcd3:a_core_dump.xcd3
  
    
Future:-
- Support multiple Devices
- Define and accept more complex local device configuration specifications 
(e.g. multi-connection, & multi-device) (medium)
"""

# ----------------------------------------------------------------------------
# Dependencies
# ----------------------------------------------------------------------------

import os
import sys

# ----------------------------------------------------------------------------
# Python version check
#----------------------------------------------------------------------------
# use sys as opposed to platform as version values are integer
def check_python_version():
    version = sys.version_info[0:3]
    if version < (2,7,0):
      sys.stderr.write("pydbg needs at least Python 2.7")
      sys.exit(1)

check_python_version()

# Prepend script-relative "./pylib" to the sys.path so a local py library 
# will override any default library.
#
my_dir = os.path.abspath(os.path.dirname(__file__))    
pylib_dir = os.path.join(my_dir, "pylib")
if pylib_dir not in sys.path:
    sys.path.insert(0, pylib_dir)
    try:
        # See http://svn.python.org/projects/sandbox/branches/setuptools-0.6/pkg_resources.txt
        # description of WorkingSet objects.
        # We do early import of pkg_resources because it is used in class
        # csr.front_end.pydbg_config which calling scripts may have already used.
        # All subsequent updates to sys.path must update the WorkingSet thus:
        import pkg_resources
        pkg_resources.working_set = pkg_resources.WorkingSet()
    except ImportError:
        # import failed; setuptools not installed on this PC
        pass
   
del my_dir
del pylib_dir
    
from csr.front_end.pydbg_front_end import PydbgFrontEnd
        
# ----------------------------------------------------------------------------
# Entry
# ----------------------------------------------------------------------------
def pkg_main():
    # default mode of executing pydbg doesn't work
    # when running from a package; use alternative
    from csr.front_end.pydbg_front_end import PydbgRlCodeInteractFrontEnd
    PydbgFrontEnd = PydbgRlCodeInteractFrontEnd
    PydbgFrontEnd.main_wrapper(shell=globals())

def main():
    """
    A simple routine entry point that setuptools:setup.py can 
    provide as an executable entry point when project is built via setup.py
    """
    PydbgFrontEnd.main_wrapper(shell=globals())

if __name__ == "__main__":
    main()
