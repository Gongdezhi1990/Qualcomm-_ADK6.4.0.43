# Copyright (c) 2014 - 2018 Qualcomm Technologies International, Ltd
# Part of the Python bindings for the kalaccess library.


class KalaccessError(RuntimeError):
    """
    Encapsulates an error raised from the kalaccess library.
    """
    def __init__(self, message):
        super(KalaccessError, self).__init__(message)


class NotConnectedError(RuntimeError):
    def __init__(self, message="No chip connection"):
        super(NotConnectedError, self).__init__(message)


class UnknownRegister(RuntimeError):
    def __init__(self, reg):
        self.reg = reg

    def __str__(self):
        return "Unknown register: %s" % self.reg


class UnsupportedArchitecture(RuntimeError):
    pass
