##############################################################################
#                                                                            #
#   Copyright (c) 2009 - 2018 Qualcomm Technologies International, Ltd.      #
#   All Rights Reserved.                                                     #
#                                                                            #
#   Qualcomm Technologies International, Ltd. Confidential and Proprietary.  #
#                                                                            #
##############################################################################
#
# Usage:
#           python adkid.py {-d|--devkit_root} <path>
#                           [-r|--results]     <path>[\<filename>]
#                           [{-x|--xuv_file}   <filename>]
#
#   -d :    a path to a directory structure to scan for files of interest, or,
#           a workspace file (x2w); if latter that workspace is analysed
#   -r :    (optional) a path/full filename of CSV file to store the results in
#               - if not specified and -d gave a directory to analyse
#                 it will put the timestamped log file into that directory
#               - if not specified and -d gave a workspace file (.x2w)
#                 it will put the timestamped log file in the root offset
#                 that workspace (_not_ where the workspace file is)
#   -x :    a full path and filename of flash_image.xuv file to analyse
#           not implemented yet
#
#   Scanning a directory structure:
#
#       This operation looks for the following files/file patterns:
#
#           ("curator",      r'subsys0_patch0_fw.*\.hcf\Z'),  # Curator patches
#           ("janitor",      r'subsys6_patch0_fw.*\.hcf\Z'),  # Janitor patches
#           ("apps_p0",      r'.*app[s]*_p0_.*\.xuv\Z'),      # Apps P0 patches
#           ("private_lib",  r'.*\.pa\Z'),                    # Private libraries
#           ("bt",           r'.*subsys1_patch0_fw.*\.hcf\Z'),# BT patches
#           ("edkcs",        r'.*\.edkcs\Z'),                 # Kymera capabilities EDKCS
#           ("dkcs",         r'.*\.dkcs\Z'),                  # Kymera capabilities DKCS
#           ("audio",        r'.*subsys7_.*\.hcf\Z'),         # Audio patches
#           ("apps_p1",      r'.*\.xuv\Z')                    # Apps P1 file(s)
#
#       Having found one of the above file types, the script attempts to
#       extract version information. Teh algorithms empployed are:
#
#           Curator/Janitor patches :-
#               File is read as a binary file.
#               First 4 bytes are base version as little endian uint32.
#               Next 2 bytes are padding.
#               Next two bytes indicate subsystem and file type.
#               Next 4 bytes are patch ID as little endian uint32.
#
#           Apps P0 patches :-
#               XUV file is scanned for a magic number marking SLT structure.
#               SLT structure is analysed to extract both binary patch ID
#               and its corresponding string information.
#
#           Private libraries :-
#               Private library binaries are scanned for a string which
#               encodes the version ("lib_version_<version>\0")
#
#           BT patches :-
#               File is read as binary file.
#               First 4 bytes are base version.
#               Next 2 bytes identify file as a BT patch file.
#               What follows is a list of PS KEYS.
#               PS KEY with ID 0x2588 holds the BT patch version.
#               PS KEYS are in the following format:
#                   2 bytes PS KEY ID
#                   2 bytes PS KEY length
#                   1 byte magic 0xA1
#                   <length - 1> bytes value - patch ID is uint16.
#               Read PS KEYS. Skip uninteresting ones.
#               Return patch ID, if found.
#
#           EDKCS files :-
#               Version is uint32 at location 0x4C of the binary file.
#
#           DKCS files :-
#               Version is uint32 at location 0x24 of the binary file.
#
#           Audio patches :-
#               Version is uint32 at location 0x8 of the binary file.
#
#           Apps P1 :-
#               Same as Apps P0 but the offset of XUV addresses is 0x78000000
#               rather than 0x70000000. For this file type we also return a
#               list of libraries it includes with their versions.
#
#               At the moment, any XUV file not conforming to Apps P0
#               naming convention is treated as an Apps P1 file. This may
#               lead to files returning no, or nonsensical, information.
#
##############################################################################



