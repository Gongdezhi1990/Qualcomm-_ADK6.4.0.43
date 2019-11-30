"""
Copyright (c) 2016 - 2018 Qualcomm Technologies International, Ltd 

Provides facilities for convenient extraction of devkit properties
"""
import xml.etree.cElementTree as ET
import os.path
import maker.exceptions as bdex

VERSION_STRING = "1.0"

class Devkit(object):
    """ Contains contents of the Devkit file usually named csr.xml """

    def __init__(self, devkit_file):
        """ parse an XML file and store the root element, file and dir name
            devkit_file may be a string or a file object """
        if not os.path.isfile(devkit_file):
            bdex.raise_bd_err('INVALID_DEVKIT', devkit_file)
        self.devkit_file = None
        self.devkit_fname = None
        try:
            self.devkit_fname = devkit_file.name
            self.devkit_file = devkit_file
        except AttributeError:
            self.devkit_fname = devkit_file
            self.devkit_file = open(devkit_file)

        self.devkit_dirname = os.path.dirname(self.devkit_fname)
        tree = ET.ElementTree(file=self.devkit_file)
        self.tree_root = tree.getroot()

    def get_identity(self):
        """ Return a dictionary of all the devkit identity """
        identity = {}

        for id_el in self.tree_root.iterfind('./id/*'):
            if id_el.tag == 'compatibility':
                comp_list = []
                for comp_el in self.tree_root.iterfind('./id/compatibility/*'):
                    comp_list.append(comp_el.text)
                identity['compatibility_list'] = comp_list
            else:
                identity[id_el.tag] = id_el.text

        identity['URI'] = 'kitres://' + identity['uuid']

        return identity

    def get_info(self):
        """ Return a dictionary of all the devkit info """
        info = {}

        for info_el in self.tree_root.iterfind('./info/*'):
            info[info_el.tag] = info_el.text

        return info


    def get_top_elements(self):
        """ return a list of the top-level elements in a devkit """
        elements = {}

        for plat_el in self.tree_root.iterfind('./*'):
            elements[plat_el.tag] = True
        return elements

    def get_plugins(self):
        """ return a list of the available plugins """
        plugins = {}

        for plugin_el in self.tree_root.iterfind('./plugins/debug/plugin'):
            a_plugin = {}
            plugin_name = plugin_el.attrib['name']
            a_plugin['device'] = plugin_el.find('./device').text

            # Core Groups
            core_groups = {}
            for group_el in plugin_el.iterfind('./group'):
                a_group = {}
                group_name = group_el.attrib['name']
                # Core elements
                for core_el in group_el.iterfind('./core'):
                    core_name = core_el.attrib['name']
                    a_group[core_name] = core_el.text
                core_groups[group_name] = a_group

            a_plugin['core_groups'] = core_groups

            plugins[plugin_name] = a_plugin
            print '===================='

        return plugins

    def get_toolchain(self):
        """ return contents of the toolchain """
        a_toolchain = {}

        for tc_el in self.tree_root.iterfind('./toolchain/*'):
            if tc_el.tag != 'property':
                a_toolchain[tc_el.tag] = tc_el.text

        properties = {}
        for prop_el in self.tree_root.iterfind('./toolchain/property'):
            a_property = {}
            property_name = prop_el.attrib['name']
            a_property['description'] = prop_el.find('./description').text
            a_property['argument'] = prop_el.find('./argument').text
            a_property['default'] = prop_el.find('./default').text
            properties[property_name] = a_property
        a_toolchain['properties'] = properties

        return a_toolchain

    def get_commands(self):
        """ return a list of available commands """
        commands = {}

        for command in self.tree_root.iterfind('./commands/command'):
            a_command = {}
            command_name = command.attrib['name']
            a_command['run'] = command.find('./run').text
            a_command['description'] = command.find('./description').text
            options = {}
            for option in command.findall('./options/option'):
                option_name = option.attrib['name']
                options[option_name] = option.text

            commands[command_name] = a_command

        return commands

    def get_configurations(self):
        """ return a list of the available configurations """
        configurations = {}

        for config in self.tree_root.iterfind('./configurations/configuration'):
            config_elements = {}
            properties = {}
            for prop in config.iterfind('./property'):
                property = {}
                for prop_el in prop.iterfind('./*'):
                    property[prop_el.tag] = prop_el.text
                properties[prop.attrib['name']] = property
            config_elements['properties'] = properties
            configurations[config.attrib['name']] = config_elements
            print '===================='

        return configurations
