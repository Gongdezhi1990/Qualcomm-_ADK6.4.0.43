# ***********************************************************************
# * Copyright 2016 Qualcomm Technologies International, Ltd.
# ***********************************************************************

from contextlib import contextmanager
from ctypes import c_int, c_uint, c_ubyte, c_void_p, c_char_p, c_char, c_bool, c_uint16, byref, POINTER, Structure, \
    cdll, c_uint8
import os
import sys


class TrbDriver(object):
    """
    Corresponds to the C enum 'trb_driver'.
    """
    MUR     = 0
    USB2TRB = 1


class TrbActionType(object):
    """
    Corresponds to the C enum 'trb_action_type'.
    """
    DEBUG_READ  = 0  # Uses debug read request transactions
    DEBUG_WRITE = 1  # Uses debug write request transactions
    DATA_READ   = 2  # Uses data read request transactions
    DATA_WRITE  = 3  # Uses data write request transactions


class TrbOpcodes(object):
    """
    Transaction Bus opcodes.
    """
    OPCODE_VM_LOOKUP_REQUEST   = 1
    OPCODE_VM_LOOKUP_RESPONSE  = 2
    OPCODE_VM_WRITEBACK        = 3
    OPCODE_DATA_WRITE          = 4
    OPCODE_DATA_WRITE_REQUEST  = 5
    OPCDOE_DATA_WRITE_RESPONSE = 6
    OPCODE_DATA_READ_REQUEST   = 7
    OPCODE_DATA_READ_RESPONSE  = 8
    OPCODE_EXTENDED            = 9
    OPCODE_STREAM              = 10
    OPCODE_DEBUG               = 11


class TrbDebugOpcodeSubtypes(object):
    """
    Subtypes for Debug opcodes (OPCODE_DEBUG) transactions.
    """
    DEBUG_OPCODE_DATA_WRITE_REQUEST  = 1
    DEBUG_OPCODE_DATA_WRITE_RESPONSE = 2
    DEBUG_OPCODE_DATA_READ_REQUEST   = 3
    DEBUG_OPCODE_DATA_READ_RESPONSE  = 4


class TBusDebugStatusCode(object):
    """
    TBus status codes for data or debug read and write responses.
    """
    TBUS_STATUS_NO_ERROR          = 0
    TBUS_SUBSYSTEM_NO_POWER       = 1
    TBUS_SUBSYSTEM_ASLEEP         = 2
    TBUS_ROUTING_ERROR            = 3
    TBUS_LOCK_ERROR               = 4
    # 5-9 unused
    TBUS_DEBUG_TIMEOUT            = 10
    TBUS_ACCESS_PROTECTION_ERROR  = 11
    TBUS_NO_MEMORY_HERE           = 12
    TBUS_TRANSACTION_WRONG_LENGTH = 13
    TBUS_MEMORY_NOT_WRITABLE      = 14
    TBUS_ACCESS_BAD_ALIGNMENT     = 15


class UsbConnectionSpeed(object):
    """
    Possible values returned by Trb.get_usb_connection_speed().
    """
    USB_CONNECTION_UNKNOWN_SPEED = 0
    USB_CONNECTION_FULL_SPEED    = 1
    USB_CONNECTION_HIGH_SPEED    = 2
    USB_CONNECTION_SUPER_SPEED   = 3


class TrbAction(Structure):
    """
    Corresponds to the C struct 'trb_action'.
    """
    _fields_ = [("type", c_int),  # Typically, TrbActionType.DEBUG_READ or TrbActionType.DEBUG_WRITE
                ("dest_subsys_id", c_int),
                ("dest_block_id", c_int),
                ("addr", c_uint),
                ("num_bytes", c_uint),
                ("bytes_per_transaction", c_int),
                ("data", POINTER(c_ubyte))]
                

class Transaction(Structure):
    """Corresponds to the C struct 'transaction_with_timestamp'."""
    _fields_ = [("timestamp", c_uint),
                ("opcode_and_src_subsys_id", c_ubyte),
                ("src_block_id_and_dest_subsys_id", c_ubyte),
                ("dest_block_id_and_tag", c_ubyte),
                ("payload", c_ubyte * 9)]


class TrbDongleDetails(Structure):
    """Corresponds to the C struct 'trb_dongle_details'."""
    _fields_ = [("driver", c_int),
                ("id", c_int),
                ("description", c_char_p)]


class ScarDmaChannelStatistics(Structure):
    """Corresponds to the C struct 'scar_dma_channel_stats'."""
    _fields_ = [("channel_state", c_uint),
                ("total_buffer_size_in_bytes", c_uint),
                ("producer_transfer_byte_count", c_uint),
                ("consumer_transfer_byte_count", c_uint)]


class ScarDmaStatistics(Structure):
    """Corresponds to the C struct 'scar_dma_device_stats'."""
    _fields_ = [("channel_stats", ScarDmaChannelStatistics * 7)]


class ScarUsbStatistics(Structure):
    """Corresponds to the C struct 'scar_usb_device_stats'."""
    _fields_ = [("phy_error_count", c_uint),
                ("link_error_count", c_uint)]


class ScarDeviceStatsRaw(Structure):
    """Corresponds to the C struct 'scar_device_stats'."""
    _fields_ = [("usb_stats", ScarUsbStatistics),
                ("dma_stats", ScarDmaStatistics)]


class TrbError(Exception):
    def __init__(self, message, failed_action_num):
        Exception.__init__(self, message)
        self.failed_action_num = failed_action_num

    def add_extended_error_info(self, trb):
        pass


class TrbErrorCouldNotEnumerateDevice(TrbError):
    pass


class TrbErrorDriverIOFailed(TrbError):
    pass


