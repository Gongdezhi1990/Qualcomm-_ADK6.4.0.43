# Copyright (c) 2014 - 2018 Qualcomm Technologies International, Ltd
# Part of the Python bindings for the kalaccess library.

from ctypes import c_byte, c_int, c_void_p, c_char_p, c_uint, c_uint32, c_uint16, POINTER, byref
from ka_ctypes import ka_err, ka_connection_details, CTypesStrIn, from_cstr, ctypes_arr_type
import math
from itertools import groupby


class KaDevice(object):
    def __init__(self, transport_string, subsystem_id, processor_id, dongle_id):
        self.transport_string = transport_string
        self.subsystem_id     = subsystem_id
        self.processor_id     = processor_id
        self.dongle_id        = dongle_id
    
    def __repr__(self):
        return "Transport string: {0}\n\t   Subsystem id: {1}\n\t   Processor id: {2}".format(
            self.transport_string, self.subsystem_id, self.processor_id)


class KaDeviceList(object):
    def __init__(self, devices):
        self.devices = devices
        
    def __repr__(self):
        if len(self.devices) == 0:
            return "No connected Kalimbas"
        
        result = ["Connected Kalimbas:"]        
        sorted_devs = sorted(self.devices, key = lambda d: d.dongle_id)
        groups = groupby(sorted_devs, lambda d: d.dongle_id)
        for i, group in enumerate(groups):
            result.append("{0}. Debug dongle '{1}':".format(i + 1, group[0]))
            for j, device in enumerate(group[1]):
                result.append("\t%s. %r" % (chr(j + ord('a')), device))
        return "\n".join(result)


class KaTrans:
    def __init__(self, core):
        self._core = core
        self._cfuncs = {}
        self._core.add_cfunc(
            self._cfuncs,
            'ka_trans_get_var',
            c_char_p,
            [c_void_p, CTypesStrIn]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_trans_set_var',
            None,
            [c_void_p, CTypesStrIn, CTypesStrIn]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_trans_build_device_table',
            POINTER(ka_err),
            [POINTER(POINTER(ka_connection_details)), POINTER(c_int)]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_trans_free_device_table',
            None,
            [POINTER(ka_connection_details), c_int]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_hal_spi_read',
            POINTER(ka_err),
            [c_void_p, c_uint, POINTER(c_uint16)]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_hal_spi_write',
            POINTER(ka_err),
            [c_void_p, c_uint, c_uint16]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_hal_jtag_read',
            POINTER(ka_err),
            [c_void_p, c_uint, POINTER(c_byte), c_uint]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_hal_jtag_write',
            POINTER(ka_err),
            [c_void_p, c_uint, CTypesStrIn, c_uint]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_hal_jtag_lock',
            POINTER(ka_err),
            [c_void_p, c_uint32]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_hal_jtag_unlock',
            POINTER(ka_err),
            [c_void_p]
        )

    def enumerate_transports(self):
        """
        Enumerates all connected Kalimba devices.
        Returns information in a KaDeviceList.
        """
        table, count = self._trans_build_device_table()
        devices = [KaDevice(
            from_cstr(table[i].transport_string),
            table[i].subsys_id,
            table[i].processor_id,
            from_cstr(table[i].dongle_id)
        ) for i in range(count)]
        self._trans_free_device_table(table, count)
        return KaDeviceList(devices)
        
    def trans_get_var(self, var):
        """Read a variable from the active pttransport SPI connection.
        Common variables are SPICLOCK, SPI_DELAY, SPI_DELAY_MODE, SPIMUL, SPIPORT."""
        return from_cstr(self._cfuncs['ka_trans_get_var'](self._core.get_ka(), var))

    def trans_set_var(self, var, val):
        """Set a variable on the active pttransport SPI connection.
        Common variables are SPICLOCK, SPI_DELAY, SPI_DELAY_MODE, SPIMUL, SPIPORT."""
        return self._cfuncs['ka_trans_set_var'](self._core.get_ka(), var, val)

    def _trans_build_device_table(self):
        con_details = POINTER(ka_connection_details)()
        count = c_int()
        err = self._cfuncs['ka_trans_build_device_table'](byref(con_details), byref(count))
        self._core.handle_error(err)
        return con_details, count.value

    def _trans_free_device_table(self, table, count):
        self._cfuncs['ka_trans_free_device_table'](table, count)

    def spi_read(self, addr):
        """Reads directly from the SPI map. You should NOT need to do this in normal circumstances."""
        data = c_uint16()
        err = self._cfuncs['ka_hal_spi_read'](self._core.get_ka(), addr, byref(data))
        self._core.handle_error(err)
        return data.value

    def spi_write(self, addr, data):
        """Writes directly to the SPI map. You should NOT need to do this in normal circumstances."""
        err = self._cfuncs['ka_hal_spi_write'](self._core.get_ka(), addr, data)
        self._core.handle_error(err)

    def jtag_read(self, addr, num_octets):
        """Reads directly from the JTAG address space. You should NOT need to do this in normal circumstances.
           Returns the read data as a list of octet values."""
        data = ctypes_arr_type(c_byte, num_octets)()
        err = self._cfuncs['ka_hal_jtag_read'](self._core.get_ka(), addr, data, num_octets)
        self._core.handle_error(err)
        return list(data)

    def jtag_write(self, addr, data):
        """Writes directly to the JTAG address space. You should NOT need to do this in normal circumstances.
           Data may be either a string (null terminator will not be written), or a list of 32-bit integers."""
        if isinstance(data, str):
            data_as_str = data
        else:
            data_as_str = ""
            for v in data:
                data_as_str = data_as_str + \
                              ("%c%c%c%c" % (v & 0xFF, (v >> 8) & 0xFF, (v >> 16) & 0xFF, (v >> 24) & 0xFF))
        err = self._cfuncs['ka_hal_jtag_write'](self._core.get_ka(), addr, data_as_str, len(data_as_str))
        self._core.handle_error(err)
        
    def jtag_lock_access(self, timeoutSeconds):
        """Takes exclusive access to the JTAG connection. While this is held, attempts by any other process to
           use the jtagtrans library will block. You generally shouldn't need to use this but it might be
           necessary to avoid multi-command activities being interrupted by other processes. If you do not call
           jtag_unlock_access before the timeout has passed, the lock will be released anyway.
           Note that this does nothing to stop a SPI or TrB connection from accessing the device."""
        err = self._cfuncs['ka_hal_jtag_lock'](self._core.get_ka(), c_uint32(int(math.ceil(timeoutSeconds * 1000.0))))
        self._core.handle_error(err)
              
    def jtag_unlock_access(self):
        """Releases any lock taken by a call to jtag_lock_access. Does nothing if no lock is held."""
        err = self._cfuncs['ka_hal_jtag_unlock'](self._core.get_ka())
        self._core.handle_error(err)
