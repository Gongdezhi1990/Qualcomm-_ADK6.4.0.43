############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
from re import *
from os.path import exists
import sys

test_define_re = compile(r'[\$(][\S]+[)]')

class comp_defines:
    "A class for accessing and manipulating Compiler definitions"
    def __init__(self, filename):
        "comp_defines constructor reads in the list of defines and values"
        if not exists(filename):
            print("The definition file {0} does not exist!  EXITING...".format(filename))
            sys.exit()

        #  The definition dictionary is created and then populated from the file
        self.defs = {}
        
        fh = open(filename, 'r')
        for line in fh:
            words = line.split("=")
            # if a value was assigned to the define it is specified in the file
            # with an '=' sign. If no value is defined then default to 1
            if len(words) == 2:
                if "u" in words[1]:
                    self.defs[words[0]] = int(words[1][0:-2], 16)
                elif len(test_define_re.findall(words[1])):
                    # For now patchmaker doesn't need to handle any of these 
                    #types of definition and probably never will.
                    pass
                else:
                    try:
                        self.defs[words[0]] = int(words[1].strip(), 16)
                    except ValueError:
                        self.defs[words[0]] = 1
            elif len(words) == 1:
                self.defs[words[0].strip()] = 1
                
    def __str__(self):
        "Formats the defines into a table"
        text = ""
        for define in self.defs.keys():
            text += "%35s" % define + "\t" + str(self.defs[define]) + "\n"
        return text


    def get_define(self, name):
        "Returns the value assigned to this definition in this build"
        return self.defs[name]

    def append(self, name, val = 1):
        """Adds a new define to the set of definitions, if no value is specified
        it defaults to 1"""
        # check define is not already in the list. This is an append after all!
        if name in self.defs:
            print("Define already exists. Key {0} not added to defines list".format(name))
            return
        self.defs[name] = val
        
    def delete(self, name):
        "Removes an entry in the defines list"
        self.defs.pop(name)
        
    def edit(self, name, val):
        "Edits a key if it exists. If the key does not exist no change is made"
        if name in self.defs:
            self.defs[name] = val
        else:
            print("The define {0} does not exist and has not been edited".format(name))
        

if __name__ == "__main__":
    deffile = "last_defines.txt"
    cdefs = comp_defines(deffile)
    
    print(cdefs)
    