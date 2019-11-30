# Copyright (c) 2014 - 2018 Qualcomm Technologies International, Ltd
from __future__ import print_function


def load_klo_file(klo_filename):
    """
    Parses the specified KLO file and stores it for the other Kalimba debug functions to make use of.
    """

    constants = {}      # Contains tuples (constant name, value)
    source_lines = {}   # Contains tuples (module name, filename, line number). Index is pm addr
    variables = {}      # Dict of tuples. [variable name] -> (size, address)
    labels = []         # Contains tuples (label name, addresse)
    static_dm = {}      # The contents of statically initialised DM. Tuples are (group, val)
    static_pm = {}      # The contents of statically initialised PM. Tuples are (group, val)

    klo_path = klo_filename[:klo_filename.rfind('/') + 1]

    try:
        in_file = open(klo_filename, 'r')
    except IOError:
        print("Couldn't open file '%s'" % klo_filename)
        return

    for line in in_file.readlines():
        if line[0:4] == "DATA":
            (group, addr, val) = line.split()
            static_dm[addr] = (group, val)
            continue

        if line[0:4] == "CODE":
            (group, addr, val) = line.split()
            static_pm[addr] = (group, val)
            continue

        # check if we have a data line - will lead with "DEBUG"
        if line[0:5] != "DEBUG":
            continue

        line = line[6:]
        line = line.strip()

        # look for the three ascii(7) marker chars
        parts = line.split('\7')

        if len(parts) < 4:
            continue

        # now determine where to store the data
        if parts[1] == "c":                        # Constant
            constants[parts[0]] = int(parts[3])
        elif parts[1] == "l":                      # Source line
            filename = parts[4]
            if filename[1] != ':':
                filename = klo_path + filename
            source_lines[int(parts[3])] = (parts[0], filename, int(parts[2]))
        elif parts[1] == "v":                      # Variable
            variables[parts[0]] = (int(parts[2]), int(parts[3]))
        elif parts[1] == "a":                      # PM label
            labels.append((parts[0], int(parts[3])))
    in_file.close()

    return constants, source_lines, variables, labels, static_dm, static_pm
