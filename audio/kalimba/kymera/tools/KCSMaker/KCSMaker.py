############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2014 - 2018 Qualcomm Technologies International, Ltd.
#
############################################################################
import imp
import sys
import os
import build_controller
import download_build_info
import capability
import get_python_tools
import make_kdc
import make_kcs
from kcs import KCSFile
import shutil
import glob
import logging
import chip_info
import re
import bundles_for_config


def get_immediate_subdirectories(a_dir):
    return [name for name in os.listdir(a_dir) if os.path.isdir(os.path.join(a_dir, name))]

def find_build_name(path):
    build_name = ""

    # Return first the _external
    list_elfs = [x for x in os.listdir(path) if x.endswith(".elf")]
    # remove extension
    list_elfs = [os.path.splitext(x)[0] for x in list_elfs]
    if len(list_elfs) > 2:
        raise Exception(
            "Unexpected amount of ELF ({num_elfs}) files found in {mypath}".format(
                num_elfs = len(list_elfs),
                mypath = path
            )
        )
    elif len(list_elfs) == 0:
        raise Exception("No ELF files found in {mypath}".format(mypath = path))

    # If there are two ELF files we expect one of them to be the external version of the other
    if len(list_elfs) == 2:
        try:
            external_build_name = [x for x in list_elfs if x.endswith("_external")][0]
        except:
            raise Exception("Couldn't find external ELF")
        try:
            internal_build_name = [x for x in list_elfs if not x.endswith("_external")][0]
        except:
            raise Exception("Couldn't find internal ELF")
        if internal_build_name != external_build_name.replace("_external",""):
            raise Exception(
                "{first} and {second} names do not match!".format(
                first = internal_build_name,
                second = external_build_name
            )
        )
        else:
            return external_build_name

    return list_elfs[0]