import argparse
import datetime as dt
import os
import random as rnd
import re
import subprocess
import sys
import xml.etree.ElementTree as ET

class Versionable:
    valid_filename_regexes = {
        'std': [
            ("curator",      r'subsys0_patch0_fw.*\.hcf\Z'),  # Curator patches
            ("janitor",      r'subsys6_patch0_fw.*\.hcf\Z'),  # Janitor patches
            ("apps_p0",      r'.*app[s]*_p0_.*\.xuv\Z'),      # Apps P0 patches
            ("private_lib",  r'.*\.pa\Z'),                    # Private libraries
            ("bt",           r'.*subsys1_patch0_fw.*\.hcf\Z'),# BT patches
            ("edkcs",        r'.*\.edkcs\Z'),                 # Kymera capabilities EDKCS
            ("dkcs",         r'.*\.dkcs\Z'),                  # Kymera capabilities DKCS
            ("audio",        r'.*subsys7_.*\.hcf\Z'),         # Audio patches
            ("apps_p1",      r'.*\.xuv\Z'),                   # Apps P1 file(s)
            ("project",      r'.*\.x2p\Z')                    # Projects
        ],
        'xuv': [
            ("curator",      r'subsys0_patch0_fw.*\.hcf\Z'),  # Curator patches
            ("janitor",      r'subsys6_patch0_fw.*\.hcf\Z'),  # Janitor patches
            ("app_p0",       r'.*app[s]*_p0_.*\.xuv\Z'),      # Apps P0 patches
            ("private_lib",  r'.*\.pa\Z'),                    # Private libraries
            ("bt",           r'.*subsys1_patch0_fw.*\.hcf\Z'),# BT patches
            ("edkcs",        r'.*\.edkcs\Z'),                 # Kymera capabilities EDKCS
            ("dkcs",         r'.*\.dkcs\Z'),                  # Kymera capabilities DKCS
            ("audio",        r'.*subsys7_.*\.hcf\Z')          # Audio patches
        ]
    }

    def __init__(self, path='', ftypes='std'):
        assert(self.isValid(path=path, file_types=ftypes)), 'Invalid file {}'.format(path)
        self.file_types = ftypes
        self.path = path
        self.ext = os.path.splitext(path)[1]
        self.foldername = os.path.dirname(path)
        self.basename = os.path.basename(path)

    def read_uint16_l(self, patch):
        """
        Read a uint16 little endian from an open file ("rb")
        """
        b = ord(patch.read(1))
        b = b + (ord(patch.read(1)) << 8)
        return b

    def read_uint32_l(self, patch):
        """
        Read a uint32 little endian from an open file ("rb")
        """
        b = ord(patch.read(1))
        b = b + (ord(patch.read(1)) << 8)
        b = b + (ord(patch.read(1)) << 16)
        b = b + (ord(patch.read(1)) << 24)
        return b

    # Curator and Janitor patches
    def parse_curator_janitor_patch(self):
        """
        Read version (and other) information from an HCF file.
        Guaranteed to work for Janitor and Curator patches.
        """
        try:
            ss_lookup = ["curator","bluetooth","wlan","audio","gnss","nfc","janitor","apps"]
            file_type_lookup = ["patch", "config", "data"]

            with open(self.path,"rb") as patch:
                #First 4 bytes are the base fw version number
                f_fw_version = self.read_uint32_l(patch)

                #Next we have the file type stuff

                #Ignore first two bytes - not sure what these should be.
                patch.read(2)

                # Next byte contains the layer in the lower  nibble
                # and the subsytem ID in the top nibble.
                byte = ord(patch.read(1))

                f_layer = byte & 0xf
                if (byte >> 4) < 8:
                    f_subsys = ss_lookup[byte >> 4] # we're currently not reporting this
                else:
                    return 'unknown_subsystem'

                # This byte contains the type of this file.
                byte = ord(patch.read(1))

                if byte < 3:
                    f_type = file_type_lookup[byte] # we're currently not reporting this
                else:
                    return 'unknown_patch_type'

                #Next we have the patched fw version
                f_patched_fw_version = self.read_uint32_l(patch)

            return 'patch_id_{}_rom_fw_id_{}'.format(hex(f_patched_fw_version), hex(f_fw_version))
        except:
            return 'invalid_file_(too_short?)'


    def get_xuv_slt_addr(self, line_1, line_2, xuv_offset):
        """
        Convert XUV lines into an address further in the same XUV file
        """
        # combine into 32-bit hex
        addr = re.match(r'^\s*@.{6}\s+(.*)', line_2).group(1) + re.match(r'^\s*@.{6}\s+(.*)', line_1).group(1)
        # need to subtract 0x70000000 and divide by 2
        addr_n = (int('0x' + addr, 16) - xuv_offset) >> 1
        # now build the 6-digit @-notation
        return '@' + re.match(r'0X(.*)', hex(addr_n).upper()).group(1).zfill(6)

    def get_xuv_slt_int_ver(self, line_1, line_2):
        """
        Convert SLT 32-bit value into integer version of patch ID
        """
        addr = re.match(r'^\s*@.{6}\s+(.*)', line_2).group(1) + re.match(r'^\s*@.{6}\s+(.*)', line_1).group(1)
        return int('0x' + addr, 16)

    def read_slt_entry(self, p_file, idx):
        """
        Build the string which is ID of an SLT entry
        """
        type = re.match(r'^\s*@.{6}\s+(.*)', p_file[idx]).group(1) + re.match(r'^\s*@.{6}\s+(.*)', p_file[idx+1]).group(1)
        return type

    def get_xuv_two_bytes(self, line):
        """
        Return a 16 bit integer encoded in a line of XUV file.
        """
        value = int('0x' + re.match(r'^\s*@.{6}\s+(.*)', line).group(1), 16)
        return value

    def read_string_from_xuv(self, p_file, idx):
        """
        Given XUV file address read a null terminated string
        """
        ver_str = ''

        while (idx + 1) < len(p_file):
            char1 = int('0x' + re.match(r'^\s*@.{6}\s+(.{2})(.{2})', p_file[idx]).group(2), 16)
            if char1 != 0:
                ver_str = ver_str + chr(char1)
            else:
                return ver_str

            char2 = int('0x' + re.match(r'^\s*@.{6}\s+(.{2})(.{2})', p_file[idx]).group(1), 16)
            if char2 != 0:
                ver_str = ver_str + chr(char2)
            else:
                return ver_str

            idx = idx + 1

        if ver_str != '':
            return ver_str
        else:
            return 'unknown'


    def parse_apps_pX_patch(self, patch_type):
        """
        Get version information from an Apps P0 patch file or Apps P1 XUV file.
        """
        version_integer = 0
        version_string  = 'unknown'

        # offset to be subtracted from XUV addresses
        if patch_type == 'apps_p0':
            xuv_offset = 0x70000000
            ver_info = 'build_'
        elif patch_type == 'apps_p1':
            xuv_offset = 0x78000000
            ver_info = ''
        else:
            return 'unknown_file_type'

        with open(self.path,"r") as patch:
            # read in the whole XUV file as lines
            p_file = patch.read().splitlines()

            # try to read a magic SLT number and SLT address
            for line in p_file:
                match = re.search(r'^\s*@000040[ ]*([0-9A-F]{4})\Z', line)
                if match:
                    slt_magic_1 = match.group(1)
                match = re.search(r'^\s*@000041[ ]*([0-9A-F]{4})\Z', line)
                if match:
                    slt_magic_2 = match.group(1)
                match = re.search(r'^\s*@000042[ ]*([0-9A-F]{4})\Z', line)
                if match:
                    slt_addr_1 = match.group(1)
                match = re.search(r'^\s*@000043[ ]*([0-9A-F]{4})\Z', line)
                if match:
                    slt_addr_2 = match.group(1)

            # build the SLT magic number...
            try:
                slt_magic = slt_magic_2 + slt_magic_1
                # ...and check if it's correct
                if slt_magic != "41707073":
                    return 'slt_magic_wrong_' + slt_magic
            except:
                return 'slt_magic_not_found'

            # raw SLT address should be...
            try:
                slt_addr = '0x' + slt_addr_2 + slt_addr_1
                slt_addr_n = (int(slt_addr, 16) - xuv_offset) >> 1
            except:
                return 'slt_address_malformed'

            # ...but should be >= 0 at least
            if slt_addr_n < 0:
                return 'slt_address_corrupted'

            # all seems fine, build the address line start we're after
            try:
                slt_addr_line = '@' + re.match(r'0X(.*)', hex(slt_addr_n).upper()).group(1).zfill(6)
            except:
                return 'unspecified_error'

            # try to locate any libraries in an Apps P1 XUV file
            if patch_type == "apps_p1":
                # first library info should be at this location
                i = -1  # just in case we've a weirdly short file
                for line in p_file:
                    match = re.search(r'^\s*@000044', line)
                    if match:
                      i = p_file.index(line)

                count = 0               # count old style "entries"
                libraries_string = ''
                # until end of file or too many old style "entries"
                while (i >= 0) and ((i + 3) < len(p_file)) and (count < 64):
                    # hopefully the first two numbers are integer version
                    try:
                        lib_version_int = self.get_xuv_slt_int_ver(p_file[i], p_file[i + 1])
                    except:
                        # something is wrong; this probably means:
                        self.libs.append('no_library_info_on_line_{}_({})'.format(i, p_file[i]))
                        break
                    # unless we reached the magic end marker (new style XUVs)
                    if lib_version_int == 0x17BEC0DE:
                        break
                    # so far so good - try to get pointer to string version
                    try:
                        addr = self.get_xuv_slt_addr(p_file[i+2], p_file[i+3], xuv_offset)
                    except:
                        # something is wrong; this probably means:
                        self.libs.append('no_library_string_info_on_line_{}_({})'.format(i, p_file[i]))
                        break
                    # now see if the address we got is actually in the file
                    for line in p_file:
                        match = re.search(addr, line)
                        if match:
                            k = p_file.index(line)
                            try:
                                lib_version_str = self.read_string_from_xuv(p_file, k)
                                self.libs.append('lib_{}_{}'.format(lib_version_str, lib_version_int))
                            except:
                                self.libs.append('lib_version_string_malformed_from_line_{}_({})'.format(k, p_file[k]))
                    # move to next entry
                    i = i + 4
                    # just in case increase lib count so we stop at some point
                    count = count + 1

            # now we look for the actual Apps P0/P1 version information

            # go through the whole SLT and analyse all elements found
            for line in p_file:
                match = re.search(slt_addr_line, line)
                if match:
                    i = p_file.index(line)
                    while (i + 3) < len(p_file):
                        try:
                            slt_entry = self.read_slt_entry(p_file, i)
                        except:
                            return ver_info + 'slt_corrupted_on_line_{}_({})'.format(i, p_file[i])

                        if slt_entry == '00000000':         # if we got to this entry it's the end of SLT
                            break

                        elif slt_entry == '00010000':       # this is the integer version of patch ID
                            try:
                                addr = self.get_xuv_slt_addr(p_file[i+2], p_file[i+3], xuv_offset)
                            except:
                                return ver_info + 'bad_id_on_line_{}_({})'.format(i+2, p_file[i+2])
                            # find which lines have it
                            for ver_line in p_file:
                                match = re.search(addr, ver_line)
                                if match:
                                    k = p_file.index(ver_line)
                                    try:
                                        version_integer = self.get_xuv_slt_int_ver(p_file[k], p_file[k+1])
                                    except:
                                        return ver_info + 'bad_id_on_line_{}_({})'.format(k, p_file[k])

                        elif slt_entry == '00030000':       # this is where the string version of patch ID should be
                            try:
                                addr = self.get_xuv_slt_addr(p_file[i+2], p_file[i+3], xuv_offset)
                            except:
                                version_string = 'bad_version_string_address_on_line_{}_({})'.format(i+2, p_file[i+2])
                            else:
                                for line in p_file:
                                    match = re.search(addr, line)
                                    if match:   # found the starting line so read the null terminated version string
                                        k = p_file.index(line)
                                        try:
                                            version_string = self.read_string_from_xuv(p_file, k)
                                        except:
                                            version_string = 'bad_version_string_on_line_{}_({})'.format(k, p_file[k])

                        i = i + 4   # found the starting line so read the null terminated version string

        return ver_info + 'id_{}_({})'.format(version_integer, version_string)


    def parse_edkcs(self):
        """
        Get ELF ID for EDKCS files
        """
        try:
            with open(self.path,"rb") as patch:
                patch.seek(0x004C)  # magic location
                elf_id = self.read_uint32_l(patch)
                return 'elf_id_{}'.format(hex(elf_id))
        except:
            return 'invalid_file_(too_short?)'


    def parse_dkcs(self):
        """
        Get ELF ID for DKCS files
        """
        try:
            with open(self.path,"rb") as patch:
                patch.seek(0x0024)  # magic location
                elf_id = self.read_uint32_l(patch)
                return 'elf_id_{}'.format(hex(elf_id))
        except:
            return 'invalid_file_(too_short?)'


    def parse_audio_patch(self):
        """
        Get patch ID for Audio patch files
        """
        try:
            with open(self.path,"rb") as patch:
                patch.seek(0x0008)
                audio_patch_id = self.read_uint32_l(patch)
                return "patch_id_{}".format(hex(audio_patch_id))
        except:
            return 'invalid_file_(too_short?)'


    def parse_pa_file(self):
        """
        Get version information from a private library file.
        """
        with open(self.path, 'rb') as patch:
            strings=set(patch.read().split(b'\0'))

        for s in strings:
            if b'lib_version' in s:
                match = re.match(r'\$_.*_(.*)\Z', s.decode('ascii', 'ignore'))
                if match:
                    return 'lib_version_' + match.group(1)

        return 'lib_version_not_present'


    def parse_bt_patch(self):
        """
        Read version information from a BT patch file.
        """
        with open(self.path,"rb") as patch:
            try:
                # first 4 bytes are ROM FW version (unpatched)
                bt_patch_rom_fw_id = self.read_uint32_l(patch)

                # then follow 0x0000 and encoded 1 for subsystem1 - easier tested as "magic"
                magic = self.read_uint32_l(patch)
                if magic != 0x01100000:
                    return 'not_bt_patch_file'
            except:
                return 'corrupted_file_(too_short?)'

            # Loop through all PS KEYS until we hit patch version.
            # One of the try blocks will catch EOF unless we have
            # found a valid version PS key after which we return.
            while True:
                try:
                    # two bytes are key ID
                    key_id = self.read_uint16_l(patch)
                except:
                    return 'no_version_ps_key'

                # two bytes are key length (inclusive)
                # next byte is data header magic 0xA1
                # next byte is ps data length
                try:
                    key_length = (self.read_uint16_l(patch) - 2)
                    vldata_header = ord(patch.read(1))
                    if vldata_header != 0xA1:   # data header must be 0xA1
                        return 'corrupted_ps_key_header'
                    vldata_len = ord(patch.read(1))
                    # next 2 bytes are version ID if PS key is 0x2588
                    version_id = self.read_uint16_l(patch)
                    if key_id == 0x2588:    # we found version ID so report and return
                        return 'patch_id_{}_rom_fw_id_{}'.format(hex(version_id), hex(bt_patch_rom_fw_id))
                    # skip the rest of PS key data
                    dummy = patch.read(vldata_len - 2)
                except:
                    return 'corrupted_file_(too_short?)'

        return 'unknown_file_type'


    @classmethod
    def isValid(cls, path=None, file_types='std'):
        basename = os.path.basename(path)
        for patt_tup in cls.valid_filename_regexes[file_types]:
            patt = patt_tup[1]
            if re.match(patt, basename):
                return True
        return False

    def get_tag_text(self, parent, subtag):
        val = ''
        el = parent.find(subtag)
        if el is not None:
            val = el.text

        return val

    def parse_project_file(self):
        """
        Read Devkt references from a project file
        """
        tree = ET.parse(self.path)
        root = tree.getroot()

        rx = r'kitres:\/\/{(.+)}'
        self.dk_refs = []
        self.devkits = set()
        self.default_devkits = set()
        self.devkitGroups = set()

        defkit = root.find('./configurations/[@defaultKit]')
        if defkit is not None:
            defkit = defkit.attrib['defaultKit']
            defkit = re.search(rx, defkit).group(1).upper()
            # collect all referenced default devkits
            self.default_devkits.add(defkit)
        descrip = 'Default Devkit UUID: {}'.format(defkit)
        self.dk_refs.append(descrip)

        for config in root.findall('./configurations/configuration'):
            config_name = config.attrib['name']
            if 'kit' in config.attrib:
                devkit = config.attrib['kit']
                devkit = re.search(rx, devkit).group(1).upper()
                # collect all devkits referenced in configurations
                self.devkits.add(devkit)
            else:
                devkit = ''

            devkitConfig = self.get_tag_text(config, 'devkitConfiguration')

            devkitPath = self.get_tag_text(config, 'devkitPath')
            if devkitPath:
                devkitPath = os.path.abspath(os.path.join(os.path.dirname(self.path), devkitPath))

            devkitGroup = self.get_tag_text(config,'devkitGroup')
            if devkitGroup:
                devkitGroup = re.search(rx, devkitGroup).group(1).upper()
                self.devkitGroups.add(devkitGroup)

            config_info = Configuration(name=config_name,
                                         devkitConfig=devkitConfig,
                                         devkitPath=devkitPath,
                                         devkit=devkit,
                                         devkitGroup=devkitGroup
                                         )
            self.dk_refs.append(config_info)
        return ''

    def process_file(self):
        self.type = 'unknown'
        self.version = 'unknown'
        self.libs = []
        self.dk_refs = []

        for patt_tup in self.valid_filename_regexes[self.file_types]:
            self.type = patt_tup[0]
            patt = patt_tup[1]
            if re.match(patt, self.basename):
                if  self.type == "curator":
                    self.version = self.parse_curator_janitor_patch()
                    return
                elif self.type == "janitor":
                    self.version = self.parse_curator_janitor_patch()
                    return
                elif self.type == "apps_p0":
                    self.version = self.parse_apps_pX_patch(self.type)
                    return
                elif self.type == "private_lib":
                    self.version = self.parse_pa_file()
                    return
                elif self.type == "bt":
                    self.version = self.parse_bt_patch()
                    return
                elif self.type == "edkcs":
                    self.version = self.parse_edkcs()
                    return
                elif self.type == "dkcs":
                    self.version = self.parse_dkcs()
                    return
                elif self.type == "audio":
                    self.version = self.parse_audio_patch()
                    return
                elif self.type == "apps_p1":
                    self.version = self.parse_apps_pX_patch(self.type)
                    return
                elif self.type == "project":
                    self.version = self.parse_project_file()
                    return
                else:
                    self.type = "unknown"
                    self.version = "unknown"
                    return

