#! /usr/bin/env python
############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2016 Qualcomm Technologies International, Ltd.
#   
#
############################################################################# 

# Create an LPC file system

import sys, optparse, subprocess, os, tempfile

scriptdir = os.path.dirname(os.path.abspath(__file__))
pylibdir = os.path.join(scriptdir, "pylib")
if not pylibdir in sys.path:
    sys.path.insert(0, pylibdir)

from csr.dev.fw.meta.xuv_stream_decoder import XUVStreamDecoder
from csr.dev.fw.meta.xuv_stream_encoder import XUVStreamEncoder
from csr.dev.fw.meta.read_only_fs import PackfileReadOnlyFS
from csr.wheels.bitsandbobs import bytes_to_words
from csr.dev.tools.flash_image_utils import loadable_to_bytes

DEFAULT_OFFSET = 0x80000
DEFAULT_PACKFILE = "/home/devtools/packfile/20130603/packfile"
REQUIRED_FILE_EXTENSION = ".xuv"

def add_cmd_line_options(parser):
    parser.add_option("-d", "--dir",
                      dest="dir_to_pack",
                      type = "string",
                      help = "Directory to pack",
                      default = ".")

    parser.add_option("-o", "--output",
                      dest="output_file",
                      type = "string",
                      help="Name of output file",
                      default = "")

    parser.add_option("-p", "--packfile",
                      dest="packfile",
                      type = "string",
                      help = "Path to packfile executable",
                      default = DEFAULT_PACKFILE)

    parser.add_option("-f", "--offset",
                      dest="offset",
                      type= "int",
                      help="Address offset",
                      default = DEFAULT_OFFSET)

    parser.add_option("-w", "--program",
                      dest="program",
                      type="string",
                      help = "romloadercmd binary (optional)",
                      default="")

    parser.add_option("-a", "--appsfs",
                      dest="appsfs",
                      action="store_true",
                      help = "Prepares the filesystem for the application subsystem",
                      default=False)

def pack_dir(dir_to_pack, out, packfile=DEFAULT_PACKFILE, offset=DEFAULT_OFFSET, appsfs=False):
    """
    Create a filesystem image starting at the specified address offset
    """
    
    #1. Run packfile on the specified directory
    print "Packing directory..."
    f, tmp_packed_img = tempfile.mkstemp(suffix = ".fs")
    os.close(f) # We close this since we need packfile to write to it first
    
    if subprocess.call([packfile,
                        dir_to_pack,tmp_packed_img], stderr=subprocess.STDOUT) != 0:
        raise RuntimeError("%s failed" % packfile)
    
    #2. Translate the XUV file to the offset addresses
    print "Translating filesystem image by 0x%06x" % offset
    with open(tmp_packed_img) as raw_img:
        decoder = XUVStreamDecoder(raw_img)
        encoder = XUVStreamEncoder(out)
        if appsfs:
            profs = PackfileReadOnlyFS([(a,v) for a,v in decoder.address_value_pairs])
            data = loadable_to_bytes(profs.loadable_le32, verbose=True)
            if len(data) & 1:
                print "padding odd length with a single byte"
                data += bytearray([0xff])
            a = 0
            for v in bytes_to_words(data):
                encoder.address_value_pair(a + offset, v)
                a += 1
        else:
            for a,v in decoder.address_value_pairs:
                encoder.address_value_pair(a+offset,v)
    os.remove(tmp_packed_img)
    
def program_img(program,output_file):
    """
    Use the specified LPC programming tool (romloadercmd) to write the filesystem
    image onto the LPC flash.
    """
    if subprocess.call([program,"program","lpc0",output_file]) != 0:
        raise RuntimeError("%s failed" % program)

def main():
    'main routine used by hydra fw tools package entry point - see setup.py'

    PARSER = optparse.OptionParser()
    add_cmd_line_options(PARSER)
    (options, args) = PARSER.parse_args()

    if options.output_file:
        output_file = options.output_file
        if os.path.splitext(output_file)[1].strip() != REQUIRED_FILE_EXTENSION:
            output_file = output_file + REQUIRED_FILE_EXTENSION
    else:
        output_file = "/tmp/packed.xuv"
        
    with open(output_file,"w") as out:
        
        pack_dir(options.dir_to_pack,out,
                 packfile=options.packfile, offset=options.offset,
                 appsfs=options.appsfs)
        print "Successfully wrote %s" % output_file

    if options.program:
        program_img(options.program, output_file)

if __name__ == "__main__":
    main()
