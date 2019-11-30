#!/usr/bin/env python
# Copyright (c) 2017 - 2018 Qualcomm Technologies International, Ltd.
#   Part of 6.3
"""
    Create a flash image file from the current workspace.
    Provides BuildRunner class to build projects before they are collected into an image.
"""
# Python 2 and 3
from __future__ import print_function
from __future__ import division

import sys
import os
from xml.etree.cElementTree import parse as xml_parse
import shutil

import elf2xuv
from workspace_parse.workspace import Workspace


class PrepareSingleImage(object):
    """
    Collects workspace projects into a single flash image.
    """
    # List the configs that _search_project_for_flash_config_file is not interested in
    _excludedConfigs=["prebuilt_image","filesystem","makefile_project"]

    def __init__(self, devkit_root, workspace_file, workspace, build_output_folder = None, generate_audio_image_xuv = False, device_uri = None):
        """
        Class init
        """
        self._devkit_root = devkit_root
        self._workspace_file = workspace_file
        self._workspace = workspace
        self._projects = {}
        self._generate_audio_image_xuv = generate_audio_image_xuv
        self._device_uri = device_uri
        self._build_output_folder = build_output_folder
        self._determine_flash_config()

    def _determine_flash_config(self):
        """
        Determine the flash configuration that needs to be used.
        Look for a flash configuration file in the default configuration of the
        app/p1 project file. If found, use the flash configuration in that.
        """
        flash_config_file = self._search_projects_for_flash_config_file()
        if flash_config_file:
            # The flash configuration file has been found
            if not os.path.isfile(flash_config_file):
                flash_config_file = os.path.normpath(
                    os.path.join(os.path.dirname(self._workspace_file), flash_config_file))

            if os.path.isfile(flash_config_file):
                try:
                    with open(flash_config_file, 'r') as fd:
                        contents = fd.read()
                        self.flash_config = eval(contents)
                    print("Using FLASH_CONFIG %s" % (flash_config_file))
                    sys.stdout.flush()
                    return
                except IOError as err:
                    print("Unable to open and read %s: %s" % (flash_config_file, err))
            else:
                print("FLASH_CONFIG file %s not found" % (flash_config_file))
        else:
            print("FLASH_CONFIG missing or unset in project")

        sys.stdout.flush()
        if not hasattr(self, 'flash_config') or not self.flash_config:
            raise RuntimeError("A valid FLASH_CONFIG was not found")

    def _search_projects_for_flash_config_file(self):
        """
        Look for a flash configuration file in the default configuration of the
        app/p1 project file.
        """
        ws_projects = Workspace(self._workspace_file).parse()
        # Go through the projects in the x2w file
        for project in ws_projects.values():
            # Try and get the flash configuration file from the project
            flash_config_file = \
                self._search_project_for_flash_config_file(project.filename)
            if flash_config_file:
                # The flash configuration file has been found so return it
                return flash_config_file
        # The flash configuration file has not been found in the projects
        return None

    def _search_project_for_flash_config_file(self, project):
        """
        Look through the configurations in a project for what may be the
        app/p1 and, if found, process the configuration keys.
        """
        import maker.parse_proj_file as pproj
        proj_parser = pproj.Project(project, self._devkit_root, self._workspace_file)
        config_list = proj_parser.get_configurations()
        for config in config_list:
            if not config in self._excludedConfigs and \
                self._apps1_project_is_default(project, config):
                # Could be the apps/p1 project
                config_keys = \
                    proj_parser.get_properties_from_config(config)
                flash_config_file = \
                    self._search_config_keys_for_flash_config_file(config_keys)
                if flash_config_file:
                    return flash_config_file
        return None

    def _search_config_keys_for_flash_config_file(self, config_keys):
        """
        Process the configuration keys in what may be the app/p1.
        """
        if "DBG_CORE" in config_keys and \
            config_keys["DBG_CORE"] == "app/p1" and \
            "FLASH_CONFIG" in config_keys:
            return config_keys["FLASH_CONFIG"]
        return None

    def _do_elf2xuv(self, source, destination=None):
        """
        Convert an elf file to xuv format.
        """
        elf2xuv.elf2xuv(0, source, xuv_filename=destination)

    def _project_is_deployable(self, proj_parser, config_name):
        """
        Determine if the config_name build configuration of the given project is deployable.
        """
        attrib = proj_parser.get_attributes_from_config(config_name)
        return "deploy" in attrib["options"]

    def _relocate_image(self, input_file, output_file):
        """
        This method relocates the address in the "input_file" to start from @0000000 and
        write the output into the "output_file".
        """
        in_file = open(input_file,'r')
        out_file = open(output_file,'w')
        linenum = 0

        for line in in_file:
            temp1 = format(linenum, '06X')
            linenum += 1
            temp2 = line[10:14]
            line = '@' + temp1 + '   ' + temp2 + '\n'
            out_file.write(line)

    def _find_and_copy_xuv_file(self, proj_parser, outpath):
        """
        Process a project to find the name of any file path ending with
        .xuv, and - if found, form the path and filename to the source file
        indicated and copy that file (as it is a pre-built image) to the required
        destination. If copied successfully, return the filename of the destination
        file, else raises an exception.
        """
        source = None
        destination = None
        files = proj_parser.get_source_files()
        for filepath in files:
            if filepath.endswith(".xuv"):
                print(filepath)
                source = os.path.abspath(os.path.join(proj_parser.proj_dirname, filepath))
                destination = os.path.join(outpath, os.path.basename(filepath))
                # Only take the first xuv filename found
                break

        if source:
            self._do_copy(source, destination)
        else:
            print("Unable to find file path ending in xuv")
            return None

        return destination

    def _process_curator_config_project(self, proj_parser, outpath):
        """
        Copy the filesystem xuv file to the required
        destination. If the copy was successful, return the filename of the
        destination file, else raises an exception.
        """
        if self._build_output_folder:
            build_output_folder = self._build_output_folder
        else:
            build_output_folder = proj_parser.proj_dirname

        source = os.path.abspath(os.path.join(build_output_folder, "curator_config_filesystem.xuv"))
        destination = os.path.join(outpath, "curator_config_filesystem.xuv")
        self._do_copy(source, destination)

        return destination

    def _process_apps_filesystem_project(self, proj_parser, project_type, outpath):
        """
        Copy the filesystem xuv file to the required
        destination. If the copy was successful, return the filename of the
        destination file, else raises an exception.
        """
        if self._build_output_folder:
            build_output_folder = self._build_output_folder
        else:
            build_output_folder = proj_parser.proj_dirname

        source = os.path.abspath(os.path.join(build_output_folder, project_type + "_filesystem_dfu.xuv"))
        destination = os.path.join(outpath, project_type + "_filesystem.xuv")
        self._do_copy(source, destination)

        return destination

    def _process_apps1_project(self, proj_parser, output, outpath, config):
        """
        Process an apps p1 project. First attempt to find the name of the output
        folder that the build output elf file will be put in from the SDK XML file.
        Then convert the elf file to an xuv file in the outpath location
        and return that xuv file location.
        If there are any errors, it might raise IOError.
        """
        import maker.util as util
        fs_prefix = util.get_fsprefix(self._devkit_root)
        if not fs_prefix:
            print("Unable to find fs_prefix in the SDK xml file")
            return None
        per_config_depend = 'depend_%s_%s' % (config, fs_prefix)

        elf_filename = output + ".elf"

        if self._build_output_folder:
            build_output_folder = self._build_output_folder
        else:
            build_output_folder = os.path.join(proj_parser.proj_dirname, per_config_depend)


        source = os.path.abspath(os.path.join(build_output_folder, elf_filename))
        destination = os.path.join(outpath, output + ".xuv")

        self._do_elf2xuv(source, destination)

        return destination

    def _do_copy(self, source, destination):
        if source != destination:
            try:
                shutil.copy(source, destination)
                print("Copied %s to %s" % (source, destination))
            except IOError as err:
                raise IOError("Unable to copy %s to %s: %s" % (source, destination, err))

    def _apps1_project_is_default(self, project, config_name):
        """
        Determine if the config_name build configuration is the default build
        configuration in the .x2p apps1 project.
        """
        proj_xml = xml_parse(project).getroot()
        configurations = proj_xml.findall("configurations")
        if len(configurations) != 1:
            raise RuntimeError("Multiple configurations elements in %s" % project)
        configuration = configurations[0].findall("configuration")

        for config in configuration:
            if config.attrib["name"] == config_name:
                if "default" in config.attrib["options"]:
                    return True

        return False

    def _add_layout(self, file_type, xuv_filename, start_tuple, encrypted):
        """
        Create a layout tuple for the given partition from a start_tuple. If the
        xuv_filename parameter is not None and the encrypted parameter is True
        then ensure that the "authenticate" item is True, unless for "audio"
        which is always "authenticate": False.
        """

        layout_dict = {}
        for item, value in start_tuple[1].items():
            if item == "authenticate":
                if xuv_filename != None and encrypted == True:
                    # If encrypted then all files supplied are authenticated
                    value = True
                if file_type == "audio":
                    # Apart from audio which is never authenticated
                    value = False
            layout_dict[item] = value
        if xuv_filename is not None:
            layout_dict["src_file"] = xuv_filename
        layout_tuple = (file_type, layout_dict)
        return layout_tuple

    def _attach_to_device(self):
        from csr.front_end.pydbg_front_end import PydbgFrontEnd

        if self._device_uri is None:
            print("Attach to Device failed, no device URI supplied")
            return None

        device_uri = self._device_uri
        target_list = device_uri.split('://')[1].split('/')[:-1]
        transport = target_list[1]
        trans_id = "trb:scar"
        if transport == "usb2tc":
            trans_id = "usb"

        print('Attaching to Device @ "%s"...' % trans_id)
        sys.stdout.flush()
        device, _ = PydbgFrontEnd.attach({"firmware_builds" : None,
                                          "device_url" : trans_id},
                                          shell=None)
        return device

    def process_project(self, project, outpath, build_output_folder=None):
        """
        Process a project supplied as an x2p xml file. Find all configurations in
        the x2p file given. Look for the project types that are of interest.
        If the configuration is not for a project type we are interested in, or if
        we have already processed a project of that type, then ignore the
        configuration. If a configuration is for a project thype that is of interest
        and we don't already have a project of that type, then process the project
        and, if processed successfully, add its project type and target filename to
        the dictionary of projects. If a project cannot be processed successfully,
        print an error message and exit.
        """
        print("Processing project %s" % project)
        build_output_folder = self._build_output_folder

        import maker.parse_proj_file as pproj
        chip_type = None
        proj_parser = pproj.Project(project, self._devkit_root, self._workspace_file)
        config_list = proj_parser.get_configurations()
        for config in config_list:
            project_type = None
            config_keys = proj_parser.get_properties_from_config(config)
            proj_name = proj_parser.proj_projname
            if config == "prebuilt_image":
                try:
                    project_type = config_keys["SUBSYSTEM_NAME"]
                except KeyError as excep:
                    print("ERROR! Build Setting %s missing." % excep)
                    return False, chip_type
            elif config == "filesystem":
                try:
                    project_type = config_keys["TYPE"]
                except KeyError as excep:
                    print("ERROR! Build Setting %s missing." % excep)
                    return False, chip_type
            elif config in ("makefile_project", "deploy"):
                continue
            else:
                # User defined configuration (should be the P1 application project)
                try:
                    project_type = config_keys["DBG_CORE"]
                except KeyError as excep:
                    print("ERROR! Build Setting %s missing." % excep)
                    return False, chip_type

            # Make sure we only handle one of each project_type supported
            if project_type is not None:
                if project_type not in self._projects.keys():


                    if build_output_folder and os.path.isdir(build_output_folder) and not build_output_folder.endswith(proj_name):
                        possible_outputfolder = os.path.join(build_output_folder, proj_name)
                        if os.path.isdir(possible_outputfolder):
                            self._build_output_folder = possible_outputfolder
                        else:
                            self._build_output_folder = build_output_folder

                    xuv_filename = None
                    if project_type == "apps0":
                        xuv_filename = self._find_and_copy_xuv_file(proj_parser, outpath)
                    elif project_type == "curator_config":
                        xuv_filename = self._process_curator_config_project(proj_parser, outpath)
                    elif project_type in ("firmware_config", "device_config", "customer_ro", "user_ps"):
                        xuv_filename = self._process_apps_filesystem_project(proj_parser, project_type, outpath)
                    elif project_type == "app/p1":
                        try:
                            output = config_keys["OUTPUT"]
                            chip_type = config_keys["CHIP_TYPE"].split('_')[0].upper()
                        except KeyError as excep:
                            print("ERROR! Build Setting %s missing." % excep)
                            return False, chip_type
                        # Make sure this is the default project to build
                        if self._apps1_project_is_default(project, config):
                            xuv_filename = self._process_apps1_project(proj_parser, output, outpath, config)
                        else:
                            # Not the default build we are interested in
                            continue
                    elif project_type in ("btss", "curator", "audio"):
                        try:
                            if self._project_is_deployable(proj_parser, "prebuilt_image"):
                                xuv_filename = self._find_and_copy_xuv_file(proj_parser, outpath)
                        except (IOError, RuntimeError) as error:
                            print(error)
                            return False, chip_type
                    else:
                        # Not a project_type we are interested in
                        continue

                    if xuv_filename:
                        self._projects[project_type] = xuv_filename
                    elif project_type not in ("btss", "curator", "audio"):
                        print("failed to process %s; exiting" % project_type)
                        return False, chip_type

                else:
                    print("Already have a %s" % project_type)

            sys.stdout.flush()

        self._build_output_folder = build_output_folder
        return True, chip_type

    def collect_all_workspace_projects(self, workspace, inputpath, build_output_folder=None):
        """
        Collect all workspace projects and combine them into a single image.
        """
        # Build and collect the input xuv files from all the projects in the workspace
        ws_projects = Workspace(workspace).parse()
        for project in ws_projects.values():
            proceed, _ = self.process_project(project.filename, inputpath)

            if not proceed:
                # An error has occured
                print("Failed to process %s" % project.filename)
                sys.stdout.flush()
                return False

        sys.stdout.flush()

        return True

    def projects_exist(self):
        """
        Determine if any projects have been processed.
        """
        if len(self._projects) == 0:
            return False

        return True

    def create_flash_image(self, devkit, output_path, encrypted = False,
        hardware_encrypted = False, software_encryption_file = False,
        include_all_flash_sections = True, include_header_section = True):
        """
        Create the configuration for the ImageBuilder and run it.
        """
        from csr.dev.tools.flash_image_builder import ImageBuilder
        flash0 = self.flash_config
        map = {}
        for i in range(len(flash0["layout"])):
            map[flash0["layout"][i][0]] = i

        # Put together the configuration needed by ImageBuilder
        flash_image_xuv = os.path.join(output_path, "flash_image.xuv")
        flash_image_parameters = {
            "flash_image_xuv": flash_image_xuv,
            "bank": "bank0"
        }

        audio_image_xuv = None

        authenticate = None
        if encrypted == True:
            authenticate = True

        xuv_filename = None
        if "curator_config" in self._projects:
            xuv_filename = self._projects["curator_config"]
        flash0["layout"][map["curator_fs"]] = self._add_layout("curator_fs",
            xuv_filename, flash0["layout"][map["curator_fs"]], encrypted)

        xuv_filename = None
        if "apps0" in self._projects:
            xuv_filename = self._projects["apps0"]
        flash0["layout"][map["apps_p0"]] = self._add_layout("apps_p0",
            xuv_filename, flash0["layout"][map["apps_p0"]], encrypted)

        xuv_filename = None
        if "audio" in self._projects:
            xuv_filename = self._projects["audio"]
        if self._generate_audio_image_xuv and xuv_filename:
            # Copy the audio image directly to the output folder.
            source = xuv_filename
            destination = os.path.join(output_path, "flash_audio_image.xuv")
            try:
                shutil.copy(source, destination)
                audio_image_xuv = destination
                print("Copied %s to %s" % (source, destination))
            except IOError as err:
                print("Unable to copy %s to %s: %s" % (source, destination, err))
                return None, None
        try:
            flash0["layout"][map["audio"]] = self._add_layout("audio",
                xuv_filename, flash0["layout"][map["audio"]], False)
        except KeyError:
            pass

        xuv_filename = None
        if "app/p1" in self._projects:
            xuv_filename = self._projects["app/p1"]
        flash0["layout"][map["apps_p1"]] = self._add_layout("apps_p1",
            xuv_filename, flash0["layout"][map["apps_p1"]], encrypted)

        xuv_filename = None
        if "device_config" in self._projects:
            xuv_filename = self._projects["device_config"]
        flash0["layout"][map["device_ro_fs"]] = self._add_layout("device_ro_fs",
            xuv_filename, flash0["layout"][map["device_ro_fs"]], encrypted)

        xuv_filename = None
        if "firmware_config" in self._projects:
            xuv_filename = self._projects["firmware_config"]
        flash0["layout"][map["ro_cfg_fs"]] = self._add_layout("ro_cfg_fs",
            xuv_filename, flash0["layout"][map["ro_cfg_fs"]], encrypted)

        xuv_filename = None
        if "customer_ro" in self._projects:
            xuv_filename = self._projects["customer_ro"]
        flash0["layout"][map["ro_fs"]] = self._add_layout("ro_fs",
            xuv_filename, flash0["layout"][map["ro_fs"]], encrypted)

        xuv_filename = None
        if "user_ps" in self._projects:
            xuv_filename = self._projects["user_ps"]
        flash0["layout"][map["rw_config"]] = self._add_layout("rw_config",
            xuv_filename, flash0["layout"][map["rw_config"]], encrypted)

        flash0["layout"][map["rw_fs"]] = self._add_layout("rw_fs",
            None, flash0["layout"][map["rw_fs"]], None)

        flash0["flash_image"] = flash_image_parameters
        flash0["encrypt"] = encrypted

        import pprint
        pp = pprint.PrettyPrinter(indent=2)
        print("flash0 ",)
        pp.pprint(flash0)
        sys.stdout.flush()

        # Create an instance of ImageBuilder with the required configuration.
        if encrypted == True:
            if hardware_encrypted == True:
                # Need to attach to a device to be able to encrypt from it
                from csr.transport.trbtrans import TrbErrorCouldNotEnumerateDevice
                try:
                    device = self._attach_to_device()
                except (TrbErrorCouldNotEnumerateDevice, NotImplementedError):
                    print("Unable to attach to device to encrypt")
                    sys.stdout.flush()
                    sys.exit(1)
                if device is None:
                    print("Unable to attach to device to encrypt")
                    sys.stdout.flush()
                    sys.exit(1)
                try:
                    builder = ImageBuilder(flash0, device.chip.apps_subsystem.dma,
                        select_bank=flash0["flash_image"]["bank"])
                    device.chip.curator_subsystem.siflash.identify(4,0)
                except:
                    print("Unable to communicate with the device to encrypt")
                    sys.stdout.flush()
                    sys.exit(1)
            else:
                fname = os.path.join(devkit, "tools", "bin", "SecurityCmd.exe")
                if os.path.isfile(fname):
                    host_tools_parameters = { "SecurityCmd": r"%s" % fname }
                    flash0["host_tools"] = host_tools_parameters

                try:
                    builder = ImageBuilder(flash0, file=software_encryption_file,
                        select_bank=flash0["flash_image"]["bank"])
                except KeyError:
                    print("No software encryption file supplied")
                    sys.stdout.flush()
                    sys.exit(1)
        else:
            builder = ImageBuilder(flash0, select_bank=flash0["flash_image"]["bank"])

        builder.build()
        print("Boot Image - offset 0x%x, size 0x%x" % (
            builder.boot_image["offset"], builder.boot_image["size"]))

        print("Image Header:")
        print(builder.image_header)

        PrepareSingleImage.print_flash_usage_table(builder)

        print("Writing '%s'" % flash0["flash_image"]["flash_image_xuv"])
        builder.write_image_section_xuv_files(flash0["flash_image"]["flash_image_xuv"])

        if include_all_flash_sections:
            builder.write_complete_flash_image_xuv(flash0["flash_image"]["flash_image_xuv"])
        else:
            builder.write_image_xuv(flash0["flash_image"]["flash_image_xuv"], include_header=include_header_section)

        if encrypted == True and "audio" in map:
            self._relocate_image(os.path.join(output_path, "flash_image.xuv_audio.xuv"),os.path.join(output_path, "flash_audio.xuv"))

        print("Build completed\n")
        sys.stdout.flush()
        return flash_image_xuv, audio_image_xuv

    @staticmethod
    def print_flash_usage_table(builder):
        """Prints a table with flash usage information for each section in the image

        Arguments:
            builder {ImageBuilder} -- Builder object that has been built already.
                I.e. builder.build() must be called before running this function.
        """

        block_size = builder.config["flash_device"]["block_size"]

        column_width = 14

        header_row = "{:^{w}s} |" * 5
        unit_blocks = "Blocks"
        unit_bytes = "Bytes"
        table_header = [
            header_row.format("Name", "Capacity", "Used", "Unused", "Image size", w=column_width),
            header_row.format("", unit_blocks, unit_blocks, unit_blocks, unit_bytes, w=column_width)
        ]

        data_row = "{:<{w}s} |" + "{:^{w}s} |" * 4
        totals = {
            "capacity": 0,
            "used": 0,
            "unused": 0,
            "size": 0
        }
        table_data = []
        for section in builder.image_header.sections:
            name = section["name"]

            capacity_blocks = section.get("capacity", 0) // block_size
            capacity = "{:2d}".format(capacity_blocks)

            used_blocks = section["size"] // block_size + (section["size"] % block_size > 0)
            used = "{:2d}".format(used_blocks)

            unused_blocks = section.get("unused", 0) // block_size
            unused = "{:2d}".format(unused_blocks)

            size = "{:9d}".format(section["size"])

            table_data.append(data_row.format(name, capacity, used, unused, size, w=column_width))

            totals['capacity'] += capacity_blocks
            totals['used'] += used_blocks
            totals['unused'] += unused_blocks
            totals['size'] += section["size"]

        totals['capacity'] = "{:2d}".format(totals['capacity'])
        totals['used'] = "{:2d}".format(totals['used'])
        totals['unused'] = "{:2d}".format(totals['unused'])
        totals['size'] = "{:9d}".format(totals['size'])

        table_data.append(data_row.format("Totals:",
            totals['capacity'], totals['used'], totals['unused'], totals['size'],
            w=column_width))

        print("Block size: {:d} Bytes\n".format(block_size))
        print("\n".join(table_header + table_data) + "\n")

    def get_prebuilt_image(self, id):
        retval = None
        if id in self._projects:
            retval = self._projects[id]
        return retval

