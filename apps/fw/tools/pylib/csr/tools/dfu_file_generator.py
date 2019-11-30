#!/usr/bin/env python
# Copyright (c) 2017 Qualcomm Technologies International, Ltd.
#   
#
# This tool creates an xuv image for programming into the flash memory
# device. It generate images for all sections seprately
# according to the specification
# http://cognidox/
#
import os
import sys
import struct
import subprocess
import time
import argparse

scriptdir = os.path.dirname(os.path.abspath(__file__))
pylibdir = os.path.join(scriptdir, "..", "..")
if not pylibdir in sys.path:
    sys.path.insert(0,pylibdir)
from csr.dev.tools.flash_image_builder import ImageBuilder

# Filename MUST match those used in the "partition" lines of the (flash_layout_config.py)
# flash0["host_tools"]["crescendo_upd_config"] file (i.e. in the crescendo.upd file)
# $CWD is \depot\app_ss\main\fw\tools\pylib\csr\tools unless running from other location
# In the crescendo.upd file, name of the nonce image file
NONCEIMAGEFILE = "partition_0_nonce.xuv"
# In the crescendo.upd file, name of the image header
IMAGEHEADERFILE = "partition_1_image_header.xuv"
# In the crescendo.upd file, name of the curator fs file
CURFS = "curator_config_filesystem.xuv"
# In the crescendo.upd file, name of the P0 image file
P0IMAGEFILE = "app_p0.xuv"
# In the crescendo.upd file, name of the P1 image file
P1IMAGEFILE = "app_p1.xuv"
# In the crescendo.upd file, name of the ro fs image file
ROFS = "customer_ro_filesystem.xuv"
# In the crescendo.upd file, name of the apps config fs file
APPSCONFIGFS = "firmware_config_filesystem.xuv"
# In the crescendo.upd file, name of the device config fs file
DEVICECONFIGFS = "device_config_filesystem.xuv"
# In the crescendo.upd file, the name of the footer file
FOOTERFILE = "footer.xuv"
# The name of the file containing the private RSA key
PRIVATEKEY = "private.pem"
# Image file $CWD\dfu_files\for_signing.xuv file used for signing
FILEFORSIGNING = "for_signing.xuv"

# Size of individually erasable blocks for image header in the main section
# of the flash device (i.e. other than boot blocks).
MAX_IMAGE_HEADER = 1024
MAX_NONCE = 8