class Configuration:
    def __init__(self, name=None, devkitConfig=None, devkitPath=None, devkit=None, devkitGroup=None):
        self.name = name
        self.devkitConfig = devkitConfig
        self.devkitPath = devkitPath
        self.devkit = devkit
        self.devkitGroup = devkitGroup

    def __str__(self):
        s = 'name:     {}\n'.format(self.name)
        s += 'dkconf:  {}\n'.format(self.devkitConfig)
        s += 'dkpath:  {}\n'.format(self.devkitPath)
        s += 'dk:      {}\n'.format(self.devkit)
        s += 'dkgroup: {}\n'.format(self.devkitGroup)
        return s

class InfoDB:
    def __init__(self, dk_root, logpath, version=None, file_type='std'):
        self.dk_root = dk_root
        # If the log path is a dir, create a time-stamped filename
        if os.path.splitext(logpath)[-1] is '':
            self.logdir = logpath
            self.logbasename = self.get_logfilename()
            self.custom_basename = self.get_custom_name(file_type)
            self.logfile = os.path.join(self.logdir, self.custom_basename)
        else:
            self.logdir = os.path.dirname(logpath)
            self.logbasename = os.path.basename(logpath)
            if file_type == 'std':
                self.logfile = logpath
            else:
                self.custom_basename = self.get_custom_name(file_type)
                self.logfile = os.path.join(self.logdir, self.custom_basename)

        if not os.path.exists(self.logdir):
            os.makedirs(self.logdir)
        #Ensure we overwrite any older log file of the same name
        with open(self.logfile, 'w') as lf:
            print('DK ROOT: {}'.format(self.dk_root))
            print('LOGFILE: {}'.format(self.logfile))
            lf.write('TYPE, VERSION, LIB, PATH\n')
            if version == None:
                lf.write('{}, {}, {}, {}\n'.format('RootFolder', '', '', self.dk_root))
            else:
                lf.write('{}, {}, {}, {}\n'.format('Devkit', version, '', ''))

    def get_custom_name(self, file_type):
        name = os.path.splitext(self.logbasename)[0]
        ext = os.path.splitext(self.logbasename)[1]
        custom_name = '{}_{}{}'.format(name, file_type, ext)
        return custom_name

    @classmethod
    def locate_sdk_xml(cls):
        start_dir = os.path.dirname(os.path.realpath(__file__))
        cdir = start_dir
        while 'sdk.xml' not in os.listdir(cdir):
            cdir = os.path.abspath(os.curdir)
            if os.path.split(cdir)[1] == '':
                break
            os.chdir(os.path.pardir)
        os.chdir(start_dir)
        return cdir

    def get_logfilename(self):
        now = dt.datetime.now()
        return now.strftime('%Y%m%d%H%M%S_infodb.csv')

    def write_log(self, v_info=None, blank=False):
        with open(self.logfile, 'a') as lf:
            if blank:
                lf.write('{},{},{},{}\n'.format('', '', '', ''))
                return

            v_info.version = v_info.version.replace(',','|')

            rel_path = os.path.relpath(v_info.path, self.dk_root)
            if v_info.type == 'project':
                # This is a project file with devkit references
                lf.write('{}, {},{},{}\n'.format(v_info.type, v_info.version, '', rel_path))

                if v_info.dk_refs:
                    # Get default Devkit
                    lf.write('{},{},{},{}\n'.format('', v_info.dk_refs[0], '', ''))

                    # Log sorted configurations
                    configs = v_info.dk_refs[1:]
                    for dk_ref in sorted(configs, key=lambda dk_ref: dk_ref.name):
                        if dk_ref.__class__.__name__ == 'Configuration':
                            lf.write(',Configuration: {},,\n'.format(dk_ref.name))
                            lf.write(',    Devkit Configuration: {},,\n'.format(dk_ref.devkitConfig))
                            lf.write(',    Devkit Path: {},,\n'.format(dk_ref.devkitPath))
                            lf.write(',    Devkit UUID: {},,\n'.format(dk_ref.devkit))
                            lf.write(',    Devkit Group UUID: {},,\n'.format(dk_ref.devkitGroup))

                    # Summarize Devkit References
                    lf.write('{},{},{},{}\n'.format('', 'SUMMARY', '', ''))
                    lf.write('{},    {},{},{}\n'.format('', 'Devkits in this project', '', ''))
                    for dk in v_info.devkits:
                        lf.write('{},        {},{},{}\n'.format('', dk, '', ''))

                    lf.write('{},    {},{},{}\n'.format('', 'Default Devkits in this project', '', ''))
                    for dk in v_info.default_devkits:
                        lf.write('{},        {},{},{}\n'.format('', dk, '', ''))

                    lf.write('{},    {},{},{}\n'.format('', 'Devkit Groups in this project', '', ''))
                    for dk in v_info.devkitGroups:
                        lf.write('{},        {},{},{}\n'.format('', dk, '', ''))

            else:
                lf.write('{},{},{},{}\n'.format(v_info.type, v_info.version, '', rel_path))
                for lib in v_info.libs:
                    lf.write('{},    {},{},{}\n'.format('', v_info.version, lib, ''))



