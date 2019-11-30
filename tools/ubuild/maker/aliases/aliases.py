"""
Copyright (c) 2018 Qualcomm Technologies International, Ltd.
"""

import os
import re
import xml.etree.cElementTree as ET
import copy
import warnings

from .exceptions import *

STOP = "://"
SDK_PREFIX = 'sdk' + STOP
WSROOT_PREFIX = 'wsroot' + STOP
PATTERN = re.compile(r'(\w+' + STOP + r')')
ALIAS_XPATH = ".//alias"
INCLUDE_XPATH = ".//include"
WORKSPACE_EXTENSION = '.x2w'
WSROOT_EXTENSION = '.wsroot'


class Aliases(object):
    """ Handle aliases from MDE projects and workspaces
    """
    def __init__(self, xml_filename, sdk=None, wsroot=None):
        """ Reads an x2w or x2p file, extracts the aliases definitions and
            constructs an Aliases object

        Arguments:
            xml_filename {string} -- Path to the x2w or x2p file to load

        Keyword Arguments:
            sdk {string} -- Path to the SDK installation (default: {os.environ['ADK_ROOT']})
            wsroot {string} -- Path to the wsroot:// location (default: {None})
        """
        self.xml_filename = xml_filename
        self.processed_files = set([])
        self.alias_dict = {
            SDK_PREFIX: self._get_sdk(sdk),
            WSROOT_PREFIX: self._get_wsroot(wsroot)
        }
        self._update_aliases(xml_filename)
        self._check_undefined_aliases()
        self._normalize_aliases()

    def expand(self, path):
        """ Expand aliases in the specified path

        Arguments:
            path {str} -- Path with aliases

        Returns:
            str -- Path with aliases expanded and normalized
        """
        return os.path.normpath(self.__expand(path))

    def __expand(self, path):
        try:
            found_aliases = PATTERN.search(path).groups()
        except AttributeError:
            return path
        for alias in found_aliases:
            expanded_alias = self.alias_dict.get(alias, alias)
            expanded_alias = expanded_alias if expanded_alias.endswith(('\\', '/')) else expanded_alias + os.path.sep
            path = path.replace(alias, expanded_alias)
        return path

    def expand_all(self, paths_with_aliases):
        """ Expand aliases in the specified paths

        Arguments:
            paths_with_aliases {dict/list/tuple} -- Object containing paths

        Returns:
            dict/list/tuple -- Returns a new object of the same type as the input
                               with aliases expanded in each path and normalized
        """
        return {key: os.path.normpath(value) for (key, value) in self.__expand_all(paths_with_aliases).items()}

    def __expand_all(self, paths_with_aliases):
        return {key: self.__expand(paths_with_aliases[key]) for key in self._seq_iter(paths_with_aliases)}

    @property
    def wsroot(self):
        return self.alias_dict[WSROOT_PREFIX]

    @property
    def sdk(self):
        return self.alias_dict[SDK_PREFIX]

    def _seq_iter(self, obj):
        return obj if isinstance(obj, dict) else range(len(obj))

    def _get_sdk(self, sdk):
        if not sdk:
            sdk = os.environ.get('ADK_ROOT', '')
            if not sdk:
                raise AliasUndefinedError(SDK_PREFIX)
        return sdk

    def _get_wsroot(self, wsroot):
        if not wsroot:
            if self._is_workspace():
                try:
                    wsroot_file = os.path.splitext(self.xml_filename)[0] + WSROOT_EXTENSION
                    self._add_to_processed_files(wsroot_file)
                    wsroot_dict = self._get_direct_children(ET.parse(wsroot_file))
                    wsroot = wsroot_dict[WSROOT_PREFIX]
                    wsroot = self._make_path_absolute(wsroot, wsroot_file)
                    if len(wsroot_dict) > 1:
                        warnings.warn("Additional entries found in {} ignored"
                                      .format(wsroot_file), AliasWarning)
                except (IOError, KeyError):
                    wsroot = os.path.dirname(self.xml_filename)
            else:
                raise AliasUndefinedError(WSROOT_PREFIX)
        return os.path.normpath(wsroot)

    def _normalize_aliases(self):
        for key, value in self.alias_dict.items():
            self.alias_dict[key] = os.path.normpath(value)

    def _make_path_absolute(self, path, anchor_file):
        if not os.path.isabs(path):
            return os.path.join(os.path.dirname(anchor_file), path)
        return path

    def _is_workspace(self):
        return self.xml_filename.endswith(WORKSPACE_EXTENSION)

    def _update_aliases(self, xml_filename):
        self._add_to_processed_files(xml_filename)
        xml_tree = ET.parse(xml_filename)

        aliases = self._get_direct_children(xml_tree)
        self._check_all_duplicates(aliases, self.alias_dict)
        self.alias_dict.update(aliases)
        self.alias_dict = self.__expand_all(self.alias_dict)

        for i in xml_tree.findall(INCLUDE_XPATH):
            include_path = self.__expand(i.attrib['src'])
            include_path = self._make_path_absolute(include_path, xml_filename)
            self._update_aliases(include_path)

    def _add_to_processed_files(self, filename):
        if filename in self.processed_files:
            raise AliasCyclicInclusionError(filename)
        self.processed_files.add(filename)

    def _get_direct_children(self, aliases_root):
        aliases = {}
        for a in aliases_root.findall(ALIAS_XPATH):
            key = a.attrib['name'] + STOP
            value = a.text
            self._check_duplicate(key, value, aliases)
            aliases[key] = value
        return aliases

    def _check_all_duplicates(self, aliases_a, aliases_b):
        for key in aliases_a:
            if key in aliases_b:
                values = (aliases_b[key], aliases_a[key])
                raise AliasDuplicateError(key, values=values)

    def _check_duplicate(self, key, value, aliases):
        if key in aliases:
            raise AliasDuplicateError(key, values=(aliases[key], value))

    def _check_undefined_aliases(self):
        for expanded in self.alias_dict.values():
            m = PATTERN.search(expanded)
            if m:
                raise AliasUndefinedError(m.group(1))

    def __iter__(self):
        return iter(self.alias_dict)

    def __getitem__(self, key):
        return self.alias_dict[key]

    def __len__(self):
        return len(self.alias_dict)
