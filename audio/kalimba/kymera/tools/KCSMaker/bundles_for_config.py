#!/usr/bin/env python

import argparse
import json
import logging
import re
import sys


# -----------------------------------------------------------------------------
def main():
    """
    Generates the list of bundles needed for the specified build configuration.
    The list is ordered to cope with any bundle dependencies.
    """

    args = parse_arguments()
    show_parameters(args)

    bundleDefinitions = get_bundle_definitions(args.bfile)
    show_bundle_definitions(bundleDefinitions)

    check_definition_integrity(bundleDefinitions)

    bundlesForConfig = determine_bundles_for_config(args.config, bundleDefinitions)
    show_bundles_for_config(args.config, bundlesForConfig)

    output_result(bundlesForConfig, args.ofile)

    sys.exit(0)


# -----------------------------------------------------------------------------
def parse_arguments():
    """
    Parse command line arguments.
    -c/--config and -b/--bundle_file are mandatory.
    """
    
    parser = argparse.ArgumentParser(description='Generate ordered list of bundles required by the specified build configuration')

    parser.add_argument( '-c', '--config',
         help='Build configuration for which bundle generation is required',
         action='store',
         dest='config',
         required=True,
    )

    parser.add_argument( '-b', '--bundle_file',
         help='Bundle definition file to be used - expected to be JSON format',
         action='store',
         dest='bfile',
         default='bundle_list.json',
         required=True,
    )

    parser.add_argument( '-o', '--output_file',
         help='Output to be written to this file',
         action='store',
         dest='ofile',
    )

    parser.add_argument( '-l', '--logging',
        help='Increasing levels of verbosity for logging (default is WARNING)',
        choices=('INFO', 'WARNING', 'ERROR', 'CRITICAL'),
        default='WARNING',
    )

    return parser.parse_args()


# ----------------------------------------------------------------------------
def get_bundle_definitions(bundleFile):
    """
    The JSON file holds the definitions for all of the available bundles.
    Each definition contains :
       * Bundle name
       * List of unsupported build configs for this bundle - full names or 'grep' pattern matches
       * List of unsupported build variants for this bundle - full names or 'grep' pattern matches
       * List of bundles that this bundle depends on - used to determine required order of bundle generation
    The only mandatory entry is the bundle name, all the lists may be empty.
    The unsupported build variants list is not relevant to this program and is ignored.
    """

    try:
        with open(bundleFile) as fh_json:
            bundleDefinitions = json.load(fh_json, object_pairs_hook=check_for_duplicate_key)

    except IOError as err:
        logging.error("Failed to access JSON file: '{0}'".format(bundleFile))
        sys.exit(1)

    except ValueError as err:
        logging.error("{0}".format(err))
        logging.error("JSON file '{0}' has syntax errors".format(bundleFile))
        sys.exit(1)

    return bundleDefinitions


# ----------------------------------------------------------------------------
def check_for_duplicate_key(pairs):
    """
    Hook used by JSON parser.
    Called whenever a set of key/value pairs has been parsed.
    A check is made for duplicate key(s) and an error is raised if any are found.
    All duplicate keys are reported, not just the first one.
    """

    duplicates = []

    dict = {}
    for key, value in pairs:
        if key in dict:
           duplicates.append(key)
        else:
           dict[key] = value

    if duplicates:
        raise ValueError("Duplicate key:\n    {0}".format('\n    '.join(str(x) for x in duplicates)))

    return dict


# ----------------------------------------------------------------------------
def check_definition_integrity(bundleDefinitions):
    """
    Integrity checks on the bundle defintion :
      * ensure that all the bundles referenced in 'bundle_dependency' lists are defined.
      * ensure there are no circular dependencies.
    NOTE: duplicate key checks are done by the JSON file loader.
    """

    undeclared = find_undeclared_bundles(bundleDefinitions)
    if undeclared:
        logging.error('\n{0}\nUnreferenced dependencies found :\n    {1}\n    '.format('-'*60, '\n    '.join(str(x) for x in undeclared)))

    circularDependencies = find_circular_dependencies(bundleDefinitions, undeclared)
    if circularDependencies:
        logging.error('\n{0}\nCircular dependencies found :'.format('-'*60))
        for circDep in circularDependencies:
            logging.error('    {0}'.format(' => '.join(str(x) for x in circDep[:-1])))

    if undeclared or circularDependencies:
        sys.exit(1)


