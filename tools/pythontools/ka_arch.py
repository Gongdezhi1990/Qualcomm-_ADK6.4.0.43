# Copyright (c) 2014 - 2018 Qualcomm Technologies International, Ltd
# Part of the Python bindings for the kalaccess library.

from ctypes import c_int, c_char_p, c_uint, c_byte, c_void_p
from ka_ctypes import from_cstr, CTypesStrIn


class KaArch:
    """
    Functions to query Kalimba architecture information.
    """

    def __init__(self, core):
        self._core = core
        self._cfuncs = {}
        self._core.add_cfunc(self._cfuncs, 'ka_get_chip_rev'         , c_int   , [c_void_p])
        self._core.add_cfunc(self._cfuncs, 'ka_get_global_chip_version', c_uint, [c_void_p])
        self._core.add_cfunc(self._cfuncs, 'ka_get_chip_name'        , c_char_p, [c_void_p])
        self._core.add_cfunc(self._cfuncs, 'ka_check_chip_name'      , c_byte,   [c_void_p, CTypesStrIn])
        self._core.add_cfunc(self._cfuncs, 'ka_get_arch'             , c_uint  , [c_void_p])
        self._core.add_cfunc(self._cfuncs, 'ka_get_arch_from_name'   , c_uint  , [CTypesStrIn])
        self._core.add_cfunc(self._cfuncs, 'ka_get_address_width'    , c_uint  , [c_void_p])
        self._core.add_cfunc(self._cfuncs, 'ka_get_data_width'       , c_uint  , [c_void_p])
        self._core.add_cfunc(self._cfuncs, 'ka_dm_subword_addressing', c_byte  , [c_void_p])
        self._core.add_cfunc(self._cfuncs, 'ka_pm_subword_addressing', c_byte  , [c_void_p])
        self._core.add_cfunc(self._cfuncs, 'ka_hal_is_hydra'         , c_byte  , [c_void_p])
        self._core.add_cfunc(self._cfuncs, 'ka_hal_is_bluecore'      , c_byte  , [c_void_p])
        self._core.add_cfunc(self._cfuncs, 'ka_hal_is_always_enabled', c_byte  , [c_void_p])
        self._core.add_cfunc(self._cfuncs, 'ka_get_subsystem_id'     , c_int   , [c_void_p])
        self._core.add_cfunc(self._cfuncs, 'ka_supports_minim'       , c_byte  , [c_void_p])

    def _lib(self):
        return self._core.get_ka()

    def get_chip_rev(self):
        return self._cfuncs['ka_get_chip_rev'](self._lib())

    def get_global_chip_version(self):
        return self._cfuncs['ka_get_global_chip_version'](self._lib())

    
    def get_chip_name(self):
        """
        Returns the name of the chip.
        """
        return from_cstr(self._cfuncs['ka_get_chip_name'](self._lib()))

    def check_chip_name(self, name):
        """
        Check if the chip associated with the given kalaccess connection is a match for the given name.
        This function checks against the name returned by get_chip_name() as well as any known aliases.
        The match is case-insensitive.
        """
        return 0 != self._cfuncs['ka_check_chip_name'](self._lib(), name)

    def get_arch(self):
        """
        Return the architecture number of the attached Kalimba core.
        """
        return self._cfuncs['ka_get_arch'](self._lib())

    def get_arch_from_name(self, name):
        """
        Return the architecture number of the Kalimba core, given a chip name.
        """
        return self._cfuncs['ka_get_arch_from_name'](name)

    def get_address_width(self):
        """
        Return the number of bits in a address on the target.
        """
        return self._cfuncs['ka_get_address_width'](self._lib())

    def get_data_width(self):
        """
        Return number of bits in one word of data memory.
        """
        return self._cfuncs['ka_get_data_width'](self._lib())

    def dm_subword_addressing(self):
        """
        Query if DM on the target is octet addressable.
        :return: True if octet addressable, False if word addressable.
        """
        return 0 != self._cfuncs['ka_dm_subword_addressing'](self._lib())

    def pm_subword_addressing(self):
        """
        Query if PM on the target is octet addressable.
        :return: True if octet addressable, False if word addressable.
        """
        return 0 != self._cfuncs['ka_pm_subword_addressing'](self._lib())
    
    def pm_address_inc_per_word(self):
        """
        Returns the increment to a PM address from one word to the next.
        """
        return 4 if self.pm_subword_addressing() else 1
        
    def dm_address_inc_per_word(self):
        """
        Return the increment to a DM address from one word to the next.
        """
        return (self.get_data_width() >> 3) if self.dm_subword_addressing() else 1
        
    def is_hydra(self):
        """
        Query if the connected chip has a Hydra architecture.
        :return: True or False
        """
        return 0 != self._cfuncs['ka_hal_is_hydra'](self._lib())
        
    def is_bluecore(self):
        """
        Query if the connected chip is a BlueCore chip.
        :return: True or False
        """
        return 0 != self._cfuncs['ka_hal_is_bluecore'](self._lib())
        
    def supports_minim(self):
        """
        Query if the connected Kalimba supports the MiniMode instruction set.
        :return: True or False
        """
        return 0 != self._cfuncs['ka_supports_minim'](self._lib())
        
    def is_always_enabled(self):
        """
        Query if the connected Kalimba is always considered to be enabled by kalaccess.
        :return: True or False
        """
        return 0 != self._cfuncs['ka_hal_is_always_enabled'](self._lib())
        
    def get_subsystem_id(self):
        """
        Retrieve the ID of the currently connected Hydra subsystem.
        :return: the ID, or -1 if connected to non-Hydra architecture chip.
        """
        return int(self._cfuncs['ka_get_subsystem_id'](self._lib()))
