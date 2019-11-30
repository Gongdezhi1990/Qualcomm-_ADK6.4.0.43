############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2012 - 2019 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
'''
ACAT's setup file
'''
import distutils
import shlex
import subprocess
import sys

from setuptools import setup, find_packages

# this will load version.py which should contain a __version__ = 'version'
exec(open('ACAT/_version.py').read())  # @UndefinedVariable pylint: disable=exec-used

PACKAGE_NAME = 'ACAT'
VERSION = __version__  # @UndefinedVariable pylint: disable=undefined-variable
LICENSE = 'Other/Proprietary License'


setup(
    name=PACKAGE_NAME,

    # Versions should comply with PEP440.  For a discussion on single-sourcing
    # the version across setup.py and the project code, see
    # https://packaging.python.org/en/latest/single_source_version.html
    version=VERSION,

    include_package_data=True,

    description='Audio Coredump Analysis Tool',

    author='Qualcomm',
    author_email='acat_dev@qti.qualcomm.com',
    maintainer='QCSR Audio FW',
    maintainer_email='acat_dev@qti.qualcomm.com',
    url='http://www.qualcomm.com/',
    license=LICENSE,

    classifiers=[

        'Development Status :: 7',

        'Environment :: Console',

        'Intended Audience :: Customer Service',
        'Intended Audience :: Developers'
        'Intended Audience :: Information Technology',
        'Intended Audience :: Manufacturing',
        'Intended Audience :: Telecommunications Industry',

        'License :: %s' % (LICENSE),

        'Operating System :: Microsoft :: Windows',
        'Operating System :: POSIX',

        'Programming Language :: Python',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.4',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
        'Topic :: Software Development :: Testing'
    ],

    python_requires='>=2.7, !=3.0.*, !=3.1.*, !=3.2.*, !=3.3.*, <4',

    install_requires=[
    ],

    packages=find_packages(
        exclude=[
            'contrib',
            'coredump_tests',
            'ADK',
            'unit_tests'
        ]
    ),

    package_data={
        '': ['config/*.json'],
    },

    entry_points={
        'console_scripts': [
            'acat=ACAT:main',
        ]
    }
)
