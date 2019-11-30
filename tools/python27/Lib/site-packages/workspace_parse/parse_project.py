"""
Copyright (c) 2018 Qualcomm Technologies International, Ltd.
"""

import os
from .configuration import Configuration
from .exceptions import InvalidProjectElement
import parsers
try:
    from maker.aliases import Aliases
except ImportError:
    from .aliasesNull import Aliases


# xml elements and attributes
file_xpath = "file"
configurations_xpath = "./configurations"
configuration_xpath = "configuration"
path_attr_str = "path"


class ParseProject(object):
    """ Parse an x2p project
    """
    def __init__(self, project_file, sdk=None, wsroot=None):
        self.xml = parsers.get_xml(project_file)
        self.aliases = Aliases(project_file, sdk=sdk, wsroot=wsroot)
        self.xml_tree = self.xml.parse()
        self.files = self._parse_files()
        self.configurations = self._parse_configurations()

    def _parse_files(self):
        files = []
        for file_element in self.xml_tree.iter(file_xpath):
            file_path = file_element.get(path_attr_str)
            file_path = self.aliases.expand(file_path)
            if not os.path.isabs(file_path):
                file_path = os.path.normpath(os.path.join(self.xml.base_dir, file_path))
            if not os.path.isfile(file_path):
                raise InvalidProjectElement("File listed in project {} doesn't exist: {}".format(self.xml.filename, file_path))
            files.append(file_path)
        return files

    def _parse_configurations(self):
        configurations_elements = self.xml_tree.findall(configurations_xpath)
        if len(configurations_elements) > 1:
            raise InvalidProjectElement("Multiple <configurations> elements in {}".format(self.xml.filename))
        elif len(configurations_elements) < 1:
            raise InvalidProjectElement("No <configurations> elements found in {}".format(self.xml.filename))

        configurations = {}
        for config in configurations_elements[0].findall(configuration_xpath):
                configuration = Configuration(config)
                configurations[configuration.name] = configuration
        return configurations

    def parse(self):
        return self.files, self.configurations
