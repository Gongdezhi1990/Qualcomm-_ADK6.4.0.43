"""
Copyright (c) 2016 - 2018 Qualcomm Technologies International, Ltd

Merge multiple ext.xml files into sdk.xml and write out the result
"""
import xml.etree.ElementTree as ET
import xml.dom.minidom as MD
import argparse
import StringIO
import os
import sys
import logging

import exceptions as bdex

################################################################################
# Configure the logging regime
################################################################################

class InfoFilter(logging.Filter):
    """
    Reject all loog records apart from DEBUG and INFO
    """
    def filter(self, record):
        return record.levelno == logging.INFO

class DebugFilter(logging.Filter):
    """
    Reject all loog records apart from DEBUG and INFO
    """
    def filter(self, record):
        return record.levelno == logging.DEBUG

logger = logging.getLogger()

h_stdout_info = logging.StreamHandler(sys.stdout)
h_stdout_info.addFilter(InfoFilter())
info_formatter = logging.Formatter('%(message)s')
h_stdout_info.setFormatter(info_formatter)
h_stdout_info.setLevel(logging.INFO)
logger.addHandler(h_stdout_info)

h_stdout_debug = logging.StreamHandler(sys.stdout)
h_stdout_debug.addFilter(DebugFilter())
debug_formatter = logging.Formatter('%(levelname)s: %(message)s')
h_stdout_debug.setFormatter(debug_formatter)
h_stdout_debug.setLevel(logging.DEBUG)
logger.addHandler(h_stdout_debug)

h_stderr = logging.StreamHandler()
error_formatter = logging.Formatter('%(levelname)s: %(message)s')
h_stderr.setFormatter(error_formatter)
h_stderr.setLevel(logging.WARNING)
logger.addHandler(h_stderr)


logger.setLevel(logging.INFO)

################################################################################


def ShallowEqual(src, dst):
    """
    Check equality of this element's tag and attributes.
    Do not recurse into any child elements
    """
    logger.debug('ShallowEqual: SRC %s, <%s>, %s', src, src.tag, src.attrib.items())
    logger.debug('ShallowEqual: DST %s, <%s>, %s', dst, dst.tag, dst.attrib.items())

    if (src.tag == dst.tag
            and sorted(src.attrib.items()) == sorted(dst.attrib.items())):
        # It's a match!
        logger.debug('  ShallowEqual MATCH')
        return True

    logger.debug('  ShallowEqual FAIL')
    return False

def dump_child_elements(in_src, in_dst):
    """
    Log the children of a src and dest pair of elements
    """
    if len(in_src) > 0:
        logger.debug('  inSRC Children')
        for child in list(in_src):
            logger.debug('    %s: %s', child, child.attrib)
    if len(in_dst) > 0:
        logger.debug('  inDST Children')
        for child in list(in_dst):
            logger.debug('    %s: %s', child, child.attrib)

def merge_elements(in_dst, in_src, depth=0):
    """
    Merge an element from the source tree into the destination
    """
    logger.debug('+'*80)
    logger.debug('Enter merge %s +++++ DST: %s, SRC: %s', depth, in_dst.tag, in_src.tag)
    dump_child_elements(in_src, in_dst)

    for index, src_el in enumerate(list(in_src)):
        logger.debug('L[%s] I[%s]: src_el %s', depth, index, src_el)
        do_move = False
        dst_el = None
        # Look for an equivalent name in the desintation
        if (('type' in src_el.attrib) and
                (src_el.attrib['type'] == 'move')):
            # Remove the type attrib so element comparisions work correctly
            # i.e. src & dest elements have same attribs
            src_el.attrib.pop('type')
            do_move = True

        if do_move:
            logger.debug('M+++MOVE-START')
            for dst_el in list(in_dst):
                logger.debug('dst_el %s', dst_el)
                # A src element is only a match if the tag and attributes all match
                if ShallowEqual(src_el, dst_el):
                    logger.debug('src_el %s  dst_el %s', src_el, dst_el)
                    break
                else:
                    #Force a copy as there is no match in the destination
                    logger.debug('force a copy src_el %s', src_el)
                    dst_el = None

        if dst_el is None:
            # Copy this node and add to dst at current level
            # Always add at the end, incase there's an equivalent named node
            dst_el = ET.SubElement(in_dst, src_el.tag, attrib=dict(src_el.items()))
            dst_el.text = src_el.text
            logger.debug('  COPY: SRC %s, DST %s', src_el, dst_el)

        if dst_el is not None and src_el is not None:
            merge_elements(dst_el, src_el, depth=depth+1)
    logger.debug('Exit merge %s ^^^^^^^^^ DST: %s, SRC: %s', depth, in_dst.tag, in_src.tag)
    logger.debug('-'*80)

