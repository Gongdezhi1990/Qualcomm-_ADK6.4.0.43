"""
%%fullcopyright(2016 - 2018)

Provides the command line interface for deployment on attached hardware or simulator
"""
# Python 2 and 3
from __future__ import print_function

import glob
import os
import pprint
import re
import sys
import time
import subprocess
import tempfile
import collections
import shutil

import maker.proclauncher as pl
import maker.parse_proj_file as pproj
import maker.parse_devkit as pdk
import maker.sim_launch as sim
import maker.exceptions as bdex
import maker.nvscmd as nvs
import maker.subsystem_numbers as subsys_numbers

SQIF0_CONFIG_FILE = "flash_layout_config.py"

FLASH_DEVICE_PARAMETERS = {
    # Size of individually erasable blocks in the main section
    # of the flash device (i.e. other than boot blocks).
    # Re-writable sections will be placed on these boundaries
    # and padding added afterwards if necessary
    "block_size": 64 * 1024,

    # Size of the boot block section at the lower address range
    # of the flash device. Set to None if a boot block is not
    # available
    "boot_block_size": None,

    # Offset within the flash device where the alternative image
    # will be written and executed from when performing an
    # update in the field.
    "alt_image_offset": 128 * 64 * 1024
}

def create_default_layout():
    return  {

        "flash_device": FLASH_DEVICE_PARAMETERS,
        
        "encrypt": False,
        
        "layout": []
    }




class DeploymentError(RuntimeError):
    """
    Something went wrong with the deploy process
    """
    
class FirmwareLoadDotPyFailed(DeploymentError):
    """
    An invocation of firmware_load.py via pydbg in a subprocess failed with a
    non-zero exit status
    """

class NoDeviceDetected(DeploymentError):
    """
    Heracles has passed an unexpanded argument to deploy.py because it hasn't
    detected a device to which it could point.  Arguably Heracles shouldn't be
    trying to invoke deploy.py in this case, but it's easy enough to detect 
    here
    """

class BadDeploymentOrder(DeploymentError):
    """
    The layout of SQIF0 is a delicate business.  To cut a long story short, you
    have to flash the Apps P0 firmware first.  This exception is raised when
    that hasn't been done (so far as the local record of the current layout
    is concerned) 
    """

def load_apps_sqif0_layout(devkit_path, transport_uri):
    """
    Load the current SQIF configuration file, or create a default one if 
    necessary.  Note: if a file is hanging around from a previous instance of
    Heracles then unexpected things may happen on the first attempt to deploy. 
    """
    global SQIF0_CONFIG_FILE
    config_path = os.path.join(devkit_path, "tmp", transport_uri.replace(":","_"), 
                               SQIF0_CONFIG_FILE)
    if os.path.isfile(config_path):
        config_globals = {}
        exec(compile(open(config_path).read(), config_path, 'exec'), config_globals)
        return config_globals["flash0"]
    
    print("No pre-existing config found: creating default")
    return create_default_layout()

def create_apps_sqif0_layout(devkit_path, transport_uri, layout_dict):
    """
    Write the new image layout to the cache location and execute it.
    """
    global SQIF0_CONFIG_FILE
    from csr.dev.tools.flash_image_builder import ImageBuilder
    
    builder = ImageBuilder(layout_dict)
    builder.build()

    print("Boot Image - offset 0x%x, size 0x%x" % (builder.boot_image["offset"], builder.boot_image["size"]))

    print("Image output sections:")
    for section in builder.image:
        print("Flash offset 0x%08x size 0x%x" % (section[0], len(section[1])))
    print("Image Header:" )
    print(builder.image_header)

    fd, tmpfile = tempfile.mkstemp(suffix=".xuv")
    os.close(fd)
    builder.write_complete_flash_image_xuv(tmpfile)

    # Write the new configuration to the local cache
    sqif0_config_file = os.path.join(devkit_path, "tmp", 
                                     transport_uri.replace(":","_"), 
                                     SQIF0_CONFIG_FILE)
    if not os.path.isdir(os.path.dirname(sqif0_config_file)):
        os.makedirs(os.path.dirname(sqif0_config_file), 0o775)
    with open(sqif0_config_file,"w") as config_file:
        config_file.write("flash0 = %s" % pprint.pformat(layout_dict, indent=2, width=80))

    return tmpfile

