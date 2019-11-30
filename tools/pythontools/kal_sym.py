# Copyright (c) 2014 - 2018 Qualcomm Technologies International, Ltd

from __future__ import print_function
import bisect
import os
import kalelfreader_lib_wrappers
from kalelfreader_lib_wrappers import Ker, KerSymbolType
import re
import sys
from glob import glob
import read_klo
from ctypes import create_string_buffer
from struct import pack_into, unpack_from

try:
    # noinspection PyUnresolvedReferences
    integer_types = (int, long)
except NameError:
    integer_types = (int,)


class NoSymbolsError(Exception):
    def __str__(self):
        return "Symbol information not yet loaded. Call sym.load()"


class InvalidChoice(Exception):
    pass


class WindowsPathError(RuntimeError):
    pass


class SymbolSearchError(Exception):
    def __init__(self, sym_type, sym_name, num_found):
        self.sym_type = sym_type
        self.sym_name = sym_name
        self.num_found = num_found

    def __str__(self):
        if self.num_found < 1:
            return "Could not find %s matching search string '%s'" % (self.sym_type, self.sym_name)
        else:
            return "Expected one %s matching search string '%s'. Found %d." % \
                   (self.sym_type, self.sym_name, self.num_found)


def toHex(data):
    """
    Returns a string representing an array/list of data in hexadecimal.
    """
    try:
        rv = []
        for i in data:
            rv.append("0x%x" % i)
        return " ".join(rv)
    except TypeError:
        return "0x%x" % data
 

def read_dm_string(addr, kal=None, dm_data_width=None, read_dm_block=None):
    if dm_data_width is None:
        if kal is None:
            raise Exception("Either kal or dm_data_width must not be None")
        dm_data_width = kal.sym.dm_data_width
    if read_dm_block is None:
        if kal is None:
            raise Exception("Either kal or read_dm_block must not be None")
    word_bytes = dm_data_width // 8
    a0 = addr & ~(word_bytes - 1)
    if read_dm_block:
        w = read_dm_block(a0, 1)[0]
    else:
        w = kal.dm[a0]
    return 'string at ' + toHex(a0) + ' word_bytes ' + ("%d" % word_bytes) + " w=" + toHex(w)
    