class dfu_file_generator():
    def __init__(self, config_path):
        self.dfu_file_generator_config_python = config_path
        self.dfu_dir = os.path.join(os.getcwd(), "dfu_files")

    def initiate(self):
        self.check_config_file()
        try:
            # If the config file has specified a different output folder, use that
            self.user_dfu_dir = flash0["host_tools"]["dfu_dir"]
            if self.user_dfu_dir != "":
                self.dfu_dir = self.user_dfu_dir
        except:
            pass
        if flash0["dfu_image"]["gen_flash_image"] == "True":
            self.build_flash_image()
        self.nonce = self.create_nonce()
        self.image_header = self.create_image_header()
        self.curfs = self.create_cur_fs()
        self.apps_config_fs = self.create_apps_ro_config_fs()
        self.ro_fs = self.create_rofs()
        self.P0_image = self.create_P0_image()
        self.P1_image = self.create_P1_image()
        self.footer = self.create_footer()
        self.upgrade_file_generator()

        if self.nonce and self.image_header and self.curfs and self.apps_config_fs and self.ro_fs \
                and self.P0_image and self.P1_image and self.footer:
            return True
        else:
            return False

    def check_config_file(self):
        if self.dfu_file_generator_config_python.endswith(".py"):
            execfile(self.dfu_file_generator_config_python, globals())
        else:
            print"Config file format is wrong"
            sys.stdout.flush()

    # Create nonce image from original image generated using flash_image_builder.py
    def create_nonce(self):
        ret = False
        nonce_file = os.path.join(self.dfu_dir, "nonce.xuv")
        try:
            self.nonce_fd = open(nonce_file, 'w')
        except IOError as exception:
            print "Error {} on file {}\n".format(exception.errno, nonce_file)
            return False
        try:
            with open(os.path.join(self.dfu_dir, "dfu_image_header.xuv"), 'r') as self.fp:
                self.nonce_data = self.fp.readlines()
                for x in range(len(self.nonce_data)):
                    self.counter = x
                    for _ in range(MAX_NONCE):
                        self.nonce_fd.write(self.nonce_data[self.counter])
                        self.counter += 1
                    break
                self.nonce_fd.close()
                ret = self.offset_xuv(nonce_file, NONCEIMAGEFILE)
                os.remove(nonce_file)
                return ret
        except IOError as exception:
            print "Error {} on file {}\n".format(exception.errno,
                os.path.join(self.dfu_dir, "dfu_image_header.xuv"))
            return False

    # Create P0 image header from original image generated using flash_image_builder.py
    def create_image_header(self):
        ret = False
        header_file = os.path.join(self.dfu_dir, "image_header.xuv")
        try:
            self.image_header_fd = open(header_file, 'w')
        except IOError as exception:
            print "Error {} on file {}\n".format(exception.errno, header_file)
            return False
        try:
            with open(os.path.join(self.dfu_dir, "dfu_image_header.xuv"), 'r') as self.fp:
                self.image_header_data = self.fp.readlines()
                self.wrt_flag = True
                for x in range(MAX_NONCE, len(self.image_header_data)):
                    self.image_header_fd.write(self.image_header_data[x])
            self.image_header_fd.close()
            ret = self.offset_xuv(header_file, IMAGEHEADERFILE)
            os.remove(header_file)
            return ret
        except IOError as exception:
            print "Error {} on file {}\n".format(exception.errno,
                os.path.join(self.dfu_dir, "dfu_image_header.xuv"))
            return False

    # Create curator FS with an offset of zero.
    def create_cur_fs(self):
        ret = False
        for file_type in range(len(flash0["layout"])):
            if flash0["layout"][file_type][0] == "curator_fs":
                try:
                    ret = self.offset_xuv(os.path.join(self.dfu_dir, "dfu_curator_fs.xuv"), CURFS)
                except KeyError:
                    print "Partial update: no %s\n" % flash0["layout"][file_type][0]
                    sys.stdout.flush()
                    return True
        return ret

    # Create P0 image with an offset of zero.
    def create_P0_image(self):
        ret = False
        for file_type in range(len(flash0["layout"])):
            if flash0["layout"][file_type][0] == "apps_p0":
                try:
                    ret = self.offset_xuv(os.path.join(self.dfu_dir, "dfu_apps_p0.xuv"), P0IMAGEFILE)
                except KeyError:
                    print "Partial update: no %s\n" % flash0["layout"][file_type][0]
                    sys.stdout.flush()
                    return True
        return ret

    # Create P1 image with an offset of zero.
    def create_P1_image(self):
        ret = False
        for file_type in range(len(flash0["layout"])):
            if flash0["layout"][file_type][0] == "apps_p1":
                try:
                    ret = self.offset_xuv(os.path.join(self.dfu_dir, "dfu_apps_p1.xuv"), P1IMAGEFILE)
                except KeyError:
                    print "Partial update: no %s\n" % flash0["layout"][file_type][0]
                    sys.stdout.flush()
                    return True
        return ret

    # Apps read-only config filesystem
    def create_apps_ro_config_fs(self):
        ret = False
        for file_type in range(len(flash0["layout"])):
            if flash0["layout"][file_type][0] == "ro_cfg_fs":
                try:
                    ret = self.offset_xuv(os.path.join(self.dfu_dir, "dfu_ro_cfg_fs.xuv"), APPSCONFIGFS)
                except KeyError:
                    print "Partial update: no %s\n" % flash0["layout"][file_type][0]
                    sys.stdout.flush()
                    return True
        return ret


    # Create RO FS image with an offset of zero.
    def create_rofs(self):
        ret = False
        for file_type in range(len(flash0["layout"])):
            if flash0["layout"][file_type][0] == "ro_fs":
                try:
                    ret = self.offset_xuv(os.path.join(self.dfu_dir, "dfu_ro_fs.xuv"), ROFS)
                except KeyError:
                    print "Partial update: no %s\n" % flash0["layout"][file_type][0]
                    sys.stdout.flush()
                    return True
        return ret

    def offset_xuv(self, file, finalfile):
        self.nvscmd = flash0["host_tools"]["NvsCmd"]
        self.file = os.path.join(os.getcwd(), file)
        self.finalfile = finalfile
        if not os.path.exists(self.dfu_dir):
            os.makedirs(self.dfu_dir)
        else:
            self.fileList = os.listdir(self.dfu_dir)
            for self.f in self.fileList:
                if self.f == self.finalfile:
                    self.f = os.path.join(self.dfu_dir, self.f)
                    os.remove(self.f)

        self.finalfile = os.path.join(self.dfu_dir, self.finalfile)
        try:
            with open(self.file, 'r') as self.fp:
                self.data = self.fp.readlines()
        except IOError as exception:
            print "Error {} on file {}\n".format(exception.errno, self.file)
            return False

        if self.data[0][-5:-1] == 'FFFF' and self.data[1][-5:-1]  == 'FFFF':
            print("File {}\n\tline 0: {}\n\tline 1: {}").format(
                file, self.data[0], self.data[1])
            print("File cannot start with FFFF FFFF\nPlease try again\n")
            sys.stdout.flush()
            return False

        for x in range(len(self.data)):
            if "@" in self.data[x]:
                self.start_data = self.data[x]
                break
        self.startaddress = "-" + str(int(self.start_data[1:-7], 16))
        subprocess.call([self.nvscmd, "relocatexuv", self.file, self.finalfile, self.startaddress])
        return True


    # Create an xuv file containing the footer from all partitions to go in the
    # DFU file
    def create_footer(self):
        # Determine if SECURITYCMD_XUVE is already set and, if it is, store it
        prev = None
        try:
            prev = os.environ["SECURITYCMD_XUVE"]
        except KeyError:
            pass
        # Set SECURITYCMD_XUVE to "B" for big-endian
        os.environ["SECURITYCMD_XUVE"] = "B"

        # We use all partitions to go in the DFU file for signing, but swap the
        # endian-ness and concatonate into a single file to get it working with
        # the SecurityCmd tool.
        try:
            with open(flash0["host_tools"]["crescendo_upd_config"], 'r') as f:
                lines = f.readlines()
        except IOError as err:
            print "Error {} on file {}\n".format(err.errno,
                flash0["host_tools"]["crescendo_upd_config"])
            sys.stdout.flush()
            return False

        # Look for lines in the upd file with the format
        # "partition x y file" where:
        # - x is expected to be 0 for the SQIF number
        # - y is the partition number and we include all partitions
        # - file is the name of the partition xuv file
        upd_partitions = []

        for line in lines:
            split_line = line.split()
            if len(split_line) >= 4 and \
                split_line[0] == "partition" and \
                split_line[1] is "0":
                    if flash0["signing_mode"] == "header":
                        if split_line[2] is "1":
                            upd_partitions.append(split_line[4])
                    else:
                        upd_partitions.append(split_line[4])

        if len(upd_partitions) == 0:
            print "File {} contains no partitions\n".format(
                flash0["host_tools"]["crescendo_upd_config"])
            sys.stdout.flush()
            return False

        linenum = 0
        try:
            with open(os.path.join(self.dfu_dir, FILEFORSIGNING), 'w') as ff:
                for partition_file in upd_partitions:
                    print "Including {} in signature".format(partition_file)
                    sys.stdout.flush()
                    try:
                        with open(partition_file, 'r') as pf:
                            lines = pf.readlines()
                    except IOError as err:
                        print "Error {} on file {}\n".format(err.errno,
                            partition_file)
                        sys.stdout.flush()
                        return False

                    # Swap the endian-ness of the uint16s read from the xuv file
                    for line in lines:
                        temp1 = format(linenum, '06X')
                        linenum += 1
                        temp2 = line[12:14]
                        temp3 = line[10:12]
                        line = '@' + temp1 + '   ' + temp2 + temp3 + '\n'
                        ff.write(line)

        except IOError as err:
            print "Error {} on file {}\n".format(err.errno,
                os.path.join(self.dfu_dir, FILEFORSIGNING))
            sys.stdout.flush()
            return False

        self.SecurityCmd = flash0["host_tools"]["SecurityCmd"]
        headerfile = os.path.join(self.dfu_dir, FILEFORSIGNING)
        footerfile = os.path.join(self.dfu_dir, FOOTERFILE)

        givenprivatekey = False
        try:
            # See if the RSA-2048 private key file has been specified explicitly
            privatekey = os.path.abspath(os.path.join(
                flash0["host_tools"]["folder_for_rsa_files"], PRIVATEKEY))
            givenprivatekey = True
        except KeyError:
            # The RSA-2048 private key file has not been specified explicitly
            # The RSA-2048 private key is in the folder two levels above the
            # self.dfu_dir folder. I.e. above <YYYMMDDHHMMSS>\output
            privatekey = os.path.abspath(os.path.join(
                self.dfu_dir, "..", "..", PRIVATEKEY))

        if not os.path.isfile(headerfile):
            print "{} file does not exist".format(headerfile)
            sys.stdout.flush()
            sys.exit(1)

        if not os.path.isfile(privatekey):
            print "Private key file {} does not exist".format(privatekey)
            if not givenprivatekey:
                print "Please run the QMDE 'Tools|Setup DFU security' menu option"
            sys.stdout.flush()
            sys.exit(1)
        else:
            print "Using private key {}".format(privatekey)
            sys.stdout.flush()

        subprocess.call([self.SecurityCmd, "-product", "hyd", "sign", headerfile, footerfile, privatekey])
        if prev != None:
            # SECURITYCMD_XUVE was set before so restore it
            os.environ["SECURITYCMD_XUVE"] = prev
        else:
            # SECURITYCMD_XUVE was not set before so delete it
            del os.environ["SECURITYCMD_XUVE"]
        if os.path.isfile(footerfile):
            return True
        else:
            return False

    def upgrade_file_generator(self):
        self.upgradefilegen = flash0["host_tools"]["UpgradeFileGen"]
        self.crescendoupdconfig = flash0["host_tools"]["crescendo_upd_config"]
        self.crescendo_ufg = os.path.join(self.dfu_dir, "dfu_file.xuv")
        filename = "{}_dfu_file.bin".format(flash0["chip_type"])
        self.crescendo_dfu = os.path.join(self.dfu_dir, filename)
        subprocess.call([self.upgradefilegen, self.crescendoupdconfig, self.crescendo_ufg])
        time.sleep(1)
        subprocess.call([self.nvscmd, "xuv2bin", self.crescendo_ufg, self.crescendo_dfu])
        sys.stdout.flush()

    def attach_to_device(self):
        from csr.front_end.pydbg_front_end import PydbgFrontEnd
        print 'Attaching to Device @ "trb:scar"...'
        sys.stdout.flush()
        device, _ = PydbgFrontEnd.attach({"firmware_builds" : None,
                                          "device_url" : "trb:scar"},
                                          shell=None)
        return device
        
    def build_flash_image(self):
        if flash0["encrypt"] == True:
            if flash0["hardware_encrypted"] == True:
                # Need to attach to a device to be able to encrypt from it
                tools_spec = os.path.join(flash0["host_tools"]["devkit"], "apps", "fw", "tools"
                    ).replace('/', os.path.sep)
                if not os.path.isdir(tools_spec):
                    print "Folder {} not found\n".format(tools_spec)
                    return None
                sys.path.insert(0,tools_spec)
                # Add the path to apps\fw\tools\pylib to sys.path
                pylib_spec = os.path.join(flash0["host_tools"]["devkit"], "apps", "fw",
                    "tools", "pylib").replace('/', os.path.sep)
                if not os.path.isdir(pylib_spec):
                    print "Folder {} not found\n".format(pylib_spec)
                    return None
                sys.path.insert(0,pylib_spec)
                from csr.transport.trbtrans import TrbErrorCouldNotEnumerateDevice
                try:
                    device = self.attach_to_device()
                except (TrbErrorCouldNotEnumerateDevice, NotImplementedError):
                    print "Unable to attach to device to encrypt"
                    sys.stdout.flush()
                    sys.exit(1)
                try:
                    builder = ImageBuilder(flash0, device.chip.apps_subsystem.dma,
                        select_bank=flash0["dfu_image"]["bank"])
                    device.chip.curator_subsystem.siflash.identify(4,0)
                except:
                    print "Unable to communicate with the device to encrypt"
                    sys.stdout.flush()
                    sys.exit(1)
            else:
                try:
                    builder = ImageBuilder(flash0, file=flash0["encryption_file"],
                        select_bank=flash0["dfu_image"]["bank"])
                except KeyError:
                    print "No software encryption file supplied"
                    sys.stdout.flush()
                    sys.exit(1)
        else:
            # Not encrypted, so don't need to attach to a device
            builder = ImageBuilder(flash0, select_bank=flash0["dfu_image"]["bank"])

        builder.build()
        print "Boot Image - offset 0x%x, size 0x%x" % (
            builder.boot_image["offset"], builder.boot_image["size"])

        print "Image output sections:"
        for section in builder.image:
            print "Flash offset 0x%08x size 0x%x" % (section[0], len(section[1]))
        print "Image Header:"
        print builder.image_header

        print "Writing image section xuv files to %s" % (self.dfu_dir)
        builder.write_image_section_xuv_files(os.path.join(self.dfu_dir, "dfu"))
        print "Build completed\n"
        sys.stdout.flush()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description='''DFU File Generator Tool''',
        epilog="""Python config file default path: depot\\app_ss\main\\fw\config\\flash_layout_config.py""")
    parser.add_argument('config_file' , help='Path to python config file: flash_layout_config.py')
    args = parser.parse_args()

    file_generator = dfu_file_generator(args.config_file)
    ret = file_generator.initiate()
    tag = "DFU FILE GENERATION COMPLETED"
    if ret:
        print "\nFiles are created at %s" % file_generator.dfu_dir
    else:
        print "\nWARNING: Failed to generate all the files at %s" % file_generator.dfu_dir

    print "*" * len(tag) + "****"
    print "**" + tag + "**"
    print "*" * len(tag) + "****"
    sys.stdout.flush()