class TrbErrorDriverIOTimeout(TrbError):
    pass


class TrbErrorDeviceVersionTooOld(TrbError):
    pass


class TrbErrorBadDataReceived(TrbError):
    pass


class TrbErrorTBusAccessFailed(TrbError):
    def __init__(self, message, failed_action_num):
        TrbError.__init__(self, message, failed_action_num)
        self.tbus_error_code = TBusDebugStatusCode.TBUS_STATUS_NO_ERROR

    def add_extended_error_info(self, trb):
        self.tbus_error_code = trb.get_last_tbus_error()


class TrbErrorActionNeedsTooManyTransactions(TrbError):
    pass


class TrbErrorLoggingAlreadyEnabled(TrbError):
    pass


class TrbErrorCouldNotOpenFile(TrbError):
    pass


class TrbErrorInvalidStream(TrbError):
    pass


class TrbErrorBridgeLinkIsDown(TrbError):
    pass


class TrbErrorOSUnsupported(TrbError):
    pass


class TrbErrorOSError(TrbError):
    pass


class TrbErrorUnknownDriver(TrbError):
    pass


class TrbErrorDriverBufferWrapped(TrbError):
    pass


class TrbErrorBadParameter(TrbError):
    pass


class TrbErrorCouldNotQueryDriverInfo(TrbError):
    pass


class TrbErrorBufferTooShort(TrbError):
    pass


class TrbErrorOperationNotSupported(TrbError):
    pass


class TrbErrorDeviceIncompatible(TrbError):
    pass


class TrbErrorLinkReset(TrbError):
    pass


TRB_ERROR_MAP = [
    None,                                    # TRB_ERR_NO_ERROR
    TrbErrorCouldNotEnumerateDevice,         # TRB_ERR_COULD_NOT_ENUMERATE_DEVICE
    TrbErrorDriverIOFailed,                  # TRB_ERR_DRIVER_IO_FAILED
    TrbErrorDriverIOTimeout,                 # TRB_ERR_DRIVER_IO_TIMEOUT
    TrbErrorDeviceVersionTooOld,             # TRB_ERR_DEVICE_VERSION_TOO_OLD
    TrbErrorBadDataReceived,                 # TRB_ERR_BAD_DATA_RECEIVED
    TrbErrorTBusAccessFailed,                # TRB_ERR_TBUS_ACCESS_FAILED
    TrbErrorActionNeedsTooManyTransactions,  # TRB_ERR_ACTION_NEEDS_TOO_MANY_TRANSACTIONS
    TrbErrorLoggingAlreadyEnabled,           # TRB_ERR_LOGGING_ALREADY_ENABLED
    TrbErrorCouldNotOpenFile,                # TRB_ERR_COULD_NOT_OPEN_FILE
    TrbErrorInvalidStream,                   # TRB_ERR_INVALID_STREAM
    TrbErrorBridgeLinkIsDown,                # TRB_ERR_BRIDGE_LINK_IS_DOWN
    TrbErrorOSUnsupported,                   # TRB_ERR_OS_UNSUPPORTED
    TrbErrorOSError,                         # TRB_ERR_OS_ERROR
    TrbErrorUnknownDriver,                   # TRB_ERR_UNKNOWN_DRIVER
    TrbErrorDriverBufferWrapped,             # TRB_ERR_DRIVER_BUFFER_WRAPPED
    TrbErrorBadParameter,                    # TRB_ERR_BAD_PARAMETER
    TrbErrorCouldNotQueryDriverInfo,         # TRB_ERR_COULD_NOT_QUERY_DRIVER_INFO
    TrbErrorBufferTooShort,                  # TRB_ERR_BUFFER_TOO_SHORT
    TrbErrorOperationNotSupported,           # TRB_ERR_OPERATION_NOT_SUPPORTED
    TrbErrorDeviceIncompatible,              # TRB_ERR_DEVICE_INCOMPATIBLE
    TrbErrorLinkReset                        # TRB_ERR_LINK_RESET
]


class TrbTransLibraryFunctionMissingOrMismatch(RuntimeError):
    def __init__(self, lib_name, func_name):
        RuntimeError.__init__(
            self, "Trbtrans library function '{0}' not found in shared library '{1}', or the prototype didn't match.\n"
                  "Check that the shared library is the same version as these Python bindings.".format(func_name,
                                                                                                       lib_name))


class TrbDongle(object):
    def __init__(self, raw_details):
        self.driver      = raw_details.driver
        self.id          = raw_details.id
        self.description = raw_details.description


class TrbDongleList(list):
    def __init__(self, devices):
        super(TrbDongleList, self).__init__(devices)

    def __repr__(self):
        result = []
        if len(self) == 0:
            result = ["No debug dongles found"]
        else:
            for i, dev in enumerate(self):
                result.append("{0}. Name: {1}\n   ID:   {2}".format(i + 1, dev.description, dev.id))
        return "\n".join(result)


