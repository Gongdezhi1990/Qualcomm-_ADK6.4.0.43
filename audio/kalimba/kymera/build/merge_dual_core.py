############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
#! /usr/bin/python
import os
import sys
import re

debugbin = "/build/debugbin"
exe_name = "kymera_crescendo_audio.xuv"

def print_help():
    print('''
        Merge P0 and P1 XUV and ROM files to a single one
    
        takes as command line arguments
        (1) P0 input image full path
        (2) P1 input image full path 
        (3) Merge output directory name
    ''')


def abspath(path):
    p = os.path.abspath(path)
    m = re.compile("^[a-zA-Z]:").match(p)
    if m:
        p = "\\" + m.group()[0] + p[m.end():]
    return p

def read_last_address(xuv_file_name):

    xuv_file = open(xuv_file_name)
    lines = xuv_file.readlines()
    last_line = lines[len(lines)-1]

    saddr, sdata = last_line.split()
    addr = int(saddr.strip('@'), 16)
    xuv_file.close()
    return addr; 

def read_first_address(xuv_file_name):
    xuv_file = open(xuv_file_name)
    line = xuv_file.readline()
    
    saddr, sdata = line.split()
    addr = int(saddr.strip('@'), 16)
    xuv_file.close()
    return addr; 

def validate_addrs(p0_xuv, p1_xuv):

    p0_last_address =  read_last_address(p0_xuv)
    p1_first_address = read_first_address(p1_xuv)

    if p0_last_address >= p1_first_address: 
        return False;

    return True;
 
def merge_exe_file(p0_file_name, p1_file_name, out_file_name): 
    print("Merging ...\n(1)"+p0_file_name+"\n(2)"+p1_file_name+"\n")
    with open(out_file_name, 'w') as out_file:
        with open(p0_file_name) as p0_file:
            for line in p0_file:
                out_file.write(line)
        saddr, __ = line.split()
        last_addr = int(saddr.strip('@'), 16)


        with open(p1_file_name) as p1_file:
            line = p1_file.readline()
            saddr, __ = line.split()
            next_addr = int(saddr.strip('@'), 16)
            for addr in range(last_addr+1, next_addr):
                out_file.write("@{0:0>8X} {1:0>4X}\n".format(addr, 0))
            out_file.write(line)
            for line in p1_file:
                out_file.write(line)
            saddr, __ = line.split()
            last_addr = int(saddr.strip('@'), 16)
    print("Merge completed\n");  
 

if __name__ == '__main__':
    #sanity check
    if len(sys.argv) < 3:
        print_help()
        sys.exit(2)

    curdir = os.getcwd()

    p0_xuv_file_name = sys.argv[1]
    p1_xuv_file_name = sys.argv[2]
    merge_dir = sys.argv[3]

    outdir = abspath(curdir+"/..")+"/output"

    merge_path = outdir+"/"+merge_dir+debugbin

    if os.path.isfile(p0_xuv_file_name) != True: 
        print(p0_xuv_file_name+" does not exist")
        sys.exit(2) 

    if os.path.isfile(p1_xuv_file_name) != True: 
        print(p1_xuv_file_name+" does not exist")
        sys.exit(2) 

    if validate_addrs(p0_xuv_file_name, p1_xuv_file_name) != True:
        print("Addresses are not validated")

    if not os.path.exists(merge_path):
        os.makedirs(merge_path)

    __ , exe_name = os.path.split(p1_xuv_file_name) 

    out_xuv_file_name = merge_path+"/"+exe_name 
    merge_exe_file(p0_xuv_file_name, p1_xuv_file_name, out_xuv_file_name)

    print("Created new XUV file"+out_xuv_file_name+"\n")





            


