# Copyright (c) 2014 - 2018 Qualcomm Technologies International, Ltd
# Part of the Python bindings for the kalaccess library.

from ctypes import c_int, c_uint, c_void_p, c_char_p, POINTER, c_uint32
from ka_ctypes import ka_err, to_ctypes_array, CTypesStrIn, ctypes_arr_type, from_cstr
from ka_exceptions import UnknownRegister


class KaReg:
    def __init__(self, core):

        # These are assigned in this funny way to avoid warnings from __setattr__ (see below).
        self.__dict__['_core'] = core
        self.__dict__['_cfuncs'] = {}

        self._core.add_cfunc(
            self._cfuncs,
            'ka_get_register_width',
            c_uint,
            [c_void_p, c_int]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_get_register_id',
            c_int,
            [c_void_p, CTypesStrIn]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_get_register_names',
            POINTER(c_char_p),
            []
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_read_register',
            POINTER(ka_err),
            [c_void_p, c_int, POINTER(c_uint32)]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_write_register',
            POINTER(ka_err),
            [c_void_p, c_int, c_uint32]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_read_register_set',
            POINTER(ka_err),
            [c_void_p, POINTER(c_int), POINTER(c_uint32), c_int]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_write_register_set',
            POINTER(ka_err),
            [c_void_p, POINTER(c_int), POINTER(c_uint32), c_int]
        )

        # We need a list of all possible register names before we connect, so that attempting
        # to get a member called "pc" raises a NotConnected exception but attempting to get
        # "wombles" raises a UnknownAttribute exception.
        self.__dict__['_reg_names'] = self.get_register_names()
            
    def is_register(self, name):
        try:
            self.get_register_id(name)
            return True
        except UnknownRegister:
            return False

    def _check_reg_name(self, name):
        return name.upper() in self._reg_names
        
    def __getattr__(self, elem):
        """
        Magic routine to let us trap register read accesses, such as 'kal.pc'
        """
        if not self._check_reg_name(elem):
            raise UnknownRegister(elem)
        else:
            reg_id = self.get_register_id(elem)
            return self.read_register(reg_id)

    def __setattr__(self, elem, val):
        """
        Magic routine to let us trap register writes, such as 'kal.pc = 123'
        """
        if not self._check_reg_name(elem):
            raise UnknownRegister(elem)
        else:
            reg_id = self.get_register_id(elem)
            self.write_register(reg_id, val)

    def __dir__(self):
        # Enable some level of tab completion.
        # Without with addition of "self.__dict__['_reg_names']",
        # we do not include the register names in the tab completion list. This is
        # intentional, otherwise every tab completion would imply a read of every register
        # from the Kalimba (__getattr__ will be called for each)
        return list(self.__dict__.keys())
        
    def get_register_width(self, id):
        """
        Returns the bit width of the specified register. Pass in the register ID.
        """
        return self._cfuncs['ka_get_register_width'](self._core.get_ka(), id)

    def get_register_id(self, name):
        """
        Returns the numerical id corresponding to the given register name. The name is case-insensitive.
        """
        res = self._cfuncs['ka_get_register_id'](self._core.get_ka(), name.upper())
        if res == 0xFFFF:  # NO_SUCH_ID
            raise UnknownRegister(name)
        return res

    def get_register_names(self):
        raw_names = self._cfuncs['ka_get_register_names']()
        # 'NumRegs' marks the end of the register name list.
        end_marker = 'NumRegs'
        results = []
        for name in raw_names:
            name = from_cstr(name)
            if name == end_marker:
                break
            results.append(name)
        return results

    def read_register(self, register):
        """
        Reads the value of a register. The register may be specified by id or name.
        """
        if isinstance(register, str):
            register = self.get_register_id(register)
        result = c_uint32()
        err = self._cfuncs['ka_read_register'](self._core.get_ka(), register, result)
        self._core.handle_error(err)
        return result.value
            
    def write_register(self, register, val):
        """
        Writes a value of a register. The register may be specified by id or name.
        """
        if isinstance(register, str):
            register = self.get_register_id(register)
        err = self._cfuncs['ka_write_register'](self._core.get_ka(), register, val)
        self._core.handle_error(err)
    
    def _get_c_reg_ids(self, registers):
        registers = list(map(lambda r: self.get_register_id(r) if isinstance(r, str) else r, registers))
        return to_ctypes_array(registers, c_int)
        
    def read_register_set(self, registers):
        """
        Reads the values of the set of registers specified in the iterable 'registers'.
        Elements of 'registers' may be register names or register ids.
        """
        num_registers = len(registers)
        reg_ids_for_c = self._get_c_reg_ids(registers)
        reg_values = ctypes_arr_type(c_uint32, num_registers)()

        err = self._cfuncs['ka_read_register_set'](self._core.get_ka(), reg_ids_for_c, reg_values, num_registers)
        self._core.handle_error(err)
        return list(reg_values)

    def write_register_set(self, registers, values):
        """
        Writes values to the set of registers specified in the iterable 'registers'.
        Elements of 'registers' may be register names or register ids.
        """
        num_registers = len(registers)
        if len(values) != num_registers:
            raise ValueError("Not enough values specified to write to register set")
        reg_ids_for_c = self._get_c_reg_ids(registers)
        values_for_c  = to_ctypes_array(values, c_uint32)

        err = self._cfuncs['ka_write_register_set'](self._core.get_ka(), reg_ids_for_c, values_for_c, num_registers)
        self._core.handle_error(err)
