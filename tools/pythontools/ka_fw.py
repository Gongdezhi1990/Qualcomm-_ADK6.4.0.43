# Copyright (c) 2014 - 2018 Qualcomm Technologies International, Ltd
# Part of the Python bindings for the kalaccess library.

from ctypes import c_void_p, POINTER, c_uint32, c_uint16
from ka_ctypes import ka_err


class KaFw:
    def __init__(self, core):
        self._core = core
        self._cfuncs = {}
        self._core.add_cfunc(
            self._cfuncs,
            'ka_read_onchip_signature',
            POINTER(ka_err),
            [c_void_p, POINTER(c_uint32)]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_hal_get_interlock_host_status_addr',
            POINTER(ka_err),
            [c_void_p, POINTER(c_uint16)]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_hal_get_interlock_fw_status_addr',
            POINTER(ka_err),
            [c_void_p, POINTER(c_uint16)]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_hal_get_interlock_host_status',
            POINTER(ka_err),
            [c_void_p, POINTER(c_uint16)]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_hal_get_interlock_fw_status',
            POINTER(ka_err),
            [c_void_p, POINTER(c_uint16)]
        )

    def read_onchip_signature(self):
        result = c_uint32()
        err = self._cfuncs['ka_read_onchip_signature'](self._core.get_ka(), result)
        self._core.handle_error(err)
        return result.value

    def get_interlock_host_status_addr(self):
        result = c_uint16()
        err = self._cfuncs['ka_hal_get_interlock_host_status_addr'](self._core.get_ka(), result)
        self._core.handle_error(err)
        return result.value

    def get_interlock_fw_status_addr(self):
        result = c_uint16()
        err = self._cfuncs['ka_hal_get_interlock_fw_status_addr'](self._core.get_ka(), result)
        self._core.handle_error(err)
        return result.value

    def get_interlock_host_status(self):
        result = c_uint16()
        err = self._cfuncs['ka_hal_get_interlock_host_status'](self._core.get_ka(), result)
        self._core.handle_error(err)
        return result.value

    def get_interlock_fw_status(self):
        result = c_uint16()
        err = self._cfuncs['ka_hal_get_interlock_fw_status'](self._core.get_ka(), result)
        self._core.handle_error(err)
        return result.value
