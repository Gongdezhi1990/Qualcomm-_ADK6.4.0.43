############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
"""
Scan Python and make files for Linux lines that are likely to break a Windows/DOS build
"""
import sys
import re
import os
import pprint
import fnmatch
import argparse

# Add dictionaries to this list to scan extra file types.
# Add regular expressions to the REGEX_LIST in the dictionary to scan a file type for more patterns
MATCH_LIST = [
    # Each dict has a glob for each file type of interest
    # and an associated list of regex to be applied to files of that type

    # Unix non-portables
    {'FILE_GLOB': [r'*.mkf', r'makef*'],
     'REGEX_LIST':[
         [r'^\t#',                      ' Guideline 7 :  <TAB># comments in recipes.'],
         [r'\$\(subst +/, *\\',         ' Replace with ospath'],
         [r'^\t.*grep ',                ' Guideline 2 : Unix utils'],
         [r'^\t.*gdb ',                 ' Guideline 2 : Unix utils'],
         [r'^\t.*awk ',                 ' Guideline 2 : Unix utils'],
         [r'^\t.*ls ',                  ' Guideline 2 : Unix utils'],
         [r'^\t.*if \[',                ' Guideline 10: Embedded bash "if"'],
         [r'^\t.*for ',                 ' Guideline 10: Embedded bash "for"'],
         [r'^\t.*\|\|',                 ' Guideline 10: Embedded bash. Compound condition'],
         [r'^\t.*rm ',                  ' Guideline 4 : rm'],
         [r'^\t.*nul',                  ' Guideline 6 : Redirection of I/O'],
         [r'^.* nul',                   ' Guideline 6 : Redirection of I/O'],
         [r'^\t.*/dev/null',            ' Guideline 6 : Redirection of I/O'],
         [r'^.*/dev/null',              ' Guideline 6 : Redirection of I/O'],
         [r'^.*= *\.\.[\\\/]',          ' Outstanding task 12 : relative paths leaking into -I options'],
         [r'^.*mkdir',                  ' Guideline 3 : mkdir, mkdir -p'],
         [r'^\.*true',                  ' Guideline 2 : Replace with $(TRUE)'],
         [r'^\t.*\$\$',                 ' Guideline 2 : Replace with Python or a new macro'],
         [r'^\t.*cat',                  ' Guideline 2 : Replace with $(CAT)'],
         [r'^\t.*python',               ' Guideline ? : Replace with $(PYTHON)'],
         [r'^\t.*kdynmem',              ' Guideline ? : Replace with $(DYNMEM)'],
         [r'^\t.*perl',                 ' Guideline ? : Replace with Python'],
         [r'^\t.*PERL',                 ' Guideline ? : Replace with Python'],
         [r'^\t.*upath',                ' Highlight areas using upath '],
         [r'^\t.*UPATH',                ' Highlight areas using upath'],
         [r'^\t.*cpp',                  ' Replace with $(CPP)'],
         [r'^\t.*touch',                ' Outstanding task 11 :'],
         [r'^\t.* \$@ ',                ' Wrap in ospath'],
         [r'^\t.* \$< ',                ' Wrap in ospath'],
         [r'^\t.*MAKE.*;$',             ' Guideline 9 : Recursive make followed by ";"'],
         [r'^\t.*;$',                   ' Guideline 9 : Recipe line ending in ";"'],
         [r'^\t.*if \[',                ' Guideline 10 : Embedded bash'],
         [r'OSTYPE',                    ' Highlight areas that detect OS type'],
         [r'OS',                        ' Highlight areas that detect OS type'],
         [r'/[a-zA-Z]/',                ' Eliminate MSYS path'],
         [r'[a-zA-Z]:[\\/]',            ' Eliminate DOS drive letter with mixed slash style in path'],
     ]
    },
    # Windows-specific - that may be lying around
    {'FILE_GLOB': [r'*.mkf', r'makef*'],
     'REGEX_LIST':[
         [r'\$\(subst +/, *\\',         ' Replace with ospath'],
         [r'^\t.*del ',                 ' Replace with $(RM), $(RM_F) etc.'],
         [r'^\t.*nul',                  ' Replace with $(STDOUT_DEV_NULL) etc.'],
         [r'^.* nul',                   ' Replace with $(STDOUT_DEV_NULL) etc.'],
         [r'OS',                        ' Replace with $(OSTYPE)'],
         [r'/[a-zA-Z]/',                ' Eliminate MSYS path'],
         [r'[a-zA-Z]:[\\/]',            ' Eliminate DOS drive letter with mixed slash style in path'],
     ]
    },

#    # Unsuppressed recipes and redefined globals and roots like OUTPUT_ROOT and COMPONENT_ROOT
#    {'FILE_GLOB': [r'*.mkf', r'makef*'],
#     'REGEX_LIST':[
#         [r'^\t[^@]', 'No guidance'],
#         [r'_ROOT.*=', 'No guidance']
#     ]
#    },

#    # Macros and scripts that might be merged, speeded up or replaced
#    {'FILE_GLOB': [r'*.mkf', r'makef*'],
#     'REGEX_LIST':[
#         [r'\t.*ospath',    'No guidance'],
#         [r'\t.*abspath',    'No guidance'],
#         [r'\t.*myabspath',    'No guidance'],
#         [r'\t.*winpath',    'No guidance'],
#         [r'\t.*upath',    'No guidance'],
#     ]
#    },

    # Python non-portables
    {'FILE_GLOB': [r'*.py'],
     'REGEX_LIST':[
         [r'/cygdrive',               'No guidance'],
         [r'from csr.*import',        'No guidance'],
     ]
    },
] # END MATCH_LIST

