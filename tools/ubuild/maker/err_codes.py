"""
Copyright (c) 2016 - 2018 Qualcomm Technologies International, Ltd 

List of error codes and strings returned by build system modules to external
callers i.e. GUIs and OS shells
"""

CODES = {
    'FATAL_ERROR' :{'val':1, 'msg':'Fatal error'},
    'INVALID_WORKSPACE_FILE' :{'val':2, 'msg':'Invalid workspace file'},
    'INVALID_PROJECT_FILE' :{'val':2, 'msg':'Invalid project file'},
    'INVALID_SOURCE_FILE' :{'val':3, 'msg':'Invalid source file in project file'},
    'INVALID_DEVKIT_FILE' :{'val':4, 'msg':'Invalid devkit file'},
    'INVALID_CONFIG' :{'val':5, 'msg':'Invalid configuration'},
    'INVALID_COMMAND_PROPERTY' :{'val':6, 'msg':'Invalid devkit command property'},
    'INVALID_XML' :{'val':7, 'msg':'Invalid XML'},
    'INVALID_BUILD_STYLE' :{'val':8, 'msg':'Invalid build style'},

    #'INVALID_XXXX' :{'val':0X, 'msg':'XXXXXX'},
}

