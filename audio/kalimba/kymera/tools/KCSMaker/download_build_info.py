############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
from os.path import isdir, exists
import sys
import sym_table
import comp_defines
import subprocess
import os
import re

mem_sect_re = re.compile(r'[\d]+[\s]+[\S]+[\s]+([0-9a-f]{8})[\s]+[0-9a-f]{2}([0-9a-f]{6})[\s]+[0-9a-f]{8}[\s]+[0-9a-f]{8}  2\*\*[\d]{1}')


class build_info:

    def __del__(self):
        "build_info destructor removes the temporary files used during if any."
        if exists(self.outputdir + "/readelf.txt"):
            os.remove(self.outputdir + "/readelf.txt")
        if exists(self.outputdir + "/code_sect.txt"):
            os.remove(self.outputdir + "/code_sect.txt")

    def __init__(self, buildroot, buildname, ktoolsroot, outputdir):
        "build_info constructor takes the path of the build directory root"
        # We only bother to read the elf file if we need to so save build
        # location so we can access it later
        self.buildroot = os.path.abspath(buildroot)
        # Work out the build config from the path name
        (path, unused) = os.path.split(self.buildroot)
        (unused, self.config) = os.path.split(path)

        self.ktoolsroot = os.path.abspath(ktoolsroot)
        self.read_elf_flag = False
        self.reade_code_sect_flag = False
        self.buildname = buildname
        self.outputdir = outputdir
        self.elfname = self.buildroot + "/debugbin/" + self.buildname + ".elf"
        self.kal = None

        # before we load the files check that the path is valid
        if isdir(buildroot):
            self.symbols = sym_table.sym_table(buildroot + "/debugbin/" + buildname + ".sym")
            self.cdefs = comp_defines.comp_defines(buildroot + "/../last_defines.txt")
        else:
            print("The build path {0} is invalid. Exiting".format(buildroot))
            sys.exit(-1)

    def get_data_location_start(self):
        "returns the first writable DM address"
        return int(self.symbols.get_symbol("$DM1_ZI_REGION.__Limit"), 16)

    def get_data_location_end(self):
        "returns the last writable DM address"
        #last writable address is the start of the preserved block -1
        return int(self.symbols.get_symbol("$DM1_PRESERVE_BLOCK_REGION.__Base"), 16) - 1

    def get_data_location_range(self):
        return self.get_data_location_start(), self.get_data_location_end()

    def get_elf_name(self):
        return self.elfname

    def get_PMRAM_end(self):
        "returns the usable end of PM ram space, defined by start of ramrun PM space"
        return int(self.cdefs.get_define("RAMRUN_ADDRESS")) - 1

    def get_PMRAM_start(self):
        "returns the start of free PM RAM space (i.e. not used for either cache or code in RAM)"
        try:
            pm_start = int(self.symbols.get_symbol("$PM_CODE_REGION.__Limit"), 16)
        except:
            pm_start = int(self.cdefs.get_define("PM_RAM_FREE_START"))
        # Force 32-bit alignment
        pm_start = (pm_start+3) & (0xfffffc)
        return pm_start

    def read_code_sect(self):
        """This function gets a list of code section data from the elf file.
        This can be used to detemine whether hardware patches are patching MiniM
        or MaxiM instructions. We use the tool kobjdump with the option -h to
        do this."""
        print("Reading code sections...\n")

        sect = subprocess.call(self.ktoolsroot + "/kobjdump -h " + self.elfname + " >"+ self.outputdir +"/code_sect.txt", shell=True)
        if sect != 0:
            print("\nReading code sections from the .elf file failed.\nExiting...")
        print("Reading code sections complete.\n")

    def set_minim_bit(self, address):
        """ This function finds the address in the firmware output and sets bit
        0 to indicate whether the function is MiniM (set) or Maxim (unset)"""
        # If this is the first hardware patch we need to read the elf file first
        if self.reade_code_sect_flag == False:
            self.read_code_sect()
            self.reade_code_sect_flag = True

        sect = open(self.outputdir + "/code_sect.txt")
        found_flag = False

        # Read the file and look for the code section containing the address to
        # be patched. Check the next line to determine if the section is marked
        # as MiniM.
        for line in sect.readlines():
            val = mem_sect_re.findall(line)
            if found_flag == True:
                if "MINIM" in line:
                    # Set the LSB if the user didn't already
                    address = address | 1
                break
            if 0 != len(val):
                size = int(val[0][0], 16)
                start = int(val[0][1], 16)
                end = start + size
                if (start != 0) and (address >= start) and (address < end):
                    found_flag = True

        if False == found_flag:
            print("A HW patch address was not found in the firmware\nExiting...")
            sys.exit(-1)

        return address

    def read_elf_file(self):
        """This function turns the elf file into a readable text file. """
        print("Reading .elf file. This will take some time...\n")

        readelf = subprocess.call(self.ktoolsroot + "/kreadelf -wi " + self.elfname + " > " + self.outputdir + "/readelf.txt", shell=True)
        if readelf != 0:
            print("\nReading .elf file failed.\nExiting...")
            sys.exit(-1)
        print("Reading .elf file complete.\n")

    def get_SW_patch_enum_val(self, name):
        # If this is the first SW patch we need to read the elf file first.
        # This is a long process.
        if self.read_elf_flag == False:
            self.read_elf_file()
            self.read_elf_flag == True

        # Open and search the elf file
        elf = open(self.outputdir + "/readelf.txt")
        val_found_flag = False

        # Tell the user what we're doing as this is a big file
        print("Searching the .elf for the SW patch table....")
        for line in elf:
            if val_found_flag:
                return line.split(" ")[-1]
            elif name in line:
                # We've found the enum name, it's value is at the end of the next line
                val_found_flag = True

        print("Couldn't find the SW patch point in the SW patch table.\nExiting...")
        sys.exit(-1)

    def get_chipname(self):
        # Extract the chip name
        (chipname, unused) = self.config.split('_', 1)
        # Adjust the chipname for a7da as internally we call it a7da_kas
        if chipname == "a7da":
            chipname = "a7da_kas"
        return chipname

    def loadKal(self):
        import kalaccess
        if self.kal == None:
            self.kal = kalaccess.Kalaccess()
            self.kal.sym.load(self.elfname)

    def get_build_id(self):
        self.loadKal()
        addr=self.kal.sym.varfind("$_build_identifier_integer")[0][2]
        if self.kal.sym.dm_octet_addressing:
            if addr%4 == 0:
                return (self.kal.sym.static_dm[addr] & 0xFFFF)
            else:
                return ((self.kal.sym.static_dm[addr & ~3] & 0xFFFF0000)>>16)
        else:
            return (self.kal.sym.static_dm[addr] & 0xFFFF)

    def get_rom_kalaccess(self):
        self.loadKal()
        return self.kal

if __name__ == "__main__":
    buildroot = "../../output/amber_emu_lpc"
    bi = build_info(buildroot)
    rng = bi.get_data_location_range()
    print("Start addr: %.6x\tEnd addr: %.6x" % (rng[0], rng[1]))
    print("RAMRUN_ADDRESS: %.6x" % bi.get_PMRAM_end())
    print("PM Start %.6x" % bi.get_PMRAM_start())
    print(bi.get_SW_patch_enum_val("patch_fn_subreport_event_index"))

