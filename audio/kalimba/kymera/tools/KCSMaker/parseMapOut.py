############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
import subprocess
import re
import logging
import os

sectionPMRe = re.compile(".*PM.*\?(.*[__minim|__maxim])$",re.IGNORECASE)
addressPM = re.compile(".*0x([0-9a-fA-F]{8}) *0x([0-9a-fA-F]*) *(.*)",re.IGNORECASE)
text_reset = re.compile(".text_reset")
varStartRe = re.compile("^ ((DMZI)|(DM)|(DM1)|(DM2)|(CONST16)|(CONST)|(DMCONST))\?(.*)",re.IGNORECASE)

def getModules(fileName, mappings):
    class module(object):
        def __init__(self, m_name):
            self._mod_name = m_name
            self._low_pc = 0
            self._high_pc = 0
        def __repr__(self):
            return "{fname} ({low} - {high})\n".format(fname=self.get_name(), low=hex(self.get_low_pc()), high=hex(self.get_high_pc()))
        def __cmp__(self, other):
            if other == None:
                return -1
            return self.get_low_pc() > other.get_low_pc()
        def __lt__(self, other):
            if other == None:
                return True
            return self.get_low_pc() < other.get_low_pc()
        def _set_pc(self, pc):
            return pc & 0x7FFFFFFF
        def set_low_pc(self, pc):
            self._low_pc = self._set_pc(pc)
        def set_high_pc(self, pc):
            self._high_pc = self._set_pc(pc)
        def get_low_pc(self):
            return self._low_pc
        def get_high_pc(self):
            return self._high_pc
        def get_name(self):
            return self._mod_name
        def set_file_name(self, f_name, mappings):
            if not self._mod_name.startswith("$"):
                if self._mod_name in mappings:
                    if mappings[self._mod_name].startswith("$"):
                        # Then it's a global symbol so don't mangle the name
                        return
                if f_name.find("(") != -1:
                    # Example: turn
                    # /basic_passthrough/debugbin/libbasic_passthrough/basic_passthrough/debugbin/libbasic_passthrough.a(basic_passthrough.o)
                    # into
                    # /basic_passthrough/debugbin/libbasic_passthrough/basic_passthrough/debugbin/libbasic_passthrough.a@basic_passthrough.o%
                    f_name = f_name.replace("(","@")
                    f_name = f_name.replace(")","%")
                self._mod_name = f_name + self._mod_name


    f = open(fileName,"r")
    mod_list = []
    cur_mod = None
    start_matching = False
    for line in f:
        if not start_matching:
            m = text_reset.match(line)
            if m is not None:
               start_matching = True
        else:
            # is this is a start of a PM module?
            if cur_mod != None:
                m = addressPM.match(line)
                if m is None:
                    raise Exception("Expecting a PM address here: {}".format(line))
                else:
                    cur_mod.set_low_pc(int(m.group(1),16))
                    cur_mod.set_high_pc(cur_mod.get_low_pc() + int(m.group(2),16) - 1)
                    cur_mod.set_file_name(m.group(3), mappings)
                cur_mod=None
            else:
                m = sectionPMRe.match(line)
                if m is not None:
                    mod_name = m.group(1)
                    cur_mod = module(mod_name)
                    mod_list.append(cur_mod)
    mod_list = sorted(mod_list)
    # check for gaps
    prev_mod = mod_list[0]
    firstGap = True
    MAX_GAP_MINIM_BYTES = 2
    for mod in mod_list[1:]:
        if mod.get_low_pc() != prev_mod.get_high_pc() + 1:
            if firstGap:
                if mod.get_low_pc() > prev_mod.get_high_pc() + 1 + MAX_GAP_MINIM_BYTES:
                    raise Exception("Gap between {m1} and {m2} is too large".format(m1=prev_mod,m2=mod))
                else:
                    logging.debug("Gap between {m1} and {m2} is {m3} bytes".format(m1=prev_mod,m2=mod,m3=str(prev_mod.get_high_pc() - mod.get_low_pc())))
                firstGap = False
            else:
                raise Exception("Error. More than 1 gaps in PM sections")
        prev_mod = mod
    f.close()
    return mod_list

def getVariables(fileName, start_address):

    def extendVarName(varName, prefix):
        fullname = prefix.replace("(","@")
        fullname = fullname.replace(")","%")
        return fullname + varName

    f = open(fileName,"r")
    dm2StartText = "{}                . = $DM2_RAM_BASE_ADDRESS".format(hex(start_address)[:-1])
    start_matching = False
    waitForAddr = False
    vars ={}
    for line in f:
        if not start_matching:
            if line.lstrip().startswith(dm2StartText):
               start_matching = True
        elif not waitForAddr:
            m=varStartRe.match(line)
            if m is not None:
                splitLine = m.groups()[8].split()
                varName = splitLine[0]
                varType = m.groups()[0].upper()
                if varType != "CONST" and varType != "CONST16" and varType != "DMCONST":
                    varType = "NOT_CONST"
                if len(splitLine) > 1:
                    varAddr = int(splitLine[1],16)
                    varSize = int(splitLine[2],16)
                    if not varName.startswith("$"):
                        varName = extendVarName(varName, splitLine[3])
                    # Add variable
                    vars[varName] = (varAddr, varSize, varType)
                else:
                    waitForAddr = True
        else:
            splitLine = line.split()
            varAddr = int(splitLine[0],16)
            varSize = int(splitLine[1],16)
            if not varName.startswith("$"):
                varName = extendVarName(varName, splitLine[2])
            # Add variable
            vars[varName] = (varAddr, varSize, varType)
            waitForAddr = False
    return vars
