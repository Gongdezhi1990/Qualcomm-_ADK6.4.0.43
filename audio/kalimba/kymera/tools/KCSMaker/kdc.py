############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
import types
import logging
from downloadFile import downloadFile

class KDCFile(downloadFile):
    """Container format that extends the built-in list to inclue comment strings
    for the output kdc file."""
    def __init__(self, varLut):
        self.varLut = varLut
        self.capabilities = []
        self.PM_size = 0
        self.DM1_size = 0
        self.DM2_size = 0
        self.CONST_size = 0
        self.CONST16_size = 0
        super(KDCFile,self).__init__()
    
    def print_memory_usage(self):
        print("-> PM Size:      {s1} bytes ({s2} KiB)".format(s1=self.PM_size, s2=round(self.PM_size/1024.0,3)))
        print("-> DM1 Size:     {s1} bytes ({s2} KiB)".format(s1=self.DM1_size, s2=round(self.DM1_size/1024.0,3)))
        print("-> DM2 Size:     {s1} bytes ({s2} KiB)".format(s1=self.DM2_size, s2=round(self.DM2_size/1024.0,3)))
        print("-> CONST Size:   {s1} bytes ({s2} KiB)".format(s1=self.CONST_size, s2=round(self.CONST_size/1024.0,3)))
        print("-> CONST16 Size: {s1} bytes ({s2} KiB)".format(s1=self.CONST16_size, s2=round(self.CONST16_size/1024.0,3)))

    def addPadding(self, quantity):
        for i in range(quantity):
            self.append( 0x0000, "Padding to ensure minimum amount of data in working buffer" )
        return

    def addCapability(self, cap):
        self.capabilities.append(cap)

    def getNumCapabilities(self):
        return len(self.capabilities)

    def getCapabilities(self):
        return self.capabilities

    def dumpToTextFile(self, kdcFileT, write_mode="w", index=0):
        # KDC Files add variable information here in text mode
        logging.debug(self.varLut)
        with open(kdcFileT, write_mode) as f:
            for (name, v) in sorted(self.varLut.items(), key=lambda x: x[1].addr):
                if v.blck is None:
                    f.write("# %d : %s\n" % (v.id, name))
                else:
                    f.write("# (%d) : %s\n" % (v.blck.id, name))
        # Redirect normal execution to parent, append to existing
        return super(KDCFile,self).dumpToTextFile(kdcFileT, "a", index)

    def dumpToDRAMFile(self, kdcFileD, wordready, word32, write_mode="w"):
        with open(kdcFileD, write_mode) as f:
            words16bit = ["%04x" % (k_s[0]) for k_s in self]
            words32bit = []
            if wordready == False:
                word32=""
            for word16 in words16bit:
                word32 = word32 + word16
                if wordready == True:
                    words32bit.append("0x" + word32)
                    wordready = False
                    word32 = ""
                else:
                    wordready = True
            f.write( "\n".join( words32bit ) )
            f.write("\n")
            return wordready, word16, len(words32bit)