def parse_args(args):
    """
    Parse the args ready for passing into build_devkit.main()
    """
    parser = argparse.ArgumentParser()

    parser.add_argument('-r', '--results',
                        default=None,
                        help='Specifies the path to a csv file, or a folder where a automatically-named \
                              results file will be written. Any intermediate folders will be created. \
                              Optional. If not specified the folder analysed is used.')

    parser.add_argument('-d', '--devkit_root',
                        required=True,
                        help='Specifies the path to the root folder of devkit or workspace to be analysed. \
                              Workspace is specified either as the root folder or by a workspace file (x2w). \
                              If the latter, the workspace file is analysed to determine path to analyse.')

    parser.add_argument('-x', '--xuv',
                        help='Path to an XUV file to be analysed. \
                              Not implemented yet.')

    parsed_args = parser.parse_args(args)

    return parsed_args

def process_xuv(xuv_path, logpath):
    assert(os.path.splitext()[1] == 'xuv')
    # Extract XUV file into a folder
    #...
    assert(False)
    process_folder(xuv_root, logpath, file_types='xuv')

def get_x2w_root(root):
    '''
    Scan an x2w file for the longest common path for all the x2p files therein.
    '''
    x2w_dir = os.path.dirname(root) # workspace file path
    x2w_paths = [x2w_dir]           # prime the pump

    with open(root,"r") as x2w_file:
        x2w = x2w_file.read().splitlines()  # read in the workspace file
        for line in x2w:                    # look for project files
            match = re.search(r'.*<project .*path="(.*)\.x2p".*>', line)
            if match:                       # found one, add its path to list
                x2w_paths.append(os.path.normpath(x2w_dir + '/' + os.path.dirname(match.group(1))))

    return os.path.commonprefix(x2w_paths)  # return longest common path