class ScarDeviceStats(object):
    def __init__(self, raw_stats):
        self._raw_stats = raw_stats

    def __repr__(self):
        result = ["* USB stats:",
                  "PHY error count: {0:#x}".format(self._raw_stats.usb_stats.phy_error_count),
                  "Link error count: {0:#x}".format(self._raw_stats.usb_stats.link_error_count),
                  "* DMA stats:"
        ]

        header = "Channel # | State | Total buffer size | Producer xfer bytes | Consumer xfer bytes | Diff | Description"
        header_cols = header.split('| ')
        col_widths = [len(col) for col in header_cols]
        aligned_row_format = '| '.join(['%%-%ds' % width for width in col_widths ])

        result.append(header)

        channel_descriptions = [
            "TRB Transactions: ""USB EP 0x1[OUT] -> GPIF(0)""",
            "TRB Transactions: ""GPIF(2) -> USB EP 0x81[IN]""",
            "Reg Requests: ""USB EP 0x2[OUT] -> CPU""",
            "Reg Requests: ""CPU -> GPIF(1)""",
            "S/W Demux: ""GPIF(3) -> CPU""",
            "Reg Responses: ""CPU -> USB EP 0x82[IN]""",
            "Events: ""CPU -> USB EP 0x83[IN]"""
        ]

        for i in range(len(self._raw_stats.dma_stats.channel_stats)):
            this_channel = self._raw_stats.dma_stats.channel_stats[i]

            diff = this_channel.producer_transfer_byte_count - this_channel.consumer_transfer_byte_count
            if diff < 0:
                diff = -diff

            result.append(aligned_row_format % (
                i + 1,
                this_channel.channel_state,
                this_channel.total_buffer_size_in_bytes,
                this_channel.producer_transfer_byte_count,
                this_channel.consumer_transfer_byte_count,
                diff,
                channel_descriptions[i])
            )

        return "\n".join(result)