def write_clean_tree(xml_tree, output_file):
    """
    Clean up the tree and pretty print it
    """
    #Write the tree out to a StrinIO buffer
    output = StringIO.StringIO()
    xml_tree.write(output, encoding='utf-8', xml_declaration=True)
    xml_raw = output.getvalue()

    #Split and clean the lines
    xml_lines = xml_raw.splitlines()
    stripped_lines = [line.strip(' \n\r\t') for line in xml_lines]
    xml_string = ''.join([line for line in stripped_lines if len(line) > 0])

    # Parse it with minidom to get something pretty-printable
    xml = MD.parseString(xml_string)
    pretty_xml_as_string = xml.toprettyxml(indent='    ', encoding='utf-8')

    #Write the pretty xml to the output file
    with open(output_file, 'w') as hfile:
        hfile.write(pretty_xml_as_string)

def find_xml_files(root):
    """
    Search recursively from the root folder for files that match:
    'sdk.xml', 'dk.xml', ext.xml'
    """
    extension_xml_files = []

    sdk_xml_file = os.path.join(root, 'sdk.xml')
    extensions_root = os.path.join(root, 'extensions')

    if not os.path.exists(sdk_xml_file):
        bdex.raise_bd_err('INVALID_DEVKIT', root)

    for path, dirs, files in os.walk(extensions_root):
        for a_file in files:
            if(os.path.basename(a_file) == 'ext.xml' or
               os.path.basename(a_file) == 'dk.xml'):
                extension_xml_files.append(os.path.join(path, a_file))
            else:
                pass

    return sdk_xml_file, extension_xml_files

def dump_file_paths(dkroot, sdk_xml_file, extension_file_list):
    """
    print list of xml files for user
    """
    logger.debug('Merging the following XML files...')
    logger.debug('SDK.XML: %s', sdk_xml_file)

    for fname in extension_file_list:
        frel_path = os.path.relpath(fname, start=dkroot)
        logger.debug('  %s', frel_path)

def parse_args(args):
    """ parse the command line arguments """
    parser = argparse.ArgumentParser()

    parser.add_argument('root', help='Root of tree to search for XML files')

    parser.add_argument('out_file', help='Merged ouput file path')

    parser.add_argument('-d', '--debug', action='store_true', help='turn on debugging ')

    processed_args = parser.parse_args(args)

    if not os.path.isdir(processed_args.root):
        err_msg = "root is not a readable dir {}".format(processed_args.root)
        raise argparse.ArgumentTypeError(err_msg)
    return processed_args

def merge(dk_root, out_file):
    """
    Main entry point
    """

    main_file, extension_files = find_xml_files(dk_root)
    print('    main {}'.format(main_file))
    for ff in extension_files:
        print('ext_file {}'.format(ff))

    dump_file_paths(dk_root, main_file, extension_files)

    sdktree = ET.parse(main_file)
    sdk_root = sdktree.getroot()

    for ex in extension_files:
        logger.debug('Merging {}\nInto {}'.format(ex, main_file))
        exttree = ET.parse(ex)
        ext_root = exttree.getroot()
        merge_elements(sdk_root, ext_root)

    write_clean_tree(sdktree, out_file)
    logger.info('Merged file %s', out_file)

if __name__ == '__main__':
    parsed_args = parse_args(sys.argv[1:])

    if parsed_args.debug:
        logger.setLevel(logging.DEBUG)
    else:
        logger.setLevel(logging.INFO)

    merge(parsed_args.root, parsed_args.out_file)