def disable_deep_sleep_if_possible(device):
    
    from csr.dev.fw.slt import RawSLT
    cur = device.chip.curator_subsystem.core
    try:
        if cur.fw.slt.build_id_number == 1102:
            # Crescendo A04 ROM. The "extern" row of the kip table is at 0x8309; we 
            # need to set bit 2 (DORM_STATE_NO_DEEP) to disable deep sleep
            cur.data[0x8309] |= 4
        else:
            if hasattr(cur.fw, "env"):
                # We loaded with an ELF
                dorm_state = cur.fw.env.enums["dorm_state"]
                dorm_ids = cur.fw.env.enums["dorm_ids"]
                print ("WARNING: Skipping write to kip_table, see B-253281")
                #cur.fw.gbl.kip_table[dorm_ids["DORMID_EXTERNAL"]].value |= dorm_state["DORM_STATE_NO_DEEP"]
            else:
                print ("WARNING: Unsupported core firmware: can't disable deep "
                       "sleep during QSPI writing")
    except RawSLT.BadFingerprint:
        print ("WARNING: Couldn't find core firmware: can't disable deep sleep during flashing")
    
def disable_curator_usb_interrupts(device):

    from csr.dev.hw.address_space import AddressSpace
    cur = device.chip.curator_subsystem.core
    
    try:
        cur.fields.USB2_CURATOR_INTERRUPT_ENABLE = 0
        cur.fields.USB2_CURATOR_EVT_DESC_ENABLE = 0
    except (AddressSpace.ReadFailure, AddressSpace.WriteFailure):
        print ("WARNING: Couldn't find core firmware: can't disable USB interrupts during flashing")
    
    return True
    
def write_sqif_image(image_type, image_path, transport_uri, devkit_path, workspace_file, use_nvscmd, proj_parser, build_output_folder=None):
    """
    Write the varying types of SQIF image into the appropriate SQIF using
    pydbg's native functionality or the NvsCmd tool.
    """
 
    if (use_nvscmd == True) and (image_type != "curator"):
        return nvscmd_flash_image(image_type, image_path, transport_uri, devkit_path, proj_parser, workspace_file, build_output_folder)
    else:
        return pydbg_flash_image(image_type, image_path, transport_uri, devkit_path)
        
def pydbg_flash_image(image_type, image_path, transport_uri, devkit_path):
    """
    Write the varying types of SQIF image into the appropriate SQIF using
    pydbg's native functionality.
    """
    from maker.pydbg_flash_image import Pyflash

    print("Program " + image_type + " with image " + image_path)
    pyflash = Pyflash(devkit_path, transport_uri)
    result = pyflash.burn(image_type, image_path, reset_device=True)
    
    return result
    
def get_subsystem_name(proj, tgt_config):
    config = proj.get_make_vars(tgt_config)
    return config['SUBSYSTEM_NAME']

