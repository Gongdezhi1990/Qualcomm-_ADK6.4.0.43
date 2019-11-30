############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
import sys
import os

class Capability:
    """
    A class representing a capability.
    This provides access to information on the type of the patch as well as
    interpretation of some of the associated data.
    """

    def __init__(self, cap_name, cap_id, cap_folder_name, config_files, path, cap_data_sym=0, cap_data_sym_name=""):
        # Set the basics they may be all we need
        self.cap_data_sym = cap_data_sym
        self.cap_id = cap_id
        self.cap_name = cap_name
        self.config_files = config_files
        self.cap_folder_name = cap_folder_name
        self.path = os.path.join(path,cap_folder_name)
        self.cap_data_sym_name = cap_data_sym_name
        if (not os.path.isfile(os.path.join(self.path,"makefile"))):
            raise Exception("Error. No makefile found in " + str(self.path))
        # Load and store source file names from makefile
        self.asm_src= []
        self.c_src= []
        self.dyn_src= []
        mkf = open(os.path.join(self.path,"makefile"))
        for line in mkf:
            relevant = line.split("#")[0].strip()
            if relevant:
                if (relevant.find("=")!=-1):
                    rvalue = relevant.split("=")[1]
                    if (relevant.find("C_SRC") != -1):
                        tmp = rvalue.split()
                        for item in tmp:
                            if item.lower().endswith(".c"):
                                self.c_src.append(item)
                    elif (relevant.find("S_SRC") != -1):
                        tmp = rvalue.split()
                        for item in tmp:
                            if item.lower().endswith(".asm"):
                                self.asm_src.append(item)
                    elif (relevant.find("DYN_FILES") != -1):
                        tmp = rvalue.split()
                        for item in tmp:
                            if item.lower().endswith(".dyn"):
                                self.dyn_src.append(item)


    def get_cap_data(self):
        # Change directory to the capability source code
        saved_dir = os.curdir
        os.chdir(self.path)
        # Look throughout the source code for the [capability]_cap_data variable and check the cap_name matches what we expect
        expected_cap_data = " " + self.cap_name + "_cap_data"
        found = 0
        # First get list of files
        for root, dirs, files in os.walk("."):
            for file in files:
                if file in self.c_src:
                    f = open(os.path.join(root,file))
                    for line in f:
                        if (line.find(expected_cap_data) != -1):
                            if (line.split(expected_cap_data)[0].find("const CAPABILITY_DATA")!=-1 and line.split(expected_cap_data)[1].find("=")!=-1 ):
                                last_file = file
                                found = found + 1
        # restore cur_dir
        os.chdir(saved_dir)
        if found == 0:
            raise Exception(expected_cap_data.lstrip() + " not found in any of these files: " + str(self.c_src))
        if found > 1:
            raise Exception(expected_cap_data.lstrip() + " found more than once (" + str(found) +") in any of these files: " + str(self.c_src) + ". WARNING, commented out occurrences ARE NOT ignored!")
        return (last_file, expected_cap_data.lstrip())

    def get_cap_name(self):
        return self.cap_name

    def get_config_files(self):
        return self.config_files

    def get_file_list(self):
        return (self.c_src, self.asm_src, self.dyn_src)

    def get_ofiles_list(self):
        all_files = []
        all_files.extend(self.c_src)
        all_files.extend(self.asm_src)
        all_files.extend(self.dyn_src)
        return [os.path.splitext(x)[0] + ".o" for x in all_files]

    def getCapabilityId(self):
        return self.cap_id

    def getSymCapData(self):
        return self.cap_data_sym

    def setSymCapData(self, sym):
        self.cap_data_sym = sym

    def setSymCapDataName(self, cap_data_sym_name):
        self.cap_data_sym_name = cap_data_sym_name

    def getSymCapDataName(self):
        return self.cap_data_sym_name