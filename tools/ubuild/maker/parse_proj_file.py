"""
Copyright (c) 2016 - 2018 Qualcomm Technologies International, Ltd

Provides facilities for convenient extraction of source files and project
properties from a Heracles project file
"""

import collections
import xml.etree.cElementTree as ET
import os.path
import maker.exceptions as bdex
import pprint
import glob
from maker.aliases import Aliases

class Project(object):
    """ Container for all project file infomation """

    def __init__(self, proj_file, devkit_root, workspace_file):

        
        """ parse an XML file and store the root element, file and dir name """
        if not os.path.isfile(proj_file):
            bdex.raise_bd_err('INVALID_PROJECT_FILE', proj_file)
        if not os.path.isfile(workspace_file):
            bdex.raise_bd_err('INVALID_WORKSPACE_FILE', workspace_file)
        if not os.path.isdir(devkit_root):
            bdex.raise_bd_err('INVALID_DEVKIT_ROOT', devkit_root)
        
        self.workspace_root = os.path.abspath(workspace_file)
        self.devkit_root = os.path.abspath(devkit_root)
        self.proj_fobj = None
        self.proj_fname = None


        current_wsroot = self._get_wsroot_from_workspace_file(workspace_file, devkit_root)
        self.aliases = Aliases(proj_file, sdk=devkit_root, wsroot=current_wsroot)
        try:
            self.proj_fname = proj_file.name
            self.proj_fobj = proj_file
        except AttributeError:
            self.proj_fname = proj_file
            self.proj_fobj = open(proj_file)

        basename = os.path.basename(self.proj_fname)
        self.proj_projname = os.path.splitext(basename)[0]
        self.proj_dirname = os.path.abspath(os.path.dirname(self.proj_fname))
        try:
            tree = ET.ElementTree(file=self.proj_fobj)
        except ET.ParseError as excep:
            line, col = excep.position
            file_name = os.path.abspath(proj_file)
            bdex.raise_bd_err('INVALID_XML', file_name, excep.msg, line, col)

        self.tree_root = tree.getroot()
        self.validate_xml()

    def validate_xml(self):
        """
         TODO check the XML is compliant with our form e.g. <property name= for
         Heracles files
        """
        pass

    def get_properties(self):
        """ return the properties derived from the project file and folder
        names """
        properties = collections.defaultdict(str)
        properties['OUTDIR'] = self.proj_dirname
        properties['IDE_PROJECT'] = self.proj_projname
        return properties

    def get_configurations(self):
        """
        Return  a list of all the configuration names found under the 'properties'
        tag.
        """
        config_list = []
        for config in self.tree_root.iterfind('./configurations/configuration'):
            config_list.append(config.attrib['name'])

        return config_list

    def get_elements_from_tree_path(self, tree_path, elem_name, type=""):
        """
        Return a dict of the elem_name elements of a given tree path.
        """
        def get_text(prop_val, prop):
            """
            Ensure elem_name text is never None
            Replace 'sdk://' to the path to the Devkit root folder
            """
            text = ''
            if prop.text != None:
                text = prop.text

            text = text.replace('sdk://',self.devkit_root + '\\')

            return text

        properties = collections.defaultdict(str)
        xpath_elements = "/*"
        xpath = tree_path + xpath_elements