def deploy_prebuilt_image(proj_parser, parsed_args):

    config_keys = proj_parser.get_properties_from_config(parsed_args.configuration)

    try:
        subsystem = config_keys["SUBSYSTEM_NAME"]

    except KeyError as excep:
        print("ERROR! Build Setting %s missing." % excep)
        return False

    files = proj_parser.get_source_files()
    xuv_files = [f for f in files if f.endswith(".xuv")]
    elf_files = [f for f in files if f.endswith(".elf")]
    
    if len(xuv_files) == 1:
        image_path = xuv_files[0]
    elif len(elf_files) == 1:
        image_path = elf_files[0]
    else:
        print("ERROR! Can't find an XUV or ELF file to flash!")
        print("Project files are %s" % files)
        return False

    # Now see if we can find an ELF file to tell the IDE about
    ss_name = get_subsystem_name(proj_parser, parsed_args.configuration)
    if len(elf_files) == 1:
        # If there's an ELF in the project file, use that.
        elf_file = elf_files[0]
    elif ss_name == "audio":
        # For some reason kymera_audio.x2p doesn't mention the ELF.  So we 
        # guess that it's available alongside the XUV.
        elf_file = xuv_files[0].replace(".xuv",".elf")
        if not os.path.isfile(elf_file):
            # If we guessed wrong, give up
            elf_file = None
    else:
        # No ELF that we know of
        elf_file = None
        
    if elf_file:
        # Report the ELF to the IDE for loading at debugging time 
        attribs = collections.OrderedDict()
        attribs['type'] = 'info'
        attribs['config'] = parsed_args.configuration
        # Translate the "subsystem name" as given in the x2p file to the
        # IDE's name for the core
        attribs["core"] = {"apps0" : "app/p0",
                           "audio" : "audio/p0"}.get(ss_name, None)
        attribs['module'] = 'deploy'
        bdex.log_buildproc_output('elfpath', attribs, elf_file)
        

    return write_sqif_image(subsystem, image_path, parsed_args.deploy_target, 
                            parsed_args.devkit_root, parsed_args.workspace_file, use_nvscmd(parsed_args), proj_parser)

                            
def deploy_filesystem(proj_parser, parsed_args, build_output_folder=None):
    """
    Invoke the prebuilt image writer to flash the given filesystem type to the 
    Apps SQIF. 
    """
    working_dir = proj_parser.proj_dirname
    config_keys = proj_parser.get_properties_from_config("filesystem")
    try:
        output_filename = config_keys["OUTPUT"]
    except KeyError:
        output_filename = None
    try:
        filesystem_type = config_keys["TYPE"]
    except KeyError as excep:
        print("ERROR! Build Setting %s missing." % excep)
        return False

    if not output_filename:
        output_filename = filesystem_type + "_filesystem"
        if use_nvscmd(parsed_args) and filesystem_type != "curator_config":
            # This is a build for an apps filesystem that is needed in the form
            # required by DFU, rather than in the form required for deploy
            output_filename = output_filename + "_dfu"

    if build_output_folder is not None: 
       xuv_path = os.path.join(build_output_folder, output_filename + '.xuv')
    else:
       xuv_path = os.path.join(working_dir, output_filename + '.xuv')
    
    # Maps from ADK names to Pylib names
    config_name = {"curator_config"  : "curator_fs",
                   "firmware_config" : "p0_ro_cfg_fs",
                   "device_config"   : "p0_device_ro_fs",
                   "user_ps"         : "p0_rw_config",
                   "customer_ro"     : "p0_ro_fs"}[filesystem_type]
                   
    if os.path.isfile(xuv_path):
        print("Deploying %s filesystem" % filesystem_type)
        if filesystem_type == "customer_ro":
            bundle_files = [x for x in proj_parser.get_source_files() if x.endswith(".dkcs")]
            if len(bundle_files) > 0:
                for bundle in bundle_files:
                    # Get the associated ELF
                    bundle_elf = os.path.splitext(bundle)[0] + ".elf"
                    if os.path.isfile(bundle_elf):
                        # Now report the ELF to the IDE for loading when debugging
                        attribs = collections.OrderedDict()
                        attribs['type'] = 'info'
                        attribs['config'] = parsed_args.configuration
                        # bundles are only for audio SS
                        attribs["core"] = "audio/p0"
                        attribs['module'] = 'deploy'
                        bdex.log_buildproc_output('elfpath', attribs, bundle_elf)
        
        return write_sqif_image(config_name, xuv_path, parsed_args.deploy_target, 
                                parsed_args.devkit_root, parsed_args.workspace_file, use_nvscmd(parsed_args), proj_parser, build_output_folder)
    else:
        print("Nothing to deploy for %s filesystem" % filesystem_type)
        return True

        
