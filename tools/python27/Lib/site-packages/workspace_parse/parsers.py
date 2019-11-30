"""
Copyright (c) 2018 Qualcomm Technologies International, Ltd.
"""

import os
import xml.etree.ElementTree as ET
from .exceptions import ParseError

def get_xml(filename):
    xml = None
    if isinstance(filename, (XmlFileName, XmlString)):
        xml = filename
    elif os.path.isfile(filename):
        xml = XmlFileName(filename)
    elif isinstance(filename, str):
        xml = XmlString(filename)
    else:
        raise ParseError("Unsupported project file: {}".format(filename))
    return xml


class XmlFileName(object):
    """ Parse an xml file given it's path
    """
    def __init__(self, filename):
        self.filename = os.path.abspath(filename)
        self.base_dir = os.path.dirname(self.filename)

    def __repr__(self):
        return self.filename

    def __call__(self):
        return self.filename

    def parse(self):
        try:
            return ET.parse(self.filename)
        except ET.ParseError as e:
            msg = "ERROR parsing: {}".format(self.filename)
            raise ParseError("{}\n{}".format(e, msg))

    def parse_sub_elements(self):
        return True


class XmlString(object):
    """ Parse an xml string
    """
    def __init__(self, xml):
        self.xml = xml
        self.base_dir = os.getcwd()

    def __repr__(self):
        return "String Representation"

    def __call__(self):
        return ("String Representation")

    def parse(self):
        return ET.fromstring(self.xml)

    def parse_sub_elements(self):
        return False

