#!/usr/bin/env python
############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
import sys
import re

def print_help():
    print('''
    xuv2rom - Converts 16-bit xuv format into 8-bit rom format.
              Called from makerules_src.mkf.
               
    takes as command line arguments:
        (1) input xuv files
    ''')

#sanity checks
if len(sys.argv) != 2:
    print_help()
    sys.exit(1)

try:
    input_file=open(sys.argv[1],"r")
except:
    print('Input file %s cannot be opened'%(sys.argv[1]))
    print_help()
    sys.exit(1)

# search for the following pattern: @[address in hexadecimal] [data in hexadecimal (16 bit)]
pattern = re.compile("^@([0-9a-fA-F]+) ([0-9a-fA-F]+)")
for line in input_file:
    match = re.search(pattern,line)
    if match != None:
        addr = int(match.group(1),16)
        data = int(match.group(2),16)
        # print the data byte by byte
        print ("@%06X %02X"% (2*addr, data&0xff))
        print ("@%06X %02X"% (2*addr+1, data/256))

       
    
 