def deploy_built_project(proj_parser, dk_parser, parsed_args, build_output_folder=None):
    """
    Invoke the prebuilt image write to flash the P1 firmware from the location
    of the build output
    """
    if build_output_folder is None:
        build_output_folder = os.path.join(proj_parser.proj_dirname, 
			'depend_%s_%s' % (parsed_args.configuration, dk_parser.get_identity()['fsprefix']))

    chip_prefix = dk_parser.get_identity()["fsprefix"]      
    if chip_prefix == "crs01":
        chip_prefix = "crescendo"
    image_file = '%s_app_p1.elf' % chip_prefix
    image_path = os.path.join(proj_parser.proj_dirname, build_output_folder,
                              image_file)

    return write_sqif_image("apps1", image_path, parsed_args.deploy_target, 
                            parsed_args.devkit_root, parsed_args.workspace_file, use_nvscmd(parsed_args), proj_parser, build_output_folder)


def deploy_makefile_project(proj_parser, parsed_args):
    config_keys = proj_parser.get_properties_from_config(parsed_args.configuration)

    try:
        xuv_filename = config_keys["OUTPUT"]
        working_dir = proj_parser.proj_dirname
        xuv_path = os.path.abspath(os.path.join(working_dir, xuv_filename))
    except KeyError as excep:
        print("ERROR! Build Setting %s missing." % excep)
        return False

    try:
        subsystem = config_keys["SUBSYSTEM_NAME"]

    except KeyError as excep:
        print("ERROR! Build Setting %s missing." % excep)
        return False

    return write_sqif_image(subsystem, xuv_path, parsed_args.deploy_target,
                            parsed_args.devkit_root, parsed_args.workspace_file, use_nvscmd(parsed_args), proj_parser)

                            
def deploy_special_deploy_project(parsed_args, build_output_folder=None):
    """
    Deploy single image to SQIF based on the workspace dependencies  .
    """
    if not use_nvscmd(parsed_args):
        return True

    options = parsed_args.special.split(" ")
    deploy_all = False
    for opt in options:
        cmd = opt.split("=")
        if "deploy_all" in cmd[0]:
            if "yes" in cmd[1]:
                deploy_all = True
                
    tools_root = os.path.join(parsed_args.devkit_root,"tools","bin") # Assume devkit contains NvsCmd tool
    if build_output_folder is None:
        args = ["-k",parsed_args.devkit_root,"-w",parsed_args.workspace_file,"-t",tools_root,"-d",parsed_args.deploy_target]
    else:
        args = ["-k",parsed_args.devkit_root,"-w",parsed_args.workspace_file,"-t",tools_root,"-d",parsed_args.deploy_target, "-o", build_output_folder]

    if not deploy_all:
        args.append("-a")

    from . import deploy_single_image
    return deploy_single_image.main(args)
    
    
def deploy_to_device(proj_parser, dk_parser, parsed_args, build_output_folder=None):
    """
    Invoke pydbg to push the given configuration to the device
    """
    
    if "nodevice" in parsed_args.deploy_target:
        raise NoDeviceDetected("IDE has not detected any attached device!")
    
    print("Deploying %s to '%s'" % (parsed_args.configuration,
                                    parsed_args.deploy_target))

    if parsed_args.configuration == "prebuilt_image":
        return deploy_prebuilt_image(proj_parser, parsed_args)
 
    elif parsed_args.configuration == "filesystem":
        return deploy_filesystem(proj_parser, parsed_args, build_output_folder)

    elif parsed_args.configuration == "makefile_project":
        return deploy_makefile_project(proj_parser, parsed_args)
        
    elif parsed_args.configuration == "deploy":
        return deploy_special_deploy_project(parsed_args, build_output_folder)

    else:
        return deploy_built_project(proj_parser, dk_parser, parsed_args, build_output_folder)


