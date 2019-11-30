# Copyright (c) 2014 - 2018 Qualcomm Technologies International, Ltd
# Python bindings for the kalelfreader library.
# $Change: 3012700 $

from __future__ import print_function
from ctypes import byref, c_bool, c_char_p, c_int, c_ubyte, c_uint, c_uint32, c_void_p, cast, cdll, POINTER,\
    Structure
import os
import sys
from contextlib import contextmanager


def ctypes_arr_type(elem_type, length):
    """
    Create a ctypes array type of 'length' elements of type 'elem_type'
    :type elem_type: Type
    :return Type
    """
    return elem_type * length


class CTypesStrIn(object):
    """
    Python 2/3 compatible wrapper for passing strings into ctypes wrapped functions.
    """
    @classmethod
    def from_param(cls, value):
        """
        Called by ctypes when the given value is going to be passed as a parameter.
        """

        # Already bytes: nothing to do
        if isinstance(value, bytes):
            return value

        # Python 3 (or in principle Python 2 unicode type) => encode to ASCII
        return value.encode('ascii')


def from_cstr(s):
    """
    Python 2 and 3 compatible ASCII-encoded C string to Python-string conversion.
    Used when bringing values back from the C interface.
    """
    if s is None:
        return s

    # Python 2: no-op
    if isinstance(s, str):
        return s

    # Python 3: from bytes
    return s.decode('ascii')


class NoFileLoadedError(Exception):
    def __str__(self):
        return "No ELF file is loaded"


class LibraryVersionMismatchError(Exception):
    pass


class KerLibraryError(Exception):
    def __init__(self, code, message):
        Exception.__init__(self, message)
        self.code = code


class KerErrorCouldntOpenFile(KerLibraryError):
    pass


class KerErrorNoSuchFunction(KerLibraryError):
    pass


class KerErrorNoSuchVariable(KerLibraryError):
    pass


class KerErrorInvalidVariable(KerLibraryError):
    pass


class KerErrorInvalidIndex(KerLibraryError):
    pass


class KerErrorIncorrectTableSize(KerLibraryError):
    pass


class KerErrorInvalidArgument(KerLibraryError):
    pass


class KerErrorSectionDataNotFound(KerLibraryError):
    pass


# Indices correspond to the values of the C enum 'KerErrorId'.
KER_ERROR_MAP = [
    None,  # KerErrorNoError
    KerErrorCouldntOpenFile,
    KerErrorNoSuchFunction,
    KerErrorNoSuchVariable,
    KerErrorInvalidVariable,
    KerErrorInvalidIndex,
    KerErrorIncorrectTableSize,
    KerErrorInvalidArgument,
    KerErrorSectionDataNotFound
]

# Type ID indicating either 'no known type' or 'error'.
INVALID_TID = 0xffffffff


# ctypes Structure-derived classes, corresponding to the C structures
# defined in kalelfreader_c_wrapper.h.
class KerError(Structure):
    _fields_ = [("code", c_int),
                ("message", c_char_p)]


class KerElfSectionHeader(Structure):
    _fields_ = [("name", c_char_p),
                ("addr", c_uint),
                ("loadAddr", c_uint),
                ("num_bytes", c_uint),
                ("type", c_int)]

                
class KerElfSectionHeaderEx(Structure):
    _fields_ = [("name", c_char_p),
                ("addr", c_uint),
                ("loadAddr", c_uint),
                ("num_bytes", c_uint),
                ("type", c_int),
                ("flags", c_uint32),
                ("is_pm", c_bool),
                ("hw_addr", c_uint)]
                

class KerStatement(Structure):
    _fields_ = [("module", c_char_p),
                ("source_line", c_int),
                ("addr", c_uint32),
                ("source_file", c_char_p)]


class KerMember(Structure):
    _fields_ = [("type_id", c_uint32),
                ("name", c_char_p),
                ("offset", c_uint32),
                ("bit_offset_from_offset", c_uint32),
                ("bit_size", c_uint32)]


class KerEnumValue(Structure):
    _fields_ = [("name", c_char_p),
                ("value", c_uint32)]


# Note that unspecified parameters (e.g. ellipsis in C) are represented by name="...", type_id=INVALID_TID
class KerFunctionParameter(Structure):
    _fields_ = [("name", c_char_p),
                ("type_id", c_uint32)]


# form: 0-1-2-3-4-5-6-7-8-9 : Base-Struct-Union-Array-Typedef-Pointer-Const-Volatile-Enum-Function
# type_id: index of self in the type table
# member_count: non-zero for structs or unions (form=1 or 2)
# members: valid for structs or unions (form=1 or 2)
# array_count: valid for arrays (form=3)
# array_type_id: valid for arrays (form=3)
# enum_values: valid for enums (form=8)
# return_type: return type, valid only for functions (form=9)
# parameter_count: number of parameters, valid only for functions (form=9)
# parameters: parameters, valid only for functions (form=9)
class KerType(Structure):
    _fields_ = [("form", c_uint32),
                ("type_id", c_uint32),
                ("name", c_char_p),
                ("size_in_addressable_units", c_uint32),
                ("ref_type_id", c_uint32),
                ("is_declaration", c_bool),
                ("member_count", c_int),
                ("members", POINTER(KerMember)),
                ("array_count", c_int),
                ("array_type_id", c_uint32),
                ("enum_value_count", c_int),
                ("enum_values", POINTER(KerEnumValue)),
                ("return_type", c_uint32),
                ("parameter_count", c_int),
                ("parameters", POINTER(KerFunctionParameter))]


class KerVariable(Structure):
    _fields_ = [("name", c_char_p),
                ("size_in_addressable_units", c_uint32),
                ("addr", c_uint32),
                ("type_id", c_uint32),
                ("source_file", c_char_p),
                ("line_num", c_int)]


class KerConstant(Structure):
    _fields_ = [("name", c_char_p),
                ("value", c_uint32)]


class KerLabel(Structure):
    _fields_ = [("name", c_char_p),
                ("addr", c_uint32)]


