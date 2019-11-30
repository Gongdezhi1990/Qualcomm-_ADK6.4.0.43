#!/usr/bin/env python
############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
# makeproj - (1) makes the xIDE project files suitable for either Kalsim or HAPS debugging
#           (2) adds a list of all source files under the main directory to the project file
import os
import glob
import sys
from optparse import OptionParser
import re

# called with
# python makeproj.py crescendo_rom_h7 /path/to/kalimba/kymera/output/crescendo_rom_h7/build/debugbin kymera_crescendo_audio
parser = OptionParser(description= "makeproj - (1) makes the xIDE project files suitable for either Kalsim or HAPS debugging (2) adds a list of all source files under the main directory to the project file called from makerules_src.mkf in same dir as makeproj.py");
parser.add_option("-c", "--config", dest="config",
        help="The config of the build. i.e crescendo_rom or crescendo.");
parser.add_option("-d", "--debug_bin_dir", dest="debug_bin_dir",
        help="The folder containing the build output, i.e. the *elf, etc");
parser.add_option("-t", "--target_root", dest="target_root",
        help="The target root, i.e. \"kymera_crescendo_audio\"");
parser.add_option("--subsysid", dest="subsysid", default=3,
        help="The subsystem id that will be baked into the project file.");
parser.add_option("--debugtrans", dest="debugtrans", default="usb",
        help="The debug transport that will be used when connecting a debugger.");
(options, args) = parser.parse_args()

#variables
global isKalsim
isKalsim = 0

global interesting_files
interesting_files = ''

global flist_name;
flist_name = options.debug_bin_dir + "/.gdb_info_sources.txt";

if 'kalsim' in options.config or options.debugtrans == "kalsim":
    transport='SPITRANS=KALSIM SPIPORT=1'
    isKalsim = 1
elif options.debugtrans == "lpt":
    transport='[SPITRANS=LPT SPIPORT=1 SPIMUL=4]'
elif options.debugtrans == "trb":
    transport='[trb/scar/0]'
else:
    transport='[SPITRANS=USB SPIPORT=0 SPIMUL=4]'

#functions
def file_is_interesting(fn): #i.e. returns true if file 'fn' ends in .c/.cpp/.asm and isn't in another config's folder
    if ('kalsim' in fn) and (isKalsim == 0):
        return 0
    if ('/output/' in fn) and not('kalsim' in fn) and (isKalsim==1):
        return 0

    ext =  os.path.splitext(fn)[1]

    if (ext == '.c') or (ext == '.cpp') or (ext == '.asm'):
        return 1
    else:
        return 0

def add_file(path, start): #adapted from os.path.relpath, available in 2.6, replace when external machines get updated

    start_list = os.path.abspath(start).split('/')
    path_list = os.path.abspath(path).split('/')

    # Work out how much of the filepath is shared by start and path.
    i = len(os.path.commonprefix([start_list, path_list]))

    rel_list = ['..'] * (len(start_list)-i) + path_list[i:]
    rel_path = os.path.join(*rel_list)
    global interesting_files    
    interesting_files += ' <file path="' + rel_path + '" />\n'

def flist_exists():
    try:
        with open(flist_name) as f:
            line_cnt = 0;
            for line in f:
                striped_line = line.rstrip('\n');
                #print "adding line " + striped_line + "< output dir " + output_dir + "<";
                add_file(striped_line, options.debug_bin_dir);
                line_cnt += 1;
            f.close();
            return line_cnt;
    except IOError:
            return 0;
    return 0;

def search_files(src_dir): #recursive search
    flist = flist_exists();
    if flist:
        print "Flist loaded from file [" + flist_name + "](loaded " + str(flist) +" files)";
        return flist;
    found = 0
    files = glob.glob(src_dir+'/*') #gives all files/folders in this directory
        
    for file in files: 
        if search_files(file):
            found = 1
        elif file_is_interesting(file):
            add_file(file, options.debug_bin_dir)
            found = 1
    
    return found

#main
print 'Generating xIDE project files...'
#make the .xiw
f = open ((options.debug_bin_dir+'/'+options.target_root+'.xiw'), 'w')
f.write(('<workspace>\n <project path="'+options.target_root+'.xip" />\n</workspace>\n'))
f.close()

#make the .xip
src_dir = os.path.dirname(os.getcwd())
print '...Searching for source...'
search_files(src_dir)
print '...Adding source list...'
f = open ((options.debug_bin_dir+'/'+options.target_root+'.xip'), 'w') #this build/execution environament stuff is all hard-coded, watch this space
f.write('<project buildenvironment="{6907f3f0-0ed4-11d8-be02-0050dadfe87d}" \
 buildenvironmentname="kalimba" executionenvironmentoption="binutils"\
  buildenvironmentoption="" executionenvironmentname="kalimba"\
executionenvironment="{2ca81310-0ecd-11d8-be02-0050dadfe87d}" >\n')
f.write(interesting_files)
f.write(' <properties currentconfiguration="Release" >\n\
  <configuration name="Release" >\n\
   <property key="buildMatch" >(?:ERROR|FATAL|WARNING|MESSAGE)\
:\s+(file)\s+(line):</property>\n\
   <property key="debugtransport" >')
f.write(transport)
f.write('</property>\n\
   <property key="hardware" >0</property>\n');
f.write('   <property key="subsysid" >'+ str(options.subsysid)+ '</property>\n');
f.write(' </configuration>\n\
 </properties>\n\
</project>\n')
f.close()
print '...done.'