def deploy(proj_parser, dk_parser, parsed_args):
    tgt_config = parsed_args.configuration
    build_output_folder = parsed_args.build_output_folder
    props = proj_parser.get_properties_from_config(tgt_config)

    if build_output_folder is None:
        build_output_folder = props.get('BUILD_OUTPUT_FOLDER')

    if build_output_folder is None:
        build_output_folder = os.getenv("BUILD_OUTPUT_FOLDER")

    if build_output_folder is not None:
        proj_name = proj_parser.proj_projname
        if parsed_args.configuration != "deploy":
            build_output_folder = os.path.join(build_output_folder, proj_name)

        if not os.path.exists(build_output_folder):
            os.makedirs(build_output_folder)
    config_keys = proj_parser.get_properties_from_config(parsed_args.configuration)

    # Launch kalsim if selected
    if config_keys["SUBSYSTEM_NAME"] == "audio" and "KALSIM_MODE" in config_keys:
        if config_keys["KALSIM_MODE"] == "true":
            kse_path = os.path.abspath(os.path.join(proj_parser.proj_dirname, config_keys["KALSIM_KSE_PATH"]))
            if "BUNDLE_NAME" in config_keys:
                # Deploying a KCS in kalsim means copying it to the scripts folder
                src_dkcs_path = os.path.abspath(os.path.join(proj_parser.proj_dirname, config_keys["OUTPUT"]))
                src_dkcs_path = os.path.splitext(src_dkcs_path)[0] + ".dkcs"
                bundle_name = config_keys["BUNDLE_NAME"].split("BUNDLE_NAME=")[1]
                dst_dkcs_path = os.path.join(kse_path, "script/kalsim/{bdl_name}.dkcs".format(bdl_name=bundle_name))
                print("{s_dkcs_path} file copied to {d_dkcs_path}".format(s_dkcs_path=src_dkcs_path, d_dkcs_path=dst_dkcs_path))
                shutil.copyfile(src_dkcs_path, dst_dkcs_path)
            fw_path = os.path.abspath(os.path.join(proj_parser.proj_dirname, config_keys["KALSIM_FIRMWARE"]))
            patch_path = config_keys.get("KALSIM_PATCH_BUNDLE", "")
            if patch_path:
                patch_path = os.path.abspath(os.path.join(proj_parser.proj_dirname, patch_path))
            audio_elf_path = os.path.abspath(os.path.join(proj_parser.proj_dirname, config_keys["OUTPUT"]))
            # Get the associated ELF
            if os.path.isfile(audio_elf_path):
                # Now report the ELF to the IDE for loading when debugging
                attribs = collections.OrderedDict()
                attribs['type'] = 'info'
                attribs['config'] = parsed_args.configuration
                attribs["core"] = "audio/p0"
                attribs['module'] = 'deploy'
                bdex.log_buildproc_output('elfpath', attribs, audio_elf_path)
            if audio_elf_path != fw_path and os.path.isfile(fw_path):
                attribs = collections.OrderedDict()
                attribs['type'] = 'info'
                attribs['config'] = parsed_args.configuration
                attribs["core"] = "audio/p0"
                attribs['module'] = 'deploy'
                bdex.log_buildproc_output('elfpath', attribs, fw_path)
            else:
                audio_elf_path = ""
            return sim.launch_audio_kse_env(parsed_args.devkit_root, kse_path, fw_path, config_keys["KALSIM_NAME"], config_keys["KALSIM_ENABLE_DEBUGGER"], config_keys["KALSIM_SCRIPTS"], config_keys["KALSIM_PLATFORM"], patch_path, audio_elf_path)
        elif parsed_args.configuration == "debug":
            raise Exception("Error. Cannot deploy downloadable bundle on device from this project. KALSIM_MODE is set to false. Rebuild the project, add the .dkcs to the customer_ro_filesystem project and deploy from there")

    if re.search(r'(/sim/)', parsed_args.deploy_target):
        #URI is a simulator
        simulator = dk_parser.get_toolchain()['properties']['simulator']['argument']
        sim_path = os.path.normpath(os.path.join(dk_parser.devkit_dirname, simulator))
        build_output_folder = 'depend_%s_%s' % (parsed_args.configuration,
                                      dk_parser.get_identity()['fsprefix'])
        working_dir = os.path.join(proj_parser.proj_dirname, build_output_folder)
        kap_files = glob.glob(os.path.join(working_dir,'*.kap'))
        if kap_files:
            runnable = kap_files[0]
        else:
            runnable = os.path.join(working_dir, proj_parser.proj_projname)

        print("WORKING DIR: %s" % working_dir)
        print("SIMULATOR PATH: %s" % sim_path)
        print("RUNNABLE: %s" % runnable)
        sim.launch_sim(working_dir, sim_path, runnable)
    else:
        #URI is an attached physical device
        return deploy_to_device(proj_parser, dk_parser, parsed_args, build_output_folder)


