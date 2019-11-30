#!/usr/bin/env python
############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
import os
import re
import sys
import argparse

# Enable disable debug mode
debug = False

### Some utility functions

def update_file(file_to_write, content):
    """ This function updates a file. If the files doesn't exist
    it will be created. Only writes to the file if the content is different."""
    if read_file(file_to_write, stop_if_failure = False, split_to_lines = False) != content:
        if debug:
            print file_to_write + " was updated!"
        with open(file_to_write,"w") as targetfile:
            targetfile.write(content)
        return True
    else:
        if debug:
            print file_to_write + " remains the same."
        return False

def read_file(file_name, stop_if_failure = True, split_to_lines = True):
    """ Reads the content of a file. For gaining some performance all the content is
    read in one go. Additionally we can split the file into lines."""
    content = None
    try:
        with open(file_name) as file:
            content = file.read()
        if split_to_lines:
            return content.split("\n")
    except IOError as E:
        if stop_if_failure:
            sys.stderr.write("%s: unable to open %s\n" % (sys.argv[0], file_name))
            raise E

    return content

def grep_words(words, file_name):
    """A function to see if any of the strings in the list of words occurs as in a file"""
    data = read_file(file_name, split_to_lines=False)
    for word in words:
        if word in data:
            return True
    return False

def matching_file(suffixes, file_name):
    """See if a filename has one of a list of suffixes"""
    (root, ext) = os.path.splitext(file_name)

    if len(ext) > 1:
        return ext in suffixes
    else:
        return False

def touch_file(file_name):
    """A function to touch a source file. This will modify the last modified date to now and
    so it will be rebuild. """
    os.utime(file_name, None)

# Find all the files on the directories with one of a list of suffices
# containing one of the changed definitions. Those files will be marked for rebuild.
def touch_files_dependent_on_changes(kymera_path, dirs, suffixes, changes):
    """Find all the files dependent on the changes in the given directories and touch them.
    Touching is updating the time so the build will think it was changed and so it will be
    rebuild."""
    for dir in dirs:
        if dir[0] != '/':
            # This is a relative path to kymera root
            dir = kymera_path + dir
        if not os.path.exists(dir):
            print "Directory %s included in ALL_SRCDIRS, ALL_INCDIRS or CFG_LIBS doesn't exist, continuing..." % dir
        else:
            for file_name in os.listdir(dir):
                full_file_path= os.path.join(dir, file_name)
                # Filter a list of filenames down to those with one of the given suffixes"
                if matching_file(suffixes, full_file_path):
                    # Find all the files from a set with one of a list of suffices
                    # containing one of the changed definitions
                    if grep_words(changes, full_file_path):
                        print "Mark file for rebuild:", full_file_path
                        touch_file(full_file_path)

def defs_from_file(file_name):
    """A function to take a last_defines.txt file and return defines from the  file. """
    data = read_file(file_name, stop_if_failure = False)
    if data == None:
        return None

    # Push the read data to a DataStack() which will remove all the comments and empty
    # lines when we extracting them
    stack = DataStack()
    stack.push(data)
    # Every line (the comments and empty lines were already removed) form the
    # last_defines.txt is define.
    return ConfigDefs("cpp",stack.get_all())

