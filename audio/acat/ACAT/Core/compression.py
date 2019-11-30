############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2018 - 2019 Qualcomm Technologies, Inc. and/or its
# subsidiaries.  All rights reserved.
#
############################################################################
"""
Handle compressed files.
"""
import logging
import zipfile

from ACAT.Core.constants import COREDUMP_EXTENSION
from ACAT.Core.exceptions import UsageError

logger = logging.getLogger(__name__)


class CompressionException(UsageError):
    """Base class for exceptions thrown by the compression class.

    Args:
        filename
        msg
    """

    def __init__(self, filename, msg):
        full_msg = "%s: %s" % (filename, msg)
        logger.error(full_msg)
        super(CompressionException, self).__init__("ERROR: " + full_msg)


class CoredumpNotFoundError(CompressionException):
    """The zip file didn't contain any coredumps.

    Args:
        filename
    """

    def __init__(self, filename):
        super(CoredumpNotFoundError, self).__init__(
            filename, "No coredumps found")


class TooManyCoredumpsFoundError(CompressionException):
    """The zip file contained multiple coredumps.

    Args:
        filename
    """

    def __init__(self, filename):
        super(TooManyCoredumpsFoundError, self).__init__(
            filename, "Multiple coredumps found")


def is_zip(filename):
    """Check whether the given filename is a zip file.

    Args:
        filename (str): The full path to a physical file on the disk.

    Returns:
        bool: True if the given filename is a zip file, False otherwise.
    """
    return zipfile.is_zipfile(filename)


def extract(zip_path):
    """Extract a zip file with single file inside it.

    Args:
        zip_path(str): Zip filename.

    Returns:
        The contents of the zipped file.
    """
    with zipfile.ZipFile(zip_path) as zip_handler:
        filename = zip_handler.filelist.pop().filename

        with zip_handler.open(filename) as handler:
            return handler.read()


def get_coredump_lines(zip_filename):
    """Extracts a zipped coredump into a temp file and return the filename.

    This function inspects the given zip file for coredumps which are
    distinguished by their extensions. When there is a coredump, it
    extracts the coredump and returns it as a list of lines.

    If the zip file has more than one coredump or none, the function
    raises an exception.

    Args:
        zip_filename(str): Zip filename.

    Returns:
        contents (list): A list of lines in the coredump file.

    Raises:
        BadZipFile: If the given zip file is not a valid zip file.
        TooManyCoredumpsFoundError: When there are multiple coredump files
            in the zip file.
        CoredumpNotFoundError: When there is no coredump file in the zip
            file.
    """
    with zipfile.ZipFile(zip_filename) as zipped_coredump:

        coredump_filenames = [
            zipped_file.filename
            for zipped_file in zipped_coredump.filelist
            if zipped_file.filename.endswith(COREDUMP_EXTENSION)
        ]

        if len(coredump_filenames) == 0:
            raise CoredumpNotFoundError(zip_filename)
        elif len(coredump_filenames) > 1:
            raise TooManyCoredumpsFoundError(zip_filename)

        # One and only one coredump filename available in the given zip
        # file
        coredump_filename = coredump_filenames.pop()

        with zipped_coredump.open(coredump_filename) as coredump_handler:
            return coredump_handler.readlines()