# ----------------------------------------------------------------------------
def find_undeclared_bundles(bundleDefinitions):
    """
    Identify any bundles listed in the 'bundle_dependency' section which do not have definitons.
    """

    logging.info("\n#{0}\nChecking for undeclared dependencies".format('-'*60))

    undeclared = []

    for bundle in bundleDefinitions:
        logging.info('    {0}'.format(bundle))
        for dependentBundle in bundleDefinitions[bundle]['bundle_dependency']:
            if not dependentBundle in bundleDefinitions:
                undeclared.append(dependentBundle)

    return undeclared


# ----------------------------------------------------------------------------
def find_circular_dependencies(bundleDefinitions, undeclared):
    """
    Identify any circular dependencies by constructing the dependency paths.
    Any item added to a path must not alread be in the path.
    """

    logging.info("\n#{0}\nChecking for circular dependencies".format('-'*60))

    circularDependencies = []

    for bundle in sorted(bundleDefinitions):

        # Don't continue with check if the bundle is already part of a circular dependency
        if not bundle_already_in_circ_dep(bundle, circularDependencies) :
            logging.info('    {0}'.format(bundle))
            dependencyPath = [bundle]

            for dependentBundle in bundleDefinitions[bundle]['bundle_dependency']:
                if dependentBundle not in undeclared :
                    add_dependency(dependentBundle, dependencyPath, bundleDefinitions, undeclared, circularDependencies)
                    if "_CIRC_DEP_" in dependencyPath:
                        break
                    dependencyPath.remove(dependentBundle)

    return circularDependencies


# ----------------------------------------------------------------------------
def bundle_already_in_circ_dep(bundle, circularDependencies):
    """
    Checks whether a bundle is part of an already discovered circular dependency.
    If it is then there is no need to check again!!
    """

    for circDep in circularDependencies:
        if bundle in circDep:
            return True

    return False


# ----------------------------------------------------------------------------
def add_dependency(bundle, dependencyPath, bundleDefinitions, undeclared, circularDependencies):
    """
    Build up the dependency path.
    A circular dependency occurs if the bundle to be added is already in the path.
    Any undeclared dependencies.wil be ignored.
    """

    if bundle in dependencyPath:
        dependencyPath.append(bundle)                   # Add the path, so circular dependency report shows full loop
        dependencyPath.append("_STOP_")                 # Add a marker - to force a break out of the loop
        circularDependencies.append(dependencyPath)

    else:
        dependencyPath.append(bundle)
        logging.info('        {0}'.format(' => '.join(str(x) for x in dependencyPath)))
        if bundleDefinitions[bundle]['bundle_dependency']:
            for dependentBundle in bundleDefinitions[bundle]['bundle_dependency']:
                if dependentBundle not in undeclared :
                    add_dependency(dependentBundle, dependencyPath, bundleDefinitions, undeclared, circularDependencies)
                    if "_STOP_" in dependencyPath:      # Use the marker - to break out of the loop
                        break
                    dependencyPath.remove(dependentBundle)


# ----------------------------------------------------------------------------
def determine_bundles_for_config(buildConfig, bundleDefinitions):
    """
    Generate the list of bundles to be generated for the specified build configuration.
    Only the bundles supported by the requested build configuration are added.to the list.
    The list is in the correct order to cope with bundle dependencies.
    """

    logging.info("\n#{0}\nDetermining bundle order for {1}".format('-'*60, buildConfig))

    bundlesForConfig = []
    for bundle in sorted(bundleDefinitions):
        bundleReqdForConfig = True
        for unsupported in bundleDefinitions[bundle]['not_supported_configs']:
            if re.match(unsupported, buildConfig):
                bundleReqdForConfig = False
                break

        if bundleReqdForConfig:
            recurseDepth = 1
            insert_bundle_in_list(bundle, bundlesForConfig, bundleDefinitions, recurseDepth)

    return bundlesForConfig