def read_config_file(config_path, config_file, config_list=None, config_cpp_defs=None, config_build_defs=None):
    # these things will hold us the useful information
    if config_list == None:
        config_list = ConfigList()
    if config_cpp_defs == None:
        config_cpp_defs = ConfigDefs("cpp")
    if config_build_defs == None:
        config_build_defs = ConfigDefs("build")
    # this stack holds lines from a config file
    stack = DataStack()
    supported_states = [
        "setCurrentFile",# Sets the value of the current file.  In other words signals when
                      # the script is returning to the previous config file.
        "include",    # State signals that the next action is including another config file
        "list",       # State used for reading the name of the config list.
        "listContinue",# State for reading a config list previously defined.
        "cpp",        # State for reading the cpp defs
        "build"       # State for reading the build defs
        ]

    # Set the initial state of the state machine as invalid to make sure the first valid
    # config line will change the state.
    state = "invalid"
    # Set the current file
    current_file = config_file
    # Set the first config line as one which includes the main config.
    # This is to kick the state machine.
    stack.push("%include " + config_file)

    # pattern Used to read the the config file line by line
    pattern = re.compile("^((%[a-zA-Z0-9]+)|([^\s]+))\s*([^#]*)")
    """
    Explaining the regular expression:
    "^((%[a-zA-Z0-9]+)|([^\s]+))\s*([^#]*)"
    |              group  0               |# The whole match is called group 0
      (         group 1        )           # Group 1 only exist to have a logical "or" between group 2 and 3
       (   group 2   )                     # Search for lines containing %<new_state> [optional parameter]as first part
                       ( g 3 )             # Serf for lines not starting with %
                                   (g 4 )  # What remains from the line. This is "" and not None if nothing found.
    """
    # loop while we have data
    config_line = stack.get()
    while config_line:
        match = re.search(pattern,config_line)
        if debug:
            print "current_file  ", current_file
            print "config_line ", config_line
            print "match.groups() starting with group 1 ", match.groups(), "\n"

        # Set new state
        if match.group(2) != None: # State changer instruction read!
            # something starting a new state
            # search is %new_state
            # read the new state.
            state = match.group(2)[1:]
            if not(state in supported_states):
                print "State %s not supported!"%state
                sys.exit(1)

        # Additional actions depending on the input and current state.
        if (match.group(3) != None) or (match.group(4) != ""):

            if state == "include":######################### Actions for "include" state

                # strip is used to remove leading and trailing white spaces
                next_file = config_path + match.group(4).strip()
                data = read_file(next_file, stop_if_failure = False)
                if data == None:
                    print "Unable to open %s included in %s\n"%(next_file,current_file)
                    sys.exit(1)
                # Push a set current file config line to the stack to signal when returning to
                # the previous config file. This is very useful when debugging config files.
                stack.push("%setCurrentFile " + current_file)
                stack.push(data)
                # Set the current file and pus the data to the stack.
                current_file = next_file
                # Set the new state as invalid to make sure the following config line
                # will change the state.
                state = "invalid"

            elif state == "listContinue":############# Actions for "listContinue" state

                config_list.add(match.group(3))

            elif state == "list":############################# Actions for "list" state

                config_list.set_current_list(match.group(4))
                state = "listContinue"

            elif state == "cpp":############################### Actions for "cpp" state

                if match.group(4) != "":
                    config_cpp_defs.add(match.group(3) + " "+ match.group(4))
                else:
                    config_cpp_defs.add(match.group(3))

            elif state == "build":########################## Actions for "build" state

                if match.group(4) != "":
                    config_build_defs.add(match.group(3) + " "+ match.group(4))
                else:
                    config_build_defs.add(match.group(3))

            elif state == "setCurrentFile":######### Actions for "setCurrentFile" state

                if match.group(2) != None and state == match.group(2)[1:]:
                    # Returning from a config file. Set the current file.
                    current_file = match.group(4)
                else:
                    print "Bad config line %s in %s."%(state,current_file)
                    sys.exit(1)

            else:######################### State not supported
                print "Bad state %s. %s is the current config file processed."%(state,current_file)
                sys.exit(1)

        # read the next config line and process it in the next cycle
        config_line = stack.get()

    # end of loop, no more data
    # return the read values
    return (config_list, config_cpp_defs, config_build_defs)

def update_config_mkf_file(file_name, config_file_full_path, config, config_cpp_defs, config_list):
    # header
    content = "# Configuration file generated automatically by %s\n# from %s\n\
# Any changes will be lost the next time the configuration is generated.\n"%(sys.argv[0],config_file_full_path)

    content += "\nCONFIG = " + config
    content += str(config_list)
    content += config_cpp_defs.to_string_for_compile_extra_defs()
    content += "\n"
    return update_file(file_name, content)

def update_defs_in_file(file_name, defs):

    content = str(defs)
    content += "\n"
    return update_file(file_name, content)

### Helper classes

