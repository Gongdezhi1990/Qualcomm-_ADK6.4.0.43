############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
import subprocess
import sys
import os
import capability
import logging
import filecmp
import shutil

class Build_Controller(object):
    '''
    classdocs
    '''


    def __init__(self, buildroot, builddir, outputdir, kcsmaker_path, download_config, elf_name, extra_incdirs, build_name, code_path, os_type, patch_dir):
        self.buildroot = buildroot
        self.outputdir = outputdir
        self.kcsmaker_path = kcsmaker_path
        self.download_config = download_config
        self.builddir = builddir
        self.elf_name = elf_name
        self.extra_incdirs = extra_incdirs
        self.build_name = build_name
        self.codepath = code_path
        self.linkscript_name = ""
        self.linkscript_path_name = ""
        self.kdc_start_path_name = ""
        self.makefile_name = ""
        self.os_type = os_type
        self.patch_dir = patch_dir

    def convertPath(self,path):
        if (self.os_type == "linux"):
            # Convert a windows absolute path into a linux path
            if (path.find(":")!=-1):
                if (path.find(":")!=1):
                    raise Exception("Wrong path:" + str(path))
                else:
                    path = "/" + (path.replace("\\","/")).replace(":","")
            return path
        else:
            return path

    def replace_if_diff(self, target_file, new_version):
        if not os.path.exists(target_file) or not filecmp.cmp(target_file,new_version):
            shutil.copy(new_version, target_file)

    def write_linkscript_and_start_file(self, chip_name, start_name_list, linkscript_template):
        # Takes the template linkscript file and creates a build specific one
        # Open the template and bespoke file to write
        link_tmplt = open(linkscript_template)
        kdc_start_tmplt = open(os.path.join(self.kcsmaker_path,"templates", "kdc_start_tmpl"))
        self.linkscript_name = "linkscript_download"
        self.linkscript_path_name = os.path.abspath(os.path.join(self.outputdir,self.linkscript_name))
        self.kdc_start_path_name = os.path.abspath(os.path.join(self.outputdir,"kdc_start.asm"))
        temp_script_name = self.linkscript_name+".new_vsn"
        temp_script_path_name = os.path.abspath(os.path.join(self.outputdir,temp_script_name))
        temp_start_path_name = self.kdc_start_path_name+".new_vsn"

        if not os.path.exists(os.path.dirname(self.linkscript_path_name)):
            os.makedirs(os.path.dirname(self.linkscript_path_name))
        link_dnld = open(temp_script_path_name, "w")

        for line in link_tmplt:
            if chip_name in ["crescendo", "stre", "auraplus", "napier"]:
                if "ENTRY($dummy)" in line:
                    line = "ENTRY($kdc_start)\n"
            else:
                if "start $" in line:
                    line = "start $kdc_start;\n"
            link_dnld.write(line)

        link_dnld.close()
        link_tmplt.close()

        kdc_start = open(temp_start_path_name, "w")

        for line in kdc_start_tmplt:
            if "r0" in line:
                line = ""
                for name in start_name_list:
                    line+= "r0 = " + str(name) + ";\n"
            kdc_start.write(line)

        kdc_start.close()
        kdc_start_tmplt.close()

        self.replace_if_diff(self.linkscript_path_name, temp_script_path_name)
        self.replace_if_diff(self.kdc_start_path_name, temp_start_path_name)

    def write_makefile(self, cap_list, exe_name):
        "Takes the makefile_tmpl file and creates a build specific one"
        # get a list of download files to compile
        c_list = []
        asm_list = []
        dyn_list = []
        cap_name_list = []
        cfg_file_list = []
        for cap in cap_list:
            (c_list_extend, asm_list_extend, dyn_list_extend) = cap.get_file_list()
            c_list.extend(c_list_extend)
            asm_list.extend(asm_list_extend)
            dyn_list.extend(dyn_list_extend)
            cap_name_list.append(cap.get_cap_name())
            for cfg_files in cap.get_config_files():
                cfg_file_list.append(cfg_files)

        logging.debug("cap_name_list=" + str(cap_name_list))
        logging.debug("cfg_file_list=" + str(cfg_file_list))
        self.makefile_name = self.outputdir + "/makefile"
        temp_makefile_name = self.makefile_name+".new_vsn"

        # Open the template and the makefile to create
        make_tmplt = open(self.kcsmaker_path + "/templates/makefile_tmpl")
        make_patch = open(temp_makefile_name, "w")
        # Save the patch director pdir so we can delete the temporary makefile later
        self.pdir = os.path.abspath(".")
        logging.debug("Makefile in " + self.pdir)

        for line in make_tmplt:
            if "EXTRA_INCDIRS +=" in line:
                line = "EXTRA_INCDIRS += " + self.extra_incdirs + "\n"
            elif "export DNLD_CAP_NAMES =" in line:
                line = "export DNLD_CAP_NAMES = " + " ".join(cap_name_list) + "\n"
            elif "export DNLD_CONFIG_FILES =" in line:
                line = "export DNLD_CONFIG_FILES = " + " ".join(cfg_file_list) + "\n"
            elif "BUILD_ROOT = " in line:
                line = "BUILD_ROOT = $(abspath " + self.convertPath(self.buildroot) + ")\n"
            elif "export DOWNLOAD_CODE_ROOT = ./build" in line:
                line = "export DOWNLOAD_CODE_ROOT = " + self.convertPath(self.codepath) + "\n"
            elif "export DOWNLOAD_OUTPUT = kymera_download" in line:
                line = "export DOWNLOAD_OUTPUT = " + self.convertPath(self.outputdir) + "\n"
            elif "export DOWNLOAD_BUILD_DIR =" in line:
                line = "export DOWNLOAD_BUILD_DIR = " + self.convertPath(self.builddir) + "\n"
            elif "export TARGET_ELF_BUILD_NAME =" in line:
                line = "export TARGET_ELF_BUILD_NAME = " + self.build_name + "\n"
            elif "export LINKSCRIPT_PATH =" in line:
                line = "export LINKSCRIPT_PATH = " + self.convertPath(self.outputdir) + "\n"
            elif "export TARGET_MAKEFILE =" in line:
                line = "export TARGET_MAKEFILE = $(abspath " + self.convertPath(self.makefile_name) + ")\n"
            elif "export LINK_EXTRA_FLAGS =" in line:
                line = "export LINK_EXTRA_FLAGS = --just-symbols=" + self.convertPath(self.elf_name) + "\n"
            elif "TGT_EXE = \"kymera-download\"" in line:
                line = "TGT_EXE = " + exe_name + "\n"
            elif "H_PATH+=" in line:
                line = "H_PATH+=" + self.patch_dir + "\n"

            make_patch.write(line)

        make_patch.close()
        make_tmplt.close()

        self.replace_if_diff(self.makefile_name, temp_makefile_name)

    def build(self, binfo, cap_list, exe_name, linkscript_template, make_cmd, clean_first=True, make_jobs=True):
        """Invokes make to make the patches. This function isn't called if the
        patch only contains data patches hence we should always find one"""
        start_name_list = []
        for cap in cap_list:
            (file_name ,start_name) = cap.get_cap_data()
            logging.debug("Found in " + file_name + ", capability data: " + start_name)
            # Save the cap_data variable name inside python's capability structure
            cap.setSymCapDataName(start_name)
            # Add this variable name to a list of names
            start_name_list.append("$_" + start_name.strip("$_"))

        logging.debug("Generating linkscript and start file...")
        # Create the linkscript and the makefile
        self.write_linkscript_and_start_file(binfo.get_chipname(), start_name_list, linkscript_template)
        logging.debug("Generating makefile...")
        self.write_makefile(cap_list, exe_name)
        # Do a clean build first to be safe, then the real build
        logging.debug("Cleaning output directory.")
        import os
        logging.debug(os.path.abspath(os.curdir))

        if make_jobs:
            make_cmd += " --jobs=16"
        if clean_first:
            logging.debug("Starting clean. " + make_cmd + " -s -f " + self.makefile_name + " CONFIG=" + self.download_config + " OSTYPE=" + self.os_type + "clean")
            clean = subprocess.call(make_cmd + " -s -f " + self.makefile_name + " CONFIG=" + self.download_config + " OSTYPE=" + self.os_type + " clean", shell=True)
            if clean != 0:
                logging.error("\nClean build failed.\nExiting...")
                sys.exit(-1)
        logging.debug("Starting make. " + make_cmd + " -s -f " + self.makefile_name + " CONFIG=" + self.download_config + " OSTYPE=" + self.os_type)
        make = subprocess.call(make_cmd + " -s -f " + self.makefile_name + " CONFIG=" + self.download_config + " OSTYPE=" + self.os_type, shell=True)
        # Check that the build process completed successfully
        if make != 0:
            logging.error("\nBuild Process failed.\nExiting...")
            sys.exit(-1)

if __name__ == "__main__":
    b = Build_Controller()
    b.build()
