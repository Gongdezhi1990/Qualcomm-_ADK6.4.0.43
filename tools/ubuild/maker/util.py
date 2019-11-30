"""
Copyright (c) 2016 - 2018 Qualcomm Technologies International, Ltd

ubuild utility functions.
"""

import os
from xml.etree import ElementTree

import maker.mergexml as mxml

MERGED_SDK_XML_NAME = 'merge_ubuild.chk'

def get_sdk_xml_file(devkit_path):
    """
    Search for a merged sdk.xml file in the root of the Devkit at devkit_path.
    If nothing found run the merge
    """
    sdk_xml_path = os.path.join(devkit_path, MERGED_SDK_XML_NAME)

    if os.path.isfile(sdk_xml_path):
        return os.path.abspath(sdk_xml_path)

    mxml.merge(devkit_path, sdk_xml_path)
    return os.path.abspath(sdk_xml_path)

def get_fsprefix(devkit):
    """
    Given a devkit will return the value of fsprefix within sdk.xml
    """
    devkit_config_xml = get_sdk_xml_file(devkit)
    devkit_config_tree = ElementTree.parse(devkit_config_xml)
    devkit_config_root = devkit_config_tree.getroot()

    for fsprefix in devkit_config_root.iter('fsprefix'):
        return fsprefix.text

    return None
