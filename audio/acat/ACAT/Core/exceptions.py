############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2018 - 2019 Qualcomm Technologies, Inc. and/or its
# subsidiaries.  All rights reserved.
#
############################################################################
"""
"""
import os
import sys
if sys.version_info.major == 2:
    # Python 2
    # pylint: disable=redefined-builtin
    from future_builtins import hex


class UsageError(Exception):
    """Something has been called in the wrong way."""


class FormatterError(Exception):
    """Signals a formatter error."""


class AnalysisError(Exception):
    """Executing the analysis faces a problem.

    An error occurred; we can't complete this analysis.  When raising the
    exception, we can choose to pass a simple string argument(as normal
    for exceptions), or pass a tuple of (string, argument).
    """

    def __str__(self):
        if len(self.args) == 1:
            apology = self.args[0]
        else:
            apology = self.args[0] + " " + str(self.args[1])
        return apology


class FatalAnalysisError(Exception):
    """Executing the analysis faces a problem that we can not recover from.

    A fatal error occurred; we should stop all analysis and bail out!
    (This is drastic: only use if we really can't do *any* work at all.)
    """


class OutdatedFwAnalysisError(Exception):
    """The firmware is outdated compared to the analyses."""


class CoredumpParsingError(Exception):
    """An error occurred while parsing the coredump."""


class OutOfRangeError(KeyError):
    """The supplied address was valid, but the range was not.

    When raising the exception, we pass a tuple of (string,
    last_valid_key).
    """

    def __str__(self):
        return self.args[0] + "; last valid key was: " + hex(self.args[1])


class ChipNotPoweredError(Exception):
    """The connection to the Live chip appears to have been lost."""


class AmbiguousSymbolError(Exception):
    """Multiple symbol matches were found and couldn't decide which to return.

    When raising the exception, we can choose to pass a simple string
    argument(as normal for exceptions), or pass a tuple of (string,
    args_list).
    """

    def __str__(self):
        if len(self.args) == 1:
            apology = self.args[0]
        else:
            try:
                output_string = ""
                # Sort matches by name
                sorted_match = sorted(
                    self.args[1],
                    key=lambda match: match["name"]
                )
                for match in sorted_match:
                    output_string += match["name"] + \
                        "\n\tElf id:" + hex(match["elf_id"])

                    filename = os.path.split(match["file_path"])[-1]
                    output_string += "\n\t%s\n" % filename
                apology = self.args[0] + "\n" + output_string
            except TypeError:
                apology = self.args[0] + "  " + str(self.args[1])

        return apology


class BundleMissingError(Exception):
    """A downloadable bundle elf is missing."""


class DebugAndChipdataError(Exception):
    """General exceptions for debug information and chipdata."""


class ChipdataError(DebugAndChipdataError):
    """General exceptions for chipdata errors."""


class DebugInfoError(DebugAndChipdataError):
    """General exceptions for debug information."""


class DebugInfoNoVariableError(DebugInfoError):
    """Variable not found in debug info."""


class DebugInfoNoLabelError(DebugInfoError):
    """Code label not found in debug info."""


class InvalidPmAddressError(DebugInfoError):
    """The address is an invalid PM address."""


class UnknownPmEncodingError(DebugInfoError):
    """Unknown PM encoding for the address."""


class InvalidDebuginfoCallError(DebugInfoError):
    """DebugInfo received an invalid call."""


class InvalidDebuginfoTypeError(DebugInfoError):
    """DebugInfo received an invalid call."""


class InvalidDebuginfoEnumError(DebugInfoError):
    """DebugInfo received an invalid call."""


class InvalidDmConstAddressError(DebugInfoError):
    """The address is an invalid DM constant address."""


class InvalidDmConstLengthError(DebugInfoError):
    """The length of the DM constant read is invalid.

    Args:
        max_length: Maximum read size length.
        address (int): Words address.
    """

    def __init__(self, max_length, address):
        DebugInfoError.__init__(self)
        self.max_length = max_length
        self.address = address

    def __str__(self, *args, **kwargs):
        return (
            "Can only read " + hex(self.max_length) + "words at address " +
            hex(self.address)
        )


class InvalidDmAddressError(ChipdataError):
    """The address is an invalid DM address."""


class InvalidDmLengthError(ChipdataError):
    """The length of the DM read is invalid.

    Args:
        max_length: Maximum read size length.
        address (int): Words address.
    """

    def __init__(self, max_length, address):
        ChipdataError.__init__(self)
        self.max_length = max_length
        self.address = address

    def __str__(self, *args, **kwargs):
        return (
            "Can only read " + hex(self.max_length) + "words at address " +
            hex(self.address)
        )
