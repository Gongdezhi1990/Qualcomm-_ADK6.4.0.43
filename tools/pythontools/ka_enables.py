# Copyright (c) 2014 - 2018 Qualcomm Technologies International, Ltd
# Part of the Python bindings for the kalaccess library.

from ctypes import c_int, c_byte, c_void_p, POINTER
from ka_ctypes import ka_err


class KaSubsystemStatus(object):

    def __init__(self, ssid, powered, clocked, up, run_enabled):
        self.ssid = ssid
        self.powered = powered
        self.clocked = clocked
        self.up = up
        self.run_enabled = run_enabled

    def __repr__(self):
        return "Subsystem {0}:\nPowered: {1}\nClocked: {2}\nUp: {3}\nRun-enabled: {4}".format(
            self.ssid, self.powered, self.clocked, self.up, self.run_enabled)


class KaEnables(object):
    """
    Functions to query the state of the Kalimba DSP, and enable or disable it.
    """

    def __init__(self, core):
        self._core = core
        self._cfuncs = {}
        self._core.add_cfunc(
            self._cfuncs,
            'ka_hal_read_dsp_enable',
            POINTER(ka_err),
            [c_void_p, POINTER(c_byte)]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_hal_write_dsp_enable',
            POINTER(ka_err),
            [c_void_p, c_byte]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_hal_read_dsp_clock_enable',
            POINTER(ka_err),
            [c_void_p, POINTER(c_byte)]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_hal_write_dsp_clock_enable',
            POINTER(ka_err),
            [c_void_p, c_byte]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_hal_read_subsystem_up',
            POINTER(ka_err),
            [c_void_p, c_int, POINTER(c_byte)]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_hal_read_subsystem_clocked',
            POINTER(ka_err),
            [c_void_p, c_int, POINTER(c_byte)]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_hal_read_subsystem_powered',
            POINTER(ka_err),
            [c_void_p, c_int, POINTER(c_byte)]
        )
        self._core.add_cfunc(
            self._cfuncs,
            'ka_hal_read_subsystem_run',
            POINTER(ka_err),
            [c_void_p, c_int, POINTER(c_byte)]
        )
    
    def _lib(self):
        return self._core.get_ka()

    def _handle_return_code(self, res):
        self._core.handle_error(res)
        
    def read_dsp_enable(self):
        """
        Query if the Kalimba processor is enabled.
        Does not apply to Hydra architecture chips, and will raise an exception if called; see instead the
        *_subsystem family of functions. Similarly, does not apply to Napier accessed over SPI, because
        the corresponding register is in an area of the chip that SPI cannot access.
        :return: True or False
        """
        result = c_byte()
        res = self._cfuncs['ka_hal_read_dsp_enable'](self._lib(), result)
        self._handle_return_code(res)
        return bool(result.value)

    def write_dsp_enable(self, enable):
        """
        Enables the Kalimba processor.
        Does not apply to Hydra architecture chips, and will raise an exception if called; see instead the
        *_subsystem family of functions. Similarly, does not apply to Napier accessed over SPI, because
        the corresponding register is in an area of the chip that SPI cannot access.
        """
        res = self._cfuncs['ka_hal_write_dsp_enable'](self._lib(), enable)
        self._handle_return_code(res)

    def read_dsp_clock_enable(self):
        """
        Query if the Kalimba processor is clocked.
        Does not apply to Hydra architecture chips, and will raise an exception if called; see instead the
        *_subsystem family of functions.
        :return: True or False
        """
        result = c_byte()
        res = self._cfuncs['ka_hal_read_dsp_clock_enable'](self._lib(), result)
        self._handle_return_code(res)
        return bool(result.value)

    def write_dsp_clock_enable(self, enable):
        """
        Enables the clock on the Kalimba processor.
        Does not apply to Hydra architecture chips, and will raise an exception if called; see instead the
        *_subsystem family of functions.
        """
        res = self._cfuncs['ka_hal_write_dsp_clock_enable'](self._lib(), enable)
        self._handle_return_code(res)

    def enable_and_clock_dsp(self):
        """
        Convenience function to enable the Kalimba and provide a clock.
        Equivalent to write_dsp_clock_enable(True) and write_dsp_enable(True).
        Does not apply to Hydra architecture chips, and will raise an exception if called; see instead the
        *_subsystem family of functions. Similarly, does not apply to Napier accessed over SPI, because
        the corresponding register is in an area of the chip that SPI cannot access.
        """
        self.write_dsp_clock_enable(True)
        self.write_dsp_enable(True)
    
    def read_subsystem_up(self, sub_system_id):
        """
        Query if the specified Hydra subsystem is up and able to respond to transaction.
        :param sub_system_id: Hydra subsystem ID
        :return: True or False
        """
        result = c_byte()
        res = self._cfuncs['ka_hal_read_subsystem_up'](self._lib(), sub_system_id, result)
        self._handle_return_code(res)
        return bool(result.value)

    def read_subsystem_clocked(self, sub_system_id):
        """
        Query if the specified Hydra subsystem's clock is enabled.
        :param sub_system_id: Hydra subsystem ID
        :return: True or False
        """
        result = c_byte()
        res = self._cfuncs['ka_hal_read_subsystem_clocked'](self._lib(), sub_system_id, result)
        self._handle_return_code(res)
        return bool(result.value)

    def read_subsystem_powered(self, sub_system_id):
        """
        Query if the specified Hydra subsystem is powered.
        :param sub_system_id: Hydra subsystem ID
        :return: True or False
        """
        result = c_byte()
        res = self._cfuncs['ka_hal_read_subsystem_powered'](self._lib(), sub_system_id, result)
        self._handle_return_code(res)
        return bool(result.value)

    def read_subsystem_run(self, sub_system_id):
        """
        Query if the specified Hydra subsystem is allowed to run.
        :param sub_system_id: Hydra subsystem ID
        :return: True or False
        """
        result = c_byte()
        res = self._cfuncs['ka_hal_read_subsystem_run'](self._lib(), sub_system_id, result)
        self._handle_return_code(res)
        return bool(result.value)

    def subsystem_summary(self, sub_system_id):
        """
        Retrieve a summary of the status of the given Hydra subsystem.
        :param sub_system_id: Hydra subsystem ID
        :return: a KaSubsystemStatus instance
        """
        return KaSubsystemStatus(
            sub_system_id,
            self.read_subsystem_powered(sub_system_id),
            self.read_subsystem_clocked(sub_system_id),
            self.read_subsystem_up(sub_system_id),
            self.read_subsystem_run(sub_system_id)
        )
