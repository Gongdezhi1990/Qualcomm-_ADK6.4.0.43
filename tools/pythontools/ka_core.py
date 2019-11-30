# Copyright (c) 2014 - 2018 Qualcomm Technologies International, Ltd
# Part of the Python bindings for the kalaccess library.
# $Change: 3012700 $

from ctypes import c_int, c_byte, c_void_p, c_char_p, POINTER, byref, cast, cdll
from ka_ctypes import ka_err, ka_connection_details, CTypesStrIn, from_cstr
from ka_exceptions import KalaccessError, NotConnectedError
import os
import sys


class KaCore:
    """
    KaCore handles loading the kalaccess library, and exposes the core session-creation/destruction APIs.
    """

    def _declare_cfunc(self, name, restype, argtypes):
        """
        Simple helper function to provide a concise syntax for declaring ctypes function prototypes.
        """
        func = getattr(self._ka_dll, name)
        func.argtypes = argtypes
        func.restype  = restype
        return func

    def add_cfunc(self, cfuncs, name, restype, argtypes):
        cfuncs[name] = self._declare_cfunc(name, restype, argtypes)

    def extract_c_integer_constant(self, name):
        # This trick grabs the exported symbol (disguised as a function pointer), and casts it back into the actual
        # type (int, here).
        as_pointer = cast(getattr(self._ka_dll, name), POINTER(c_int))
        return as_pointer.contents.value
        
    def handle_error(self, err):
        """
        Internal routine to convert kalaccess error structures into Python exceptions.
        """
        if bool(err):
            err_string = from_cstr(err.contents.err_string)
            self._free_error(err)
            raise KalaccessError(err_string)

    def _free_error(self, err):
        """
        Internal function used to free the error structures returned by kalaccess in the event of an error.
        """
        self._cfuncs['ka_free_error'](err)

    def get_ka(self):
        """
        Returns the session pointer kalaccess gave to us on connection.
        If not connected, raise a NotConnectedError."""
        if self._core is None:
            raise NotConnectedError()
        return self._core
    
    @staticmethod
    def _add_script_dir_to_path():
        """
        Add the absolute path of this script to the system path.
        """
        filename = __file__
        mydir = os.path.abspath(os.path.dirname(filename))
        os.environ["PATH"] = mydir + ";" + os.environ["PATH"]

    def _load_kalaccess(self):
        filename = __file__
        mydir = os.path.abspath(os.path.dirname(filename))

        # Load the C++ library. Give a bit of support to try to ease troubleshooting of common problems.
        if sys.platform.startswith('linux'):
            try:
                self._ka_dll = cdll.LoadLibrary(os.path.join(mydir, "libkalaccess_shared.so"))
            except Exception as ex:
                message = ("Could not find or load libkalaccess_shared.so (or one of its dependencies).\n"
                           "If the library is present, check that your Python installation type (32/64-bit) matches "
                           "the architecture of the kalaccess shared library (e.g. via the 'file' command)."
                           "The LD_LIBRARY_PATH used for the search was:\n    ")
                message += "\n    ".join(os.environ.get('LD_LIBRARY_PATH', '').split(":"))
                message += "\n\nInner Python exception : %r" % ex
                raise OSError(message)

        elif sys.platform.startswith('cygwin') or sys.platform.startswith('win32'):
            # On Cygwin, path elements are separated by colons, but on win32 it's a semi-colon.
            path_element_separator = ":" if sys.platform.startswith('cygwin') else ";"

            # Add the absolute path of this script to the system path
            if os.environ.get("PATH", "").find(mydir) == -1:
                os.environ["PATH"] = mydir + path_element_separator + os.environ.get("PATH", "")

            try:
                self._ka_dll = cdll.LoadLibrary("kalaccess.dll")
            except Exception as ex:
                message = ("Could not find or load kalaccess.dll (or one of its dependencies).\n"
                           "If the library is present, check that your Python installation type (32/64-bit) matches "
                           "the kalaccess DLL (likely 32-bit).\n"
                           "Sometimes this error can be fixed by installing a Visual C++ Redistributable package.\n"
                           "The system PATH used for the search was:\n    ")
                message += "\n    ".join(os.environ.get('PATH', '').split(path_element_separator))
                message += "\n\nInner Python exception : %r" % ex
                raise OSError(message)
        else:
            raise OSError("Cannot load the kalaccess library. The system '%s' you are using is not supported."
                          % sys.platform)

    def _declare_kalaccess_functions(self):
        """
        Construct ctypes declarations of the C functions we need from the kalaccess library.
        """
        self._cfuncs = {}
        self.add_cfunc(
            self._cfuncs,
            'ka_is_connected',
            c_byte,
            [c_void_p]
        )
        self.add_cfunc(
            self._cfuncs,
            'ka_get_ka_version',
            c_char_p,
            []
        )
        self.add_cfunc(
            self._cfuncs,
            'ka_connect',
            POINTER(ka_err),
            [POINTER(ka_connection_details), c_byte, POINTER(c_void_p)]
        )
        self.add_cfunc(
            self._cfuncs,
            'ka_connect_uri',
            POINTER(ka_err),
            [CTypesStrIn, POINTER(c_void_p)]
        )
        self.add_cfunc(
            self._cfuncs,
            'ka_disconnect',
            None,
            [c_void_p]
        )
        self.add_cfunc(
            self._cfuncs,
            'ka_free_error',
            None,
            [POINTER(ka_err)]
        )

    def __init__(self):
        # The '_core' variable tracks if we have a connection.
        self._core   = None
        self._ka_dll = None

        self._add_script_dir_to_path()
        self._load_kalaccess()

        self._declare_kalaccess_functions()

    def is_connected(self):
        """
        Returns true if this instance has an active debug connection to the chip.
        """
        if self._core is None:
            return False
        return bool(self._cfuncs['ka_is_connected'](self.get_ka()))

    def get_ka_version(self):
        """
        Returns the version of the kalaccess library.
        """
        return from_cstr(self._cfuncs['ka_get_ka_version']())

    def connect(self, trans, ignore_fw, ignored_ptr):
        self._core = c_void_p()
        err = self._cfuncs['ka_connect'](trans, ignore_fw, byref(self._core))
        if err:
            self._core = None
        self.handle_error(err)
        
    def connect_with_uri(self, core_uri):
        """
        Connect using a Heracles core URI.
        :param core_uri: a valid Heracles core URI. Examples include:
        - device://trb/usb2trb/0/csra68100/app/p0
        - device://spi/usb2spi/0/csr6870/audio
        - device://tcp/localhost:31400/spi/sim/csr6870/audio
        """
        self._core = c_void_p()
        err = self._cfuncs['ka_connect_uri'](core_uri, byref(self._core))
        if err:
            self._core = None
        self.handle_error(err)

    def disconnect(self):
        """
        Disconnects any active debug connection / session.
        Does nothing if not connected.
        """
        rv = self._cfuncs['ka_disconnect'](self.get_ka())
        self._core = None
        return rv
