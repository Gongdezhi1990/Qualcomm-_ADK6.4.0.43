#!/usr/bin/env python
############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
import sys
import os
import re
import optparse

### Helper functions
def read_file(file_name, split_to_lines = True):
    """ Reads the content of a file. For gaining some performance all the content is 
    read in one go. Additionally the content is split into lines."""
    try:
        with open(file_name) as file:
            content = file.read()
        if split_to_lines:
            return content.split("\n")
        else:
            return content
    except IOError as E:
        return None

def update_file(file_to_write, content):
    """ This function updates a file. If the files doesn't exist 
    it will be created. Only writes to the file if the content is different."""
    if read_file(file_to_write, split_to_lines = False) != content:
        with open(file_to_write,"w") as targetfile:
            targetfile.write(content)
        
usage = """
make_patch_defs -o dest_file srcfile1 srcfile2 srcfile3 ...

when -c or --collate
make_patch_defs -c -o dest_file srcdir bits_in_word

Goes through supplied list of source files and generates 
patchpoint ids in dest_file"""   
 
parser = optparse.OptionParser(usage=usage)
parser.add_option("-o", action="store", type="string", dest="dest_file", default="", help="The destination file")
parser.add_option("-c", "--collate", action="store_true", dest="collate", default=False,  
            help="This signals that the temporary files for each component should be collated into a single large patch id table")

(options, args) = parser.parse_args()

# Sanity check.
if options.dest_file == "" or len(args)==0 or (options.collate and len(args)!=2): 
    parser.print_help()
    sys.exit(1)
    
dest_file_content = ""

if options.collate:
    patch_num = 1
    dir = args[0]
    bits_in_word = (int)(args[1])
    # check if dir exist.
    if os.path.exists(dir):
        for file_name in sorted(os.listdir(dir)):
            full_file_path= os.path.join(dir, file_name)
            for line in read_file(full_file_path):
                if line.strip() !="":
                    # Turn the line into a patch point
                    dest_file_content += ".CONST " + line + " " + str(patch_num) +";\n"
                    patch_num += 1
    dest_file_content +=  "\n.CONST $patch.SLOW_TABLE_SIZE " + str(int((patch_num+bits_in_word-1)/bits_in_word)) + ";\n"
    update_file(options.dest_file, dest_file_content)
else:
    pattern = re.compile("[^\(]+\(([^,\)]+)")
    for file_name in args:
        for line in  read_file(file_name):
            # Remove commentS from the line
            line = line.strip().split(r"//")[0]
            if "SLOW_SW_ROM_PATCH_POINT" in line:
                # Extract the first parameter (from left-bracket to comma)             
                match = re.search(pattern,line)
                if match != None:
                    dest_file_content += match.group(1)+ "\n"
    update_file(options.dest_file, dest_file_content)