class Sym(object):
    """
    This class can be used to load a symbol file (either ELF or KLO), and query details of constants, variables,
    code labels and modules.
    """

    def __init__(self, core):
        self._core = core
        self._func_lookup_by_pc = None
        self._func_lookup_by_pc_keys = None

    def assert_have_symbols(self):
        if not self.have_symbols():
            raise NoSymbolsError()

    def have_symbols(self):
        return 'variables' in self.__dict__

    @staticmethod
    def _matches_simple_escape_sequences(test_str):
        # This doesn't handle escape sequences with "arguments", e.g.
        # Unicode characters.
        simple_escapes = [
            '\'',
            '\"',
            '\a',
            '\b',
            '\f',
            '\n',
            '\r',
            '\t',
            '\v',
        ]
        for esc in simple_escapes:
            index = test_str.find(esc)
            if index >= 0:
                return index

        return -1

    def load(self, filename = ""):
        """
        Parses the symbol information in the specified KLO/ELF (.klo, .elf) file and stores it
        for the other Kalimba debug functions to make use of.
        If the specified filename is a directory, that directory is searched for KLO/ELF files.
        If no KLO/ELF file is specified, we search the current directory for one.
        """
        path = os.getcwd()
        if os.path.isdir(filename):
            print("Searching {0} for KLO/ELF files...".format(filename))
            path = filename
            filename = ""

        if filename == "":
            # The caller didn't specify a file, so lets go fishing for one
            file_list = glob(os.path.join(path, "*.klo"))
            file_list.extend(glob(os.path.join(path, "*.elf")))
            file_index = 0
            
            if len(file_list) == 0:
                raise IOError("No KLO/ELF files present in this directory")

            if len(file_list) > 1:
                print("Enter the number of the KLO/ELF file to load:\n")
                for index, symbol_file in enumerate(file_list):
                    print("{0}. {1}".format(index + 1, symbol_file))
                print("\n? ", end=' ')
                try:
                    user_input = int(sys.stdin.readline().strip())
                    file_index = user_input - 1
                    # noinspection PyStatementEffect
                    file_list[file_index]  # Test validity of index
                except IndexError:
                    raise InvalidChoice

            filename = file_list[file_index]

        print("Loading", filename)

        if not os.path.exists(filename):
            if sys.platform.startswith("win32"):
                esc_index = self._matches_simple_escape_sequences(filename)
                if esc_index >= 0:
                    window_start = max(esc_index - 5, 0)
                    window_end = min(esc_index + 6, len(filename))
                    raise WindowsPathError(
                        "File not found:\n  {0}\n"
                        "Likely escaped character found around '{1}'. Escape backslashes in Windows paths, use a raw "
                        "string ('r' prefix), or use forward slashes.".format(
                            filename,
                            filename[window_start:window_end]
                        )
                    )

            raise IOError("File not found:\n  {0}".format(filename))

        if filename[-4:] == ".klo":
            (self.constants, self.source_lines, self.variables,
             self.labels, self.static_dm, self.static_pm) = read_klo.load_klo_file(filename)
            self.dm_data_width = 24
            self.pm_data_width = 32
        elif filename[-4:] == ".elf" or filename[-2:] == ".o":
            ker = kalelfreader_lib_wrappers.Ker()
            (self.symfile_dsp_rev,  self.constants,     self.source_lines,
             self.variables,        self.labels,        self.static_dm,
             self.static_pm,        machine_id,         self.is_big_endian,
             self.addr_width,       self.dm_data_width, self.pm_data_width,
             self.types,            self.enums,         self.loadsym_reports,
             self.elf_sec_headers,  self.funcs, 
             self.dm_octet_addressing, self.pm_octet_addressing,
             self.all_symbols) = ker.loadelf(filename)
        else:
            raise IOError("Specified filename must end with .klo, .elf or .o.\nAlternatively, just specify the "
                          "directory containing the symbol file and a search will be performed.")

        print("Symbols loaded")

    class FileLineAndPcResult(object):
        def __init__(self, pc, file, line):
            self.pc = pc
            self.file = file
            self.line = line
            
        def __repr__(self):
            return "Program address: %#x. File: %s. Line number: %d" % (self.pc, self.file, self.line)
    
    def fileline_to_pc(self, filename, line_num):
        """
        Searches for source lines that match the specified filename and line number.
          'filename' - A string to search for in file names. Substring matches will be included.
                       Regular expressions are supported. Pass "" to match all files.
          'line_num' - An integer line number. Passing -1 will match all lines.
        Returns a list of FileLineAndPcResult instances, containing program address, filename, and 
        line number information.
        """
        self.assert_have_symbols()
        
        matches = []
        for (pc, line_info) in self.source_lines.items():
            if re.search(filename, line_info[1]):
                if line_info[2] == line_num or line_num == -1:
                    matches.append(Sym.FileLineAndPcResult(pc, line_info[1], line_info[2]))
        
        if len(matches) == 0:
            raise Exception("Couldn't find any matches for '%s', %d" % (filename, line_num))
        return matches

    def _get_final_addr(self, addr):
        if isinstance(addr, str):
            if addr.lower() == "pc":
                addr = self._core.reg.read_register(addr)
            else:
                raise TypeError("address must be numerical, or 'pc'")
        return addr

    def modname(self, addr="pc"):
        """
        Returns the name of the module or function at the program address provided.
        If no address is specified, the value read from the PC is used.
        """
        self.assert_have_symbols()
        
        addr = self._get_final_addr(addr)

        if hasattr(self, 'funcs'):
            return self._elf_accelerated_modname(addr)
        else:
            # We loaded a KLO file, therefore have to use source_lines
            # Is this a valid address?
            if addr in self.source_lines:
                return self.source_lines[addr][0]
                
        raise ValueError("Could not find a module matching the address %#x" % addr)

    class PcPairAndFuncName(object):
        def __init__(self, func_name, low_pc, high_pc):
            self.func_name = func_name
            self.low_pc = low_pc
            self.high_pc = high_pc

    def _elf_accelerated_modname(self, addr):
        """
        Provide the lookup for modname without doing a brute-force search of PC ranges each time.
        """
        if self._func_lookup_by_pc is None:
            # Lazy initialise lookup table
            if not hasattr(self, 'funcs'):
                raise RuntimeError("_elf_accelerated_modname only works with ELF files, not KLO")

            self._func_lookup_by_pc = []

            for name, func_tuple in self._core.sym.funcs.items():
                self._func_lookup_by_pc.append(
                    Sym.PcPairAndFuncName(name, func_tuple[2], func_tuple[3])
                )

            self._func_lookup_by_pc.sort(key = lambda item: item.low_pc)
            self._func_lookup_by_pc_keys = list(map(lambda item: item.low_pc, self._func_lookup_by_pc))

        # Find rightmost value less than or equal to addr.
        insert_point = bisect.bisect_right(self._func_lookup_by_pc_keys, addr)
        if insert_point:
            result = self._func_lookup_by_pc[insert_point - 1]
            # Check that the function encloses address by looking at the upper bound:
            # ">" is correct: the high PC value is the first location past the last
            # instruction of the subprogram, according to the DWARF spec.
            if result.high_pc > addr:
                return result.func_name

        raise ValueError("Could not find a module matching the address {0:#x}".format(addr))

    def print_source_lines(self):
        """Prints all debug source line information. Columns correspond to 
        (program address, module/function name, file name, line number)."""
        for pc, line_info in self.source_lines.items():
            print("%08x %s %s %d" % (pc, line_info[0], line_info[1], line_info[2]))

    def print_mems(self, mems, data_width):
        """
        Takes a dictionary representing some memory contents and pretty-prints it.
        Each dictionary key should be an address, and each value should be a word of data.
        The target data width should be in bits.
        """

        def clamp(x):
            return '.' if x < 32 or x > 127 else x

        result_str = "%%0%dX %%0%dX %%s" % (self.addr_width // 4, data_width // 4)
        for addr, word in sorted(mems.items()):
            c3 = clamp((word & 0xff000000) >> 24)
            c2 = clamp((word & 0x00ff0000) >> 16)
            c1 = clamp((word & 0x0000ff00) >> 8)
            c0 = clamp(word & 0x000000ff)
            val = "%c %c %c %c" % (c3, c2, c1, c0)
            if data_width == 24:
                val = val[2:]

            print(result_str % (addr, word, val))

    def print_static_dm(self):
        self.print_mems(self.static_dm, self.dm_data_width)

    def print_static_pm(self):
        self.print_mems(self.static_pm, self.pm_data_width)

    def readval(self, name_pattern):
        """
        Searches registers, constants and variables for a match against the specified regular
        expression. Internally, this function calls varfind() and constfind(). See the docs
        for those functions for more information.
        Returns:
          a list of (name, [values], address) of all matches.
          The "address" element will be either:
          - the address of the variable
          - the literal "const" if the match is a const
          - the literal "reg" if it is a register.

        Notes for octet-addressable Kalimbas (CSRA68100 family, etc):

          Each element in the list of values is a word. Therefore, for octet-addressable Kalimbas, the length of the
          returned list will not be the same as the variable size reported by varfind() -- the latter is in addressable
          units.

          For variables whose size is not a whole number of words the whole last word is read and returned (of which
          some octets will not be part of the variable).
          In such cases, the true size of the variable can be retrieved by calling varfind().
        """
        class ReadValResults(list):
            """
            Simple sub class of the builtin list class used to store results from readval().
            Provides pretty-printing.
            """
            def __repr__(self):
                results = []
                for element in self:

                    address_or_special = element[2]

                    # Py 2/3
                    try:
                        basestring
                    except NameError:
                        basestring = str

                    if isinstance(address_or_special, basestring):
                        if address_or_special == "val":
                            address = element[0]
                            name = "Value (&%d - 0x%06X)" % (address, address)
                        elif address_or_special == "reg":
                            name = element[0]
                        else:
                            name = element[0] + " 'CONST'"
                    else:
                        var_name = element[0]
                        address  = address_or_special
                        var_size = len(element[1])
                        name = "%s[%d] (&%d - 0x%06X)" % (var_name, var_size, address, address)

                    this_result = "%-50s - " % name
                    var_data = element[1]
                    this_result += ("\n" + " " * 53).join("%7d (0x%06X)" % (datum, datum) for datum in var_data)
                    results.append(this_result)

                return "\n".join(results)

            def addval(self, lit_address, value):
                self.addgeneric(lit_address, value, "val")

            def addreg(self, name, value):
                self.addgeneric(name, value, "reg")

            def addconst(self, name, value):
                self.addgeneric(name, value, "const")

            def adddata(self, name, value, addr):
                self.addgeneric(name, value, addr)

            def addgeneric(self, name, value, address_or_special):
                try:
                    len(value)
                except TypeError:
                    value = [value]
                self.append((name, value, address_or_special))

        rv = ReadValResults()
        if isinstance(name_pattern, integer_types):
            literal_address = name_pattern
            rv.addval(literal_address, self._core.dm[literal_address])
        else:
            # Handle requests for registers by name
            if self._core.reg.is_register(name_pattern):
                val = self._core.reg.read_register(self._core.reg.get_register_id(name_pattern))
                rv.addreg(name_pattern, val)

            # It's not a register, so try to match a variable or a constant.
            symbol_matches = self.varfind(name_pattern)
            if symbol_matches:
                for variable in symbol_matches:
                    var_name                   = variable[0]
                    var_size_addressable_units = variable[1]
                    var_address                = variable[2]

                    # _read_dm_block needs a number of words. If the variable isn't an exact number of words, we
                    # over-read, so all the data is returned. The rest of the code is word-wise, so it doesn't attempt
                    # to prune irrelevant octets.
                    if self.dm_octet_addressing:
                        word_size_octets = self.dm_data_width >> 3
                        var_size_words = var_size_addressable_units // word_size_octets
                        if var_size_addressable_units % word_size_octets != 0:
                            var_size_words += 1
                    else:
                        var_size_words = var_size_addressable_units

                    variable_data = self._core._read_dm_block(var_address, var_size_words)
                    rv.adddata(var_name, variable_data, var_address)

            symbol_matches = self.constfind(name_pattern)
            if symbol_matches:
                for constant in symbol_matches:
                    constant_name  = constant[0]
                    constant_value = constant[1]
                    rv.addconst(constant_name, constant_value)

        rv.sort()
        return rv

    class FindResults(list):
        """Sub class of the builtin list class, used to contain results from
        searching symbols.

        Extra functionality provided by the class:
        - intelligent representation method that presents the search results
          in a legible format.
        - friendly attributes, e.g. 'foo.name', note these will return the
          relevant attribute for the first and only search result, if
          multiple results match then a SymbolSearchError is raised.
        """
        def __init__(self, search, kind, fields, *args):
            self.__dict__["_search"] = search
            self.__dict__["_kind"]   = kind
            self.__dict__["_fields"] = fields
            list.__init__(self, *args)

        def __repr__(self):
            format_str = "%-50s : %d - 0x%06X"
            return "\n".join(
                map(lambda name_and_val:
                        format_str % (name_and_val[0], name_and_val[1], name_and_val[1]),
                    self
                )
            )

        def __getattr__(self, attr):
            if len(self) != 1:
                raise SymbolSearchError(self._kind, self._search, len(self))
            if attr in self._fields:
                return self[0][self._fields[attr]]
            raise AttributeError("%s has no attribute '%s'" % (str(self.__class__), attr))

    def constfind(self, name_pattern):
        """
        Searches for constants matching the specified regular expression pattern, returning
        a list of tuples, one for each match. The tuple is (name, value).


        Note: $ is treated specially. Normally $ in a regular expression
        tells the search to look for the end of a line. However, because global Kalimba
        variables look like "$foo", we escape the $ before passing it to the regular
        expression matching engine. The escaping only occurs if the $ is the first
        character in the search string.
        """
        class ConstFindResults(Sym.FindResults):
            """Used to contain results from searching the constants.

            Extra functionality provided by the class:
            - intelligent representation method that presents the search results
              in a legible format.
            - "name", and "value" attributes, note these will return the
              relevant attribute for the first and only search result, if
              multiple results match then a SymbolSearchError is raised.
            """
            def __init__(self, search, *args):
                Sym.FindResults.__init__(self, search, "constant", {"name" : 0, "value" : 1}, *args)

        self.assert_have_symbols()

        search_string = name_pattern.lower()
        results = ConstFindResults(search_string)
        if len(search_string) > 0 and search_string[0] == '$':
            search_string = "\\" + search_string

        for name, value in self.constants.items():
            if re.search(search_string, name.lower()):
                results.append((name, value))

        results.sort()
        return results

    def labelfind(self, search_string):
        """
        Searches for labels matching the specified regular expression pattern, or for
        labels at the specified address. Returns a list of tuples, one for each match.
        The tuple is (name, address).


        Note: $ is treated specially in text searches. Normally $ in a regular expression
        tells the search to look for the end of a line. However, because global Kalimba
        variables look like "$foo", we escape the $ before passing it to the regular
        expression matching engine. The escaping only occurs if the $ is the first
        character in the search string.
        """
        class LabelFindResults(Sym.FindResults):
            """Used to contain results from searching the labels.

            Extra functionality provided by the class:
            - intelligent representation method that presents the search results
              in a legible format.
            - "name", and "addr" attributes, note these will return the relevant
              attribute for the first and only search result, if multiple
              results match then a SymbolSearchError is raised.
            """
            def __init__(self, search, *args):
                Sym.FindResults.__init__(self, search, "label", {"name" : 0, "addr" : 1}, *args)

        if "labels" not in self.__dict__:
            print("Label info not yet loaded. Call load()")
            return

        results = LabelFindResults(search_string)
        if isinstance(search_string, integer_types):
            # Treat search string is an address
            addr = int(search_string)

            for i in self.labels:
                # i is (labelname, address)
                if i[1] == addr:
                    results.append(i)
        else:
            if len(search_string) > 0 and search_string[0] == '$':
                search_string = "\\" + search_string

            # Treat search string as a regexp to match against var name
            search_string = search_string.lower()
            for i in self.labels:
                # i is (labelname, address)
                if re.search(search_string, i[0].lower()):
                    results.append(i)

        results.sort()
        return results

    def varfind(self, search_string):
        """
        Searches for variables matching the specified regular expression pattern, or for
        variables at the specified address. Returns a list of tuples, one for each match.
        The tuple is (name, size_in_target_addressable_units, address).

        Note: $ is treated specially in text searches. Normally $ in a regular expression
        tells the search to look for the end of a line. However, because global Kalimba
        variables look like "$foo", we escape the $ before passing it to the regular
        expression matching engine. The escaping only occurs if the $ is the first
        character in the search string.
        """
        class VarFindResults(Sym.FindResults):
            """
            Used to contain results from searching the variables.

            Named fields: "name", "size_in_addressable_units" and "addr".
            These are also accessible via list indices 0, 1, 2 (respectively).
            Note: these will return the relevant attribute for the first and only search result; if
            multiple results match then a SymbolSearchError will be raised.
            """
            def __init__(self, search, *args):
                Sym.FindResults.__init__(
                    self,
                    search,
                    "variable",
                    {"name": 0, "size_in_addressable_units": 1, "addr": 2},
                    *args
                )

            def __repr__(self):
                formatted_list = []
                for element in self:
                    name                      = element[0]
                    size_in_addressable_units = element[1]
                    address                   = element[2]

                    name_str = "%s[%d]" % (name, size_in_addressable_units)
                    formatted_list.append("%-50s : &%d - 0x%06X" % (name_str, address, address))

                return "\n".join(formatted_list)

        self.assert_have_symbols()

        results = VarFindResults(search_string)
        if isinstance(search_string, integer_types):
            # Treat search string as an address
            search_address = search_string

            for var_name, variable in self.variables.items():
                # Translate to a decipherable notation.
                var_size_addressable_units = variable[0]
                var_address                = variable[1]

                offset_from_search_address = search_address - var_address

                # Is the search address between the start and end addresses of this variable?
                if offset_from_search_address >= 0 and offset_from_search_address < var_size_addressable_units:
                    results.append((var_name, var_size_addressable_units, var_address))
        else:
            if len(search_string) > 0 and search_string[0] == '$':
                search_string = "\\" + search_string

            # Treat search string as a regexp to match against var name
            search_string = search_string.lower()
            for var_name, variable in self.variables.items():
                if re.search(search_string, var_name.lower()):
                    var_size_addressable_units = variable[0]
                    var_address                = variable[1]

                    results.append((var_name, var_size_addressable_units, var_address))

        results.sort()
        return results

    def anysymbolfind(self, search_string):
        """
        Searches for any symbol matching the specified regular expression pattern, or for
        symbols at the specified address. Returns a list of tuples, one for each match.
        The tuple is (name, address).

        Note that if you search by address then it might find results in both PM and DM.

        Note: $ is treated specially in text searches. Normally $ in a regular expression
        tells the search to look for the end of a line. However, because global Kalimba
        variables look like "$foo", we escape the $ before passing it to the regular
        expression matching engine. The escaping only occurs if the $ is the first
        character in the search string.
        """
        class SymbolFindResults(Sym.FindResults):
            """Used to contain results from searching the symbols.

            Named fields: "name", "type", "is_pm", "address_or_value", "size_in_addressable_units".
                type is a value from the KerSymbolType enumeration (variable, label, constant or other)
                is_pm is True if addressOrValue is an address in PM memory, False if not (DM memory or not an address).
                address_or_value is the value of a constant or the address of any other symbol type               
            Note these will return the relevant attribute for the first and only search
            result, if multiple results match then a SymbolSearchError is raised.
            """
            def __init__(self, search, *args):
                Sym.FindResults.__init__(
                    self,
                    search,
                    "symbol",
                    {"name" : 0, "type": 1, "is_pm": 2, "address_or_value": 3, "size_in_addressable_units" : 4},
                    *args
                )

            def __repr__(self):
                formatted_list = []
                for element in self:
                    name                      = element[0]
                    type                      = element[1]
                    is_pm                     = element[2]
                    address_or_value          = element[3]
                    size_in_addressable_units = element[4]
                    
                    if type == KerSymbolType.KER_SYM_VARIABLE:
                        name_str = "%s[%d]" % (name, size_in_addressable_units)
                        memside_str = "PM" if is_pm else "DM"
                        formatted_list.append(
                            "VARIABLE: %-50s : %s &%d - 0x%06X" %
                            (name_str, memside_str, address_or_value, address_or_value)
                        )
                    elif type == KerSymbolType.KER_SYM_LABEL:
                        memside_str = "PM" if is_pm else "DM"
                        formatted_list.append(
                            "LABEL:    %-50s : %s &%d - 0x%06X" %
                            (name, memside_str, address_or_value, address_or_value)
                        )
                    elif type == KerSymbolType.KER_SYM_CONSTANT:
                        formatted_list.append(
                            "CONSTANT: %-50s : &%d - 0x%06X" %
                            (name, address_or_value, address_or_value)
                        )
                    else:
                        formatted_list.append(
                            "OTHER:    %-50s : &%d - 0x%06X" %
                            (name, address_or_value, address_or_value)
                        )

                return "\n".join(formatted_list)

        if "all_symbols" not in self.__dict__:
            print("Symbol info not yet loaded. Call load()")
            return

        results = SymbolFindResults(search_string)
        if isinstance(search_string, integer_types):
            # Treat search string is an address
            addr = int(search_string)

            for sym_name, sym_data in self.all_symbols.items():
                # sym_data is (type, is_pm, addressOrValue, size_in_addressable_units, type_id, source_file_name, source_line)
                if sym_data[0] != KerSymbolType.KER_SYM_CONSTANT and sym_data[2] == addr:
                    results.append((sym_name, sym_data[0], sym_data[1], sym_data[2], sym_data[3]))
        else:
            if len(search_string) > 0 and search_string[0] == '$':
                search_string = "\\" + search_string

            # Treat search string as a regexp to match against var name
            search_string = search_string.lower()
            for sym_name, sym_data in self.all_symbols.items():
                # sym_data is (type, is_pm, addressOrValue, size_in_addressable_units, type_id, source_file_name, source_line)
                if re.search(search_string, sym_name.lower()):
                    results.append((sym_name, sym_data[0], sym_data[1], sym_data[2], sym_data[3]))

        results.sort()
        return results

    def writeval(self, name_pattern, val):
        """
        Writes a single word value into a variable or register.
        :param name_pattern: the name of a variable (as would be found by varfind(), or the name of a register.
        :param val: the value to write. Only a single word is supported.
        """
        if self._core.reg.is_register(name_pattern):
            self._core.reg.write_register(self._core.reg.get_register_id(name_pattern), val)
        else:
            symbol_addr = self.varfind(name_pattern).addr
            self._core._write_dm_block(symbol_addr, [val])

    def printvar(self, name):
        """
        Pretty-print a variable's contents, with support for complex types.

        We want to print complex data types so they look like the (struct)
        definitions: this means we have to store name information and recurse
        up the typedef chain until we find the form of each variable/member,
        so we can produce output such as:
          struct pos {
            int x = 3
            int y = 4
          }
        """
        class TypeDisplayer(object):
            def __init__(self, kal, enums, variables, types, dm_octet_addressing, dm_data_width):
                self._kal = kal
                self.enums  = enums
                self.variables = variables
                self.types = types
                self.dm_octet_addressing = dm_octet_addressing
                self.dm_data_width_octets = dm_data_width // 8
                self.pad = ''
                self.type_name = ''
                self.variable_name = ''
                self.named = False

            def _get_variable_subword_mode(self, type):
                # If necessary, wind back to the start of the enclosing word:
                offset_into_word = self.addr % self.dm_data_width_octets
                actual_read_addr = self.addr - offset_into_word
                # .. And see if we need to chop off any octets off the end:
                leftover_octets = (self.addr + type.size_in_addressable_units) % self.dm_data_width_octets
                # Determine how many words we need to read to cover everything, and read them:
                octets_to_read = offset_into_word + type.size_in_addressable_units + leftover_octets
                words_to_read = (octets_to_read + (self.dm_data_width_octets - 1)) // self.dm_data_width_octets
                values = self._kal._read_dm_block(actual_read_addr, words_to_read)
                
                # Chop off the octets which are not part of this variable, at the start and the end
                if offset_into_word != 0 or leftover_octets != 0:
                    octet_buffer = create_string_buffer(words_to_read * self.dm_data_width_octets)
                    pack_into('I' * words_to_read, octet_buffer, 0, *values)
                    
                    octet_buffer = octet_buffer[offset_into_word:]
                    if leftover_octets > 0:
                        octet_buffer = octet_buffer[:-leftover_octets]
                        
                    # Pad with zeros, so we can unpack a whole number of words
                    for i in range(offset_into_word + leftover_octets):
                        octet_buffer += '\x00'
                    
                    num_to_unpack = max(type.size_in_addressable_units, self.dm_data_width_octets) // self.dm_data_width_octets
                    unpacked = unpack_from('I' * num_to_unpack, octet_buffer)
                    values = list(unpacked)
                return values
                
            def recursive_print(self, type_id):
                self.level = self.level + 1
                if self.level > 15:
                    raise Exception("TypeDisplayer reached maximum recursion level")
                    
                type = self.types[type_id]
                # Otherwise kalelfreader is not ordering types by ID, which is assumed elsewhere:
                assert(type_id == type.type_id)

                if type.form == Ker.KerTypeInfo.FORM_BASE:
                    
                    if type.name == 'char':
                        addr = self._kal._read_dm_block(self.addr, 1)[0]
                        print(
                            "%s%s %s %s %s" %
                            (
                                self.pad,
                                type.name if not self.named else "",
                                self.type_name,
                                self.variable_name,
                                read_dm_string(addr, kal = self._kal)
                            )
                        )
                    else:
                        if self.dm_octet_addressing:
                            values = self._get_variable_subword_mode(type)
                        else:
                            values = self._kal._read_dm_block(self.addr, type.size_in_addressable_units)
                            
                        # Convert to normal Python types, rather than ctypes types:
                        values = list(map(lambda v: int(v), values))
                        print(
                            "%s%s %s %s = %s" %
                            (
                                self.pad,
                                type.name if not self.named else "",
                                self.type_name,
                                self.variable_name,
                                values
                            )
                        )
                    self.addr += type.size_in_addressable_units

                elif type.form == Ker.KerTypeInfo.FORM_STRUCT or type.form == Ker.KerTypeInfo.FORM_UNION:
                    
                    fname = "struct" if type.form == Ker.KerTypeInfo.FORM_STRUCT else "union"
                    self.type_name = type.name + ' ' + self.type_name
                    print(
                        "{0} {1} {2} {3}".format(
                            self.pad,
                            fname,
                            self.type_name,
                            self.variable_name
                        ),
                        end=' '
                    )
                    
                    if not self.pointer or not self.struct:
                        self.struct = True
                        print('{')
                        self.type_name = ''
                        self.pad = self.pad + '  '
                        for i in range(type.member_count):
                            self.variable_name = type.members[i].name
                            self.named = False
                            self.recursive_print(type.members[i].type_id)
                        self.pad = self.pad[:-2]
                        print("{0} }}".format(self.pad))
                    else:
                        print('')

                elif type.form == Ker.KerTypeInfo.FORM_ARRAY:
                
                    print("%s array %s, length %d" % (self.pad, self.variable_name, type.array_count))
                    self.variable_name = ''
                    for i in range(type.array_count):
                        print('[ {0} ]'.format(i), end=' ')
                        self.recursive_print(type.array_type_id)

                elif type.form == Ker.KerTypeInfo.FORM_TYPEDEF:
                
                    if not self.named:
                        self.type_name = self.type_name + type.name
                    self.named = True # don't add name info after a typedef
                    if type.ref_type_id != 0xffffffff:
                        self.recursive_print(type.ref_type_id)

                elif type.form == Ker.KerTypeInfo.FORM_POINTER:
                
                    if type.size_in_addressable_units > 0 and type.ref_type_id != 0xffffffff:
                        # Dereference the pointer
                        pointee_address = self._kal._read_dm_block(self.addr, type.size_in_addressable_units)
                        self.addr = pointee_address[0]
                        if not self.named:
                            self.type_name = self.type_name + '*'
                        self.pointer = True
                        self.recursive_print(type.ref_type_id)
                    else:
                        print(self.pad, end=' ')
                        if not self.named:
                            print(type.name, end=' ')
                        print("{0} * {1}".format(self.type_name, self.variable_name))

                elif type.form == Ker.KerTypeInfo.FORM_CONST:
                
                    if not self.named:
                        self.type_name = 'const ' + self.type_name
                    if type.ref_type_id != 0xffffffff:
                        self.recursive_print(type.ref_type_id)

                elif type.form == Ker.KerTypeInfo.FORM_VOLATILE:
                
                    if not self.named:
                        self.type_name = 'volatile ' + self.type_name
                    if type.ref_type_id != 0xffffffff:
                        self.recursive_print(type.ref_type_id)

                elif type.form == Ker.KerTypeInfo.FORM_ENUM:
                
                    vals = self._kal._read_dm_block(self.addr, 1)
                    val = vals[0]
                    enumval = ""
                    enumvals = self.enums[type.name]
                    for key in enumvals.keys():
                        if enumvals[key] == val:
                            enumval = key
                    print(
                        "{0} enum {1} {2} = {3} ({4})".format(
                            self.pad,
                            self.type_name,
                            self.variable_name,
                            val,
                            enumval
                        )
                    )
                    self.addr += type.size_in_addressable_units

                self.type_name = ''
                self.variable_name = ''
                self.level = self.level - 1

            def print_variable(self, name):
                # Get the variable & data address.
                var = self.variables[name]
                self.level = 0
                self.variable_name = name
                self.addr = var[1]
                self.pointer = False
                self.struct = False
                type_id = var[2]
                self.recursive_print(type_id)

        sh = TypeDisplayer(
            self._core,
            self.enums,
            self.variables,
            self.types,
            self.dm_octet_addressing,
            self.dm_data_width
        )
        sh.print_variable(name)