class ConfigList:
    """ This class is responsible for storing and printing all the config list. """
    def __init__(self):
        self.current_list = None
        self.list_dict = {}

    @staticmethod
    def strip_module_name(list_with_module_name):

        module_path = set([])
        pattern = re.compile("^\s*(\S+)\/")
        for module in list_with_module_name:
            match = re.search(pattern, module)
            module_path.add(match.group(1))

        return module_path

    def set_current_list(self, list_name):
        self.current_list = list_name

    def get_list(self,list_name):
        if list_name in self.list_dict:
            return self.list_dict[list_name]
        else:
            return None

    def add(self, value):
        if self.current_list != None:
            if not (self.current_list in self.list_dict):
                # this is the first valuse we add to this list so we should create the set
                self.list_dict[self.current_list] = set()
            if value[0] == "-":
                # this is an undefine
                print "Remove", value[1:], "from", self.current_list
                self.list_dict[self.current_list].discard(value[1:])
                if len(self.list_dict[self.current_list]) == 0:
                    print "empty list", self.current_list
                    self.list_dict.pop(self.current_list)
                return
            self.list_dict[self.current_list].add(value)
        else:
            # Should never happen
            print "Config list not defined."
            sys.exit(1)

    def pop(self, value):
        if value != None:
            if not (value in self.list_dict):
                print "Error list not previously defined"
                sys.exit(1)
            self.list_dict.pop(value)
        else:
            # Should never happen
            print "Config list not defined."
            sys.exit(1)

    def remove(self, value):
        # Used for debug purposes!
        if self.current_list != None:
            self.list_dict[self.current_list].remove(value)
            if len(self.list_dict[self.current_list]) == 0:
                ## NO defs in this set
                # remove key form dict
                # this is the first valuse we add to this list so we should create the set
                self.list_dict.pop(current_list, None)
        else:
            print "Remove when current_list for ConfigList is None "
            sys.exit(1)

    def extend(self, cfgList):
        # Join the two subsets
        for key in cfgList.list_dict.keys():
            if self.list_dict.has_key(key):
                self.list_dict[key] = self.list_dict[key].union(cfgList.list_dict[key])
            else:
                self.list_dict[key] = cfgList.list_dict[key]

    def __str__(self):
        string = "\n\n#\n# List variable definitions from section %list\n#"
        for key in sorted(self.list_dict):
            string += "\n\n" + key + " = "
            for value in sorted(self.list_dict[key],key=lambda x: re.sub('[^A-Za-z]+', '', x)):
                string += value + " \\\n\t"
            # remove the last tab and slash
            string = string[:-3]

        return string


class ConfigDefs(set):
    """Class for saving the defines which is mainly a set.
    Created because of the different to string function."""

    def __init__(self, name,def_set=set([])):
        self.__name__ = name
        set.__init__(self,def_set)

    @staticmethod
    def get_def_name(define):
        # Returns the definition name
        return define.split("=")[0]

    def remove_defs_value(self):
        """ Remove the value (if exist) from all the kept defines. Used for searching
        for the usage of a definition."""
        just_defs_name = set()
        for defines in self:
            just_defs_name.add(self.get_def_name(defines))

        # Update the current defs
        self.clear()
        self.update(just_defs_name)

    def discard(self, element):
        # If the element appears by its name just call standard set discard method
        if element in self:
            super(ConfigDefs,self).discard(element)
        else:
        # otherwise be a bit more clever and see if we are removing something like "MYDEF=something"
            for el in self:
                if len(el.split("=")) == 2:
                    if el.split("=")[0].strip() == element:
                        super(ConfigDefs,self).discard(el)
                        return

    def add(self, element):
        temp = element.split("=", 1)
        if len(temp) == 1:
            element = temp[0].strip()
        else:
            element = temp[0].strip() + "=" + temp[1].strip()
        if element[0] == "-":
            # this is an undefine
            print " Remove",element[1:]
            self.discard(element[1:])
            return

        set.add(self, element)
        for key in self:
            if (self.get_def_name(element) ==  self.get_def_name(key)) and (element!= key):
                print " Redefining " + key  + " to " + element
                self.remove(key)
                return

    def to_string_for_compile_extra_defs(self):
        #print str(self.__name__), type(self.__name__)
        try:
            string = "\n\n#\n# Preprocessor definitions from section %" + str(self.__name__)+ "\n#\n\n"
        except AttributeError:
            string = ""
        string += "COMPILE_EXTRA_DEFS = "
        for value in sorted(self):
            string += "-D" + value + " \\\n\t"
        # remove the last tab and slash
        return string[:-3]

    def __str__(self):
        #print str(self.__name__), type(self.__name__)
        try:
            string = "\n\n#\n# Preprocessor definitions from section %" + str(self.__name__)+ "\n#\n\n"
        except AttributeError:
            string = ""

        for value in sorted(self):
            string +=  value + "\n"
        # remove the last tab and slash
        return string[:-1]

