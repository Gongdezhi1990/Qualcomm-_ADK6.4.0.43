############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
import sys
import subprocess as SP
import os
import shutil
import stat

# This script generate image.fs
# input: sys.argv[1] - KAPPATH : path to the .kap file
#        sys.argv[2] - OUTPUTDIR : path to the output folder
#        sys.argv[3] - OSTYPE : to check if it is Linux or Windows

def remove_readonly(func, path, excinfo):
    """ make read only files writeable, so that thay can be deleted """
    os.chmod(path, stat.S_IWRITE)
    func(path)

def rm_rf(path):
    """ mimic Unix rm -rf """
    shutil.rmtree(path, onerror=remove_readonly)

print '============================='
print 'COMMAND LINE:', sys.argv
print '============================='

KAPPATH = os.path.normpath(sys.argv[1])
OUTPUTDIR = os.path.normpath(sys.argv[2])

# Set default location of packfile tool. It is assumed to be the one under devtools
# if no OS is specified in sys.argv[3]
PAKFILE = "/home/devtools/packfile/20130603/packfile"

if len(sys.argv) > 3:
    # The OS was specified on the command line, so select the right packfile tool
    OSTYPE = sys.argv[3].lower()
    if "linux" in OSTYPE:
        PAKFILE = "/home/svc-audio_dsp/packfile/packfile_posix/packfile"
    elif "windows" in OSTYPE:
        PAKFILE  =  os.path.join(os.environ["DEVKIT_ROOT"], 'tools', 'bin', 'packfile.exe')
    else:
        sys.stderr.write('Unknown OS Type %s' % OSTYPE)
        sys.exit(-1)

PAKFILE = os.path.normpath(PAKFILE)
TEMP_DIR = os.path.join(OUTPUTDIR, 'tempdir')

# Create a temporary folder to build the directory tree
if os.path.exists(TEMP_DIR):
    rm_rf(TEMP_DIR)
IMAGE_DIR = 'image'
DSP_DIR = 'dsp'
IMAGE_FS = 'image.fs'
os.makedirs(os.path.join(TEMP_DIR, IMAGE_DIR, DSP_DIR))

# Copy and rename .kap to previn.kap for the packfile
shutil.copy(KAPPATH, os.path.join(TEMP_DIR, IMAGE_DIR, DSP_DIR,'framework.kap'))

#Execute packfile.
os.chdir(TEMP_DIR)
try:
    ret_val = SP.check_call([PAKFILE, IMAGE_DIR, IMAGE_FS, '-vmver', '1'])
except SP.CalledProcessError as exc:
    sys.stderr.write('Call to packfile failed. Return code %s' % exc.returncode)
    sys.exit(exc.returncode)

# Copy result to the output directory (given as command line argument)
os.chdir(OUTPUTDIR)
shutil.copy(os.path.join(TEMP_DIR, IMAGE_FS), IMAGE_FS)

# Remove temporary folder
rm_rf(TEMP_DIR)
print "Successfully converted to %s" % os.path.join(OUTPUTDIR, IMAGE_FS)