# ----------------------------------------------------------------------------
def insert_bundle_in_list (bundle, bundlesForConfig, bundleDefinitions, recurseDepth):
    """
    Insert the target bundle into the list in the correct place to allow for bundle dependecies.
    This is a recursive function which will consider any dependencies that the target bundle has.
    The recurseDepth is used for output formatting only (only seen when logging set to INFO)
    The '.bdl' suffix is stripped off when adding to the list.
    """

    logging.info('{0}Processing bundle : {1}'.format('    '*recurseDepth, bundle))

    bundleNoSuffix = re.sub('\.bdl$', '', bundle)
    if not bundleNoSuffix in bundlesForConfig:

        if len(bundleDefinitions[bundle]['bundle_dependency']):
            logging.info('{0}    Dependencies : {1}'.format('    '*recurseDepth, ', '.join(str(x) for x in bundleDefinitions[bundle]['bundle_dependency'])))
            for dependentBundle in bundleDefinitions[bundle]['bundle_dependency']:
                insert_bundle_in_list(dependentBundle, bundlesForConfig, bundleDefinitions, recurseDepth + 1)
            bundlesForConfig.append(bundleNoSuffix)
        else:
            bundlesForConfig.append(bundleNoSuffix)
        logging.info('{0}    {1} appended to list'.format('    '*recurseDepth, bundleNoSuffix))

    else:
        logging.info('{0}    {1} already in list'.format('    '*recurseDepth, bundleNoSuffix))


# ----------------------------------------------------------------------------
def output_result(bundlesForConfig, ofile):
    """
    Output the ordered list of bundles for the requested build configuration.
    This is written to stdout, or to the specified file.
    """

    result = ('{0}\n'.format('\n'.join(str(x) for x in bundlesForConfig)))

    if ofile:
        try:
            with open(ofile, "w") as outfile:
                outfile.write(result)

        except IOError as err:
            logging.error("{0}".format(err))
            sys.exit(1)

    else:
        print(result)


# ----------------------------------------------------------------------------
def show_parameters(args):
    """
    Show supplied command line parameters
    """

    logging.basicConfig(format='%(message)s', level=args.logging)

    logging.info('\n#{0}'.format('-'*60))
    logging.info('BUILD CONFIG : {0}'.format(args.config))
    logging.info('BUNDLE FILE  : {0}'.format(args.bfile))


# ----------------------------------------------------------------------------
def show_bundle_definitions(bundleDefinitions):
    """
    Show the bundle definitions that have been read from the JSON file.
    """

    logging.info("\n#{0}\nBundle defintions read from JSON file".format('-'*60))
    for bundle in sorted(bundleDefinitions):
        logging.info("    BUNDLE : {0}".format(bundle))

        for item in bundleDefinitions[bundle]['not_supported_configs']:
            logging.info("      UNSUPPORTED CONFIGS  : {0}".format(item))

        for item in bundleDefinitions[bundle]['not_supported_variants']:
            logging.info("      UNSUPPORTED VARIANTS : {0}".format(item))

        for item in bundleDefinitions[bundle]['bundle_dependency']:
            logging.info("      DEPENDS ON           : {0}".format(item))


# ----------------------------------------------------------------------------
def show_bundles_for_config(buildConfig, bundlesForConfig):
    """
    Show the list of the bundles needed for the specified build configuration.
    The bundles should be generated in the order shown.
    """

    logging.info("\n#{0}\nOrdered list of bundles to be generated for {1}".format('-'*60, buildConfig))
    logging.info('    {0}\n'.format( '\n    '.join(str(x) for x in bundlesForConfig)))


# -----------------------------------------------------------------------------
if __name__ == '__main__':
    main()
