# Copyright (c) 2016 Qualcomm Technologies International, Ltd.
#   
#
# This file dumps a XUV file from an ELF file
#
# The first parameter specifies the path to the ELF file of the firmware build.
# The second parameter is optional and is used to specify the output XUV file.
# If this is missing it will be derrived from the ELF file.
#

import os
import sys
import re

scriptdir = os.path.dirname(os.path.abspath(__file__))
pylibdir = os.path.join(scriptdir, "..", "pylib")
if not pylibdir in sys.path:
    sys.path.append(pylibdir)

from csr.dev.fw.meta.elf_code_reader import ElfCodeReader
from csr.dev.hw.core.meta.i_core_info import Kalimba32CoreInfo
from csr.wheels.bitsandbobs import bytes_to_words
from csr.dev.fw.meta.xuv_stream_encoder import XUVStreamEncoder


def elf2xuv(offset, elf_filename, xuv_filename = None):
    """
    Reads an elf file and dumps the loadable sections in an XUV file.
    The offset parameter is the number of words by which to shift the address
    in the XUV file. This is usually 0.
    """
    if xuv_filename is None:
        xuv_filename = elf_filename.replace(".elf", ".xuv")
    
    print "Loading elf symbols from %s" % elf_filename
    layout_info = Kalimba32CoreInfo().layout_info
    elfcode = ElfCodeReader(elf_filename, layout_info)
    
    xuv_file = open(xuv_filename,"w")
    xuv_file.write("// Apps Firmware image dumped from ELF by %s\n" %
                       sys.argv[0])
    xuv_encoder = XUVStreamEncoder(xuv_file)
    
    for section in elfcode.sections:
         addr = section.paddr
         data = section.data
         name = section.name
         print("  %s block starting at 0x%08x, %d bytes" %
                   (name, addr, len(data)))
         words = bytes_to_words(data)
         for value in words:
             xuv_encoder.address_value_pair((addr + offset) / 2, value)
             addr += 2
    
    xuv_file.close()
    print "Done.\nCreated %s" % xuv_filename


if __name__ == "__main__":
    """
    After parsing the arguments it kicks off the XUV generator.
    """
    try:
        elf_filename = sys.argv[1]
    except IndexError:
        print "Supply directory containing elf as a command line parameter"
        exit(1)
    
    if len(sys.argv) >=3:
        xuv_filename = sys.argv[2]
    else:
        xuv_filename = None
    
    elf2xuv(0, elf_filename, xuv_filename)

