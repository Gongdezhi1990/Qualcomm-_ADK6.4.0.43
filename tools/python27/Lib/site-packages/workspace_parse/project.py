"""
Copyright (c) 2018 Qualcomm Technologies International, Ltd.
"""

import os
from .parse_project import ParseProject
from .configuration import Configuration


class Project(object):
    """ Representation of a Project. Iterates over configurations
    """
    def __init__(self, name, project_file, sdk=None, wsroot=None):
        self.name = name
        self.filename = project_file
        self.dirname = os.path.dirname(project_file)
        self.dependencies = []
        parsed_project = ParseProject(self.filename, sdk=sdk, wsroot=wsroot)
        self.files = parsed_project.files
        self.configurations = parsed_project.configurations
        self.default_configuration = self._get_default_config()

    def is_deployable(self):
        return self.default_configuration.is_deployable()

    def _get_default_config(self):
        default_config = Configuration()
        for config in self.configurations.values():
            if config.is_default() or len(self.configurations) == 1:
                default_config = config
                break
        return default_config

    def __repr__(self):
        info = ("Project: {!s} \n"
                "Default config: {!s}\n"
                "Configs: {!s}"
                .format(self.name, self.default_configuration.name, ', '.join(self.configurations)))
        return info

    def __str__(self):
        return "Project: %s : %s" % (self.name, self.filename)

    def __iter__(self):
        return iter(self.configurations)

    def __getitem__(self, config):
        return self.configurations[config]

    def __len__(self):
        return len(self.configurations)