def parse_args(args):
    """ parse the command line arguments """
    parser = argparse.ArgumentParser(description='Scan a build system for Windows portability issues in GNU makefiles and Python scripts')

    parser.add_argument('-r', '--root',
                        required=True,
                        help='Specifies the root the tree to scan (can be a single file)')

    parser.add_argument('-s', '--skip_string',
                        help='do not scan files with skip_string in their full path')

    parser.add_argument('--verbose',
                        action="store_true",
                        help=' Provide more verbose output')

    return parser.parse_args(args)

def scan_file_for_patterns(tgt_file, regex_list):
    """
    Test each line for a match to each regex in the regex_list

    # Windows-specific - that may be lying around
    {'FILE_GLOB': [r'*.mkf', r'makef*'],
     'REGEX_LIST':[
         [r'\$\(subst +/, *\\',         ' Replace with ospath'],
         [r'^\t.*del ',                 ' Replace with $(RM), $(RM_F) etc.'],
         [r'^\t.*nul',                  ' Replace with $(STDOUT_DEV_NULL) etc.'],
         [r'^.* nul',                   ' Replace with $(STDOUT_DEV_NULL) etc.'],
         [r'OS',                        ' Replace with $(OSTYPE)'],
         [r'/[a-zA-Z]/',                ' Eliminate MSYS path'],
         [r'[a-zA-Z]:[\\/]',            ' Eliminate DOS drive letter with mixed slash style in path'],
     ]
    },
    """
    match_found = False

    with open(tgt_file) as f:
        #print 'File: %s' % tgt_file
        for num, line in enumerate(f):
            for ridx, regex in enumerate(regex_list):
                #print 'Trying REGEX %d: "%s"' % (ridx, regex[0])
                if re.search(regex[0], line):
                    print '%s:%d REGEX %d:"%s" LINE:"%s" - %s' % (tgt_file, num + 1, ridx, regex[0], line.rstrip('\n'), regex[1])
                    match_found = True
    return match_found

def find_files(start, match_list, skip_string):
    """
    walk the file tree and scan files whose basename matches any glob in match_list.
    skip any files whose full path contains skip_string
    """
    clean_files = []
    dirty_files = []

    if os.path.isfile(start):
        filenames = [start]
        for entry in match_list:
            for file_glob in entry['FILE_GLOB']:
                for file in fnmatch.filter(filenames, file_glob):
                    full_path = file
                    if skip_string and full_path.find(skip_string) >= 0:
                        continue

                    if scan_file_for_patterns(full_path, entry['REGEX_LIST']):
                        dirty_files.append(full_path)
                    else:
                        clean_files.append(full_path)
    else:
        for dirpath, dirnames, filenames in os.walk(start):
            for entry in match_list:
                for file_glob in entry['FILE_GLOB']:
                    for file in fnmatch.filter(filenames, file_glob):
                        full_path = os.path.join(dirpath, file)
                        if skip_string and full_path.find(skip_string) >= 0:
                            continue

                        if scan_file_for_patterns(full_path, entry['REGEX_LIST']):
                            dirty_files.append(full_path)
                        else:
                            clean_files.append(full_path)

    return clean_files, dirty_files

def main(args):
    """ main entry point.
        - Processes cmd line args.
        - Kicks of the scan walking the tree scanning files as defined in MATCH_LIST
    """
    parsed_args = parse_args(args)

    print '---- MATCH_LIST ----'
    pprint.pprint(MATCH_LIST)

    start = parsed_args.root
    print '+'*80

    print 'Scanning : "%s"' % start

    print '-'*4 + 'LOCATION OF POTENTIAL PORTABILITY ISSUES' + '-'*40 
    clean_files, dirty_files = find_files(start, MATCH_LIST, parsed_args.skip_string)

    if parsed_args.verbose:
        print '-'*4 + 'DIRTY FILES' + '-'*40 
        for f in dirty_files:
            print f

    print 'dirty file count = %d' % len(dirty_files)

    if parsed_args.verbose:
        print '-'*4 + 'CLEAN FILES' + '-'*40 
        for f in clean_files:
            print f

    print 'clean file count = %d' % len(clean_files)
    print 'File count summary - clean: %d, dirty: %d, total: %d' % (len(clean_files),
                                                                    len(dirty_files),
                                                                    len(clean_files) + len(dirty_files))
    print '+'*80
    print ' '


if __name__ == '__main__':
    main(sys.argv[1:])