class DataStack:
    """ Thist class is used to hold the data form the config file. It is similar to a
    stack because during processing we pop "config lines" out. When an include "config
    line" is read another cofig file date is pushed to the beginning of the stack. """

    def __init__(self):
        self.storage = []

    def isEmpty(self):
        return len(self.storage) == 0

    def push(self,p):
        """ Pushes a "config line" or "config file" at the beginning of the stack.
        Note: a "config file" (or config data) is made up from "config lines"."""
        if isinstance(p,list):
            self.storage = p + self.storage
        else:
            self.storage = [p] + self.storage

    def get_next_line(self):
        """ Returns the next line without comment. """
        return self.storage.pop(0).split("#")[0].strip()

    def get(self):
        """ Returns the first useful config line """
        if len(self.storage) != 0:
            line = self.get_next_line()
            while line == "":
                if len(self.storage) != 0:
                    line = self.get_next_line()
                else:
                    return None
            return line
        else:
            return None

    def get_all(self):
        """ Returns all the useful information from the stack. """
        list = []
        line = self.get()
        while line:
            list.append(line)
            line = self.get()
        return list

    def __str__(self):
        """ Used for debug purposes. """
        return str(self.storage)

########################### Main script ###############################################
if __name__ == "__main__":

    parser = argparse.ArgumentParser(description="This script parse a config file and creates " +
                                     "or updates if necessary the builddefs.mkf and <CONFIG>.mkf" +
                                     "for the build. It checks if any of the %cpp defines " +
                                     "changed from the previous build and if so it marks the " +
                                     "relevant files for rebuild.")
    parser.add_argument("-o",
                        action="store", type=str, dest="output_path", required=True,
                        help="The target build directory")
    parser.add_argument("-k",
                        action="store", type=str, dest="kymera_path", required=True,
                        help="Kymera root directory")
    parser.add_argument("-f",
                        action="store", type=str, dest="config_file", required=True,
                        help="The config file")
    parser.add_argument("--dnld-build-name",
                        action="store", type=str, dest="build_name",
                        help="Build name of the ELF (download only)")
    parser.add_argument("--dnld-config-files",
                        action="store", type=str, nargs='*', dest="download_cfg_files",
                        help="List of config files used by capabilities (download only)")
    options = parser.parse_args()

    if options.kymera_path == ""  or  options.kymera_path == "":
        parser.print_help()
        sys.exit(1)

    # folders should end with "/"
    if options.kymera_path[-1] != "/":
        options.kymera_path = options.kymera_path + "/"

    if options.output_path[-1] != "/":
        options.output_path = options.output_path + "/"

    # Is the top-level config directory
    options.config_path = options.kymera_path + "build/config/"
    options.last_defines_file = options.output_path + "last_defines.txt"
    options.builddefs_mkf_file = options.output_path + "builddefs.mkf"

    try:
        options.config =  options.config_file.split("config.")[1]
        options.config_mkf_file = options.output_path + options.config + ".mkf"
    except IndexError:
        print "config file %s not recognised " %options.config_file
        sys.exit(1)

    # Read the config files
    (config_list, config_cpp_defs, config_build_defs) = read_config_file(options.config_path, options.config_file)
    if options.download_cfg_files is not None and options.build_name is not None:
        if options.download_cfg_files == []:
            print "Error no download capability config file provided"
            sys.exit(1)
        else:
            # Remove CFG_LIBS from config.mkf file
            # (We don't want to try rebuilding bits of the base firmware.)
            config_list.pop("CFG_LIBS")
            # Remove other lists of libs from config.mkf file
            # (These are probably harmless, but not useful.)
            config_list.pop("MAXIM_LIBS")
            config_list.pop("PRIVATE_LIBS")
            # Now add CFG_LIBS and new defines added for each download capability in the list
            for cfg_file in options.download_cfg_files:
                (config_list, config_cpp_defs, config_build_defs) = read_config_file(options.config_path, cfg_file, config_list, config_cpp_defs, config_build_defs)
            # Get extra configuration information from config_deps folder
            (config_list, config_cpp_defs, config_build_defs) = read_config_file(os.path.join(options.kymera_path,"tools/KCSMaker/bundle/config_deps/"), "config." + options.build_name, config_list, config_cpp_defs, config_build_defs)

    config_mkf_changed = update_config_mkf_file(options.config_mkf_file, options.config_path + options.config_file,\
                                                options.config, config_cpp_defs, config_list)

    builddefs_mkf_changed = update_defs_in_file(options.builddefs_mkf_file, config_build_defs)

    if builddefs_mkf_changed:
        # Everything will be rebuild so just update the last_defines.txt (config_mkf  and
        # config_mkf was already updated) and exit.
        update_defs_in_file(options.last_defines_file, config_cpp_defs)
    elif config_mkf_changed:
        # We can have a change in the last_defines.txt so we need to check it.
        # This is because contains the defines

        defs = defs_from_file(options.last_defines_file)
        if defs == None:
            print "Unable to locate last_defines.txt from the previous build."
            # Create the last defines for the next build and touch builddefs_mkf_file
            # so everything get rebuild.
            update_defs_in_file(options.last_defines_file, config_cpp_defs)
            touch_file(options.builddefs_mkf_file)
            sys.exit()

        # To extract the changed defs symmetric difference is used.
        # This is a new set with elements in either the first or second set but not both
        changed_defs = defs ^ config_cpp_defs

        if len(changed_defs):
            # remove the value form the defs. This is used to find the usage of the definition in the files.
            changed_defs_name = ConfigDefs("changed", changed_defs)
            changed_defs_name.remove_defs_value()

            print "Changed definitions:"
            for define in sorted(changed_defs):
                if define in defs: # Was in the old build
                    if define in changed_defs_name: # def without value
                        # this was undefined
                        print "\tUndef " + define.replace("\n",", ")
                    else:
                        # Search for the new value
                        for new_def in config_cpp_defs & changed_defs:
                            if ConfigDefs.get_def_name(new_def) == ConfigDefs.get_def_name(define):
                                print "\tUpdate " + define.replace("\n",", ") +\
                                      " to " + new_def.replace("\n",", ")
                else: # should be in new build only
                    if define in changed_defs_name: # def without value
                        # New define
                        print "\tNew def " + define.replace("\n",", ")
                    else:
                        # An updated def which was/will be displayed
                        pass

            if debug:
                print "changed_defs_name\n",changed_defs_name

            # Search in ALL_SRCDIRS and ALL_INCDIRs
            searchdirs = config_list.get_list("ALL_SRCDIRS")
            searchdirs = searchdirs.union(config_list.get_list("ALL_INCDIRS"))
            searchdirs = searchdirs.union(ConfigList.strip_module_name(config_list.get_list("CFG_LIBS")))
            touch_files_dependent_on_changes(options.kymera_path, searchdirs, [".c",".h",".asm",".cfg",".xml",".dyn"], changed_defs_name)
            update_defs_in_file(options.last_defines_file, config_cpp_defs)
        else:
            # Last_defines.txt contains the same useful information (maybe is just differently formatted)
            pass
    else:
        # if config_mkf_changed didn't changed (which contains all the defines as well)
        # then last_defines.txt should be up to date. There is nothing to do further
        pass

#############################################################################################