#        for prop in self.tree_root.iterfind(xpath):
#            if prop.tag == elem_name:
#                # Catenate the text of properties with the 'alias' attribute onto the specified
#                # alias elem_name. properties is a defaultdict and creates the specified alias key
#                # if required.
#                # Do not add a new key to the properties dictionary for the alias itself
#                if 'alias' in prop.attrib:
#                    name = prop.attrib['alias']
#                    properties[name] += ' ' + get_text(properties[name], prop)
#                else:
#                    name = prop.attrib['name']
#                    if name in properties:
#                        print 'WARNING: Duplicate %s %s' % (elem_name, name)
#                    properties[name] = ' ' + get_text(properties[name], prop)

        for prop in self.tree_root.iterfind(xpath):
            if prop.tag == elem_name and 'alias' not in prop.attrib:
                name = prop.attrib['name']
                if name in properties:
                    print 'WARNING: Duplicate %s %s' % (elem_name, name)
                properties[name] = get_text(properties[name], prop)
                if type != "":
                    if "type" in prop.attrib:
                        if prop.attrib["type"] != type:
                            properties.pop(name)
                    else:
                        properties.pop(name)

        for prop in self.tree_root.iterfind(xpath):
            if prop.tag == elem_name:
                # Catenate the text of properties with the 'alias' attribute onto the specified
                # alias elem_name. properties is a defaultdict and creates the specified alias key
                # if required.
                # Do not add a new key to the properties dictionary for the alias itself
                if 'alias' in prop.attrib:
                    name = prop.attrib['alias']
                    properties[name] += ' ' + get_text(properties[name], prop)

                properties[name] = properties[name].strip()

        return properties

    def get_attributes_from_config(self, config_name):
        config_list = self.tree_root.iterfind("./configurations/configuration/[@name='%s']" % config_name)
        # config_list for a given config_name should always be 1 element in length
        # config_list is a 'generator', which is iterable but not indexable, so we need to loop once.
        for config in config_list:
            return config.attrib

    def get_properties_from_config(self, config_name):
        return self.get_elements_from_tree_path("./configurations/configuration/[@name='%s']" % config_name, "property")

    def get_capabilities_from_config(self, config_name):
        return self.get_elements_from_tree_path("./configurations/configuration/[@name='%s']" % config_name, "group", "capability")

    def get_cap_props_from_config(self, config_name, capability):
        return self.get_elements_from_tree_path("./configurations/configuration/[@name='%s']/group/[@name='%s']" % (config_name, capability), "property")

    def build_source_path(self, root_el, path):
        """
        Recursively search all 'folder' tags for subordinate 'file' tags and
        retrieve source file paths. xIDE paths are complete, whereas Heracles ones
        use a nested folder structure like a file system and need to be built up into
        a path string.
        All paths have seperators normalised and are prepended with the path parameter .
        """
        file_list = []
        for child in root_el:
            if child.tag == 'file':
                if child.attrib['path'].startswith('root.pri'):
                    sys.exit('ROOT PRI NO SLASHES')
                exp_path =  self.aliases.expand(child.attrib['path'])

                file_path = os.path.join(path, exp_path)
                file_path = os.path.normpath(file_path)
                file_list.append(file_path)
            elif child.tag == 'folder':
                child_paths = self.build_source_path(child, path)
                file_list.extend(child_paths)
            elif child.tag == 'sdkfile':
                file_path = os.path.join(self.devkit_root, child.attrib['path'])
                file_path = os.path.normpath(file_path)
                for f in glob.glob(file_path):
                    file_list.append(f)
            else:
                # Ignore any other kind of tag
                pass

        return file_list


    def get_source_files(self):
        """
        Recursively search all 'folder' tags for subordinate 'file' tags and
        retrieve source file paths. xIDE paths are complete, whereas Heracles ones
        use a nested folder structure like a file system and need to be built up into
        a path string.
        All paths have seperators normalised and are prepended with the path parameter .
        """
        file_list = self.build_source_path(self.tree_root, self.proj_dirname)

        # Check that all the files exist before returning the list
        try:
            for fname in file_list:
                fobj = open(fname)
                fobj.close()
        except IOError as excep:
            print "FAILED SOURCE"
            bdex.raise_bd_err('INVALID_SOURCE_FILE', fname, excep.strerror)

        return file_list

    def get_make_vars(self, config):
        """ merge project properties and config properties and return in a
        single dict """
        proj_props = self.get_properties()
        make_vars = proj_props.copy()

        make_vars['IDE_CONFIG'] = config

        # Configuration properties take priority over project properties
        config_props = self.get_properties_from_config(config)
        make_vars.update(config_props)

        return make_vars


    def get_workspace_path(self):
        """
        Find the workspace element in the file and return the path attribute,
        raising an error if this doesn't point at a real file relative to the
        project file's location.
        """
        workspace_el = self.tree_root.find("workspace")
        if workspace_el is None:
            print "No workspace element in project file"
            return None
        if "path" not in workspace_el.attrib:
            bdex.raise_bd_err("INVALID WORKSPACE ELEMENT")

        ws_path = os.path.normpath(os.path.join(self.proj_dirname,
                                                workspace_el.attrib["path"]))
        if not os.path.isfile(ws_path):
            bdex.raise_bd_err("INVALID WORKSPACE FILE")

        return ws_path

    def _get_wsroot_from_workspace_file(self, workspace_file, devkit_root):
        aliases = Aliases(workspace_file, devkit_root)
        return aliases["wsroot://"]      

    