def proc_cmd_line(args):
    """
    Processes the command line arguments passed to KCSMaker.
    """

    if 2 > len(args):
        raise Exception("No enough input arguments. Requires <bundle name>")

    # Set initial and default values
    RELEASE_FLAG = False
    DOWNLOAD_CONFIG = ""
    BUNDLE_NAME = ""
    KCS_OUTPUT_DIR = ""
    BUILD_ID = 0xFFFF
    LABEL_NAME = ""
    BUILD_DIR = ""
    PATCH_DIR = ""
    BUILD_ROOT = ""
    BUILD_NAME = ""
    EXTRA_INCLUDE= ""
    KYMERA_SRC_PATH = ""
    KYMERA_SRC_ID = ""
    LINKSCRIPT_PATH = ""
    MAKE_PATH = "make"
    clean_build = None
    MAKE_JOBS = None
    user = None

    # Flag whether BUILD_DIR has been assigned. It can be assigned by -buildpath
    # and by -buildid. The use of both causes one to override the other and is
    # not permitted.
    build_id_flag = False
    build_config_flag = False
    build_path_flag = False
    
    patch_id_flag = False
    patch_path_flag = False
    
    for arg in args:
        if "RELEASE" in arg:
            val = arg.split("=")[1]
            if ("False" == val) | ("false" == val):
                RELEASE_FLAG = False
            elif ("True" == val) | ("true" == val):
                RELEASE_FLAG = True
            else:
                raise Exception("Unrecognised command line value for RELEASE: " + str(val))
        elif "-ostype" in arg:
            OS_TYPE = arg.split("=")[1]
        elif "-makepath" in arg:
            MAKE_PATH = arg.split("=")[1]
        elif "-noclean" == arg:
            clean_build = False
        elif "-clean" == arg:
            clean_build = True
        elif "-nojobs" == arg:
            MAKE_JOBS = False
        elif "-jobs" == arg:
            MAKE_JOBS = True
        elif "-extrainclude" in arg:
            EXTRA_INCLUDE = arg.split("=")[1]
        elif "-bundlename" in arg:
            BUNDLE_NAME = arg.split("=")[1]
        elif "-linkscriptpath" in arg:
            LINKSCRIPT_PATH = arg.split("=")[1]
        elif "-buildpath" in arg:
            if build_id_flag:
                raise Exception("-buildpath and -buildid arguments cannot be used together")
            build_path_flag = True
            BUILD_DIR = os.path.abspath(arg.split("=")[1])
        elif "-buildname" in arg:
            BUILD_NAME = arg.split("=")[1]
        elif "-buildroot" in arg:
            BUILD_ROOT = os.path.abspath(arg.split("=")[1])
        elif "-patchpath" in arg:
            if patch_id_flag:
                raise Exception("-patchpath and -patchid arguments cannot be used together")
            PATCH_DIR = arg.split("=")[1]
            patch_path_flag = True
        elif "-patchid" in arg:
            if patch_path_flag:
                raise Exception("-patchpath and -patchid arguments cannot be used together")
            PATCH_ID = arg.split("=")[1]
            import build_finder
            bf = build_finder.build_finder()
            PATCH_DIR = os.path.join(bf.get_build_dir(PATCH_ID)[0], "patch")
            patch_id_flag = True
        elif "-config" in arg:
            if build_id_flag:
                raise Exception("-config and -buildid arguments cannot be used together")
            build_config_flag = True
            DOWNLOAD_CONFIG = arg.split("=")[1]
        elif "-buildid" in arg:
            if build_path_flag == True or build_config_flag == True:
                raise Exception("-buildpath or -config arguments cannot be used together with -buildid")
            build_dir_or_config_flag = True
            BUILD_ID = arg.split("=")[1]
            import build_finder
            bf = build_finder.build_finder()
            BUILD_DIR = bf.get_build_dir(BUILD_ID)
            BUILD_ROOT = os.path.abspath(os.path.join(BUILD_DIR, "..", "..", "..", "build"))
            BUILD_ID = int(BUILD_ID, 10)
            # Make sure the requested download config matches the target config
            # We get the target config from the BUILD_DIR path
            DOWNLOAD_CONFIG = os.path.basename(os.path.abspath(os.path.join(BUILD_DIR, "..")))
        elif "-outputdir" in arg:
            KCS_OUTPUT_DIR = os.path.abspath(arg.split("=")[1])
        elif "-p4label" in arg:
            LABEL_NAME = arg.split("=")[1]
        elif "-kymerasourcepath" in arg:
            KYMERA_SRC_PATH = os.path.abspath(arg.split("=")[1])
        elif "-kymerasourceid" in arg:
            KYMERA_SRC_ID = arg.split("=")[1]
            import build_finder
            bf = build_finder.build_finder()
            KYMERA_SRC_PATH = os.path.abspath(
                os.path.join(
                    bf.get_build_dir(KYMERA_SRC_ID),
                    "..", "..", ".."
                )
            )
            BUILD_ROOT = os.path.abspath(os.path.join(KYMERA_SRC_PATH, "build"))
        elif "-user" in arg:
            user = arg.split("=")[1]
        else:
            raise Exception("Unrecognised command line argument: " + str(arg))

    if DOWNLOAD_CONFIG == "":
        raise Exception("No -config or -buildid provided. Please provide at least one.")

    # Set a default bundle_name in case none is provided
    if BUNDLE_NAME == "":
        BUNDLE_NAME = DOWNLOAD_CONFIG + "_kcs"

    # If BUILD_DIR is not provided, get it from current relative directory
    if BUILD_DIR == "":
        KYMERA_SRC_PATH = os.path.abspath(os.path.join(os.path.dirname(sys.argv[0]), "..", ".."))
        BUILD_DIR = os.path.abspath(os.path.join(KYMERA_SRC_PATH, "output", DOWNLOAD_CONFIG, "build"))
        BUILD_ROOT = os.path.abspath(os.path.join(KYMERA_SRC_PATH, "build"))
    else:
        if KYMERA_SRC_PATH == "":
            # Get capabilities src path from the build we are linking against
            # if path hasn't been set previously
            KYMERA_SRC_PATH = os.path.abspath(os.path.join(BUILD_DIR, "..", "..", ".."))

    KCSMAKER_PATH = os.path.join(os.path.abspath(KYMERA_SRC_PATH), "tools", "KCSMaker")
    CAPABILITIES_PATH = os.path.abspath(os.path.join(KYMERA_SRC_PATH,"capabilities"))
    # if BUILD_NAME not provided by command line, resolve it by inspecting output .elf file
    if BUILD_NAME == "":
        BUILD_NAME = find_build_name(os.path.join(BUILD_DIR, "debugbin"))

    if KCS_OUTPUT_DIR == "":
        # For not release builds, use a special folder name,
        # otherwise use KCSMAKER_PATH/output/<BUILD_ID>/<DOWNLOAD_CONFIG>
        if BUILD_ID == 0xFFFF:
            KCS_OUTPUT_DIR = os.path.abspath(
                os.path.join(
                    KCSMAKER_PATH,
                    "out",
                    DOWNLOAD_CONFIG
                )
        )
        else:
            KCS_OUTPUT_DIR = os.path.abspath(
                os.path.join(
                    KCSMAKER_PATH,
                    "out",
                    str(BUILD_ID),
                    DOWNLOAD_CONFIG
                )
            )

    cmd_line_args = {
        "RELEASE_FLAG" : RELEASE_FLAG,
        "DOWNLOAD_CONFIG" : DOWNLOAD_CONFIG,
        "BUNDLE_NAME" : BUNDLE_NAME,
        "KCS_OUTPUT_DIR" : KCS_OUTPUT_DIR,
        "BUILD_ID" : BUILD_ID,
        "LABEL_NAME" : LABEL_NAME,
        "BUILD_DIR" : BUILD_DIR,
        "PATCH_DIR" : PATCH_DIR,
        "BUILD_ROOT" : BUILD_ROOT,
        "BUILD_NAME" : BUILD_NAME,
        "EXTRA_INCLUDE" : EXTRA_INCLUDE,
        "KYMERA_SRC_PATH" : KYMERA_SRC_PATH,
        "KYMERA_SRC_ID" : KYMERA_SRC_ID,
        "LINKSCRIPT_PATH" : LINKSCRIPT_PATH,
        "OS_TYPE" : OS_TYPE,
        "MAKE_PATH" : MAKE_PATH,
        "clean_build" : clean_build,
        "MAKE_JOBS" : MAKE_JOBS,
        "USER" : user
    }

    return cmd_line_args