class KerSymbolType:
    """The type of a symbol (the 'type' field in KerSymbol): variable, label, constant or other/unknown"""
    KER_SYM_VARIABLE = 0
    KER_SYM_LABEL = 1
    KER_SYM_CONSTANT = 2
    KER_SYM_OTHER = 3


class KerSymbol(Structure):
    _fields_ = [("name", c_char_p),
                ("type", c_uint),
                ("is_pm", c_bool),
                ("addr_or_value", c_uint32),
                ("size_in_addressable_units", c_uint32),
                ("type_id", c_uint32),
                ("source_file", c_char_p),
                ("line_num", c_int)]


class KerData(Structure):
    _fields_ = [("bit_width", c_int),
                ("byte_addressing", c_int),
                ("start_addr", c_uint32),
                ("num_bytes", c_uint32),
                ("data", POINTER(c_uint32))]


class KerArchitecture(Structure):
    _fields_ = [("dsp_rev", c_int),
                ("addr_width", c_int),
                ("dm_data_width", c_int),
                ("pm_data_width", c_int),
                ("dm_byte_addressing", c_int),
                ("pm_byte_addressing", c_int)]


class KerReport(Structure):
    _fields_ = [("severity", c_int),
                ("text", c_char_p)]


class KerEnum(Structure):
    _fields_ = [("name", c_char_p),
                ("value", c_uint)]


class KerFunction(Structure):
    _fields_ = [("function_name", c_char_p),
                ("source_file", c_char_p),
                ("line_num", c_int),
                ("low_pc", c_uint32),
                ("high_pc", c_uint32)]


class KerAddressRange(Structure):
    _fields_ = [("is_pm", c_bool), 
                ("start", c_uint32),
                ("end", c_uint32)]


class KerCompilationUnit(Structure):
    _fields_ = [("file_name", c_char_p),
                ("num_address_ranges", c_uint32),
                ("address_ranges", POINTER(KerAddressRange))]


class KerCompilationSourceFolder(Structure):
    _fields_ = [("comp_dir", c_char_p),
                ("num_units", c_uint32),
                ("units", POINTER(KerCompilationUnit))]    


class ElfSectionFlags(object):
    """
    ELF section flag values, taken from libelf.
    Most are irrelevant, but the full list of values is included for completeness.
    """
    SHF_WRITE               = 0x1
    SHF_ALLOC               = 0x2
    SHF_EXECINSTR           = 0x4
    SHF_MERGE               = 0x10
    SHF_STRINGS             = 0x20
    SHF_INFO_LINK           = 0x40
    SHF_LINK_ORDER          = 0x80
    SHF_OS_NONCONFORMING    = 0x100
    SHF_GROUP               = 0x200
    SHF_TLS                 = 0x400
    SHF_MASKOS              = 0x0ff00000
    SHF_MASKPROC            = 0xf0000000


@contextmanager
def scoped_ker(file_path, extra_non_allocable_named_sections = ()):
    ker = Ker()
    try:
        ker.open_file(file_path, extra_non_allocable_named_sections)
        yield ker
    finally:
        ker.close_file()