def prepare_apps_image(devkit, proj_parser, workspace_file, image_type, build_output_folder):
    """
    Use NvsCmd tool to deploy a single project.
    """
    from .prepare_single_image import PrepareSingleImage

    write_header = (image_type == "apps0")

    proj_path = proj_parser.proj_fname
    single_image = PrepareSingleImage(devkit, workspace_file, proj_path, build_output_folder, generate_audio_image_xuv = True)

    output_path = tempfile.mkdtemp()
    input_path = tempfile.mkdtemp()
    proceed, chip_type = single_image.process_project(proj_path, input_path)
    apps_image, audio_image = single_image.create_flash_image(devkit, output_path, include_all_flash_sections=False, include_header_section=write_header)

    return apps_image, input_path, output_path


def clean_apps_image(input_path, output_path):
    def make_writeable(func, path, exc_info):
        if func is os.remove:
            os.chmod(path, 0o640)
            func(path)
    if input_path:
        shutil.rmtree(input_path, onerror=make_writeable)
    if output_path:
        shutil.rmtree(output_path, onerror=make_writeable)

        
def get_burn_image(image_type, image_path, devkit_path, proj_parser, workspace_file, build_output_folder = None):
    input_path = None
    output_path = None
    
    # These are rolled into the apps image
    images_in_apps = ("apps0", "apps1", "curator_fs", "p0_rw_config", "p0_ro_cfg_fs", "p0_ro_fs", "p0_device_ro_fs")

    if image_type in images_in_apps:
        if image_type == "apps0":
            write_header = True
        else:
            write_header = False
        burn_image, input_path, output_path = prepare_apps_image(devkit_path, proj_parser, workspace_file, write_header, build_output_folder)
    else:
        burn_image = image_path

    return burn_image, input_path, output_path

    
def nvscmd_flash_image(image_type, image_path, transport_uri, devkit_path, proj_parser, workspace_file, build_output_folder=None):
    """
    Write the varying types of SQIF image into the appropriate SQIF using nvscmd
    """
    sys.stdout.flush()

    success = False
    subsys_id = subsys_numbers.SubsystemNumbers.get_subsystem_number_from_name(image_type)
    
    if subsys_id != -1:
        burn_image, input_path, output_path = get_burn_image(image_type, image_path, devkit_path, proj_parser, workspace_file, build_output_folder)
        
        if burn_image:
            tools_dir = os.path.join(devkit_path, "tools", "bin")
            nvscmd = nvs.NvsCmd(tools_dir, transport_uri)
            result = nvscmd.burn(burn_image, subsys_id, reset_chip=True)
            if result == 0:
                success = True

            # Give the hardware a couple of seconds prior to being run
            time.sleep(2)
 
        clean_apps_image(input_path, output_path)
    else:
        print("Unknown image type " + image_type)

    return success
    

def use_nvscmd(parsed_args):
    """
    Determine if the ubuild arguments specify to use the NvsCmd tool for flashing.
    """
    ubuild_special = parsed_args.special
    if ubuild_special is not None:
        if "nvscmd" in ubuild_special:
            return True
    return False
    