class KCSMaker(object):
    def __init__(self, cmd_line_args):
        self.release_flag = cmd_line_args["RELEASE_FLAG"]
        self.download_config = cmd_line_args["DOWNLOAD_CONFIG"]
        self.bundle_name = cmd_line_args["BUNDLE_NAME"]
        self.kcs_output_dir = cmd_line_args["KCS_OUTPUT_DIR"]
        self.build_id = cmd_line_args["BUILD_ID"]
        self.label_name = cmd_line_args["LABEL_NAME"]
        self.build_dir = cmd_line_args["BUILD_DIR"]
        self.patch_dir = cmd_line_args["PATCH_DIR"]
        self.build_root = cmd_line_args["BUILD_ROOT"]
        self.build_name = cmd_line_args["BUILD_NAME"]
        self.extra_include = cmd_line_args["EXTRA_INCLUDE"]
        self.kymera_src_path = cmd_line_args["KYMERA_SRC_PATH"]
        self.kymera_src_id = cmd_line_args["KYMERA_SRC_ID"]
        self.linkscript_path = cmd_line_args["LINKSCRIPT_PATH"]
        self.os_type = cmd_line_args["OS_TYPE"]
        self.make_path = cmd_line_args["MAKE_PATH"]
        self.clean_build = cmd_line_args["clean_build"]
        self.make_jobs = cmd_line_args["MAKE_JOBS"]
        self.user = cmd_line_args["USER"]

        self.bdl_json_path = os.path.join(self.build_root, "bundle_list.json")
        self.kcsmaker_path = os.path.join(
            os.path.abspath(self.kymera_src_path),
            "tools",
            "KCSMaker"
        )
        self.capabilities_path = os.path.abspath(
            os.path.join(
                self.kymera_src_path,
                "capabilities"
            )
        )
        self.bdl_file = os.path.abspath(
            os.path.join(
                self.kcsmaker_path,
                "bundle",
                self.bundle_name + ".bdl"
            )
        )


    def verify_bundle_name(self):
        if os.path.isfile(self.bdl_json_path):
            bundle_defs = bundles_for_config.get_bundle_definitions(self.bdl_json_path)
            if self.bundle_name not in [os.path.splitext(x)[0] for x in bundle_defs]:
                raise Exception(
                    "Error. Bundle {b} couldn't be found in {p}. Please manually add \
                    an entry for this bundle".format(
                        b = self.bundle_name,
                        p = self.bdl_json_path
                    )
                )

                bundle_list = bundles_for_config.determine_bundles_for_config(
                    self.download_config, self.bundle_defs)
                if self.bundle_name not in bundle_list:
                    raise Exception(
                        "Error. The bundle registry {p} explicitly excludes {b} for the \
                        configuration {c}. If you don't think this is correct, please fix \
                        the registry".format(
                            p = self.bdl_json_path,
                            b = self.bundle_name,
                            c = self.download_config
                        )
                    )

    def get_base_build_id_from_kymera_src_path(self):
        base_build_timestamp = re.search(r'kymera_\d+', self.kcsmaker_path).group()

        import build_finder
        bf = build_finder.build_finder()

        for (build_id, build_label) in bf.get_ids().items():
            if base_build_timestamp + "_" + self.download_config + "_rel" == build_label[0]:
                return build_id

        return None


    def get_obj_files(self, chip, get_kcc_version):
        prebuiltlibReStart = re.compile("PREBUILT_LIBS = (.*\.a)")
        cap_output_path = os.path.join(self.kcs_output_dir, "capabilities")
        support_lib_output_path = os.path.join(self.kcs_output_dir, "support_lib")
        comp_output_path = os.path.join(self.kcs_output_dir, "components")
        download_output_path = os.path.join(self.kcs_output_dir, "download")
        lib_output_path = os.path.join(self.kcs_output_dir, "lib")
        lib_private_output_path = os.path.join(self.kcs_output_dir, "lib_private")
        lib_prebuilt_libs_path = os.path.join(self.kymera_src_path, "lib_release", self.download_config)
        cap_names = get_immediate_subdirectories(cap_output_path)
        cfg_mkf = os.path.join(self.kcs_output_dir, self.download_config + ".mkf")
        prebuilt_lib_names = []

        f = open(cfg_mkf, "r")
        matched = False
        for line in f:
            if matched:
                if line.endswith("\\"):
                    prebuilt_lib_names.extend([line.lstrip().rstrip(" \\")])
                else:
                    break

            match = prebuiltlibReStart.match(line)
            if match != None:
                prebuilt_lib_names.extend([match.group(1)])
                if line.endswith("\\"):
                    matched = True
                else:
                    break
        f.close()

        # We might not have any dependency on libraries or components, so catch exceptions if those
        # folders are missing
        support_lib_names = {}

        try:
            support_lib_folders = get_immediate_subdirectories(support_lib_output_path)
            for support_lib_folder in support_lib_folders:
                makefile_path = os.path.abspath(os.path.join(self.kymera_src_path, 'support_lib', support_lib_folder, 'makefile'))
                try:
                    if os.path.isfile(makefile_path):
                        f = open(makefile_path, 'r')
                        for line in f.readlines():
                            match = re.search(r'TARGET_LIB_ROOT\s*=\s*(\w+)', line)
                            if match and match.group(1):
                                support_lib_names[support_lib_folder] = match.group(1)
                        f.close()
                except IOError:
                    pass
        except OSError as err:
            # errno 2 means "No such file or directory"
            if err.errno != 2:
                raise(err)

        try:
            comp_names = get_immediate_subdirectories(comp_output_path)
        except OSError as err:
            # errno 2 means "No such file or directory"
            if err.errno == 2:
                comp_names = []
            else:
                raise(err)

        try:
            lib_names = get_immediate_subdirectories(lib_output_path)
        except OSError as err:
            # errno 2 means "No such file or directory"
            if err.errno == 2:
                lib_names = []
            else:
                raise(err)

        try:
            lib_private_names = get_immediate_subdirectories(lib_private_output_path)
        except OSError as err:
            # errno 2 means "No such file or directory"
            if err.errno == 2:
                lib_private_names = []
            else:
                raise(err)

        logging.info("cap_names" + str(cap_names))
        logging.info("support_lib_names" + str(support_lib_names))
        logging.info("comp_names" + str(comp_names))
        logging.info("lib_names" + str(lib_names))
        logging.info("lib_private_names" + str(lib_private_names))
        logging.info("prebuilt_lib_names" + str(prebuilt_lib_names))

        o_files = []

        for cap in cap_names:
            # Exclude processing intermediate dyn object files
            o_files.extend(
                [
                    os.path.join(
                        os.path.join(cap_output_path, cap),
                        "debugbin",
                        "lib{}.a".format(cap)
                    )
                ]
            )

        for (support_lib_folder, support_lib_name) in support_lib_names.items():
            # Exclude processing intermediate dyn object files
            o_files.extend(
                [
                    os.path.join(
                        os.path.join(
                            support_lib_output_path,
                            support_lib_folder
                        ),
                        "debugbin",
                        "lib{}.a".format(support_lib_name)
                    )
                ]
            )

        obj_names_and_paths_list = [
            (comp_names, comp_output_path),
            (lib_names, lib_output_path),
            (lib_private_names, lib_private_output_path),
            (prebuilt_lib_names, lib_prebuilt_libs_path)
        ]

        for elem in obj_names_and_paths_list:
            obj_names = elem[0]
            obj_path = elem[1]
            for obj_name in obj_names:
                if obj_names == prebuilt_lib_names:
                    o_files.extend(
                        [
                            os.path.join(
                                os.path.join(obj_path, obj_name)
                            )
                        ]
                    )
                else:
                    o_files.extend(
                        [
                            os.path.join(
                                os.path.join(obj_path, obj_name),
                                "debugbin",
                                "lib{}.a".format(obj_name)
                            )
                        ]
                    )

        # Add C runtime libraries to the list of object files that could be pulled in
        crt_path = get_kcc_version.kcc_version().get_kcc_path(self.os_type)
        crt_path = os.path.join(crt_path, "lib")
        crt_path = os.path.abspath(os.path.join(crt_path, chip.get_crt_lib_name()))
        o_files.extend([os.path.join(crt_path, "libc.a")])
        o_files.extend([os.path.join(crt_path, "libcrt.a")])
        o_files.extend([os.path.join(crt_path, "libfle.a")])

        # Add kdc_start object file
        dnld_obj_path = os.path.join(download_output_path, "debugobj")
        o_files.extend([os.path.join(dnld_obj_path, "kdc_start.o")])

        return o_files

    def check_lib_release_files(self, oFiles):
        """In order to avoid leaking information from private libraries build should fail
           if non private libraries are located in lib_release folder."""
        for f in oFiles:
            if "lib_release" in f:
                dir_path = os.path.dirname(os.path.realpath(f))
                logging.info("dir path " + str(dir_path))
                for r, d, files in os.walk(dir_path):
                    for f in files:
                        if str(f).endswith('.a') or str(f).endswith('.reloc'):
                            continue
                        raise Exception(
                            """In order to avoid leaking information from private libraries only
                                .a & .reloc files should be moved to lib_release folder. While building
                                {} has been found, this file could be leaking private data and should
                                be removed from the lib_release folder.""".format(str(f)))

    def main(self, get_kcc_version):
        self.verify_bundle_name()

        # Read in the existing build info for the release
        logging.info("Loading release build info...")
        ktools_path = os.path.join(
            get_kcc_version.kcc_version().get_kcc_path(self.os_type),
            "bin"
        )
        rel_binfo = download_build_info.build_info(
            self.build_dir, self.build_name, ktools_path, self.kcs_output_dir)
        logging.info("DONE")

        # Get the build id from Kymera ELF
        self.build_id = rel_binfo.get_build_id()

        chip = chip_info.chip_info(rel_binfo.get_chipname())
        if chip.is_supported() == False:
            raise Exception("Chip NOT supported: " + str(chip.get_string()))

        try:
            os.chdir(self.kcsmaker_path)
            mod = imp.load_source("mod", self.bdl_file)
        except IOError:
            logging.error("Can't find file " + str(self.bdl_file))
            raise

        try:
            if len(mod.KDCs) > 0:
                logging.info("Building KCS " + str(self.bundle_name))
            else:
                logging.error("Error. No KDCs to be built")
                exit()
        except IOError:
            logging.error("Error. KDCs variable NOT found in " + str(self.bdl_file))
            raise

        if self.linkscript_path == "":
            self.linkscript_path = os.path.join(
                self.kcsmaker_path,
                "templates",
                chip.get_linkscript_template_name()
            )

        logging.info("DOWNLOAD_CONFIG = %s" % self.download_config)
        logging.info("RELEASE_FLAG = %s" % self.release_flag)
        logging.info("CAPABILITIES_PATH = %s" % self.capabilities_path)
        logging.info("KYMERA_SRC_PATH = %s" % self.kymera_src_path)
        logging.info("BUNDLE_NAME = %s" % self.bundle_name)
        logging.info("BUILD_DIR = %s" % self.build_dir)
        logging.info("PATCH_DIR = %s" % self.patch_dir)
        logging.info("BUILD_NAME = %s" % self.build_name)
        logging.info("BUILD_ROOT = %s" % self.build_root)
        logging.info("BUILD_ID = %s" % self.build_id)
        logging.info("KCS_OUTPUT_DIR = %s" % self.kcs_output_dir)
        logging.info("KCSMAKER_PATH = %s" % self.kcsmaker_path)
        logging.info("LINKSCRIPT_PATH = %s" % self.linkscript_path)
        logging.info("LABEL_NAME = %s" % self.label_name)

        kcsFileRoot = os.path.join(
            self.kcs_output_dir,
            "download",
            "debugbin",
            self.bundle_name
        )
        kcsFileT = kcsFileRoot + ".kcs"
        kcsFileB = kcsFileRoot + ".dkcs"
        kcsFileD = kcsFileRoot + ".kcs_DRAM"

        makeKCS = make_kcs.make_kcs(chip, self.build_id)

        logging.info("Generating KCS file " + kcsFileT + " ...")

        # Create output file if it doesn't exist
        if not os.path.exists(self.kcs_output_dir):
            os.makedirs(self.kcs_output_dir)

        # Check if we are building on the same bundle. If not then force a clean build
        if (os.path.exists(os.path.join(self.kcs_output_dir, "last_bundle.txt"))):
            last_bundle_file = open(os.path.join(self.kcs_output_dir, "last_bundle.txt"), "r")
            prev_bundle = last_bundle_file.readline()
            if prev_bundle != self.bundle_name:
                self.clean_build = True
                logging.info(
                    "Previous build detected: {prev_build}, now building: {cur_build}. \
                    Forcing clean build".format(
                        prev_build = prev_bundle,
                        cur_build = self.bundle_name
                    )
                )
            last_bundle_file.close()
        else:
            self.clean_build = True

        if self.clean_build == None:
            self.clean_build = self.release_flag

        kdc_count = 1
        for kdc_name in mod.KDCs.keys():
            logging.info("Generating KDC " + kdc_name)
            logging.info("Building " + str(mod.KDCs[kdc_name]) + " capabilities ...")
            cap_list = []
            for cap_tuple in mod.KDCs[kdc_name]:
                cap_list.append(
                    capability.Capability(
                        cap_tuple[0],
                        cap_tuple[1],
                        cap_tuple[2],
                        cap_tuple[3],
                        self.capabilities_path
                    )
                )

            # Build the elf and object files for each capability
            # Create a Build_Controller object to control the process

            if self.clean_build:
                # If Make sure the KDC file is created with an empty output directory
                for dir_or_file in os.listdir(self.kcs_output_dir):
                    dir_or_file_abs = os.path.join(self.kcs_output_dir, dir_or_file)
                    if os.path.isdir(dir_or_file_abs):
                        # Don't delete this temp directory used to store kmap files
                        if dir_or_file != "kmap_tmp":
                            shutil.rmtree(dir_or_file_abs)
                    else:
                        # It's a file, remove it
                        os.remove(dir_or_file_abs)

            # Write current bundle into a file
            last_bundle_file = open(os.path.join(self.kcs_output_dir, "last_bundle.txt"), "w")
            last_bundle_file.write(self.bundle_name)
            last_bundle_file.close()

            if self.make_jobs is None:
                self.make_jobs = True

            build = build_controller.Build_Controller(
                self.build_root,
                self.build_dir,
                self.kcs_output_dir,
                self.kcsmaker_path,
                self.download_config,
                rel_binfo.get_elf_name(),
                self.extra_include,
                self.build_name,
                self.kymera_src_path,
                self.os_type,
                self.patch_dir
            )
            build.build(
                rel_binfo,
                cap_list,
                kdc_name,
                self.linkscript_path,
                self.make_path,
                self.clean_build,
                self.make_jobs
            )

            logging.info(str(mod.KDCs[kdc_name]) + " capabilities successfully built.")

            # Copy kmap file to temp folder (it will get deleted on next KDC build)
            if not os.path.exists(os.path.join(self.kcs_output_dir, "kmap_tmp")):
                os.makedirs(os.path.join(self.kcs_output_dir,"kmap_tmp"))

            shutil.copy(
                os.path.join(self.kcs_output_dir, "download", "debugbin", kdc_name + ".kmap"),
                os.path.join(self.kcs_output_dir,"kmap_tmp", kdc_name + "(KDC).kmap")
            )

            objDumpPath = os.path.join(
                get_kcc_version.kcc_version().get_kcc_path(self.os_type),
                "bin",
                "kobjdump"
            )

            oFiles = self.get_obj_files(chip, get_kcc_version)
            logging.info("oFiles" + str(oFiles))
            self.check_lib_release_files(oFiles)

            dnldElfFile = os.path.join(
                self.kcs_output_dir, "download", "debugbin", kdc_name + ".elf"
            )
            mapOutFile = os.path.join(
                self.kcs_output_dir, "download", "debugbin", "map_out.txt"
            )

            makeKDC = make_kdc.make_kdc(
                dnldElfFile,
                rel_binfo.get_elf_name(),
                chip,
                objDumpPath,
                oFiles,
                cap_list,
                self.os_type,
                mapOutFile
            )

            kdc = makeKDC.generateKDC(kdc_count, rel_binfo.get_rom_kalaccess())
            print(("Sizes for KDC {}:".format(dnldElfFile)))
            kdc.print_memory_usage()
            makeKCS.addKDC(kdc)
            kdc_count = kdc_count + 1

        # Copy kmap files to the output folder where the KCS files are
        files = glob.iglob(os.path.join(self.kcs_output_dir, "kmap_tmp", "*.kmap"))
        for file in files:
            if os.path.isfile(file):
                debugbin_dir = os.path.join(
                    self.kcs_output_dir,
                    "download",
                    "debugbin"
                )

                if self.clean_build:
                    shutil.move(file, debugbin_dir)
                else:
                    shutil.copy(file, debugbin_dir)

        if self.clean_build:
            os.rmdir(os.path.join(self.kcs_output_dir, "kmap_tmp"))

        kcs = makeKCS.generateKCS()
        kcs.dumpToTextFile(kcsFileT)
        kcs.dumpToBinaryFile(kcsFileB)
        kcs.dumpToDRAMFile(kcsFileD)

        if self.release_flag:

            import release


            this_rel = release.release(
                self.release_flag,
                self.kcsmaker_path,
                self.build_id,
                self.bundle_name,
                self.download_config,
                self.user
            )

            release.get_lock()

            # Generate new release ID
            this_rel.generate_new_version_id()
            self.version_build_id = this_rel.get_version_id()
            logging.info("New BUILD ID: " + str(this_rel.get_version_id()))

            if self.kymera_src_id == "":
                self.kymera_src_id = self.get_base_build_id_from_kymera_src_path()

            this_rel.create_log()
            label = this_rel.label()
            this_rel.write_log("Label for released code " + str(label))
            this_rel.write_log("Base Build ID\t" + str(self.kymera_src_id))
            this_rel.end_log()

            this_rel.archive_KCS(this_rel.obtain_product_name(self.download_config))

            release.release_lock()
        else:
            # Copy binaries to a common folder so they don't get erased when a new build is triggered
            if not os.path.exists(
                os.path.join(
                    self.kymera_src_path,
                    "output_bundles",
                    self.download_config,
                    self.bundle_name
                )
            ):
                os.makedirs(
                    os.path.join(
                        self.kymera_src_path,
                        "output_bundles",
                        self.download_config,
                        self.bundle_name
                    )
                )

            file_ext_list = [".dkcs",".kcs", ".elf", ".kmap", ".lst", ".sym"]
            for file_ext in file_ext_list:
                shutil.copy(
                    os.path.join(
                        self.kcs_output_dir,
                        "download",
                        "debugbin",
                        self.bundle_name + file_ext
                    ),
                    os.path.join(
                        self.kymera_src_path,
                        "output_bundles",
                        self.download_config,
                        self.bundle_name,
                        self.bundle_name + file_ext
                    )
                )

            print("Bundle built successfully and available at: " +
                  "{0}".format(os.path.join(self.kymera_src_path,
                                            "output_bundles",
                                            self.download_config,
                                            self.bundle_name,
                                            self.bundle_name + ".dkcs")))


if __name__ == "__main__":
    # This needs at least version 2.6 of Python (needs the as keyword) not checked
    # because Python dies before doing this check
    # Process command line arguments
    logging.basicConfig(
        filename = "KCS_build.log",
        format = '%(asctime)s %(message)s',
        filemode = "w",
        level = logging.INFO
    )
    logging.info("Initialising KCSMaker...")

    cmd_line_args = proc_cmd_line(sys.argv[1:])

    # Can't import get_kcc_version until this point because we
    # need to get it from the target build path.
    sys.path.append(cmd_line_args["BUILD_ROOT"])
    # Sort out paths for python tools
    sys.path.append(
        get_python_tools.python_tools_version().get_python_tools_path(
            cmd_line_args["OS_TYPE"]
        )
    )

    import kalaccess
    import get_kcc_version

    kcsmaker = KCSMaker(cmd_line_args)
    kcsmaker.main(get_kcc_version)