class Ker:
    """Python wrapper for the kalelfreader library"""

    def _load_library(self):
        # Find the absolute path of this script
        filename = __file__
        mydir = os.path.abspath(os.path.dirname(filename))

        # Load the C++ library. Give a bit of support to try to ease troubleshooting of common problems.
        if sys.platform.startswith('linux'):
            try:
                self._ker_dll = cdll.LoadLibrary(os.path.join(mydir, "libkalelfreader.so"))
            except Exception as ex:
                message = ("Could not find or load libkalelfreader.so (or one of its dependencies).\n"
                           "If the library is present, check that the your Python installation type (32/64-bit) matches"
                           "the architecture of the kalelfreader shared library (e.g. via the 'file' command)."
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
                self._ker_dll = cdll.LoadLibrary("kalelfreader.dll")
            except Exception as ex:
                message = ("Could not find or load kalelfreader.dll (or one of its dependencies).\n"
                           "If the library is present, check that the your Python installation type (32/64-bit) matches"
                           "the kalelfreader DLL (likely 32-bit).\n"
                           "Sometimes this error can be fixed by installing a Visual C++ Redistributable package.\n"
                           "The system PATH used for the search was:\n    ")
                message += "\n    ".join(os.environ.get('PATH', '').split(path_element_separator))
                message += "\n\nInner Python exception : %r" % ex
                raise OSError(message)
        else:
            raise OSError("Cannot load the kalelfreader library. The system '%s' you are using is not supported."
                          % sys.platform)

    def _declare_cfuncs(self):
        self._cfuncs = {}

        def gen_prototype(name, argtypes, restype):
            func = getattr(self._ker_dll, name, None)
            if func is None:
                raise LibraryVersionMismatchError(
                    "Function '{0}' not found in the loaded kalelfreader library. Check that the Python bindings and"
                    " the C++ library are from the same release.".format(name))

            func.argtypes = argtypes
            func.restype = restype
            self._cfuncs[name] = func

        gen_prototype('ker_get_version',           [],                            c_char_p)
        gen_prototype('ker_open_file',             [CTypesStrIn, POINTER(c_void_p)], POINTER(KerError))
        gen_prototype('ker_open_file_req_extra_sections',
                      [CTypesStrIn, POINTER(c_char_p), c_uint, POINTER(c_void_p)],
                      POINTER(KerError))
        gen_prototype('ker_close_file',            [c_void_p],                    None)
        gen_prototype('ker_free_error',            [POINTER(KerError)],           None)

        gen_prototype('ker_get_function_count',    [c_void_p], c_int)
        gen_prototype('ker_get_function_by_index', [c_void_p, c_int,    POINTER(KerFunction)], POINTER(KerError))
        gen_prototype('ker_get_function_by_name',  [c_void_p, CTypesStrIn, POINTER(KerFunction)], POINTER(KerError))
        gen_prototype('ker_get_function_by_addr',  [c_void_p, c_uint32, POINTER(KerFunction)], POINTER(KerError))
        gen_prototype('ker_get_variable_by_name',  [c_void_p, CTypesStrIn, POINTER(KerVariable)], POINTER(KerError))

        gen_prototype('ker_get_elf_section_headers', [c_void_p, POINTER(c_int), POINTER(POINTER(KerElfSectionHeader))],
                      POINTER(KerError))
        gen_prototype('ker_get_elf_section_headers_ex',
                      [c_void_p, POINTER(c_int), POINTER(POINTER(KerElfSectionHeaderEx))],
                      POINTER(KerError))
        gen_prototype('ker_get_compilation_units',
                      [c_void_p, POINTER(c_int), POINTER(POINTER(KerCompilationSourceFolder))],
                      POINTER(KerError))
        gen_prototype('ker_get_build_tool_versions',  [c_void_p, POINTER(c_int), POINTER(POINTER(c_char_p))],
                      POINTER(KerError))
        gen_prototype('ker_get_statements', [c_void_p, POINTER(c_int), POINTER(POINTER(KerStatement))],
                      POINTER(KerError))
        gen_prototype('ker_get_variables',  [c_void_p, POINTER(c_int), POINTER(POINTER(KerVariable))],
                      POINTER(KerError))
        gen_prototype('ker_get_constants',  [c_void_p, POINTER(c_int), POINTER(POINTER(KerConstant))],
                      POINTER(KerError))
        gen_prototype('ker_get_labels',     [c_void_p, POINTER(c_int), POINTER(POINTER(KerLabel))],
                      POINTER(KerError))
        gen_prototype('ker_get_all_symbols', [c_void_p, POINTER(c_int), POINTER(POINTER(KerSymbol))],
                      POINTER(KerError))
        gen_prototype('ker_get_dm_data',    [c_void_p, POINTER(c_int), POINTER(POINTER(KerData))],
                      POINTER(KerError))
        gen_prototype('ker_get_pm_data',    [c_void_p, POINTER(c_int), POINTER(POINTER(KerData))],
                      POINTER(KerError))
        gen_prototype('ker_get_extra_named_non_allocable_section_data',
                      [c_void_p, CTypesStrIn, POINTER(POINTER(KerData))],
                      POINTER(KerError))
        gen_prototype('ker_get_types',      [c_void_p, POINTER(c_int), POINTER(POINTER(KerType))],
                      POINTER(KerError))
        gen_prototype('ker_get_reports',    [c_void_p, POINTER(c_int), POINTER(POINTER(KerReport))],
                      POINTER(KerError))
        gen_prototype('ker_get_enums',      [c_void_p, POINTER(c_int), POINTER(POINTER(KerEnum)),
                      POINTER(c_int), POINTER(POINTER(KerEnum))],
                      POINTER(KerError))

        gen_prototype('ker_get_dsp_revision', [c_void_p], c_int)
        gen_prototype('ker_get_machine_id',   [c_void_p], c_uint)
        gen_prototype('ker_is_big_endian',    [c_void_p], c_bool)
        gen_prototype('ker_get_architecture', [c_void_p, POINTER(KerArchitecture)], POINTER(KerError))
        gen_prototype('ker_is_overlapping_statements',  [c_void_p], c_bool)
        gen_prototype('ker_get_overlapping_statements', [c_void_p, POINTER(POINTER(KerStatement))], POINTER(KerError))

        gen_prototype('ker_free_elf_section_headers', [POINTER(KerElfSectionHeader)], None)
        gen_prototype('ker_free_elf_section_headers_ex', [POINTER(KerElfSectionHeaderEx)], None)
        gen_prototype('ker_free_compilation_units',   [c_int, POINTER(KerCompilationSourceFolder)], None)
        gen_prototype('ker_free_build_tool_versions', [POINTER(c_char_p)],            None)
        gen_prototype('ker_free_statements',          [POINTER(KerStatement)],        None)
        gen_prototype('ker_free_variables',           [POINTER(KerVariable)],         None)
        gen_prototype('ker_free_constants',           [POINTER(KerConstant)],         None)
        gen_prototype('ker_free_labels',              [POINTER(KerLabel)],            None)
        gen_prototype('ker_free_all_symbols',         [POINTER(KerSymbol)],           None)
        gen_prototype('ker_free_dm_data',             [POINTER(KerData)],             None)
        gen_prototype('ker_free_pm_data',             [POINTER(KerData)],             None)
        gen_prototype('ker_free_extra_named_non_allocable_section_data', [POINTER(KerData)], None)
        gen_prototype('ker_free_types',               [POINTER(KerType), c_int],      None)
        gen_prototype('ker_free_reports',             [POINTER(KerReport)],           None)
        gen_prototype('ker_free_enums',               [POINTER(KerEnum), POINTER(KerEnum)], None)

        gen_prototype('ker_get_not_in_function_count',     [c_void_p], c_int)

    def __init__(self):
        # ker will be non-null if we have a symbol file open (by a successful call to open_file)
        self._ker = None
        self._load_library()
        self._declare_cfuncs()

    def _cfunc(self, name):
        try:
            return self._cfuncs[name]
        except KeyError:
            raise AssertionError("kalelfreader library function '{0}' not declared. Please report this as a bug.".
                                 format(name))

    def get_version(self):
        return from_cstr(self._cfunc('ker_get_version')())

    def _handle_ker_error(self, err):
        """Internal routine to convert kalelfreader's error structures into Python exceptions."""
        if err.contents.code != 0:
            message = from_cstr(err.contents.message)
            code = err.contents.code
            self._free_error(err)
            # Try to raise a specific exception type if the code is mapped to one.
            try:
                ex_type = KER_ERROR_MAP[code]
            except IndexError:
                ex_type = KerLibraryError
            raise ex_type(code, message)
            
    def _free_error(self, ker_error):
        """Internal function used to free the error structures returned by kalelfreader in the
        event of an error."""
        return self._cfunc('ker_free_error')(ker_error)

    def get_function_count(self):
        return self._cfunc('ker_get_function_count')(self._get_ker())

    class KerVariableInfo(object):
        """
        Minimal structure used to take a copy of a KerVariable returned from the DLL.
        This frees the object's validity from being tied to the lifetime of the
        loaded ELF.

        Handles string conversion from C strings for string fields.

        Provides fields compatible with KerVariable.
        """
        def __init__(self, c_ker_var):
            self.name = from_cstr(c_ker_var.name)
            self.size_in_addressable_units = c_ker_var.size_in_addressable_units
            self.addr = c_ker_var.addr
            self.type_id = c_ker_var.type_id
            self.source_file = from_cstr(c_ker_var.source_file)
            self.line_num = c_ker_var.line_num

    def get_variable_by_name(self, name):
        result = KerVariable()
        err = self._cfunc('ker_get_variable_by_name')(self._get_ker(), name, result)
        self._handle_ker_error(err)
        # noinspection PyTypeChecker
        return Ker.KerVariableInfo(result)

    def get_dsp_revision(self):
        return self._cfunc('ker_get_dsp_revision')(self._get_ker())

    def get_machine_id(self):
        return self._cfunc('ker_get_machine_id')(self._get_ker())

    def is_big_endian(self):
        return 1 if self._cfunc('ker_is_big_endian')(self._get_ker()) else 0

    def is_overlapping_statements(self):
        return 1 if self._cfunc('ker_is_overlapping_statements')(self._get_ker()) else 0

    def get_overlapping_statements(self, kst):
        err = self._cfunc('ker_get_overlapping_statements')(self._get_ker(), kst)
        self._handle_ker_error(err)

    def _free_elf_section_headers(self, table):
        return self._cfunc('ker_free_elf_section_headers')(table)

    def _free_elf_section_headers_ex(self, table):
        return self._cfunc('ker_free_elf_section_headers_ex')(table)

    def _free_build_tool_versions(self, table):
        return self._cfunc('ker_free_build_tool_versions')(table)

    def _free_statements(self, table):
        return self._cfunc('ker_free_statements')(table)

    def _free_variables(self, table):
        return self._cfunc('ker_free_variables')(table)

    def _free_constants(self, table):
        return self._cfunc('ker_free_constants')(table)

    def _free_labels(self, table):
        return self._cfunc('ker_free_labels')(table)

    def _free_all_symbols(self, table):
        return self._cfunc('ker_free_all_symbols')(table)

    def _free_dm_data(self, table):
        return self._cfunc('ker_free_dm_data')(table)

    def _free_pm_data(self, table):
        return self._cfunc('ker_free_pm_data')(table)

    def _free_extra_named_non_allocable_section_data(self, data):
        return self._cfunc('ker_free_extra_named_non_allocable_section_data')(data)

    def _free_types(self, table, num_types):
        return self._cfunc('ker_free_types')(table, num_types)

    def _free_reports(self, table):
        return self._cfunc('ker_free_reports')(table)

    def _free_enums(self, table1, table2):
        return self._cfunc('ker_free_enums')(table1, table2)

    def get_not_in_function_count(self):
        return self._cfunc('ker_get_not_in_function_count')(self._get_ker())

    def _get_ker(self):
        if self._ker is None:
            raise NoFileLoadedError()
        return self._ker

    def open_file(self, path, extra_non_allocable_named_sections = ()):
        self._ker = c_void_p()
        try:
            if extra_non_allocable_named_sections:
                num_sections = len(extra_non_allocable_named_sections)
                sect_names = ctypes_arr_type(c_char_p, num_sections)()
                for i, sec in enumerate(extra_non_allocable_named_sections):
                    sect_names[i] = CTypesStrIn.from_param(sec)
                err = self._cfunc('ker_open_file_req_extra_sections')(path, sect_names, num_sections, byref(self._ker))
            else:
                err = self._cfunc('ker_open_file')(path, byref(self._ker))

            self._handle_ker_error(err)
        except:
            self._ker = None
            raise

    def close_file(self):
        rv = self._cfunc('ker_close_file')(self._get_ker())
        self._ker = None
        return rv

    class KerFunctionInfo(object):
        """
        Minimal structure used to take a copy of a KerFunction returned from the DLL.
        This frees the object's validity from being tied to the lifetime of the
        loaded ELF.

        Handles string conversion from C strings for string fields.

        Provides fields compatible with KerFunction.
        """
        def __init__(self, c_ker_func):
            self.function_name = from_cstr(c_ker_func.function_name)
            self.source_file = from_cstr(c_ker_func.source_file)
            self.line_num = c_ker_func.line_num
            self.low_pc = c_ker_func.low_pc
            self.high_pc = c_ker_func.high_pc

    def get_function_by_index(self, index):
        func = KerFunction()
        err = self._cfunc('ker_get_function_by_index')(self._get_ker(), index, byref(func))
        self._handle_ker_error(err)
        # noinspection PyTypeChecker
        return Ker.KerFunctionInfo(func)

    def get_function_by_name(self, name):
        func = KerFunction()
        err = self._cfunc('ker_get_function_by_name')(self._get_ker(), name, byref(func))
        self._handle_ker_error(err)
        # noinspection PyTypeChecker
        return Ker.KerFunctionInfo(func)

    def get_function_by_addr(self, addr):
        func = KerFunction()
        err = self._cfunc('ker_get_function_by_addr')(self._get_ker(), addr, byref(func))
        self._handle_ker_error(err)
        # noinspection PyTypeChecker
        return Ker.KerFunctionInfo(func)

    class KerElfSectionHeaderInfo(object):
        """
        Represents a section header in an ELF file.
        The type field can be one of the following constants defined in this class:
        ELF_SECTION_TYPE_UNKNOWN, ELF_SECTION_TYPE_MAXIM, ELF_SECTION_TYPE_MINIM, ELF_SECTION_TYPE_DATA.

        flags may be a combination of the SHF_* constants defined in ElfSectionFlags (taken from libelf).

        hw_addr is the address as would be presented to the device, e.g. after masking out the top bit indicating PM/DM.
        """
        ELF_SECTION_TYPE_UNKNOWN = 0
        ELF_SECTION_TYPE_MAXIM   = 1
        ELF_SECTION_TYPE_MINIM   = 2
        ELF_SECTION_TYPE_DATA    = 3

        def __init__(self, name, address, load_address, num_bytes, sec_type, flags, is_pm, hw_addr):
            self.name         = name
            self.address      = address
            self.load_address = load_address
            self.num_bytes    = num_bytes
            self.type         = sec_type
            self.flags        = flags
            self.is_pm        = is_pm
            self.hw_addr      = hw_addr

        @property
        def is_alloc(self):
            return (self.flags and ElfSectionFlags.SHF_ALLOC) != 0

        def __repr__(self):
            result = ["KerElfSectionHeaderInfo, section name: %s" % self.name]
            result.append("address: %#x" % self.address)
            result.append("load address: %#x" % self.load_address)
            result.append("num_bytes: %d" % self.num_bytes)
            result.append("type: %d" % self.type)
            result.append("flags: %#x" % self.flags)
            result.append("is_pm: %d" % self.is_pm)
            result.append("hardware address: %#x" % self.hw_addr)
            return "\n".join(result)
            
    def get_elf_section_headers(self):
        """
        Returns a dict of information about ELF sections. Each dict key is section name.
        Each entry is an KerElfSectionHeaderInfo instance.
        """
        count = c_int()
        table = POINTER(KerElfSectionHeaderEx)()
        err = self._cfunc('ker_get_elf_section_headers_ex')(self._get_ker(), byref(count), byref(table))
        self._handle_ker_error(err)
        results = {}
        for i in range(count.value):

            name = from_cstr(table[i].name)

            results[name] = Ker.KerElfSectionHeaderInfo(
                name,
                table[i].addr,
                table[i].loadAddr,
                table[i].num_bytes,
                table[i].type,
                table[i].flags,
                table[i].is_pm,
                table[i].hw_addr
            )

        self._free_elf_section_headers_ex(table)
        return results
        
    def get_compilation_units(self):
        """
        Returns a list of folders containing source files, and within each of these the compilation
        units that were compiled from files in that folder.
        The return value is a list of tuples (folder_path, units) where units is itself a list of
        tuples (file_name, address_ranges) and address_ranges is a list of tuples (is_pm, start, end).
        """
        count = c_int()
        table = POINTER(KerCompilationSourceFolder)()
        err = self._cfunc('ker_get_compilation_units')(self._get_ker(), byref(count), byref(table))
        self._handle_ker_error(err)
        results = []
        for i in range(count.value):
            units = []
            for j in range(table[i].num_units):
                ranges = []
                for k in range(table[i].units[j].num_address_ranges):
                    ranges.append(
                        (table[i].units[j].address_ranges[k].is_pm,
                         table[i].units[j].address_ranges[k].start,
                         table[i].units[j].address_ranges[k].end)
                    )
                units.append((from_cstr(table[i].units[j].file_name), ranges))

            results.append((from_cstr(table[i].comp_dir), units))

        self._cfunc('ker_free_compilation_units')(count, table)

        return results

    def get_build_tool_versions(self):
        """
        Returns a list of strings containing the version info for each of the tools that were
        used to build the ELF file.
        """
        count = c_int()
        table = POINTER(c_char_p)()
        err = self._cfunc('ker_get_build_tool_versions')(self._get_ker(), byref(count), byref(table))
        self._handle_ker_error(err)
        rv = []
        for i in range(count.value):
            rv.append(from_cstr(table[i]))
        self._free_build_tool_versions(table)
        return rv

    def get_statements(self):
        """Returns a list of statements. Each statement is represented by the following tuple:
        (function name, address, filename, line number)
        """
        count = c_int()
        table = POINTER(KerStatement)()
        err = self._cfunc('ker_get_statements')(self._get_ker(), byref(count), byref(table))
        self._handle_ker_error(err)
        rv = {}
        for i in range(count.value):
            rv[table[i].addr] = (
                from_cstr(table[i].module),
                from_cstr(table[i].source_file),
                table[i].source_line
            )
        self._free_statements(table)
        return rv

    @staticmethod
    def _ker_variable_to_tuple(ker_var):
        """
        Convert a KerVariable to the tuple format prevailing on this API.
        """
        return (
            from_cstr(ker_var.name),
            ker_var.size_in_addressable_units,
            ker_var.addr,
            ker_var.type_id,
            from_cstr(ker_var.source_file),
            ker_var.line_num
        )

    def _get_all_vars(self):
        count = c_int()
        all_vars = POINTER(KerVariable)()
        err = self._cfunc('ker_get_variables')(self._get_ker(), byref(count), byref(all_vars))
        self._handle_ker_error(err)

        for i in range(count.value):
            yield self._ker_variable_to_tuple(all_vars[i])

        self._free_variables(all_vars)

    def get_variables(self):
        """
        Returns a dictionary of variables keyed by name. Each variable is represented by the following tuple:
        (size_in_addressable_units, address, type_id, source_file_name, source_line)
        To access multiple variable entries with the same name, use get_variable_list().
        """
        results = {}
        for full_tuple in self._get_all_vars():
            # If there are multiple entries with the same name, use the first, so that the results are consistent with
            # get_variable_by_name().
            name = full_tuple[0]
            if name in results:
                continue
            results[name] = full_tuple[1:]
        return results

    def get_variable_list(self):
        """
        Returns a list of variables. Each variable is represented by the following tuple:
        (name, size_in_addressable_units, address, type_id, source_file_name, source_line)
        """
        return list(self._get_all_vars())

    def get_constants(self):
        """
        Returns a dict of constants: name => value
        """
        count = c_int()
        table = POINTER(KerConstant)()
        err = self._cfunc('ker_get_constants')(self._get_ker(), byref(count), byref(table))
        self._handle_ker_error(err)
        rv = {}
        for i in range(count.value):
            rv[from_cstr(table[i].name)] = table[i].value
        self._free_constants(table)
        return rv

    def get_labels(self):
        """Returns a list of labels. Each label is represented by the following tuple:
        (name, address)
        """
        count = c_int()
        table = POINTER(KerLabel)()
        err = self._cfunc('ker_get_labels')(self._get_ker(), byref(count), byref(table))
        self._handle_ker_error(err)
        rv = []
        for i in range(count.value):
            s = (from_cstr(table[i].name), table[i].addr)
            rv.append(s)
        self._free_labels(table)
        return rv

    def get_all_symbols(self):
        """
        Returns a list of all symbols found in the ELF. Each symbol is represented by the following tuple:
        (name, type, is_pm, address_or_value, size_in_addressable_units, type_id, source_file_name, source_line)
            type is a value from the KerSymbolType enumeration.
            is_pm is True if addressOrValue is an address in PM memory, false if not (DM memory or not an address).
            address_or_value is the value of a constant or the address of any other symbol type
            type_id is the type ID for a variable, or 0 for any other symbol type
        Note that this function may return variables that are in PM and labels that are in DM - the caller should look
        at the is_pm field to determine how to interpret the address. For constants the addressOrValue field holds the
        value of the constant.
        """
        count = c_int()
        table = POINTER(KerSymbol)()
        err = self._cfunc('ker_get_all_symbols')(self._get_ker(), byref(count), byref(table))
        self._handle_ker_error(err)
        rv = []
        for i in range(count.value):
            s = (
                from_cstr(table[i].name),
                table[i].type,
                table[i].is_pm,
                table[i].addr_or_value,
                table[i].size_in_addressable_units,
                table[i].type_id,
                from_cstr(table[i].source_file),
                table[i].line_num
            )
            rv.append(s)
        self._free_all_symbols(table)
        return rv

    def get_architecture(self):
        """Returns a the following tuple:
        (dsp_rev, addr_width, dm_data_width, pm_data_width, dm_byte_addressing, pm_byte_addressing)
        """
        a = KerArchitecture()
        err = self._cfunc('ker_get_architecture')(self._get_ker(), byref(a))
        self._handle_ker_error(err)
        return a.dsp_rev, a.addr_width, a.dm_data_width, a.pm_data_width, a.dm_byte_addressing, a.pm_byte_addressing

    @staticmethod
    def _copy_and_pad_memdata(ker_data):
        # Copy all the whole 32-bit words.
        num_whole_words = ker_data.num_bytes // 4
        data            = ker_data.data[:num_whole_words]
        remainder       = ker_data.num_bytes % 4
        
        # Round up to nearest 32-bit word. The last word will contain zero padding if needed.
        # Avoid reading beyond the buffer provided, by treating the last bytes as ubytes, not uints.
        if remainder > 0:
            as_ubyte_p = cast(ker_data.data, POINTER(c_ubyte))
            partial_last_word = 0
            partial_last_word_offset = ker_data.num_bytes - remainder
            for i in range(remainder):
                partial_last_word |= (as_ubyte_p[partial_last_word_offset + i] << (i << 3))
            data.append(partial_last_word)
        return data
        
    def get_pm_data(self):
        """Returns a list of tuples. Each tuple corresponds to a contiguous block of data.
        The tuple fields are (bit_width, byte_addressing, start_addr, num_bytes, data)"""
        count = c_int()
        table = POINTER(KerData)()
        err = self._cfunc('ker_get_pm_data')(self._get_ker(), byref(count), byref(table))
        self._handle_ker_error(err)
        rv = []
        for i in range(count.value):
            data = self._copy_and_pad_memdata(table[i])
            s = (table[i].bit_width, table[i].byte_addressing == 1, table[i].start_addr, table[i].num_bytes, data)
            rv.append(s)
        self._free_pm_data(table)
        return rv

    def get_dm_data(self):
        """Returns a list of tuples. Each tuple corresponds to a contiguous block of data.
        The tuple fields are (bit_width, byte_addressing, start_addr, num_bytes, data)"""
        count = c_int()
        table = POINTER(KerData)()
        err = self._cfunc('ker_get_dm_data')(self._get_ker(), byref(count), byref(table))
        self._handle_ker_error(err)
        rv = []
        for i in range(count.value):
            data = self._copy_and_pad_memdata(table[i])
            s = (table[i].bit_width, table[i].byte_addressing == 1, table[i].start_addr, table[i].num_bytes, data)
            rv.append(s)
        self._free_dm_data(table)
        return rv

    def get_named_non_allocable_section_data(self, section_name):
        """
        If open_file() was called with a set of ELF section names in the argument extra_non_allocable_named_sections,
        this function may be used to retrieve the data for one of those sections.
        :return: a tuple, with the same fields as an element from the list of tuples that get_pm_data() and
        get_dm_data() return.
        """
        raw_data = POINTER(KerData)()
        err = self._cfunc('ker_get_extra_named_non_allocable_section_data')(
            self._get_ker(),
            section_name,
            byref(raw_data)
        )
        self._handle_ker_error(err)
        raw_data = raw_data[0]
        data = self._copy_and_pad_memdata(raw_data)
        result = (
            raw_data.bit_width,
            raw_data.byte_addressing == 1,
            raw_data.start_addr,
            raw_data.num_bytes,
            data
        )
        self._free_extra_named_non_allocable_section_data(raw_data)
        return result

    class KerMemberInfo(object):
    
        BITFIELD_INVALID_OFFSET = 0xFFFFFFFF
        
        """Wrapper type for a structure or union member. The fields are the same as the ctypes ("raw" C)
        type KerMember; this class adds pretty printing.

        type_id: The ID of the type of this member; this may be looked up in the full list of types.
        offset: The offset of a non-bitfield member into the parent struct, measured in addressable 
            units. For bitfield members, this is used in combination with bit_offset_from_offset to
            compute the final offset.
        bit_size: the size in bits of this member.
        bit_offset_from_offset: KerMemberInfo.BITFIELD_INVALID_OFFSET for non-bitfield members.
            For bitfield members, the offset of the bitfield into the structure may be calculated
            from:
                bitfield_position (in bits) = offset (converted to bits) + bit_offset_from_offset

            where bit_offset_from_offset is measured in bits.
        """
        def __init__(self, c_member_info):
            self.type_id                = c_member_info.type_id
            self.name                   = from_cstr(c_member_info.name)
            self.offset                 = c_member_info.offset
            self.bit_offset_from_offset = c_member_info.bit_offset_from_offset
            self.bit_size               = c_member_info.bit_size

        def list_repr(self):
            result = ["KerMemberInfo"]
            result.append("name:                   %s" % self.name)
            result.append("type id:                %d" % self.type_id)
            result.append("offset (addr. units)    %s" % self.offset)
            result.append("bit_offset_from_offset: %s" % self.bit_offset_from_offset)
            result.append("bit_size:               %s" % self.bit_size)
            return result

        def __repr__(self):
            return "\n".join(self.list_repr())
            
    class KerEnumValueInfo(object):
        """Minimal structure used to take a copy of enumeration value information returned from the DLL."""
        def __init__(self, c_value_info):
            self.name = from_cstr(c_value_info.name)
            self.value = c_value_info.value
            
    class KerFunctionParameterInfo(object):
        """Minimal structure used to take a copy of function parameter information returned from the DLL."""
        def __init__(self, c_value_info):
            self.name = from_cstr(c_value_info.name)
            self.type_id = c_value_info.type_id
            
    class KerTypeInfo(object):
        """
        Represents a type present in the debugging information in the ELF file. The following fields are available:
        (form, type_id, name, size_in_addressable_units, ref_type_id, member_count, members, array_count, array_type_id,
         enum_values, return_type, parameters).
        Each member is represented by a KerMemberInfo object as an element of "members"
        """
        FORM_BASE     = 0
        FORM_STRUCT   = 1
        FORM_UNION    = 2
        FORM_ARRAY    = 3
        FORM_TYPEDEF  = 4
        FORM_POINTER  = 5
        FORM_CONST    = 6
        FORM_VOLATILE = 7
        FORM_ENUM     = 8
        FORM_FUNCTION = 9
        
        FORM_TO_STRING = { 
            FORM_BASE : '0 (base)',
            FORM_STRUCT : '1 (struct)',
            FORM_UNION : '2 (union)',
            FORM_ARRAY : '3 (array)',
            FORM_TYPEDEF : '4 (typedef)',
            FORM_POINTER : '5 (pointer)',
            FORM_CONST : '6 (const)',
            FORM_VOLATILE : '7 (volatile)',
            FORM_ENUM : '8 (enum)',
            FORM_FUNCTION : '9 (function)'
        }
        
        def __init__(self, c_type_info):
            self.form          = c_type_info.form
            self.type_id       = c_type_info.type_id
            self.name          = from_cstr(c_type_info.name)
            self.size_in_addressable_units = c_type_info.size_in_addressable_units
            self.ref_type_id   = c_type_info.ref_type_id
            self.member_count  = c_type_info.member_count  # Redundant, but keeping for now.
            self.members       = [Ker.KerMemberInfo(c_type_info.members[i]) for i in range(c_type_info.member_count)]
            self.is_declaration= c_type_info.is_declaration
            self.array_count   = c_type_info.array_count
            self.array_type_id = c_type_info.array_type_id
            self.enum_values   = [Ker.KerEnumValueInfo(c_type_info.enum_values[i])
                                  for i in range(c_type_info.enum_value_count)]
            self.return_type   = c_type_info.return_type
            self.parameters    = [Ker.KerFunctionParameterInfo(c_type_info.parameters[i])
                                  for i in range(c_type_info.parameter_count)]
        
        def __repr__(self):
            result = ["KerTypeInfo"]
            result.append("name:                     %s" % self.name)
            result.append("form:                     %s" % self.FORM_TO_STRING[self.form])
            result.append("type id:                  %d" % self.type_id)
            result.append("size (addressable units): %d" % self.size_in_addressable_units)
            result.append("reference type id:        %s" %
                          ("<none>" if self.ref_type_id == INVALID_TID else self.ref_type_id))
            result.append("array count:              %d" % self.array_count)
            result.append("array type id:            %s" %
                          ("<none>" if self.array_type_id == INVALID_TID else self.array_type_id))
            result.append("only a declaration:       %s" % ("yes" if self.is_declaration else "no"))
            result.append("member count:             %d" % self.member_count)
            if self.member_count > 0:
                result.append("members:")
                for m in self.members:
                    indented = list(map(lambda line: "    " + line, m.list_repr()))
                    result += indented
            if len(self.enum_values) > 0:
                result.append("enumerators:")
                for v in self.enum_values:
                    result.append("    %s = %d" % (v.name, v.value))
            if self.form == self.FORM_FUNCTION:
                result.append("return type:              %s" %
                              ("<none>" if self.return_type == INVALID_TID else self.return_type))
                if len(self.parameters) > 0:
                    result.append("parameters:")
                    for p in self.parameters:
                        if p.type_id != INVALID_TID or p.name != "...":
                            result.append("    %s(type %s)" %
                                          (p.name, "<none>" if p.type_id == INVALID_TID else p.type_id))
                        else:
                            result.append("    ...")

            return "\n".join(result)
            
    def get_types(self):
        """Returns a list of KerTypeInfo objects, each of which represent a type present in the 
        debugging information in the ELF file. kalelfreader returns the types ordered by type id.
        """
        count = c_int()
        table = POINTER(KerType)()
        err = self._cfunc('ker_get_types')(self._get_ker(), byref(count), byref(table))
        self._handle_ker_error(err)
        
        try:
            results = [Ker.KerTypeInfo(table[i]) for i in range(count.value)]
            return results
        finally:
            self._free_types(table, count)

    def get_reports(self):
        """Returns a list of reports. Each report has a severity (0-1-2 information-warning-error) and text
        (severity, text)
        """
        count = c_int()
        table = POINTER(KerReport)()
        err = self._cfunc('ker_get_reports')(self._get_ker(), byref(count), byref(table))
        self._handle_ker_error(err)
        rv = []
        for i in range(count.value):
            t = table[i]
            s = (t.severity, from_cstr(t.text))
            rv.append(s)
        self._free_reports(table)
        return rv

    def get_enums(self):
        """Returns a dict of enums. Each enum is itself a dict mapping name to value.
        """
        count1 = c_int()
        table1 = POINTER(KerEnum)()
        count2 = c_int()
        table2 = POINTER(KerEnum)()
        err = self._cfunc('ker_get_enums')(self._get_ker(), byref(count1), byref(table1), byref(count2), byref(table2))
        self._handle_ker_error(err)
        enums = {}
        for i1 in range(count1.value):
            record1 = table1[i1]
            enum_name = from_cstr(record1.name)
            enum_offset = record1.value
            next_enum_offset = count2.value
            if i1 < count1.value - 1:
                next_enum_offset = table1[i1 + 1].value
            enum_vals = {}
            for j in range(enum_offset, next_enum_offset):
                record2 = table2[j]
                enum_vals[from_cstr(record2.name)] = record2.value
            enums[enum_name] = enum_vals
        self._free_enums(table1, table2)
        return enums
        
    def loadelf(self, filename):
        """This function returns:
            dsp_rev         int    DSP revision
            constants       dict   (value) by name
            source_lines    dict   (function_name, source_file, line_num) by address
            variables       dict   (size, address, type_id) by name
            labels          list   (name, address)
            dm_data         dict   (value) of static DM by address
            pm_data         dict   (value) of static PM by address
            machine_id      int    machine ID from the elf header e_machine field.
            is_big_endian   int    1 if the architecture is big-endian (most significant byte at lowest memory address)
            addr_width      int    address width in bits for either PM or DM
            dm_data_width   int    bits in one word of DM
            pm_data_width   int    bits in one word of PM
            types           list   (form, type_id, name, byte_size, ref_type_id, member_count, mbr_tids, mbr_names,
                                    array_count, array_type_id) by type_id
            reports         list   warning/error strings from elf file load
            enums           dict   each entry key is the enum name, value is a dict of enum values (keyed on name)
            elf_sec_hdrs    dict   (start_addr, num_bytes, type) by name
            funcs           dict   (source_file, line_num, low_pc, high_pc) by name
            dm_octet_addressing bool True if target's DM is octet addressable, False if not.
            pm_octet_addressing bool True if target's PM is octet addressable, False if not.
            all_symbols     dict    (type, is_pm, addressOrValue, size, type_id, source_file_name, source_line) by name;
                                    values as in return value of get_all_symbols
        """

        # Load and parse the elf file.
        self.open_file(filename)

        try:
            (dsp_rev, addr_width, dm_data_width, pm_data_width, dm_octet_addressing, pm_octet_addressing) = \
                self.get_architecture()
            
            constants     = self.get_constants()
            source_lines  = self.get_statements()
            variables     = self.get_variables()
            labels        = self.get_labels()
            dm_data       = {}
            pm_data       = {}
            machine_id    = self.get_machine_id()
            types         = self.get_types()
            enums         = self.get_enums()
            reports       = self.get_reports()
            is_big_endian = self.is_big_endian()
            elf_sec_hdrs  = self.get_elf_section_headers()

            # Retrieve the DM data.
            for (bit_width, byte_addressing, start_addr, num_bytes, data) in self.get_dm_data():
                # We store each word as 4 bytes in the Elf file, regardless of whether it is 32-bit, 24-bit or 16-bit
                entries = (num_bytes + 3) // 4
                inc = 1
                if byte_addressing != 0:
                    inc = 4
                for j in range(entries):
                    dm_data[start_addr + j * inc] = data[j]

            # Retrieve the PM data.
            for (bit_width, byte_addressing, start_addr, num_bytes, data) in self.get_pm_data():
                # We store each word as 4 bytes in the Elf file, regardless of whether it is 32-bit, 24-bit or 16-bit
                entries = (num_bytes + 3) // 4
                inc = 1
                if byte_addressing != 0:
                    inc = 4
                for j in range(entries):
                    pm_data[start_addr + j * inc] = data[j]

            # Retrieve functions.
            funcs = {}
            for i in range(self.get_function_count()):
                f = self.get_function_by_index(i + 1)
                funcs[f.function_name] = (f.source_file, f.line_num, f.low_pc, f.high_pc)
                
            # Store all symbols in a dictionary.
            # get_all_symbols returns tuple: (name, type, is_pm, address_or_value, size_in_addressable_units, type_id,
            # source_file_name, source_line)
            all_symbols = {}
            for sym in self.get_all_symbols():
                all_symbols[sym[0]] = sym[1:]

            # Warn on overlapping statements.
            if self.is_overlapping_statements():
                print('Warning: possible overlapping pm regions, e.g.:')
                table = POINTER(KerStatement)()
                err = self._cfunc('ker_get_overlapping_statements')(self._get_ker(), byref(table))
                self._handle_ker_error(err)
                for i in range(2):
                    print('  module:%s PC:%08x source_line:%d source_file:%s' %
                          (
                              from_cstr(table[i].module),
                              table[i].addr,
                              table[i].source_line,
                              from_cstr(table[i].source_file)
                          )
                    )
                self._free_statements(table)

        finally:
            # Free the parsed elf file.
            self.close_file()

        # Return the ELF file data.
        return (dsp_rev, constants, source_lines, variables, labels,
                dm_data, pm_data, machine_id, is_big_endian, addr_width,
                dm_data_width, pm_data_width, types, enums, reports,
                elf_sec_hdrs, funcs, dm_octet_addressing, pm_octet_addressing,
                all_symbols)
