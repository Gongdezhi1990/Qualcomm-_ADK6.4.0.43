#!/usr/bin/python
# Copyright (c) 2018 Qualcomm Technologies International, Ltd.
#   
'''
DESCRIPTION
  Utility methods used in hydra_os package creation and library build.
'''
import os
import re
import glob
import shutil
import fileinput
import sys
import errno

# If you need a trapset defined as 1 in trapsets.h to make
# the trapsets consistent with other chips add it to this 
# list
make_trapset_define_1 = {"TRAPSET_WAKE_ON_AUDIO",
                                             "TRAPSET_CAPACITIVE_SENSOR",
                                             "TRAPSET_PROFILE",
                                             "TRAPSET_NFC",
                                             "TRAPSET_AUDIO_PWM",
                                             "TRAPSET_AUDIO_ANC",
                                             "TRAPSET_XIO"}

def create_trapset_h(path_to_build_defs, path_to_trapset_h):
    """
    Extract the trapset defines from build_defs.h and create a
    file called trapset.h with these defines in it.
    """
    trapset_defines = list()
    trapset_header_directory =  os.path.dirname(path_to_trapset_h)

    with open(path_to_build_defs) as build_def_file:
        for line in build_def_file:
            if "TRAPSET_" in line:
                for trapset in make_trapset_define_1:
                    if trapset in line:
                        line = re.sub(r' 0$',' 1',line)
                        
                trapset_defines.append(line)

    if not os.path.exists(trapset_header_directory):
        try:
            os.makedirs(trapset_header_directory)
        except OSError as e:
            if e.errno != os.errno.EEXIST:
                raise
            pass

    with open(path_to_trapset_h, 'w') as trapset_header_file:
        trapset_header_file.write("#ifndef __TRAPSETS_H__\n")
        trapset_header_file.write("#define __TRAPSETS_H__\n")

        trapset_defines.sort()

        for define in trapset_defines:
            trapset_header_file.write(define)

        trapset_header_file.write("#endif /* __TRAPSETS_H__ */\n")

def copy_file(src, dest):
    print "Copy " + src + " to " + dest
    # Intentionally use copyfile rather than copy so permissions aren't preserved.
    # The resulting file will be writable.
    shutil.copyfile(src, dest)

def create_path_and_copy(src, dest):
    try:
        copy_file(src, dest)
    except IOError as e:
        if e.errno != errno.ENOENT:
            raise
        try:
            os.makedirs(os.path.dirname(dest))
        except OSError as e:
            if e.errno != os.errno.EEXIST:
                raise
            pass
        shutil.copyfile(src, dest)

def file_copy(source_path, dest_dir):
    """
    Copy file from source to destination
    """
    for file in glob.glob(source_path):
        destination_file = os.path.join(dest_dir, os.path.basename(file))
        create_path_and_copy(file, destination_file)
        
def replace_string_in_file(filename, original_string, new_string):
    """
    Replace original string with new_string in filename
    """
    print "Replacing " + original_string + " with " + new_string + " in " + filename
    for i, line in enumerate(fileinput.input(filename, inplace=1)):
        sys.stdout.write(line.replace(original_string, new_string)) 

    backup_file = filename + ".bak"

    try:
        os.chmod(backup_file, stat.S_IWRITE)
        os.remove(backup_file)
    except:
        pass    