class Trb:
    """
    Python wrapper of the trbtrans library
    """

    def _handle_error(self, err):
        """
        Internal routine to convert trbtrans errors into Python exceptions
        """
        if bool(err):
            err_str = c_char_p()    
            err_action_num = c_int()
            err_code = self._cfuncs['trb_get_last_error'](byref(err_action_num), byref(err_str))
            try:
                ex = TRB_ERROR_MAP[err_code](err_str.value, err_action_num)
            except IndexError:
                raise TrbError('Unknown error from trbtrans: error code {0} has not been mapped to Python.'.
                               format(err_code), failed_action_num=0)
            ex.add_extended_error_info(self)
            raise ex

    @staticmethod
    def _get_path_element_separator():
        # On Cygwin, path elements are separated by colons, but on win32 it's a semi-colon.
        return ";" if sys.platform.startswith('win32') else ":"

    def prepend_path_environment_element(self, new_element):
        """
        Not currently used. Previously used by the Windows shared library loading code, which now forms absolute paths
        so that it can guarantee that the library it thinks will be loaded, will actually be loaded.
        The problem with adding PATH elements is that the PATH search is quite low down on the DLL search order, so
        another DLL might be found in one of the higher priority search locations -- this might not be the one wanted.
        """
        existing_path = os.environ.get("PATH", "")

        if existing_path.find(new_element) == -1:
            os.environ["PATH"] = new_element + self._get_path_element_separator() + existing_path

    @staticmethod
    def _get_this_file_dir():
        """
        This mechanism assumes that nothing has changed the current directory since this module was loaded (e.g.
        os.chdir()), because __file__ is a constant. It seems fairly reasonable to make this assumption.
        """
        return os.path.abspath(os.path.dirname(os.path.realpath(__file__)))

    def prepare_shared_library_load_common(self, override_lib_path, default_lib_filename):
        if override_lib_path:
            if not os.path.isabs(override_lib_path):
                # Something was supplied, but it's not absolute. Form a full path relative to this file's location.
                # This step allows the load to work whether or not the current directory is the same as this file's
                # directory (it probably isn't).
                load_lib_path = os.path.join(self._get_this_file_dir(), override_lib_path)
            else:
                load_lib_path = override_lib_path
        else:
            here_lib_path = os.path.join(self._get_this_file_dir(), default_lib_filename)
            # No override supplied, so look in this file's directory for the shared library, with the default name.
            # If it's there, use the path we formed (this way, we guarantee that we load *this* library,
            # not anything else that the dynamic loader might find via environment/search rules).
            # Otherwise throw ourselves to the mercy of the dynamic loader and
            # the environment, and just use the bare default filename.
            if os.path.isfile(here_lib_path):
                load_lib_path = here_lib_path
            else:
                load_lib_path = default_lib_filename

        return load_lib_path

    def prepare_shared_library_load_linux(self, override_lib_path):
        """
        Return a path to pass to ctypes' LoadLibrary.
        May also do environmental preparation.
        :param override_lib_path: any override path that was passed to __init__.
        """
        return self.prepare_shared_library_load_common(override_lib_path, "libtrbtrans_shared.so")

    def prepare_shared_library_load_windows(self, override_lib_path):
        """
        Return a path to pass to ctypes' LoadLibrary.
        May also do environmental preparation.
        :param override_lib_path: any override path that was passed to __init__.
        """
        return self.prepare_shared_library_load_common(override_lib_path, "trbtrans.dll")

    def load_shared_library(self, override_lib_path):
        """
        Load the native shared library. Give a bit of support to try to ease troubleshooting of common problems.
        :param override_lib_path: any override path that was passed to __init__.
        :return the path that was passed to ctypes LoadLibrary, if successful.
        """
        if sys.platform.startswith('linux'):
            load_library_path = self.prepare_shared_library_load_linux(override_lib_path)
            try:
                self._trb_dll = cdll.LoadLibrary(load_library_path)
            except Exception as ex:
                message = """Could not find or load '{0}' (or one of its dependencies).

Inner Python exception: {1}

Check that:
- the architecture of your Python interpreter (32/64-bit) matches the architecture of the trbtrans shared library being
  loaded (e.g. via the 'file' command).
- the location of the library is in the shared library search path.

The LD_LIBRARY_PATH used for the search was:
    {2}""".format(load_library_path,
                  repr(ex),
                  "\n    ".join(os.environ.get('LD_LIBRARY_PATH', '').split(":")))

                raise OSError(message)

        elif sys.platform.startswith('cygwin') or sys.platform.startswith('win32'):
            load_library_path = self.prepare_shared_library_load_windows(override_lib_path)
            try:
                self._trb_dll = cdll.LoadLibrary(load_library_path)
            except Exception as ex:
                message = """Could not find or load '{0}' (or one of its dependencies).

Inner Python exception: {1}

Check that:
- the architecture of your Python interpreter (32/64-bit) matches the architecture of the trbtrans DLL being loaded.
- the location of the library is in the DLL search path.
- the Microsoft Visual C++ 2015 Redistributable package is installed.

The system PATH used for the search was:
    {2}""".format(load_library_path,
                  repr(ex),
                  "\n    ".join(os.environ.get('PATH', '').split(self._get_path_element_separator())))

                raise OSError(message)
        else:
            raise OSError("Cannot load the trbtrans library. The system '{0}' you are using is not supported.".format(
                sys.platform))

        return load_library_path

    def __init__(self, override_lib_path = None):
        """
        :param override_lib_path: an optional path (including file name) from which to load the shared library. May be
        absolute, or relative to the directory of this file.
        """
        self._stream = c_void_p()
        self._trb_dll = None
        self._cfuncs = {}
        load_library_path = self.load_shared_library(override_lib_path)

        def gen_prototype(name, restype, argtypes):
            try:
                func = getattr(self._trb_dll, name)
            except AttributeError:
                raise TrbTransLibraryFunctionMissingOrMismatch(load_library_path, name)

            func.argtypes = argtypes
            func.restype = restype
            self._cfuncs[name] = func

        gen_prototype('trb_get_version'           , c_char_p, None)

        gen_prototype('trb_get_driver_from_name'  , c_int, [c_char_p, POINTER(c_int)])

        gen_prototype('trb_build_dongle_list'     , c_int, [c_int, POINTER(POINTER(TrbDongleDetails)), POINTER(c_uint)])
        gen_prototype('trb_free_dongle_list'      , c_int, [POINTER(TrbDongleDetails), c_uint])

        gen_prototype('trb_form_transport_string'  , c_int, [POINTER(TrbDongleDetails), c_char_p, c_uint])
        gen_prototype('trb_decode_transport_string', c_int, [c_char_p, POINTER(c_int), POINTER(c_int)])

        gen_prototype('trb_get_dongle_name'       , c_char_p, [c_void_p])
        gen_prototype('trb_get_dongle_details'    , c_int,    [c_void_p, POINTER(TrbDongleDetails)])
        gen_prototype('trb_get_firmware_version'  , c_int,    [c_void_p, POINTER(c_int)])
        gen_prototype('trb_get_fpga_version'      , c_int,    [c_void_p, POINTER(c_uint)])
        gen_prototype('trb_get_driver_version'    , c_char_p, [c_void_p])
        gen_prototype('trb_get_bridge_subsys_id'  , c_uint,   [c_void_p])

        gen_prototype('trb_stream_open'                , c_int, [c_int, c_int, POINTER(c_void_p)])
        gen_prototype('trb_sniffer_stream_open'        , c_int, [c_int, c_int, POINTER(c_void_p)])
        gen_prototype('trb_sample_stream_open'         , c_int, [c_int, c_int, POINTER(c_void_p)])
        gen_prototype('trb_stream_close'               , None,  [c_void_p])

        gen_prototype('trb_get_link_status'       , c_int, [c_void_p, POINTER(c_bool)])
        gen_prototype('trb_is_stream_valid'       , c_int, [c_void_p, POINTER(c_bool)])
        gen_prototype('trb_get_link_statistics'   , c_int, [c_void_p,
                                                            POINTER(c_uint8), POINTER(c_uint8), POINTER(c_uint8)])
        gen_prototype('trb_get_chip_version'      , c_uint16, [c_void_p])

        gen_prototype('trb_get_max_transactions'  , c_int, None)

        gen_prototype('trb_sequence'              , c_int, [c_void_p, POINTER(TrbAction), c_int])
        
        gen_prototype('trb_write_raw_transactions', c_int, [c_void_p, POINTER(Transaction), c_int])
        gen_prototype('trb_read_raw_transactions' , c_int,
                                                    [c_void_p, POINTER(Transaction), c_int, POINTER(c_int), c_int])

        gen_prototype('trb_read_dongle_reg'       , c_int, [c_void_p, c_int, c_int, POINTER(c_uint)])
        gen_prototype('trb_write_dongle_reg'      , c_int, [c_void_p, c_int, c_int, c_uint])

        gen_prototype('trb_get_last_error'        , c_int, [POINTER(c_int), POINTER(c_char_p)])
        gen_prototype('trb_get_last_tbus_error'   , c_int, [c_void_p])

        gen_prototype('trb_start_log'             , c_int, [c_void_p, c_char_p])
        gen_prototype('trb_stop_log'              , c_int, [c_void_p])
        gen_prototype('trb_get_log_path'          , c_int, [c_void_p, c_char_p, c_uint])

        gen_prototype('trb_zero_device_statistics', c_int, [c_void_p])
        gen_prototype('trb_get_device_statistics',  c_int, [c_void_p, POINTER(ScarDeviceStatsRaw)])

        gen_prototype('trb_get_usb_connection_speed', c_int, [c_void_p, POINTER(c_int)])
        gen_prototype('trb_get_firmware_uptime',      c_int, [c_void_p, POINTER(c_uint)])
        gen_prototype('trb_dongle_reset',             c_int, [c_void_p])

        gen_prototype('trb_reset_dut', c_int, [c_void_p, c_uint, POINTER(c_bool)])
        gen_prototype('trb_configure_sleep_retries', None, [c_void_p, c_int, c_uint])

    def _translate_driver(self, driver):
        if driver in [TrbDriver.MUR, TrbDriver.USB2TRB]:
            return driver

        if isinstance(driver, str):
            raw_driver = c_int()
            self._handle_error(self._cfuncs['trb_get_driver_from_name'](driver.lower(), byref(raw_driver)))
            return raw_driver.value

        raise TypeError("Driver must be specified as either a string or a member of trbtrans.TrbDriver")

    def build_dongle_list(self, driver):
        """
        Builds and returns a list of attached debug dongles of the specified type.
        :param driver: Valid values are the same as those for open().
        :return: a TrbDongleList instance
        """
        driver = self._translate_driver(driver)
        raw_dongles = POINTER(TrbDongleDetails)()
        num_dongles = c_uint()
        self._handle_error(self._cfuncs['trb_build_dongle_list'](driver, byref(raw_dongles), byref(num_dongles)))

        try:
            dongles = TrbDongleList([TrbDongle(raw_dongles[i]) for i in range(num_dongles.value)])
        finally:
            self._handle_error(self._cfuncs['trb_free_dongle_list'](raw_dongles, num_dongles))
        return dongles

    # noinspection PyCallingNonCallable
    def form_transport_string(self, dongle_details):
        """
        Given some dongle details of type TrbDongle, probably obtained from build_dongle_list, construct a transport
        string, which can be later decoded back into dongle a driver type and dongle ID by decode_transport_string.
        :param dongle_details: a TrbDongle instance containing the driver type and dongle ID.
        :return: a transport string
        """
        c_details        = TrbDongleDetails()
        c_details.driver = dongle_details.driver
        c_details.id     = dongle_details.id

        buf_len = 1
        result = 0
        trans_str_buffer = (c_char * buf_len)()

        while True:
            result = self._cfuncs['trb_form_transport_string'](byref(c_details), trans_str_buffer, buf_len)
            if result == TRB_ERROR_MAP.index(TrbErrorBufferTooShort):
                buf_len *= 2
                trans_str_buffer = (c_char * buf_len)()
            else:
                break

        self._handle_error(result)
        return trans_str_buffer.value

    def decode_transport_string(self, transport_string):
        """
        Decode a transport string into a driver and dongle ID.
        :return: a tuple (driver, dongle ID).
        """
        driver = c_int()
        dongle_id = c_int()
        self._handle_error(self._cfuncs['trb_decode_transport_string'](transport_string,
                                                                       byref(driver), byref(dongle_id)))
        return driver.value, dongle_id.value

    @staticmethod
    def _prompt_for_dongle_choice(dongles):
        print "Choose a debug dongle to connect to:"
        print dongles
        choice = raw_input(">>> ")

        try:
            choice = int(choice)
        except ValueError:
            raise ValueError("Invalid debug dongle choice: {0}".format(choice))
        # The list presented to the user is 1-based.
        if choice < 1 or choice > len(dongles):
            raise ValueError("Invalid debug dongle choice: {0}".format(choice))
        return dongles[choice - 1].id

    def _choose_dongle(self, driver):
        dongles = self.build_dongle_list(driver)
        if len(dongles) == 0:
            raise TrbErrorCouldNotEnumerateDevice("No debug dongles found", 0)
        elif len(dongles) == 1:
            return dongles[0].id
        else:
            return self._prompt_for_dongle_choice(dongles)

    def open(self, driver, dongle_id = None):
        """
        Opens a stream associated with the specified debug dongle.
        :param driver: either
        a) One of the values in trbtrans.TrbDriver
        b) A string identifier, e.g. for usb2trb: "usb2trb", plus existing aliases.
        :param dongle_id: the id of the debug dongle to connect to. If dongle_id is None, and there is more than one
        dongle of the specified type attached, the user will be prompted to choose from a list.
        """
        driver = self._translate_driver(driver)
        # If we have a stream open, close it first.
        self.close()

        if dongle_id is None:
            dongle_id = self._choose_dongle(driver)
        self._handle_error(self._cfuncs['trb_stream_open'](driver, dongle_id, byref(self._stream)))

    def sniffer_open(self, driver, dongle_id = None):
        """
        Opens a stream and prepares it for sniffing use. See open().
        On usb2trb, only one stream may be opened for sniffing, system-wide.
        """
        driver = self._translate_driver(driver)
        # If we have a stream open, close it first.
        self.close()

        if dongle_id is None:
            dongle_id = self._choose_dongle(driver)
        self._handle_error(self._cfuncs['trb_sniffer_stream_open'](driver, dongle_id, byref(self._stream)))

    def sample_stream_open(self, driver, dongle_id = None):
        """
        Open a stream intended as a destination for samples generated by the DUT.
        This stream has a fixed routing; transactions addressed to:
        Block destination ID = 0xF
        Tag = 0xE
        will be routed to this stream.
        The usb2trb driver implementation assigns a larger transaction receive buffer to this stream,
        so that buffer wrap conditions will occur less frequently when the incoming data rate is high
        (e.g. when tracing processor execution).
        There is only one same stream available; attempting to open it twice will result in an exception.
        """
        driver = self._translate_driver(driver)
        # If we have a stream open, close it first.
        self.close()

        if dongle_id is None:
            dongle_id = self._choose_dongle(driver)
        self._handle_error(self._cfuncs['trb_sample_stream_open'](driver, dongle_id, byref(self._stream)))

    def close(self):
        """
        Closes any open stream associated with this Trb instance.
        If there is no open stream, this method does nothing.
        """
        if self._stream.value is not None:
            self._cfuncs['trb_stream_close'](self._stream)
        self._stream = c_void_p()

    def __del__(self):
        # If we're called during program teardown, as opposed to just being GC'ed during execution,
        # then globals may not be available. See
        # https://docs.python.org/2/reference/datamodel.html#object.__del__
        # Specifically, we've seen the type c_void_p be None inside close(), giving a TypeError.
        # The best course of action seems to be to just swallow TypeError and NameError exceptions, which are the likely
        # exception types in this sort of situation.
        # We don't swallow *all* exceptions, because we wouldn't want to hide an OSError generated by ctypes
        # intercepting an access violation inside the trbtrans library (via Win32 SEH), for example.
        try:
            self.close()
        except TypeError:
            pass
        except NameError:
            pass
        
    def get_version(self):
        """
        Returns the version of the trbtrans library.
        """
        return self._cfuncs['trb_get_version']()

    def get_dongle_name(self):
        """
        If a stream is open, returns the descriptive name of the debug dongle.
        """
        return str(self._cfuncs['trb_get_dongle_name'](self._stream))

    def get_dongle_details(self):
        """
        If a stream is open, returns a TrbDongle containing the details of the debug dongle corresponding to that
        stream. The fields in the returned object can be passed back to open() or sniffer_open(), to open another
        stream via the same dongle.
        """
        c_details = TrbDongleDetails()
        self._handle_error(self._cfuncs['trb_get_dongle_details'](self._stream, byref(c_details)))
        return TrbDongle(c_details)

    def get_firmware_version(self):
        """
        Returns the firmware version of the usb2trb dongle associated with the active stream.
        Only supported on usb2trb; on other targets, raises TrbErrorOperationNotSupported.
        """
        fw_version = c_int()
        self._handle_error(self._cfuncs['trb_get_firmware_version'](self._stream, byref(fw_version)))
        return fw_version.value
        
    def get_fpga_version(self):
        """
        Returns the FPGA version of the debug dongle associated with the active stream.
        """
        fpga_version = c_uint()
        self._handle_error(self._cfuncs['trb_get_fpga_version'](self._stream, byref(fpga_version)))
        return fpga_version.value

    def get_driver_version(self):
        """
        Returns the installed driver version of the debug dongle associated with the active stream.
        """
        return str(self._cfuncs['trb_get_driver_version'](self._stream))

    def get_bridge_subsys_id(self):
        """
        Returns the subsystem ID of the Transaction Bridge on the attached chip.
        This ID is established by the debug probe hardware when it negotiates a connection
        to the DUT.
        """
        return self._cfuncs['trb_get_bridge_subsys_id'](self._stream)
        
    def get_link_status(self):
        """
        Returns True if the Transaction Bridge link is up.
        Note that this is simply a snapshot of the hardware link status. To query whether a given stream is usable for
        I/O, use trb_is_stream_valid().
        """
        val = c_bool()
        err = self._cfuncs['trb_get_link_status'](self._stream, byref(val))
        self._handle_error(err)
        return val.value

    def is_stream_valid(self):
        """
        Query whether this stream is usable for I/O. This requires the TRB link to be currently up, and the stream to
        not be "stale". A stream becomes stale when the TRB link goes down.
        Only supported on usb2trb; on other targets, raises TrbErrorOperationNotSupported.
        """
        val = c_bool()
        err = self._cfuncs['trb_is_stream_valid'](self._stream, byref(val))
        self._handle_error(err)
        return val.value

    def get_link_statistics(self):
        """
        Returns a tuple of information the Transaction Bridge link:
        (number of transactions received, number of transactions sent, CRC failure count).
        Be aware that these numbers are 8-bit counters, and will regularly wrap.
        """
        num_transactions_rx = c_uint8()
        num_transactions_tx = c_uint8()
        num_crc_failures    = c_uint8()

        err = self._cfuncs['trb_get_link_statistics'](self._stream,
                                                      byref(num_transactions_rx),
                                                      byref(num_transactions_tx),
                                                      byref(num_crc_failures))
        self._handle_error(err)
        return num_transactions_rx.value, num_transactions_tx.value, num_crc_failures.value

    def get_chip_version(self):
        """
        Returns the global chip version of the DUT.
        """
        return int(self._cfuncs['trb_get_chip_version'](self._stream))

    def get_max_transactions(self):
        """
        Returns the maximum number of transactions which can be handled in a single call to sequence().
        If a sequence of actions needs more than this number, multiple sequence calls are required.
        """
        return int(self._cfuncs['trb_get_max_transactions']())

    def sequence(self, actions):
        """
        Execute a sequence of actions. An action is a wrapper for a set of related Transactions that implement, say,
        a block read. The sequence of actions supplied may contain a mix of read/write operations, and may be addressed
        to different subsystems.
        """
        num_actions = len(actions)
        err = self._cfuncs['trb_sequence'](self._stream, actions, num_actions) 
        self._handle_error(err)
        
    def write_raw_transactions(self, transactions, num_transactions):
        """
        Low-level function to write a block of transaction data to the debug dongle. Any responses expected can be
        read separately via read_raw_transactions().
        :param transactions: a ctypes array of Transaction objects, formed by, e.g. (Transaction * N)().
        :param num_transactions: the number of elements in transactions
        """
        err = self._cfuncs['trb_write_raw_transactions'](self._stream, transactions, num_transactions)
        self._handle_error(err)
        
    def read_raw_transactions(self, num_transactions, timeout_millis):
        """
        Low-level function to read a block of transaction data from the debug dongle.
        :param num_transactions the maximum number of transactions to read.
        :param timeout_millis the maximum number of milliseconds to wait. If timeout_millis is negative, this method
        waits forever.

        This method succeeds if n <= num_transactions transactions were successfully
        read (whether or not the timeout was hit), and no unexpected I/O errors occurred.

        If the driver's internal data buffer overflowed, then this method succeeds, and the third element of
        the return tuple is set to True. In this case, there may still be transaction data, but the transactions will be
        discontinuous in time with respect to the last read. Inspect the other elements of the returned tuple as usual
        to get the data.

        :return a tuple: (raw ctypes array of Transactions, num_transactions_actually_read, driver_buffer_wrapped)
        Since the ctypes array is of size num_transactions, avoid using "for ... in" to iterate; instead use
        an (x)range(num_transactions_actually_read) construct.
        """
        buf = (Transaction * num_transactions)()
        num_actually_got = c_int(0)
        err = self._cfuncs['trb_read_raw_transactions'](self._stream, buf, num_transactions, byref(num_actually_got),
                                                        timeout_millis)

        driver_buffer_wrapped = False
        if TRB_ERROR_MAP[err] is TrbErrorDriverBufferWrapped:
            driver_buffer_wrapped = True
        else:
            self._handle_error(err)

        return buf, num_actually_got.value, driver_buffer_wrapped

    def read_dongle_register(self, bar, offset):
        """
        Reads a register in the debug dongle.
        :param bar: pass zero if not applicable to this debug dongle type (e.g. usb2trb).
        :param offset: an offset or address
        :return: the 32-bit value read
        """
        val = c_uint()
        err = self._cfuncs['trb_read_dongle_reg'](self._stream, bar, offset, byref(val))
        self._handle_error(err)
        return val.value

    def write_dongle_register(self, bar, offset, val):
        """
        Writes a register in the debug dongle.
        :param bar: pass zero if not applicable to this debug dongle type (e.g. usb2trb).
        :param offset: an offset or address
        :param val: the 32-bit value to write
        """
        err = self._cfuncs['trb_write_dongle_reg'](self._stream, bar, offset, val)
        self._handle_error(err)

    def read(self, subsys, block_id, addr, bytes_per_transaction, num_bytes, action_type_override = None):
        """
        Reads a block of data, by default using Debug Read Request transactions.
        Blocks until all read responses are received, or the I/O to the driver times out.
        Returns a list of integers, one per octet read.
        Params:
            subsys:     0 to 15
            block_id:   0 to 15
            addr:       0 to (2^32)-1
            bytes_per_transaction: 1, 2 or 4 [or 8 for DATA_READ actions]
            num_bytes:  0 to (5000 * bytes_per_transaction)
            action_type_override: if None, uses TrbActionType.DEBUG_READ. Set to another type to fulfill the request
                                  using a different transaction type. Currently the only alternative is
                                  TrbActionType.DATA_READ (e.g. for reading from the RAM on the usb2trb dongle).
        Note 1: Xap subsystems typically only support 2 bytes per transaction
        Note 2: Xap subsystems are Big endian (high byte first)
        """
        # noinspection PyCallingNonCallable
        a = (TrbAction * 1)()
        a[0].type = action_type_override or TrbActionType.DEBUG_READ
        a[0].dest_subsys_id = subsys
        a[0].dest_block_id = block_id
        a[0].addr = addr
        a[0].num_bytes = num_bytes
        a[0].bytes_per_transaction = bytes_per_transaction
        a[0].data = (c_ubyte * num_bytes)()    # Create a C array of unsigned chars
        self.sequence(a)
        return a[0].data[0:num_bytes]

    # noinspection PyCallingNonCallable
    def write(self, subsys, block, addr, bytes_per_transaction, data_bytes, action_type_override = None):
        """
        Writes a block of data, by default using Debug Write Request transactions.
        Blocks until all write responses are received, or the I/O to the driver times out.
        Params:
            subsys:     0 to 15
            block_id:   0 to 15
            addr:       0 to (2^32)-1
            bytes_per_transaction: 1, 2 or 4
            data_bytes: A list, tuple, or other type that supports indexing, of integers containing the data you wish to
                        write.
                        If you pass values larger than 255 only the bottom 8 bits will be used.
                        The maximum length depends on the number of bytes per transaction:
                        max length = 5000 * bytes_per_transaction (i.e. a maximum of 20000).
            action_type_override: if None, uses TrbActionType.DEBUG_WRITE. Set to another type to fulfill the request
                                  using a different transaction type. Currently the only alternative is
                                  TrbActionType.DATA_WRITE (e.g. for writing to the RAM on the usb2trb dongle).
        Note 1: Xap subsystems typically only support 2 bytes per transaction
        Note 2: Xap subsystems are Big endian (high byte first)
        """
        num_bytes = len(data_bytes)
        a = (TrbAction * 1)()
        a[0].type = action_type_override or TrbActionType.DEBUG_WRITE
        a[0].dest_subsys_id = subsys
        a[0].dest_block_id = block
        a[0].addr = addr
        a[0].num_bytes = num_bytes
        a[0].bytes_per_transaction = bytes_per_transaction
        data_carray = (c_ubyte * num_bytes)()
        a[0].data = data_carray
        data_carray[:] = data_bytes
        self.sequence(a)

    def start_log(self, path):
        """
        Turn on logging for this stream. If path is None or an empty string, the environment variable
        TRBDEBUG_LOGPATH will be used as a directory in which to create log files.
        If that is also missing, then a system default temporary path will be used.
        On Windows, this can be accessed via %TEMP%.
        On Linux, $TMPDIR is used if set, otherwise "/tmp/".

        The filename created will be "trblog_<processname>_<PID>.log". If the environment variable
        TRBDEBUG is set to "ON", then logging will begin as soon as the stream is created. In that case, calling
        this function will raise TrbErrorLoggingAlreadyEnabled.

        In addition to the above, the environment variable TRBDEBUG_FULL may be set to "ON"
        to enable more verbose logging. This is typically useful for debugging possible problems with this library.
        """
        err = self._cfuncs['trb_start_log'](self._stream, path)
        self._handle_error(err)

    def stop_log(self):
        """
        Stops logging for this object's stream.
        """
        self._handle_error(self._cfuncs['trb_stop_log'](self._stream))

    def get_log_path(self):
        """
        Returns the path of the log file for this object's stream. Returns an empty string if logging is not enabled.
        """
        # noinspection PyCallingNonCallable
        path = (c_char * 256)()
        err = self._cfuncs['trb_get_log_path'](self._stream, path, len(path))
        self._handle_error(err)
        return path.value

    def zero_device_statistics(self):
        """
        Set the USB controller link error counters to zero.
        Only supported on usb2trb; on other targets, raises TrbErrorOperationNotSupported.
        """
        err = self._cfuncs['trb_zero_device_statistics'](self._stream)
        self._handle_error(err)

    def get_device_statistics(self):
        """
        Get information from the dongle's USB controller.
        Only supported on usb2trb; on other targets, raises TrbErrorOperationNotSupported.
        """
        raw_device_stats = ScarDeviceStatsRaw()
        err = self._cfuncs['trb_get_device_statistics'](self._stream, byref(raw_device_stats))
        self._handle_error(err)

        return ScarDeviceStats(raw_device_stats)

    def get_usb_connection_speed(self):
        """
        Retrieve the USB connection speed for the usb2trb dongle associated with the given stream.
        :return: a number corresponding to one of the members of UsbConnectionSpeed.
        Only supported on usb2trb; on other targets, raises TrbErrorOperationNotSupported.
        """
        usb_speed = c_int()
        err = self._cfuncs['trb_get_usb_connection_speed'](self._stream, byref(usb_speed))
        self._handle_error(err)

        return usb_speed.value

    def get_firmware_uptime(self):
        """
        Returns the firmware up-time of the usb2trb dongle associated with this stream.
        Only supported on usb2trb; on other targets, raises TrbErrorOperationNotSupported.
        :return: a tuple (seconds, milliseconds)
        """
        raw_fw_uptime = c_uint()
        self._handle_error(self._cfuncs['trb_get_firmware_uptime'](self._stream, byref(raw_fw_uptime)))

        return int(raw_fw_uptime.value / 1000), int(raw_fw_uptime.value % 1000)

    def dongle_reset(self):
        """
        Resets the usb2trb dongle associated with this stream.
        Only supported on usb2trb; on other targets, raises TrbErrorOperationNotSupported.
        """
        self._handle_error(self._cfuncs['trb_dongle_reset'](self._stream))

    def get_last_tbus_error(self):
        """
        Returns the last Transaction Bus error recorded for the active stream.
        """
        return self._cfuncs['trb_get_last_tbus_error'](self._stream)

    def reset_dut(self, link_reset_timeout_millis = None):
        """
        Reset the DUT attached to the other end of the TBridge, e.g. via a special FPGA register poke on the debug
        dongle which generates a "reset transaction" for the DUT. This only works if both the TBridge on the debug
        dongle and the DUT actually support this operation.

        After issuing the reset:-

        - For usb2trb:-
            - first wait for stream to become invalid as determined by the driver. This ensures that the
              TRB link actually did cycle.
            - if this succeeds, wait for the TRB link to be up and recycle the stream to be usable for I/O
            again.

        - For Murphy, perform a simple wait according to link_reset_timeout_millis. The TRB link status is
        then checked; the wait is assumed to be long enough for the link to have cycled.

        These waits uses a timeout; see link_reset_timeout_millis. If the timeout expires, this method returns False. In
        this case, either the reset didn't work (but we can't be sure), or it took longer than the upper bound allowed
        for.

        Note that even if this method returns True, the DUT may not be fully booted -- we only know that the TBridge
        link is up again. The caller may want to apply chip-specific logic to wait for boot to complete.

        :param link_reset_timeout_millis: if None or zero, use a default timeout value. Otherwise the link reset wait
        uses the specified timeout.
        :return: see above notes
        """
        if link_reset_timeout_millis is None:
            link_reset_timeout_millis = 0

        saw_link_reset = c_bool()
        self._handle_error(self._cfuncs['trb_reset_dut'](
            self._stream, link_reset_timeout_millis, byref(saw_link_reset)
        ))
        return saw_link_reset.value

    def configure_sleep_retries(self, num_retries = -1, retry_interval_millis = 0):
        """
        Configure I/O retries when a TBUS_SUBSYSTEM_ASLEEP error is encountered in a Trb.read(), Trb.write(), or
        Trb.sequence() call on this stream.

        Defaults to ON, with the library's default retry number and interval.
        The library never retries when doing raw I/O via Trb.read_raw_transactions() or Trb.write_raw_transactions().

        TBUS_SUBSYSTEM_ASLEEP may occur when the chip is waking from deep sleep -- in this case the I/O should succeed
        when the subsystem has been clocked and woken up. It may also simply occur when the chip is awake, but the
        subsystem is not clocked, in which case retrying will not help. Therefore, when core dumping a crashed chip, for
        example, it may be appropriate to turn this feature off and arrange for clocks to be enabled separately.

        :param num_retries: set to a negative number to use the library's default. Set to zero to
        disable the feature. Set to a positive number to use that many retries.
        :param retry_interval_millis: how long to wait between retries. Set to zero to use the
        library's default.
        """
        self._cfuncs['trb_configure_sleep_retries'](self._stream, num_retries, retry_interval_millis)


@contextmanager
def scoped_trb_stream(driver, dongle_id=None):
    stream = Trb()
    try:
        stream.open(driver, dongle_id)
        yield stream
    finally:
        stream.close()
