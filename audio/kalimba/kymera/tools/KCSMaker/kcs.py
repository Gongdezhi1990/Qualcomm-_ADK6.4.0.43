############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
from kdc import KDCFile
from downloadFile import downloadFile
import fileinput

class KCSFile(downloadFile):
    STREAM_TYPE = 1
    RANDOM_ACCESS_TYPE = 2
    """Container format that extends the built-in list"""
    def __init__(self, chip_id, build_id):
        self.chip_id = chip_id
        self.build_id = build_id
        self.kdc_files = []
        self.kdc_offset = []
        self.kdc_num_caps = []
        self.kdc_cap_id = []

    def addKDC(self, kdcFile):
        self.kdc_files.append(kdcFile)
        
    def getKDCs(self):
        return self.kdc_files
        
    def getNumKDC(self):
        return len(self.kdc_files) 
    
    def dumpToTextFile(self, kcsFileT):
        # index gets updated every time we write to a file, pass it along 
        index = super(KCSFile,self).dumpToTextFile(kcsFileT, "w", 0)
        
        for kdc in self.kdc_files:
            index = kdc.dumpToTextFile(kcsFileT, "a", index)

    def dumpToBinaryFile(self, kcsFileB):
        super(KCSFile,self).dumpToBinaryFile(kcsFileB)
        
        for kdc in self.kdc_files:
            kdc.dumpToBinaryFile(kcsFileB,"ab+")

    def dumpToDRAMFile(self, kcsFileD):
        [wordready, word16, size_kcs] = super(KCSFile,self).dumpToDRAMFile(kcsFileD, "w")
        for kdc in self.kdc_files:
            [wordready, word16, size_kdc] = kdc.dumpToDRAMFile(kcsFileD, wordready, word16, "a")
            size_kcs = size_kcs + size_kdc
        
        if (wordready == True):
            # Add padding if file is unaligned
            with open(kcsFileD, "a") as f:
                f.write("0x" + word16 + "BEEF")
                size_kcs = size_kcs + 1
        
        # Now patch length
        first = True
        for line in fileinput.input(kcsFileD, inplace=1):
            if first == True:
                # subtract one because length field doesn't count
                print("@40000000 " + str(size_kcs - 1))
                first = False;
            else:
                print(line.rstrip())
            