def process_folder(root, logpath, version=None, file_types='std'):
    idb = InfoDB(root, logpath, version=version, file_type=file_types)
    proj_set = set()

    for root, _, files in os.walk(root):
        for fname in files:
            file_path = os.path.join(root, fname)
            try:
                if Versionable.isValid(path=file_path, file_types=file_types):
                    v = Versionable(path=file_path, ftypes=file_types)
                    #Progress indicator
                    try:
                        v.process_file()
                    except:
                        v.version = 'ERROR: {}'.format(sys.exc_info()[1])
                    if v.type == 'project':
                        proj_set.add(v)
                    else:
                        idb.write_log(v_info=v)
            except:
                print('\n\n' + '#'*80)
                print("Error: {}\nFile: {}".format(sys.exc_info()[1], file_path))
                print('#'*80)
                raise

    for v in sorted(proj_set, key=lambda v: v.path):
        # Add a blank entry before project file listing
        idb.write_log(blank=True)
        idb.write_log(v_info=v)

def log_version_info(dk_root=None, logpath=None, xuv_path=None, version=None):
    if xuv_path:
        process_xuv(xuv_path, logpath)
    else:
        if os.path.splitext(dk_root)[1] == '.x2w':
            dk_root = get_x2w_root(dk_root)
            logpath = dk_root
        else:
            if os.path.isfile(dk_root):
                sys.exit("Error: {}\nFile: {}".format('only directories and X2W files are permitted', dk_root))
            else:
                if logpath == None:
                    logpath = dk_root

        process_folder(dk_root, logpath, version=version)

if __name__ == '__main__':
    sys.stdout.write("\nRunning...\n")
    sys.stdout.flush()

    pargs = parse_args(sys.argv[1:])
    log_version_info(dk_root=pargs.devkit_root, logpath=pargs.results, xuv_path=pargs.xuv)

    sys.stdout.write('\nDone')
    sys.stdout.flush()
