# Copyright (c) 2014 - 2018 Qualcomm Technologies International, Ltd
from __future__ import print_function
import os


def is_pttrans_on_path():
    """Searches the system path looking for pttransport.dll.
       Returns the name of any directory containing it. Returns "" if none found
    """
    for i in os.environ["PATH"].split(";"):
        # Fix up msys style paths
        if i[0] == "/":
            i = i[1] + ":" + i[2:]

        # Ignore the current directory, if people happen to have that on their path
        if i == ".":
            continue

        # Get the contents of this directory
        result = []
        try:
            result = os.listdir(i)
        except WindowsError:
            pass

        # Search the contents
        for j in result:
            if j.lower() == "pttransport.dll":
                return i
    return ""


if __name__ == "__main__":
    result = is_pttrans_on_path()
    print()
    if result != "":
        print("ERROR: At least one pttransport.dll found on system path.")
        print()
        print("This is bad. Different tools need different versions of pttransport.dll")
        print("Having a copy on the path means that tools will use it, rather than")
        print("the one they were bundled with. This causes all kinds of problems.")
        print("")
        print("Please either: 1) delete '" + result + "/pttransport.dll'")
        print("            or 2) remove '" + result + "' from your system path")
        print()
        print("Re-run this script until no more pttransport.dlls are found")
    else:
        print("No pttransport.dll found on the path")
