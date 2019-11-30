################################################################################
#
#  TestEngine.py
#
#  Copyright (c) 2018 Qualcomm Technologies International, Ltd.
#  All Rights Reserved.
#  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
#
#  Auto-generated Python wrapper for TestEngine DLL (from TestEngine.h).
#
################################################################################

import os
import logging
import ctypes as CT
from functools import wraps

# Use TestEngine.debug=True to turn on DLL function trace
debug = False
_logger = logging.getLogger('TestEngine')

##############################################


def fntrace(func):
    """Decorator to monitor DLL function calls."""
    @wraps(func)
    def new_fun(*args, **kwargs):
        if debug:
            _logger.debug('TestEngine <- ' + func.func_name + '{0}'.format(args, kwargs))
        result = func(*args, **kwargs)
        if debug:
            _logger.debug('TestEngine -> ' + '{0}'.format(result))
        return result
    return new_fun

##############################################


class TestEngine(object):
    """
    A container for TestEngine DLL info.

    Usage:
    TestEngine.TestEngine("<TestEngine.dll path>" [,debug=False])
    """

    def __init__(self, tools_bin_path, debug=False):
        """Load the TestEngine DLL."""
        try:
            os.environ['PATH'] = tools_bin_path + ';' + os.environ['PATH']
            self.TestEngineDLL = CT.windll.LoadLibrary('TestEngine')
        except Exception as e:
            print('Cannot load TestEngine.dll')
            raise e

    ##########################################

    BER_BIT_COUNT = 0
    BER_BIT_ERRORS = 1
    BER_RCVD_PKTS = 3
    BER_EXP_PKTS = 4
    BER_HDR_ERRORS = 5
    BER_CRC_ERRORS = 6
    BER_UNCORR_ERRORS = 7
    BER_SYNC_ERRORS = 8
    BER_MAX = 9
    VM_STATUS_BOOT = 0
    VM_STATUS_FAIL = 1
    VM_STATUS_RUN = 2
    VM_STATUS_PANIC = 3
    VM_STATUS_EXIT = 4
    CVC_PRODTEST_PASS = 1
    CVC_PRODTEST_FAIL = 2
    CVC_PRODTEST_NO_CHECK = 3
    CVC_PRODTEST_FILE_NOT_FOUND = 4
    TE_INVALID_HANDLE = -1
    TE_ERROR = 0
    TE_OK = 1
    TE_UNSUPPORTED_FUNCTION = 2
    TE_ERROR_NONE = 0x000000
    TE_ERROR_BCCMD_NO_SUCH_VARID = 0x000001
    TE_ERROR_BCCMD_DATA_EXCEEDED = 0x000002
    TE_ERROR_BCCMD_VAR_HAS_NO_VALUE = 0x000003
    TE_ERROR_BCCMD_BAD_VALUE = 0x000004
    TE_ERROR_BCCMD_NO_ACCESS = 0x000005
    TE_ERROR_BCCMD_READ_ONLY = 0x000006
    TE_ERROR_BCCMD_WRITE_ONLY = 0x000007
    TE_ERROR_BCCMD_OTHER_ERROR = 0x000008
    TE_ERROR_BCCMD_PERMISSION_DENIED = 0x000009
    TE_ERROR_HCI_UNKNOWN_COMMAND = 0x010000
    TE_ERROR_HCI_UNKNOWN_CONNECTION_ID = 0x020000
    TE_ERROR_HCI_HARDWARE_FAILURE = 0x030000
    TE_ERROR_HCI_PAGE_TIMEOUT = 0x040000
    TE_ERROR_HCI_AUTHENTICATION_FAILURE = 0x050000
    TE_ERROR_HCI_PIN_MISSING = 0x060000
    TE_ERROR_HCI_MEMORY_CAPACITY_EXCEEDED = 0x070000
    TE_ERROR_HCI_CONNECTION_TIMEOUT = 0x080000
    TE_ERROR_HCI_CONNECTION_LIMIT_EXCEEDED = 0x090000
    TE_ERROR_HCI_SYNCHRONOUS_CONNECTION_LIMIT_EXCEEDED = 0x0a0000
    TE_ERROR_HCI_ACL_CONNECTION_ALREADY_EXISTS = 0x0b0000
    TE_ERROR_HCI_COMMAND_DISALLOWED = 0x0c0000
    TE_ERROR_HCI_CONNECTION_REJECTED_LIMITED_RESOURCES = 0x0d0000
    TE_ERROR_HCI_CONNECTION_REJECTED_SECURITY_REASONS = 0x0e0000
    TE_ERROR_HCI_CONNECTION_REJECTED_UNACCEPTABLE_BD_ADDR = 0x0f0000
    TE_ERROR_HCI_CONNECTION_ACCEPT_TIMEOUT_EXCEEDED = 0x100000
    TE_ERROR_HCI_UNSUPPORTED_FEATURE = 0x110000
    TE_ERROR_HCI_INVALID_COMMAND_PARAMETERS = 0x120000
    TE_ERROR_HCI_REMOTE_USER_TERMINATED_CONNECTION = 0x130000
    TE_ERROR_HCI_REMOTE_DEVICE_TERMINATED_CONNECTION_LOW_RESOURCES = 0x140000
    TE_ERROR_HCI_REMOTE_DEVICE_TERMINATED_CONNECTION_POWER_OFF = 0x150000
    TE_ERROR_HCI_CONNECTION_TERMINATED_BY_LOCAL_HOST = 0x160000
    TE_ERROR_HCI_REPEATED_ATTEMPTS = 0x170000
    TE_ERROR_HCI_PAIRING_NOT_ALLOWED = 0x180000
    TE_ERROR_HCI_UNKNOWN_LMP_PDU = 0x190000
    TE_ERROR_HCI_UNSUPPORTED_REMOTE_FEATURE = 0x1a0000
    TE_ERROR_HCI_SCO_OFFSET_REJECTED = 0x1b0000
    TE_ERROR_HCI_SCO_INTERVAL_REJECTED = 0x1c0000
    TE_ERROR_HCI_SCO_AIR_MODE_REJECTED = 0x1d0000
    TE_ERROR_HCI_INVALID_LMP_PARAMETERS = 0x1e0000
    TE_ERROR_HCI_UNSPECIFIED_ERROR = 0x1f0000
    TE_ERROR_HCI_UNSUPPORTED_LMP_PARAMETER_VALUE = 0x200000
    TE_ERROR_HCI_ROLE_CHANGE_NOT_ALLOWED = 0x210000
    TE_ERROR_HCI_LMP_RESPONSE_TIMEOUT = 0x220000
    TE_ERROR_HCI_LMP_ERROR_TRANSACTION_COLLISION = 0x230000
    TE_ERROR_HCI_LMP_PDU_NOT_ALLOWED = 0x240000
    TE_ERROR_HCI_ENCRYPTION_MODE_NOT_ACCEPTABLE = 0x250000
    TE_ERROR_HCI_LINK_KEY_CANNOT_BE_CHANGED = 0x260000
    TE_ERROR_HCI_REQUESTED_QOS_NOT_SUPPORTED = 0x270000
    TE_ERROR_HCI_INSTANT_PASSED = 0x280000
    TE_ERROR_HCI_PAIRING_WITH_UNIT_KEY_NOT_SUPPORTED = 0x290000
    TE_ERROR_HCI_DIFFERENT_TRANSACTION_COLLISION = 0x2a0000
    TE_ERROR_HCI_RESERVED_1 = 0x2b0000
    TE_ERROR_HCI_QOS_UNACCEPTABLE_PARAMETER = 0x2c0000
    TE_ERROR_HCI_QOS_REJECTED = 0x2d0000
    TE_ERROR_HCI_CHANNEL_CLASSIFICATION_NOT_SUPPORTED = 0x2e0000
    TE_ERROR_HCI_INSUFFICIENT_SECURITY = 0x2f0000
    TE_ERROR_HCI_PARAMETER_OUT_OF_MANDATORY_RANGE = 0x300000
    TE_ERROR_HCI_RESERVED_2 = 0x310000
    TE_ERROR_HCI_ROLE_SWITCH_PENDING = 0x320000
    TE_ERROR_HCI_RESERVED_3 = 0x330000
    TE_ERROR_HCI_RESERVED_SLOT_VIOLATION = 0x340000
    TE_ERROR_HCI_ROLE_SWITCH_FAILED = 0x350000
    TE_ERROR_OUT_OF_MEMORY = 0x01000001
    TE_ERROR_BAD_PARAM = 0x01000002
    TE_ERROR_CONNECTION_SETUP = 0x01000003
    TE_ERROR_FILE_OPEN = 0x01000004
    TE_ERROR_INSUFFICIENT_SPACE = 0x01000005
    TE_ERROR_INVALID_SEQUENCE = 0x01000006
    TE_ERROR_MIBCMD_NO_SESSION = 0x01000010
    TE_ERROR_MIBCMD_GET = 0x01000011
    TE_ERROR_MIBCMD_SET = 0x01000012
    TE_ERROR_CONFIG_READ = 0x01000020
    TE_ERROR_CONFIG_WRITE = 0x01000021
    TE_ERROR_CONFIG_ELEMENT_NOT_FOUND = 0x01000022
    TE_ERROR_CONFIG_DATABASE = 0x01000023
    TE_ERROR_CONFIG_GENERAL = 0x01000024
    TE_CHIP_FAMILY_UNKNOWN = 0x00
    TE_CHIP_FAMILY_BLUECORE = 0x01
    TE_CHIP_FAMILY_MULTI = 0x04
    TE_SUBSYSTEM_BT = 0
    TE_SUBSYSTEM_AUDIO = 1
    TE_SUBSYSTEM_APPS0 = 2
    TE_SUBSYSTEM_APPS1 = 3
    SQIF_APP_VALIDATION_RUNNING = 0x0
    SQIF_APP_VALIDATION_PASS = 0x1
    SQIF_APP_VALIDATION_PASS_NO_APP = 0x2

    #########################################################
    # TESTENGINE_API(int32) teGetVersion(char* versionStr);
    #########################################################
    # TestEngine::teGetVersion
    @fntrace
    def teGetVersion(self, _versionStr=''):
        r"""
        Function wrapper for TestEngine::teGetVersion().

        Python API:
            teGetVersion(_versionStr='')
            returns retval, versionStr.value

        Generated from C API:

            Function :      int32 teGetVersion(char * versionStr)

            Parameters :    versionStr -
                                A pointer to a pre-allocated string that will have the
                                version string copied to it.

            Returns :       Always 1.

            Description :   Retrieves the current version string.

            Example :
                {
                    char versionStr[255];

                    teGetVersion(versionStr);

                    cout << "TestEngine version = " << versionStr << endl;
                }

        """
        self.TestEngineDLL.teGetVersion.restype = CT.c_int32
        self.TestEngineDLL.teGetVersion.argtypes = [CT.c_char_p]
        versionStr = CT.create_string_buffer(_versionStr, 255)
        retval = self.TestEngineDLL.teGetVersion(versionStr)
        return retval, versionStr.value
    #########################################################
    BCSP = 0x1
    USB = 0x2
    H4 = 0x4
    H5 = 0x8
    H4DS = 0x10
    PTAP = 0x40
    TRB = 0x80
    USBDBG = 0x100
    SPI_LPT = 0x1
    SPI_USB = 0x2
    SPI_TRB = 0x3

    #########################################################
    # TESTENGINE_API(uint32) openTestEngine(int32 transport, char const * const transportDevice, uint32 dataRate, int32 retryTimeOut, int32 usbTimeOut);
    #########################################################
    # TestEngine::openTestEngine
    @fntrace
    def openTestEngine(self, transport, transportDevice, dataRate, retryTimeOut, usbTimeOut):
        r"""
        Function wrapper for TestEngine::openTestEngine().

        Python API:
            openTestEngine(transport, transportDevice, dataRate, retryTimeOut, usbTimeOut)
            returns retval

        Generated from C API:

            Function :      uint32 openTestEngine(int32 transport,
                                                    char* transportDevice,
                                                    uint32 dataRate, int32 retryTimeOut,
                                                    int32 usbTimeOut)

            Parameters :    transport -
                                Defines the protocol to be used when setting up the
                                communication with the device, where
                                    BCSP = 1
                                    USB = 2
                                    H4 = 4
                                    H5 = 8
                                    H4DS = 16
                                    PTAP = 64
                                    TRB = 128
                                    USBDBG = 256

                            transportDevice -
                                Defines the physical port to use. A string which defines which
                                port is used, e.g. "\\.\COMn" where 'n' is the number of the
                                COM port. USB device names take the form "\\.\CSRn".
                                <p>NOTE: In C/C++, and possibly some other languages, literal
                                '\' characters are written as "\\", otherwise they are
                                interpreted as the start of an escape sequence. So for these
                                languages, the literal string "\\.\COMn" is written as
                                "\\\\.\\COMn" in the code. The prefix can be dropped for COM
                                ports 1 to 9, i.e. "COM1" works the same as "\\\\.\\COM1".
                                If using PTAP then this is the HifId followed by an optional
                                COM port name if connecting via UART (i.e. "0:\\.\COM1").
                                For TRB, the serial number of the usb2trb converter
                                e.g "123456" may be used. For USBDBG and as an alternative
                                to the serial number for TRB, a number greater than or
                                equal to 1 may be used; the range of valid numbers is
                                from 1 to the number of connected devices. For TRB, these
                                numbers are in the same ascending order as the corresponding
                                usb2trb converter serial numbers, e.g. "1" represents the
                                device with lowest serial number, "2" represents the device
                                with the next lowest serial number, etc. For USBDBG, the
                                numbers reflect the order in which the devices enumerate,
                                E.g "1" should be supplied to connect to the first USBDBG
                                device which was powered up, "2" should be supplied to connect
                                to the second, etc.

                                dataRate -
                                Defines the baud rate to be used (for UART connections only).

                            retryTimeOut -
                                Defines how long the function will retry, in ms, when trying
                                to establish communication with the device. Care should be
                                taken when selecting an appropriate retry timeout. BlueCore1
                                and BlueCore2 running early firmware versions require a longer
                                timeout. 5000ms should be enough to pick up these early
                                devices but this value will make openTestEngine unresponsive.
                                It would be better to set this parameter to 1000ms and call
                                openTestEngine up to 5 times or until it passes.

                            usbTimeout -
                                Where transport = USB or USBDBG, this parameter will define how
                                long the function will retry, in ms, when trying to detect if
                                the specified USB device has enumerated. It has been found that
                                some PC's take a longer time than expected to enumerate a USB
                                device and, in some cases, the PC's internal USB hub may have
                                been reset which can cause a further delay in enumeration.
                                This parameter can be set to accommodate the extra time taken
                                to reset USB devices. If USB is not used then this value can
                                be set to 0.

            Returns :       This function will return 0 on failure or an unsigned integer
                            which defines the handle used as a parameter for all other
                            function calls.

            Description :   One of the openTestEngine... set of functions has to called
                            before any communication with the device can begin.
                            <p>This function opens a connection using a host transport
                            protocol running over USB or a UART. To open a SPI connection with
                            a device which is not SPI-locked, use openTestEngineSpi or
                            openTestEngineSpiTrans. To open a SPI connection with a device
                            which is SPI-locked, use openTestEngineSpiUnlock.

            Example :

                uint32 iHandle = 0;
                int32 iTimeout = 0;

                do
                {
                    cout << "Trying to connect..." << endl;
                    iHandle = openTestEngine(BCSP, "COM1", 115200, 1000, 0);
                    iTimeout += 1000;
                }while(iHandle == 0 && iTimeout < MAX_TIMEOUT);

                if(iHandle != 0)
                {
                    cout << "Connected!" << endl;

                    // Perform all your testing here

                    closeTestEngine(iHandle);
                }

        """
        self.TestEngineDLL.openTestEngine.restype = CT.c_uint32
        self.TestEngineDLL.openTestEngine.argtypes = [CT.c_int32, CT.c_char_p, CT.c_uint32, CT.c_int32, CT.c_int32]
        retval = self.TestEngineDLL.openTestEngine(transport, transportDevice, dataRate, retryTimeOut, usbTimeOut)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(uint32) openTestEngineUnlock(int32 transport, char const * const transportDevice, int32 retryTimeOut, int32 usbTimeOut, const char* unlockKey);
    #########################################################
    # TestEngine::openTestEngineUnlock
    @fntrace
    def openTestEngineUnlock(self, transport, transportDevice, retryTimeOut, usbTimeOut, unlockKey):
        r"""
        Function wrapper for TestEngine::openTestEngineUnlock().

        Python API:
            openTestEngineUnlock(transport, transportDevice, retryTimeOut, usbTimeOut, unlockKey)
            returns retval

        Generated from C API:

            Function :      uint32 openTestEngineUnlock(int32 transport,
                                                    char* transportDevice,
                                                    int32 retryTimeOut,
                                                    int32 usbTimeOut,
                                                    const char* unlockKey)

            Parameters :    transport -
                                Defines the protocol to be used when setting up the
                                communication with the device, where
                                    TRB = 128
                                    USBDBG = 256

                            transportDevice -
                                Defines the physical port to use.
                                For TRB, the serial number of the usb2trb converter
                                e.g "123456" may be used. For USBDBG and as an alternative
                                to the serial number for TRB, a number greater than or
                                equal to 1 may be used; the range of valid numbers is
                                from 1 to the number of connected devices. For TRB, these
                                numbers are in the same ascending order as the corresponding
                                usb2trb converter serial numbers, e.g. "1" represents the
                                device with lowest serial number, "2" represents the device
                                with the next lowest serial number, etc. For USBDBG, there
                                is no guaranteed order which would allow the user to
                                identify a specific corresponding physical device and, if
                                only one device is connected, "1" should be supplied.

                            retryTimeOut -
                                Defines how long the function will retry, in ms, when trying
                                to establish communication with the device. Care should be
                                taken when selecting an appropriate retry timeout.

                            usbTimeout -
                                Where transport = USBDBG, this parameter will define how long
                                the function will retry, in ms, when trying to detect if the
                                specified USB device has enumerated. It has been found that
                                some PC's take a longer time than expected to enumerate a USB
                                device and, in some cases, the PC's internal USB hub may have
                                been reset which can cause a further delay in enumeration.
                                This parameter can be set to accommodate the extra time taken
                                to reset USB devices. If USB is not used then this value can
                                be set to 0.

                            unlockKey -
                                Defines the unlock key.


            Returns :       This function will return 0 on failure or an unsigned integer
                            which defines the handle  used as a parameter for all other
                            function calls.

            Description :   One of the openTestEngine... set of functions has to called
                            before any communication with the device can begin.
                            <p>This function opens a connection using a transport of
                            TRB or USBDBG.

            Example :

                uint32 iHandle = 0;
                int32 iTimeout = 0;

                do
                {
                    cout << "Trying to connect..." << endl;
                    iHandle = openTestEngineUnlock(USBDBG, "1", 1000, 1000,
                    "FB459F10828E85FCC0EB1EB3D2186F58");
                    iTimeout += 1000;
                }while(iHandle == 0 && iTimeout < MAX_TIMEOUT);

                if(iHandle != 0)
                {
                    cout << "Connected!" << endl;

                    // Perform all your testing here

                    closeTestEngine(iHandle);
                }

        """
        self.TestEngineDLL.openTestEngineUnlock.restype = CT.c_uint32
        self.TestEngineDLL.openTestEngineUnlock.argtypes = [CT.c_int32, CT.c_char_p, CT.c_int32, CT.c_int32, CT.c_char_p]
        retval = self.TestEngineDLL.openTestEngineUnlock(transport, transportDevice, retryTimeOut, usbTimeOut, unlockKey)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(uint32) openTestEngineSpi(int32 port, int32 multi, int32 transport);
    #########################################################
    # TestEngine::openTestEngineSpi
    @fntrace
    def openTestEngineSpi(self, port, multi, transport):
        r"""
        Function wrapper for TestEngine::openTestEngineSpi().

        Python API:
            openTestEngineSpi(port, multi, transport)
            returns retval

        Generated from C API:

            Function :      uint32 openTestEngineSpi(int32 port, int32 multi, int32 transport)

            Parameters :    port -
                                Defines the physical port to use. An LPT port number starting
                                from 1, or the id number of a USB SPI device. Set to -1 to
                                use the default (port 1 if LPT, first USB SPI device if USB).
                                NOTE: Default may be overridden by the "SPIPORT" environment
                                variable.

                            multi -
                                A gate on a multi-spi interface. Set to zero when
                                communicating with a single device attached to a standard LPT
                                or USB-SPI port. When using the gang programmer hardware, set
                                to 0 - 15 to select the device to connect to (the port
                                parameter must be SPI_LPT in this case).

                            transport -
                                Defines the SPI transport to be used, where:
                                    SPI_LPT = 1
                                    SPI_USB = 2
                                Set to -1 to use the default (from SPITRANS environment
                                variable if present), otherwise LPT.

            Returns :       This function will return 0 on failure or an unsigned integer
                            which defines the handle used as a parameter for all other
                            function calls.

            Description :   One of the openTestEngine... set of functions has to called
                            before any communication with the device can begin.
                            <p>This function, and openTestEngineSpiTrans opens a SPI
                            connection for devices which are not SPI-locked. To open a SPI
                            connection with SPI-locked devices, use openTestEngineSpiUnlock.
                            To open a host transport connection, use openTestEngine.
                            <p>Note that over a SPI transport the function set is limited to
                            bccmd..., radiotest..., and hq... functions. HQ functions (and
                            radiotest functions that generate HQ traffic) are only supported
                            if the BlueCore firmware supports HQ over SPI. Commands which are
                            not supported will return TE_UNSUPPORTED_FUNCTION.

            Example :

                uint32 iHandle = 0;

                iHandle = openTestEngineSpi(1, 0, SPI_LPT);

                if(iHandle != 0)
                {
                    cout << "Connected!" << endl;

                    // Perform all your testing here

                    closeTestEngine(iHandle);
                }

        """
        self.TestEngineDLL.openTestEngineSpi.restype = CT.c_uint32
        self.TestEngineDLL.openTestEngineSpi.argtypes = [CT.c_int32, CT.c_int32, CT.c_int32]
        retval = self.TestEngineDLL.openTestEngineSpi(port, multi, transport)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(uint32) openTestEngineSpiUnlock(int32 port, int32 multi, int32 transport, const char* unlockKey);
    #########################################################
    # TestEngine::openTestEngineSpiUnlock
    @fntrace
    def openTestEngineSpiUnlock(self, port, multi, transport, unlockKey):
        r"""
        Function wrapper for TestEngine::openTestEngineSpiUnlock().

        Python API:
            openTestEngineSpiUnlock(port, multi, transport, unlockKey)
            returns retval

        Generated from C API:

            Function :      uint32 openTestEngineSpiUnlock(int32 port, int32 multi,
                                                           int32 transport,
                                                           const char* unlockKey)

            Parameters :    port -
                                Defines the physical port to use. An LPT port number starting
                                from 1, or the id number of a USB SPI device. Set to -1 to
                                use the default (port 1 if LPT, first USB SPI device if USB).
                                NOTE: Default may be overridden by the "SPIPORT" environment
                                variable.

                            multi -
                                A gate on a multi-spi interface. Set to zero when
                                communicating with a single device attached to a standard LPT
                                or USB-SPI port. When using the gang programmer hardware, set
                                to 0 - 15 to select the device to connect to (the port
                                parameter must be SPI_LPT in this case).

                            transport -
                                Defines the SPI transport to be used, where:
                                    SPI_LPT = 1
                                    SPI_USB = 2
                                Set to -1 to use the default (from SPITRANS environment
                                variable if present), otherwise LPT.

                            unlockKey -
                                Defines the unlock key.

            Returns :       This function will return 0 on failure or an unsigned integer
                            which defines the handle used as a parameter for all other
                            function calls.

            Description :   One of the openTestEngine... set of functions which has to called
                            before any communication with the device can begin.
                            Function openTestEngineSpiUnlockTrans can be used instead.
                            <p>To be used (only) to open a SPI connection to a SPI-locked chip.
                            If openTestEngineSpiUnlock is used to open the test connection,
                            then the functions bccmdSetWarmReset and bccmdSetColdReset will
                            automatically use the same unlock code as openTestEngineSpiUnlock
                            when re-establishing connection with the chip.
                            <p>openTestEngineSpi or openTestEngineSpiTrans should be used to
                            open a SPI connection with a device which is not SPI-locked. To
                            open a host transport connection, use openTestEngine.
                            <p>Note that over a SPI transport the function set is limited to
                            bccmd..., radiotest..., and hq... functions. HQ functions (and
                            radiotest functions that generate HQ traffic) are only supported
                            if the BlueCore firmware supports HQ over SPI. Commands which are
                            not supported will return TE_UNSUPPORTED_FUNCTION.

            Example :

                uint32 iHandle = 0;

                iHandle = openTestEngineSpiUnlock(1, 0, SPI_LPT, "123456789");

                if(iHandle != 0)
                {
                    cout << "Connected!" << endl;

                    // Perform all your testing here

                    closeTestEngine(iHandle);
                }

        """
        self.TestEngineDLL.openTestEngineSpiUnlock.restype = CT.c_uint32
        self.TestEngineDLL.openTestEngineSpiUnlock.argtypes = [CT.c_int32, CT.c_int32, CT.c_int32, CT.c_char_p]
        retval = self.TestEngineDLL.openTestEngineSpiUnlock(port, multi, transport, unlockKey)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(uint32) openTestEngineSpiTrans(char const * const trans, int32 multi);
    #########################################################
    # TestEngine::openTestEngineSpiTrans
    @fntrace
    def openTestEngineSpiTrans(self, trans, multi):
        r"""
        Function wrapper for TestEngine::openTestEngineSpiTrans().

        Python API:
            openTestEngineSpiTrans(trans, multi)
            returns retval

        Generated from C API:

            Function :      uint32 openTestEngineSpiTrans(char const * const trans,
                                                            int32 multi)

            Parameters :    trans -
                                String of space separated transport options that define the
                                transport to use. Commonly used options include:
                                    SPITRANS (The physical transport to use, e.g. LPT, USB)
                                    SPIPORT (The port number)
                                E.g. for LPT1, trans would be "SPITRANS=LPT SPIPORT=1".
                                These options override any environment variables of the same
                                names.
                                The transport string may be one of those returned by
                                teGetAvailableSpiPorts, which returns transport strings for
                                all available SPI ports.

                            multi -
                                A gate on a multi-spi interface. Set to zero when
                                communicating with a single device attached to a standard LPT
                                or USB-SPI port. When using the gang programmer hardware, set
                                to 0 - 15 to select the device to connect to (the port
                                parameter must be SPI_LPT in this case).

            Returns :       This function will return 0 on failure or an unsigned integer
                            which defines the handle used as a parameter for all other
                            function calls.

            Description :   One of the openTestEngine... set of functions has to called
                            before any communication with the device can begin.
                            <p>This function, and openTestEngineSpi opens a SPI
                            connection for devices which are not SPI-locked. To open a SPI
                            connection with SPI-locked devices, use openTestEngineSpiUnlock.
                            To open a host transport connection, use openTestEngine.
                            <p>The trans string may be one of those returned by
                            teGetAvailableSpiPorts, which returns transport strings for all
                            available SPI ports.
                            <p>Note that over a SPI transport the function set is limited to
                            bccmd..., radiotest..., and hq... functions. HQ functions (and
                            radiotest functions that generate HQ traffic) are only supported
                            if the BlueCore firmware supports HQ over SPI. Commands which are
                            not supported will return TE_UNSUPPORTED_FUNCTION.

            Example :

                uint32 iHandle(0);
                uint16 maxLen(256);
                uint16 count(0);
                char* portsStr = new char[maxLen];
                char* transStr = new char[maxLen];
                vector<string> ports; // The human readable port strings (e.g. "LPT1")
                vector<string> trans; // The transport option strings (e.g. "SPITRANS=LPT SPIPORT=1")

                int32 status = teGetAvailableSpiPorts(&maxLen, portsStr, transStr, &count);
                if( status != TE_OK && maxLen != 0 )
                {
                    // Not enough space - resize the storage
                    delete[] portsStr;
                    portsStr = new char[maxLen];
                    delete[] transStr;
                    transStr = new char[maxLen];
                    status = teGetAvailableSpiPorts(&maxLen, portsStr, transStr, &count);
                }
                if( status != TE_OK || count == 0 )
                {
                    cout << "Error getting SPI ports, or none found" << endl;
                    delete[] portsStr;
                    delete[] transStr;
                    return;
                }

                // Split up the comma separated strings of ports / transport options
                split(ports, portsStr, ','); // Use these for user selection (e.g. drop down list)
                split(trans, transStr, ','); // Use these to open a transport

                // Open the SPI port using the trans string
                // For the purposes of this example, we're just using the first in the list
                iHandle = openTestEngineSpiTrans(trans.at(0).c_str(), 0);

                if(iHandle != 0)
                {
                    cout << "Connected!" << endl;

                    // Perform all your testing here

                    closeTestEngine(iHandle);
                }

                delete[] portsStr;
                delete[] transStr;

                return;

        """
        self.TestEngineDLL.openTestEngineSpiTrans.restype = CT.c_uint32
        self.TestEngineDLL.openTestEngineSpiTrans.argtypes = [CT.c_char_p, CT.c_int32]
        retval = self.TestEngineDLL.openTestEngineSpiTrans(trans, multi)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(uint32) openTestEngineSpiUnlockTrans(char const * const trans, int32 multi, const char* unlockKey);
    #########################################################
    # TestEngine::openTestEngineSpiUnlockTrans
    @fntrace
    def openTestEngineSpiUnlockTrans(self, trans, multi, unlockKey):
        r"""
        Function wrapper for TestEngine::openTestEngineSpiUnlockTrans().

        Python API:
            openTestEngineSpiUnlockTrans(trans, multi, unlockKey)
            returns retval

        Generated from C API:

            Function :      uint32 openTestEngineSpiUnlockTrans(char const * const trans,
                                                            int32 multi,
                                                            const char* unlockKey)

            Parameters :    trans -
                                String of space separated transport options that define the
                                transport to use. Commonly used options include:
                                    SPITRANS (The physical transport to use, e.g. LPT, USB)
                                    SPIPORT (The port number)
                                E.g. for LPT1, trans would be "SPITRANS=LPT SPIPORT=1".
                                These options override any environment variables of the same
                                names.
                                The transport string may be one of those returned by
                                teGetAvailableSpiPorts, which returns transport strings for
                                all available SPI ports.

                            multi -
                                A gate on a multi-spi interface. Set to zero when
                                communicating with a single device attached to a standard LPT
                                or USB-SPI port. When using the gang programmer hardware, set
                                to 0 - 15 to select the device to connect to (the port
                                parameter must be SPI_LPT in this case).

                            unlockKey -
                                Defines the unlock key.

            Returns :       This function will return 0 on failure or an unsigned integer
                            which defines the handle used as a parameter for all other
                            function calls.

            Description :   One of the openTestEngine... set of functions which has to called
                            before any communication with the device can begin.
                            Function openTestEngineSpiUnlock can be used instead.
                            <p>To be used (only) to open a SPI connection to a SPI-locked chip.
                            If openTestEngineSpiUnlockTrans is used to open the test
                            connection, then the functions bccmdSetWarmReset and
                            bccmdSetColdReset will automatically use the same unlock code
                            as openTestEngineSpiUnlockTrans when re-establishing connection
                            with the chip.
                            <p>openTestEngineSpi or openTestEngineSpiTrans should be used to
                            open a SPI connection with a device which is not SPI-locked. To
                            open a host transport connection, use openTestEngine.
                            <p>The trans string may be one of those returned by
                            teGetAvailableSpiPorts, which returns transport strings for all
                            available SPI ports.
                            <p>Note that over a SPI transport the function set is limited to
                            bccmd..., radiotest..., and hq... functions. HQ functions (and
                            radiotest functions that generate HQ traffic) are only supported
                            if the BlueCore firmware supports HQ over SPI. Commands which are
                            not supported will return TE_UNSUPPORTED_FUNCTION.

            Example :

                uint32 iHandle(0);
                uint16 maxLen(256);
                uint16 count(0);
                char* portsStr = new char[maxLen];
                char* transStr = new char[maxLen];
                vector<string> ports; // The human readable port strings (e.g. "LPT1")
                vector<string> trans; // The transport option strings (e.g. "SPITRANS=LPT SPIPORT=1")

                int32 status = teGetAvailableSpiPorts(&maxLen, portsStr, transStr, &count);
                if( status != TE_OK && maxLen != 0 )
                {
                    // Not enough space - resize the storage
                    delete[] portsStr;
                    portsStr = new char[maxLen];
                    delete[] transStr;
                    transStr = new char[maxLen];
                    status = teGetAvailableSpiPorts(&maxLen, portsStr, transStr, &count);
                }
                if( status != TE_OK || count == 0 )
                {
                    cout << "Error getting SPI ports, or none found" << endl;
                    delete[] portsStr;
                    delete[] transStr;
                    return;
                }

                // Split up the comma separated strings of ports / transport options
                split(ports, portsStr, ','); // Use these for user selection (e.g. drop down list)
                split(trans, transStr, ','); // Use these to open a transport

                // Open the SPI port using the trans string
                // For the purposes of this example, we're just using the first in the list
                iHandle = openTestEngineSpiUnlockTrans(trans.at(0).c_str(), 0, "123456789");

                if(iHandle != 0)
                {
                    cout << "Connected!" << endl;

                    // Perform all your testing here

                    closeTestEngine(iHandle);
                }

                delete[] portsStr;
                delete[] transStr;

                return;

        """
        self.TestEngineDLL.openTestEngineSpiUnlockTrans.restype = CT.c_uint32
        self.TestEngineDLL.openTestEngineSpiUnlockTrans.argtypes = [CT.c_char_p, CT.c_int32, CT.c_char_p]
        retval = self.TestEngineDLL.openTestEngineSpiUnlockTrans(trans, multi, unlockKey)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) closeTestEngine(uint32 handle);
    #########################################################
    # TestEngine::closeTestEngine
    @fntrace
    def closeTestEngine(self, handle):
        r"""
        Function wrapper for TestEngine::closeTestEngine().

        Python API:
            closeTestEngine(handle)
            returns retval

        Generated from C API:

            Function :      int32 closeTestEngine(uint32 handle)

            Parameters :    handle -
                                Handle to the device.

            Returns :       Always 1.

            Description :   This function should always be called when terminating
                            communications with the device. This function will NOT issue a
                            reset. For CRSA681xx devices using the TRB transport, this function
                            will relock the transport if applicable. Relocking is not supported
                            when using the USBDBG transport.

            Example :

                uint32 iHandle = 0;
                int32 iTimeout = 0;

                do
                {
                    cout << "Trying to connect..." << endl;
                    iHandle = openTestEngine(BCSP, "COM1", 115200, 1000, 0);
                    iTimeout += 1000;
                }while(iHandle == 0 && iTimeout < MAX_TIMEOUT);

                if(iHandle != 0)
                {
                    cout << "Connected!" << endl;

                    // Perform all your testing here

                    closeTestEngine(iHandle);
                }

        """
        self.TestEngineDLL.closeTestEngine.restype = CT.c_int32
        self.TestEngineDLL.closeTestEngine.argtypes = [CT.c_uint32]
        retval = self.TestEngineDLL.closeTestEngine(handle)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(uint32) teGetLastError(uint32 handle);
    #########################################################
    # TestEngine::teGetLastError
    @fntrace
    def teGetLastError(self, handle):
        r"""
        Function wrapper for TestEngine::teGetLastError().

        Python API:
            teGetLastError(handle)
            returns retval

        Generated from C API:

            Function :      uint32 teGetLastError(uint32 handle)

            Parameters :    handle -
                                Handle to the device.

            Returns :       Last reported error code. If this value is zero then no error was
                            reported. A non-zero value signifies an error and the value is
                            decoded as follows:
                            <table>
                                <tr><td>Byte 1 (0x000000ff) = BCCMD error
                                <tr><td>Byte 2 (0x0000ff00) = Reserved
                                <tr><td>Byte 3 (0x00ff0000) = HCI / DM error
                                <tr><td>Byte 4 (0xff000000) = Reserved
                            </table>
                            BCCMD errors are defined as:
                            <table>
                                <tr><td>0 = No error
                                <tr><td>1 = BCCMD command ID not supported
                                <tr><td>2 = Data exceeded
                                <tr><td>3 = Variable ahs no value
                                <tr><td>4 = Get or set command held an error
                                <tr><td>5 = Value inaccessible
                                <tr><td>6 = Unwriteable
                                <tr><td>7 = Unreadable
                                <tr><td>8 = Other error
                                <tr><td>9 = Request not allowed
                            </table>
                            HCI / DM errors are defined in the Bluetooth specification.

            Description :   This function can be called to obtain a more detailed error
                            description when a TestEngine function fails.<br>
                            The return value of this function should not be relied upon unless
                            the preceding TestEngine function call returned 0 (for failure).
                            <br>
                            Error codes in the format returned by this function are defined
                            at the top of TestEngine.h.

            Example :

                uint32 iHandle = 0;
                int32 iTimeout = 0;

                do
                {
                    cout << "Trying to connect..." << endl;
                    iHandle = openTestEngine(BCSP, "COM1", 115200, 1000, 0);
                    iTimeout += 1000;
                }while(iHandle == 0 && iTimeout < MAX_TIMEOUT);

                if(iHandle != 0)
                {
                    cout << "Connected!" << endl;

                    int32 iSuccess = bccmdChargerPsuTrim(iHandle, 15);

                    if(iSuccess == TE_INVALID_HANDLE)
                        cout << "Invalid handle" << endl;
                    else if(iSuccess == TE_UNSUPPORTED_FUNCTION)
                        cout << "Unsupported function" << endl;
                    else if(iSuccess == TE_ERROR)
                    {
                        uint32 iStatus = teGetLastError(iHandle);
                        // Handle iStatus
                    }
                    else
                    {
                        cout << "Command success" << endl;
                        // Perform all your testing here
                    }

                    closeTestEngine(iHandle);
                }

        """
        self.TestEngineDLL.teGetLastError.restype = CT.c_uint32
        self.TestEngineDLL.teGetLastError.argtypes = [CT.c_uint32]
        retval = self.TestEngineDLL.teGetLastError(handle)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdSetColdReset(uint32 handle, int32 usbTimeout);
    #########################################################
    # TestEngine::bccmdSetColdReset
    @fntrace
    def bccmdSetColdReset(self, handle, usbTimeout):
        r"""
        Function wrapper for TestEngine::bccmdSetColdReset().

        Python API:
            bccmdSetColdReset(handle, usbTimeout)
            returns retval

        Generated from C API:

            Function :      int32 bccmdSetColdReset(uint32 handle, int32 usbTimeout)

            Parameters :    handle -
                                Handle to the device.

                            usbTimeout -
                                Where transport = USB, this parameter will define the timeout
                                to allow the USB device to enumerate if it has not already
                                enumerated. It has been found that some PC's take a longer
                                time than expected to enumerate a USB device and, in some
                                cases, the PC's internal USB hub may have been reset which can
                                cause a further delay in enumeration. This parameter can be
                                set to accommodate the extra time taken to reset USB devices.

            Returns :       -1 on invalid handle, 1 on success, 0 on failure.

            Description :   This command forces a hardware reset of BlueCore, deliberately
                            discarding all of its current state - the command emulates removing
                            power from the chip, then restoring it. It will attempt to
                            reinitialise communication using the same transport settings as
                            declared in openTestEngine...(). If the reset or the
                            re-establishment fails the objects associated with the
                            communication stack will be deleted.

            Example :

                uint32 iHandle = 0;
                int32 iTimeout = 0;

                do
                {
                    cout << "Trying to connect..." << endl;
                    iHandle = openTestEngine(BCSP, "COM1", 115200, 1000, 0);
                    iTimeout += 1000;
                }while(iHandle == 0 && iTimeout < MAX_TIMEOUT);

                if(iHandle != 0)
                {
                    int32 iResetError = 0;

                    do
                    {
                        cout << "Connected!" << endl;

                        int32 iSuccess = psWriteVmDisable(iHandle, 1);

                        if(iSuccess != TE_OK)
                        {
                            cout << "psWriteVmDisable error" << endl;
                            break;
                        }

                        iSuccess = bccmdSetColdReset(iHandle, 0);

                        if(iSuccess != TE_OK)
                        {
                            iResetError = 1;
                            cout << "bccmdSetColdReset error" << endl;
                            break;
                        }

                        iSuccess = hciSlave(iHandle);

                        if(iSuccess != TE_OK)
                        {
                            cout << "hciSlave error" << endl;
                            if(iSuccess == TE_UNSUPPORTED_FUNCTION)
                            {
                                cout << "Unsupported command - if using RFCOMM build use dmSlave instead"
                                     << endl;
                            }
                            break;
                        }

                        iSuccess = hciEnableDeviceUnderTestMode(iHandle);

                        if(iSuccess != TE_OK)
                        {
                            cout << "hciEnableDeviceUnderTestMode error" << endl;
                            if(iSuccess == TE_UNSUPPORTED_FUNCTION)
                            {
                                cout << "Unsupported command - if using RFCOMM build use dmEnableDeviceUnderTestMode instead"
                                     << endl;
                            }
                            break;
                        }

                        // PERFORM TESTING HERE.....
                        cout << "TESTING.........." << endl;

                        iSuccess = psWriteVmDisable(iHandle, 0);

                        if(iSuccess != TE_OK)
                        {
                            cout << "psWriteVmDisable error" << endl;
                            break;
                        }

                        iSuccess = bccmdSetColdReset(iHandle, 0);

                        if(iSuccess != TE_OK)
                        {
                            iResetError = 1;
                            cout << "bccmdSetColdReset error" << endl;
                            break;
                        }

                    }while(0);

                    if(!iResetError)
                    {
                        closeTestEngine(iHandle);
                    }
                }


        """
        self.TestEngineDLL.bccmdSetColdReset.restype = CT.c_int32
        self.TestEngineDLL.bccmdSetColdReset.argtypes = [CT.c_uint32, CT.c_int32]
        retval = self.TestEngineDLL.bccmdSetColdReset(handle, usbTimeout)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdSetWarmReset(uint32 handle, int32 usbTimeout);
    #########################################################
    # TestEngine::bccmdSetWarmReset
    @fntrace
    def bccmdSetWarmReset(self, handle, usbTimeout):
        r"""
        Function wrapper for TestEngine::bccmdSetWarmReset().

        Python API:
            bccmdSetWarmReset(handle, usbTimeout)
            returns retval

        Generated from C API:

            Function :      int32 bccmdSetWarmReset(uint32 handle, int32 usbTimeout)

            Parameters :    handle -
                                Handle to the device.

                            usbTimeout -
                                Where transport = USB, this parameter will define the timeout
                                to allow the USB device to enumerate if it has not already
                                enumerated. It has been found that some PC's take a longer
                                time than expected to enumerate a USB device and, in some
                                cases, the PC's internal USB hub may have been reset which can
                                cause a further delay in enumeration. This parameter can be
                                set to accommodate the extra time taken to reset USB devices.

            Returns :       -1 on invalid handle, 1 on success, 0 on failure.

            Description :   This forces a hardware reset of the BlueCore, arranging that some
                            elements of the chip?s current state may be available when the
                            chip is restarted, assuming the chip remains powered through the
                            reset. It will then attempt to reinitialise communication using
                            the same transport settings as declared in openTestEngine...().
                            If the reset or the re-establishment fails the objects associated
                            with the communication stack will be deleted.

            Example :

                uint32 iHandle = 0;
                int32 iTimeout = 0;

                do
                {
                    cout << "Trying to connect..." << endl;
                    iHandle = openTestEngine(BCSP, "COM1", 115200, 1000, 0);
                    iTimeout += 1000;
                }while(iHandle == 0 && iTimeout < MAX_TIMEOUT);

                if(iHandle != 0)
                {
                    int32 iResetError = 0;
                    do
                    {
                        cout << "Connected!" << endl;

                        int32 iSuccess = psWriteVmDisable(iHandle, 1);

                        if(iSuccess != TE_OK)
                        {
                            cout << "psWriteVmDisable error" << endl;
                            break;
                        }

                        iSuccess = bccmdSetWarmReset(iHandle, 0);

                        if(iSuccess != TE_OK)
                        {
                            iResetError = 1;
                            cout << "bccmdSetWarmReset error" << endl;
                            break;
                        }

                        iSuccess = hciSlave(iHandle);

                        if(iSuccess != TE_OK)
                        {
                            cout << "hciSlave error" << endl;
                            if(iSuccess == TE_UNSUPPORTED_FUNCTION)
                            {
                                cout << "Unsupported command - if using RFCOMM build use dmSlave instead"
                                     << endl;
                            }
                            break;
                        }

                        iSuccess = hciEnableDeviceUnderTestMode(iHandle);

                        if(iSuccess != TE_OK)
                        {
                            cout << "hciEnableDeviceUnderTestMode error" << endl;
                            if(iSuccess == TE_UNSUPPORTED_FUNCTION)
                            {
                                cout << "Unsupported command - if using RFCOMM build use dmEnableDeviceUnderTestMode instead"
                                     << endl;
                            }
                            break;
                        }

                        // PERFORM TESTING HERE.....
                        cout << "TESTING.........." << endl;

                        iSuccess = psWriteVmDisable(iHandle, 0);

                        if(iSuccess != TE_OK)
                        {
                            cout << "psWriteVmDisable error" << endl;
                            break;
                        }

                        iSuccess = bccmdSetWarmReset(iHandle, 0);

                        if(iSuccess != TE_OK)
                        {
                            iResetError = 1;
                            cout << "bccmdSetWarmReset error" << endl;
                            break;
                        }

                    }while(0);

                    if(!iResetError)
                    {
                        closeTestEngine(iHandle);
                    }
                }


        """
        self.TestEngineDLL.bccmdSetWarmReset.restype = CT.c_int32
        self.TestEngineDLL.bccmdSetWarmReset.argtypes = [CT.c_uint32, CT.c_int32]
        retval = self.TestEngineDLL.bccmdSetWarmReset(handle, usbTimeout)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestPause(uint32 handle);
    #########################################################
    # TestEngine::radiotestPause
    @fntrace
    def radiotestPause(self, handle):
        r"""
        Function wrapper for TestEngine::radiotestPause().

        Python API:
            radiotestPause(handle)
            returns retval

        Generated from C API:

            Function :      int32 radiotestPause(uint32 handle)

            Parameters :    handle -
                                Handle to the device

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will cause any tests running to terminate.

        """
        self.TestEngineDLL.radiotestPause.restype = CT.c_int32
        self.TestEngineDLL.radiotestPause.argtypes = [CT.c_uint32]
        retval = self.TestEngineDLL.radiotestPause(handle)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestDeepSleep(uint32 handle);
    #########################################################
    # TestEngine::radiotestDeepSleep
    @fntrace
    def radiotestDeepSleep(self, handle):
        r"""
        Function wrapper for TestEngine::radiotestDeepSleep().

        Python API:
            radiotestDeepSleep(handle)
            returns retval

        Generated from C API:

            Function :      int32 radiotestDeepSleep(uint32 handle)

            Parameters :    handle -
                                Handle to the device

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will cause the DUT to enter deep sleep.

        """
        self.TestEngineDLL.radiotestDeepSleep.restype = CT.c_int32
        self.TestEngineDLL.radiotestDeepSleep.argtypes = [CT.c_uint32]
        retval = self.TestEngineDLL.radiotestDeepSleep(handle)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestPcmExtLb(uint32 handle, uint16 pcm_mode);
    #########################################################
    # TestEngine::radiotestPcmExtLb
    @fntrace
    def radiotestPcmExtLb(self, handle, pcm_mode):
        r"""
        Function wrapper for TestEngine::radiotestPcmExtLb().

        Python API:
            radiotestPcmExtLb(handle, pcm_mode)
            returns retval

        Generated from C API:

            Function :      int32 radiotestPcmExtLb(uint32 handle, uint16 pcm_mode)

            Parameters :    handle -
                                Handle to the device

                            pcm_mode -
                                Can be:
                                <table>
                                    <tr><td>0  Slave in normal 4-wire configuration
                                    <tr><td>1  Master in normal 4-wire configuration
                                    <tr><td>2  Master in Alcatel-specific 2-wire configuration
                                </table>
                                <p>
                                For a ?normal? loopback configuration pcm_mode should be 1.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This command enables PCM external loopback test mode in which a
                            block of random data is written to the PCM output port and is read
                            back again on the PCM input port. A check is made that the data
                            matches. External wiring must be provided between the corresponding
                            pins.
                            <p>
                            If supported, use radiotestPcmExtLbIf for devices with multiple PCM
                            ports.

        """
        self.TestEngineDLL.radiotestPcmExtLb.restype = CT.c_int32
        self.TestEngineDLL.radiotestPcmExtLb.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.radiotestPcmExtLb(handle, pcm_mode)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestPcmExtLbIf(uint32 handle, uint16 pcm_mode, uint16 pcm_if);
    #########################################################
    # TestEngine::radiotestPcmExtLbIf
    @fntrace
    def radiotestPcmExtLbIf(self, handle, pcm_mode, pcm_if):
        r"""
        Function wrapper for TestEngine::radiotestPcmExtLbIf().

        Python API:
            radiotestPcmExtLbIf(handle, pcm_mode, pcm_if)
            returns retval

        Generated from C API:

            Function :      int32 radiotestPcmExtLbIf(uint32 handle, uint16 pcm_mode,
                                                       uint16 pcm_if)

            Parameters :    handle -
                                Handle to the device

                            pcm_mode -
                                Can be:
                                <table>
                                    <tr><td>0  Slave in normal 4-wire configuration
                                    <tr><td>1  Master in normal 4-wire configuration
                                    <tr><td>2  Master in Alcatel-specific 2-wire configuration
                                </table>
                                <p>
                                For a ?normal? loopback configuration pcm_mode should be 1.

                            pcm_if -
                                Selects the PCM interface:
                                <table>
                                    <tr><td>0  PCM1
                                    <tr><td>1  PCM2
                                </table>

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This command enables PCM external loopback test mode in which a
                            block of random data is written to the PCM output port and is read
                            back again on the PCM input port. A check is made that the data
                            matches. External wiring must be provided between the corresponding
                            pins.
                            <p>
                            This function should be supported in BlueCore firmware from version
                            25a onwards. Use radiotestPcmExtLb if the firmware is older.

        """
        self.TestEngineDLL.radiotestPcmExtLbIf.restype = CT.c_int32
        self.TestEngineDLL.radiotestPcmExtLbIf.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16]
        retval = self.TestEngineDLL.radiotestPcmExtLbIf(handle, pcm_mode, pcm_if)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestPcmLb(uint32 handle, uint16 pcm_mode);
    #########################################################
    # TestEngine::radiotestPcmLb
    @fntrace
    def radiotestPcmLb(self, handle, pcm_mode):
        r"""
        Function wrapper for TestEngine::radiotestPcmLb().

        Python API:
            radiotestPcmLb(handle, pcm_mode)
            returns retval

        Generated from C API:

            Function :      int32 radiotestPcmLb(uint32 handle, uint16 pcm_mode)

            Parameters :    handle -
                                Handle to the device

                            pcm_mode -
                                Can be:
                                <table>
                                    <tr><td>0  Slave in normal 4-wire configuration
                                    <tr><td>1  Master in normal 4-wire configuration
                                    <tr><td>2  Master in Alcatel-specific 2-wire configuration
                                </table>
                                <p>
                                For a ?normal? loopback configuration pcm_mode should be 1.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This command enables PCM LOOPBACK test mode in which data read
                            from the PCM input port is written back out again via the pulse
                            code modulation (PCM) output port after a delay.
                            <p>
                            If supported, use radiotestPcmLbIf for devices with multiple PCM
                            ports.

        """
        self.TestEngineDLL.radiotestPcmLb.restype = CT.c_int32
        self.TestEngineDLL.radiotestPcmLb.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.radiotestPcmLb(handle, pcm_mode)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestPcmLbIf(uint32 handle, uint16 pcm_mode, uint16 pcm_if);
    #########################################################
    # TestEngine::radiotestPcmLbIf
    @fntrace
    def radiotestPcmLbIf(self, handle, pcm_mode, pcm_if):
        r"""
        Function wrapper for TestEngine::radiotestPcmLbIf().

        Python API:
            radiotestPcmLbIf(handle, pcm_mode, pcm_if)
            returns retval

        Generated from C API:

            Function :      int32 radiotestPcmLbIf(uint32 handle, uint16 pcm_mode,
                                                     uint16 pcm_if)

            Parameters :    handle -
                                Handle to the device

                            pcm_mode -
                                Can be:
                                <table>
                                    <tr><td>0  Slave in normal 4-wire configuration
                                    <tr><td>1  Master in normal 4-wire configuration
                                    <tr><td>2  Master in Alcatel-specific 2-wire configuration
                                </table>
                                <p>
                                For a ?normal? loopback configuration pcm_mode should be 1.

                            pcm_if -
                                Selects the PCM interface:
                                <table>
                                    <tr><td>0  PCM1
                                    <tr><td>1  PCM2
                                </table>

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This command enables PCM LOOPBACK test mode in which data read
                            from the PCM input port is written back out again via the pulse
                            code modulation (PCM) output port after a delay.
                            <p>
                            This function should be supported in BlueCore firmware from version
                            25a onwards. Use radiotestPcmLb if the firmware is older.

        """
        self.TestEngineDLL.radiotestPcmLbIf.restype = CT.c_int32
        self.TestEngineDLL.radiotestPcmLbIf.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16]
        retval = self.TestEngineDLL.radiotestPcmLbIf(handle, pcm_mode, pcm_if)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestPcmTimingIn(uint32 handle, uint16 pio_out, uint16 pcm_in);
    #########################################################
    # TestEngine::radiotestPcmTimingIn
    @fntrace
    def radiotestPcmTimingIn(self, handle, pio_out, pcm_in):
        r"""
        Function wrapper for TestEngine::radiotestPcmTimingIn().

        Python API:
            radiotestPcmTimingIn(handle, pio_out, pcm_in)
            returns retval

        Generated from C API:

            Function :      int32 radiotestPcmTimingIn(uint32 handle, uint16 pio_out,
                                                        uint16 pcm_in)

            Parameters :    handle -
                                Handle to the device

                            pio_out -
                                0 to 7, PIO pin used for writing

                            pcm_in -
                                PCM pin used for reading where:
                                <table>
                                    <tr><td>0  PCM_SYNC
                                    <tr><td>1  PCM_CLK
                                </table>

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This command is similar to PCM_EXT_LB: this makes a series of
                            writes to a PIO pin and tests that the result is readable on one
                            of the PCM pins used for timing in slave mode, PCM_SYNC and
                            PCM_CLK.
                            A check is made that the data matches. External wiring must be
                            provided between the corresponding pins.
                            <p>
                            If supported, use radiotestPcmTimingInIf for devices with multiple
                            PCM ports.

        """
        self.TestEngineDLL.radiotestPcmTimingIn.restype = CT.c_int32
        self.TestEngineDLL.radiotestPcmTimingIn.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16]
        retval = self.TestEngineDLL.radiotestPcmTimingIn(handle, pio_out, pcm_in)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestPcmTimingInIf(uint32 handle, uint16 pio_out, uint16 pcm_in, uint16 pcm_if);
    #########################################################
    # TestEngine::radiotestPcmTimingInIf
    @fntrace
    def radiotestPcmTimingInIf(self, handle, pio_out, pcm_in, pcm_if):
        r"""
        Function wrapper for TestEngine::radiotestPcmTimingInIf().

        Python API:
            radiotestPcmTimingInIf(handle, pio_out, pcm_in, pcm_if)
            returns retval

        Generated from C API:

            Function :      int32 radiotestPcmTimingInIf(uint32 handle, uint16 pio_out,
                                                          uint16 pcm_in, uint16 pcm_if)

            Parameters :    handle -
                                Handle to the device

                            pio_out -
                                0 to 7, PIO pin used for writing

                            pcm_in -
                                PCM pin used for reading where:
                                <table>
                                    <tr><td>0  PCM_SYNC
                                    <tr><td>1  PCM_CLK
                                </table>

                            pcm_if -
                                Selects the PCM interface:
                                <table>
                                    <tr><td>0  PCM1
                                    <tr><td>1  PCM2
                                </table>

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This command is similar to PCM_EXT_LB_IF: this makes a series of
                            writes to a PIO pin and tests that the result is readable on one
                            of the PCM pins used for timing in slave mode, PCM_SYNC and
                            PCM_CLK.
                            A check is made that the data matches. External wiring must be
                            provided between the corresponding pins.
                            <p>
                            This function should be supported in BlueCore firmware from version
                            25a onwards. Use radiotestPcmTimingIn if the firmware is older.

        """
        self.TestEngineDLL.radiotestPcmTimingInIf.restype = CT.c_int32
        self.TestEngineDLL.radiotestPcmTimingInIf.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16, CT.c_uint16]
        retval = self.TestEngineDLL.radiotestPcmTimingInIf(handle, pio_out, pcm_in, pcm_if)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestPcmTone(uint32 handle, uint16 freq, uint16 ampl, uint16 dc);
    #########################################################
    # TestEngine::radiotestPcmTone
    @fntrace
    def radiotestPcmTone(self, handle, freq, ampl, dc):
        r"""
        Function wrapper for TestEngine::radiotestPcmTone().

        Python API:
            radiotestPcmTone(handle, freq, ampl, dc)
            returns retval

        Generated from C API:

            Function :      int32 radiotestPcmTone(uint32 handle, uint16 freq, uint16 ampl,
                                                    uint16 dc)

            Parameters :    handle -
                                Handle to the device

                            freq -
                                Controls the frequency of the sine wave. The value 0
                                corresponds to 250Hz; each increment of 1 doubles the
                                frequency. Hence, 1 corresponds to 500Hz, 2 to 1kHz and so on.
                                No bounds checking is performed on the parameter, but large
                                values will not be useful owing to hardware limitations.

                            ampl -
                                Controls the amplitude of the sine wave. 8 is full volume;
                                Each decrement of 1 reduces the amplitude by a factor of 2.
                                0 is valid and causes the hardware to be activated with
                                constant audio data.

                            dc -
                                Constant offset to be added to the audio data.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This command outputs a sine wave to the PCM port. For chips with
                            stereo codecs, use radiotestPcmToneStereo instead.
                            <p>PSKEY_HOSTIO_MAP_SCO_CODEC must be set to 1 for this function
                            to successfully generate audio through the codec.
                            <p>
                            If supported, use radiotestPcmToneIf for devices with multiple
                            PCM ports.

        """
        self.TestEngineDLL.radiotestPcmTone.restype = CT.c_int32
        self.TestEngineDLL.radiotestPcmTone.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16, CT.c_uint16]
        retval = self.TestEngineDLL.radiotestPcmTone(handle, freq, ampl, dc)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestPcmToneIf(uint32 handle, uint16 freq, uint16 ampl, uint16 dc, uint16 pcm_if);
    #########################################################
    # TestEngine::radiotestPcmToneIf
    @fntrace
    def radiotestPcmToneIf(self, handle, freq, ampl, dc, pcm_if):
        r"""
        Function wrapper for TestEngine::radiotestPcmToneIf().

        Python API:
            radiotestPcmToneIf(handle, freq, ampl, dc, pcm_if)
            returns retval

        Generated from C API:

            Function :      int32 radiotestPcmToneIf(uint32 handle, uint16 freq, uint16 ampl,
                                                      uint16 dc, uint16 pcm_if)

            Parameters :    handle -
                                Handle to the device

                            freq -
                                Controls the frequency of the sine wave. The value 0
                                corresponds to 250Hz; each increment of 1 doubles the
                                frequency. Hence, 1 corresponds to 500Hz, 2 to 1kHz and so on.
                                No bounds checking is performed on the parameter, but large
                                values will not be useful owing to hardware limitations.

                            ampl -
                                Controls the amplitude of the sine wave. 8 is full volume;
                                Each decrement of 1 reduces the amplitude by a factor of 2.
                                0 is valid and causes the hardware to be activated with
                                constant audio data.

                            dc -
                                Constant offset to be added to the audio data.

                            pcm_if -
                                Selects the PCM interface:
                                <table>
                                    <tr><td>0  PCM1
                                    <tr><td>1  PCM2
                                </table>

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This command outputs a sine wave to the PCM port. For chips with
                            stereo codecs, use radiotestPcmToneStereo instead.
                            <p>PSKEY_HOSTIO_MAP_SCO_CODEC must be set to 1 for this function
                            to successfully generate audio through the codec.
                            <p>
                            This function should be supported in BlueCore firmware from version
                            25a onwards. Use radiotestPcmTone if the firmware is older.

        """
        self.TestEngineDLL.radiotestPcmToneIf.restype = CT.c_int32
        self.TestEngineDLL.radiotestPcmToneIf.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16, CT.c_uint16, CT.c_uint16]
        retval = self.TestEngineDLL.radiotestPcmToneIf(handle, freq, ampl, dc, pcm_if)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestPcmToneStereo(uint32 handle, uint16 freq, uint16 ampl, uint16 dc, uint16 channel);
    #########################################################
    # TestEngine::radiotestPcmToneStereo
    @fntrace
    def radiotestPcmToneStereo(self, handle, freq, ampl, dc, channel):
        r"""
        Function wrapper for TestEngine::radiotestPcmToneStereo().

        Python API:
            radiotestPcmToneStereo(handle, freq, ampl, dc, channel)
            returns retval

        Generated from C API:

            Function :      int32 radiotestPcmToneStereo(uint32 handle, uint16 freq,
                                                         uint16 ampl, uint16 dc,
                                                         uint16 channel)

            Parameters :    handle -
                                Handle to the device

                            freq -
                                Controls the frequency of the sine wave. The value 0
                                corresponds to 250Hz; each increment of 1 doubles the
                                frequency. Hence, 1 corresponds to 500Hz, 2 to 1kHz and so on.
                                No bounds checking is performed on the parameter, but large
                                values will not be useful owing to hardware limitations.

                            ampl -
                                Controls the amplitude of the sine wave. 8 is full volume;
                                Each decrement of 1 reduces the amplitude by a factor of 2.
                                0 is valid and causes the hardware to be activated with
                                constant audio data.

                            dc -
                                Constant offset to be added to the audio data.

                            channel -
                                Controls the channel(s) on which the sine wave is output,
                                where:
                                <table>
                                    <tr><td>0 = Both (L & R)
                                    <tr><td>1 = Left only
                                    <tr><td>2 = Right only
                                </table>

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This command outputs a sine wave to the PCM port on either one, or
                            both stereo channels. Requires a BlueCore chip with a stereo
                            codec. For mono chips, use radiotestPcmTone.
                            <p>PSKEY_HOSTIO_MAP_SCO_CODEC must be set to 1 for this function
                            to successfully generate audio through the codec.

        """
        self.TestEngineDLL.radiotestPcmToneStereo.restype = CT.c_int32
        self.TestEngineDLL.radiotestPcmToneStereo.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16, CT.c_uint16, CT.c_uint16]
        retval = self.TestEngineDLL.radiotestPcmToneStereo(handle, freq, ampl, dc, channel)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestCtsRtsLb(uint32 handle);
    #########################################################
    # TestEngine::radiotestCtsRtsLb
    @fntrace
    def radiotestCtsRtsLb(self, handle):
        r"""
        Function wrapper for TestEngine::radiotestCtsRtsLb().

        Python API:
            radiotestCtsRtsLb(handle)
            returns retval

        Generated from C API:

            Function :      int32 radiotestCtsRtsLb(uint32 handle)

            Parameters :    handle -
                                Handle to the device

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This command performs a series of writes to the UART CTS (clear to
                            send) line and attempts to read back the value written on the UART
                            RTS (ready to send) line. External wiring must be provided between
                            the corresponding pins.

        """
        self.TestEngineDLL.radiotestCtsRtsLb.restype = CT.c_int32
        self.TestEngineDLL.radiotestCtsRtsLb.argtypes = [CT.c_uint32]
        retval = self.TestEngineDLL.radiotestCtsRtsLb(handle)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestRadioStatus(uint32 handle);
    #########################################################
    # TestEngine::radiotestRadioStatus
    @fntrace
    def radiotestRadioStatus(self, handle):
        r"""
        Function wrapper for TestEngine::radiotestRadioStatus().

        Python API:
            radiotestRadioStatus(handle)
            returns retval

        Generated from C API:

            Function :      int32 radiotestRadioStatus(uint32 handle)

            Parameters :    handle -
                                Handle to the device

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will request BlueCore to send the current radio
                            status information. Once the command is issued BlueCore will need
                            to be polled for the status using the hqGetRadioStatus(..)
                            function.
                            <p>This function is only supported over a SPI connection if HQ
                            over SPI is supported by the device firmware.
                            <p>This function is deprecated. Use radiotestRadioStatusArray
                            instead.

            Deprecated :

        """
        self.TestEngineDLL.radiotestRadioStatus.restype = CT.c_int32
        self.TestEngineDLL.radiotestRadioStatus.argtypes = [CT.c_uint32]
        retval = self.TestEngineDLL.radiotestRadioStatus(handle)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hqGetRadioStatus(uint32 handle, uint16 r[], int32 timeout);
    #########################################################
    # TestEngine::hqGetRadioStatus
    @fntrace
    def hqGetRadioStatus(self, handle, r, timeout):
        r"""
        Function wrapper for TestEngine::hqGetRadioStatus().

        Python API:
            hqGetRadioStatus(handle, r, timeout)
            returns retval

        Generated from C API:

            Function :      int32 hqGetRadioStatus(uint32 handle, uint16 * r, int32 timeout)

            Parameters :    handle -
                                Handle to the device

                            r -
                                Pointer to an array of 7 unsigned 16-bit integers. These will
                                be used to store the radio status array as follows:
                                <table>
                                    <tr><td>Index Parameter<td>Type of Status Report
                                    <tr><td>0   <td>Internal transmission level
                                    <tr><td>1   <td>External transmission level
                                    <tr><td>2   <td>Reception level
                                    <tr><td>3   <td>Receiver?s attenuation
                                    <tr><td>4   <td>Local oscillator level
                                    <tr><td>5   <td>IQ trim
                                    <tr><td>6   <td>Signal to image ratio in dB
                                </table>

                            timeout -
                                Used to specify a timeout, in ms, which the software will wait
                                for the HQ radio status packet to be returned from BlueCore.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will wait for a time period specified by "timeout"
                            for the HQ radio status packet.
                            <p>This function is deprecated. Use hqGetRadioStatusArray
                            instead.

            Deprecated :

        """
        self.TestEngineDLL.hqGetRadioStatus.restype = CT.c_int32
        self.TestEngineDLL.hqGetRadioStatus.argtypes = [CT.c_uint32, CT.c_void_p, CT.c_int32]
        __r = (CT.c_uint16 * len(r))(*r)
        retval = self.TestEngineDLL.hqGetRadioStatus(handle, CT.byref(__r), timeout)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestRadioStatusArray(uint32 handle);
    #########################################################
    # TestEngine::radiotestRadioStatusArray
    @fntrace
    def radiotestRadioStatusArray(self, handle):
        r"""
        Function wrapper for TestEngine::radiotestRadioStatusArray().

        Python API:
            radiotestRadioStatusArray(handle)
            returns retval

        Generated from C API:

            Function :      int32 radiotestRadioStatusArray(uint32 handle)

            Parameters :    handle -
                                Handle to the device

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will request BlueCore to send the current radio
                            status information. Once the command is issued BlueCore will need
                            to be polled for the status using the hqGetRadioStatusArray(..)
                            function.
                            <p>This function is only supported over a SPI connection if HQ
                            over SPI is supported by the device firmware.

            Example :

                uint32 iHandle = 0;
                int32 iTimeout = 0;

                do
                {
                    cout << "Trying to connect..." << endl;
                    iHandle = openTestEngine(BCSP, "COM1", 115200, 1000, 0);
                    iTimeout += 1000;
                }while(iHandle == 0 && iTimeout < MAX_TIMEOUT);

                if(iHandle != 0)
                {
                    uint16 aStatus[10];

                    cout << "Connected!" << endl;

                    // Put the radio into the required state

                    int32 iSuccess = radiotestRadioStatusArray(iHandle);

                    if(iSuccess == TE_OK)
                    {
                        iTimeout = 1000;
                        do
                        {
                            iSuccess = hqGetRadioStatusArray(iHandle, aStatus, 100);
                            iTimeout -= 100;
                        }while(iSuccess == TE_ERROR && iTimeout > 0);
                    }

                    if(iSuccess == TE_OK)
                    {
                        cout << "Internal transmission level = " << aStatus[0] << endl;
                        cout << "External transmission level = " << aStatus[1] << endl;
                        cout << "Reception level             = " << aStatus[2] << endl;
                        cout << "Local oscillator level      = " << aStatus[3] << endl;
                        cout << "IQ trim                     = " << aStatus[4] << endl;
                        cout << "Signal to image ratio in dB = " << aStatus[5] << endl;
                        cout << "Receiver?s attenuation      = " << aStatus[6] << endl;
                        cout << "Local oscillator amplitude  = " << aStatus[7] << endl;
                        cout << "Frequency error             = " << aStatus[8] << endl;
                        cout << "Receive frequency error     = " << aStatus[9] << endl;
                    }

                    closeTestEngine(iHandle);
                }

        """
        self.TestEngineDLL.radiotestRadioStatusArray.restype = CT.c_int32
        self.TestEngineDLL.radiotestRadioStatusArray.argtypes = [CT.c_uint32]
        retval = self.TestEngineDLL.radiotestRadioStatusArray(handle)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hqGetRadioStatusArray(uint32 handle, uint16 r[], int32 timeout);
    #########################################################
    # TestEngine::hqGetRadioStatusArray
    @fntrace
    def hqGetRadioStatusArray(self, handle, r, timeout):
        r"""
        Function wrapper for TestEngine::hqGetRadioStatusArray().

        Python API:
            hqGetRadioStatusArray(handle, r, timeout)
            returns retval

        Generated from C API:

            Function :      int32 hqGetRadioStatusArray(uint32 handle, uint16 * r,
                                                        int32 timeout)

            Parameters :    handle -
                                Handle to the device

                            r -
                                Pointer to an array of 10 unsigned 16-bit integers. These will
                                be used to store the radio status array as follows:
                                <table>
                                    <tr><td>Index Parameter<td>Type of Status Report
                                    <tr><td>0   <td>Internal transmission level
                                    <tr><td>1   <td>External transmission level
                                    <tr><td>2   <td>Reception level
                                    <tr><td>3   <td>Local oscillator level
                                    <tr><td>4   <td>IQ trim
                                    <tr><td>5   <td>Signal to image ratio in dB
                                    <tr><td>6   <td>Receiver?s attenuation
                                    <tr><td>7   <td>Local oscillator amplitude
                                    <tr><td>8   <td>Frequency error
                                    <tr><td>9   <td>Receive frequency error
                                </table>

                            timeout -
                                Used to specify a timeout, in ms, which the software will wait
                                for the HQ radio status packets to be returned from BlueCore.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will wait for a time period specified by "timeout"
                            for all of the HQ radio status array packets.

            Example :

                uint32 iHandle = 0;
                int32 iTimeout = 0;

                do
                {
                    cout << "Trying to connect..." << endl;
                    iHandle = openTestEngine(BCSP, "COM1", 115200, 1000, 0);
                    iTimeout += 1000;
                }while(iHandle == 0 && iTimeout < MAX_TIMEOUT);

                if(iHandle != 0)
                {
                    uint16 aStatus[10];

                    cout << "Connected!" << endl;

                    // Put the radio into the required state

                    int32 iSuccess = radiotestRadioStatusArray(iHandle);

                    if(iSuccess == TE_OK)
                    {
                        iTimeout = 1000;
                        do
                        {
                            iSuccess = hqGetRadioStatusArray(iHandle, aStatus, 100);
                            iTimeout -= 100;
                        }while(iSuccess == TE_ERROR && iTimeout > 0);
                    }

                    if(iSuccess == TE_OK)
                    {
                        cout << "Internal transmission level = " << aStatus[0] << endl;
                        cout << "External transmission level = " << aStatus[1] << endl;
                        cout << "Reception level             = " << aStatus[2] << endl;
                        cout << "Local oscillator level      = " << aStatus[3] << endl;
                        cout << "IQ trim                     = " << aStatus[4] << endl;
                        cout << "Signal to image ratio in dB = " << aStatus[5] << endl;
                        cout << "Receiver?s attenuation      = " << aStatus[6] << endl;
                        cout << "Local oscillator amplitude  = " << aStatus[7] << endl;
                        cout << "Frequency error             = " << aStatus[8] << endl;
                        cout << "Receive frequency error     = " << aStatus[9] << endl;
                    }

                    closeTestEngine(iHandle);
                }

        """
        self.TestEngineDLL.hqGetRadioStatusArray.restype = CT.c_int32
        self.TestEngineDLL.hqGetRadioStatusArray.argtypes = [CT.c_uint32, CT.c_void_p, CT.c_int32]
        __r = (CT.c_uint16 * len(r))(*r)
        retval = self.TestEngineDLL.hqGetRadioStatusArray(handle, CT.byref(__r), timeout)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdMemoryGet(uint32 handle, uint16 baseAddr, uint16 dataLength, uint16 data[]);
    #########################################################
    # TestEngine::bccmdMemoryGet
    @fntrace
    def bccmdMemoryGet(self, handle, baseAddr, dataLength, data):
        r"""
        Function wrapper for TestEngine::bccmdMemoryGet().

        Python API:
            bccmdMemoryGet(handle, baseAddr, dataLength, data)
            returns retval

        Generated from C API:

            Function :      int32 bccmdMemoryGet(uint32 handle, uint16 baseAddr,
                                                uint16 dataLength, uint16* data)

            Parameters :    handle -
                                Handle to the device.

                            baseAddr -
                                Address to read data from.

                            dataLength -
                                Length (in words) of the array pointed to by data. The
                                function can retrieve a maximum of 48 words.

                            data -
                                Pointer to an array of words to retrieve contents of memory.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function can be used to read the contents of the BlueCore
                            memory.
                            <p>Note that the data is specified in terms of 16-bit words and
                            not bytes.
                            <p><b>See also</b> bccmdMemorySet.

        """
        self.TestEngineDLL.bccmdMemoryGet.restype = CT.c_int32
        self.TestEngineDLL.bccmdMemoryGet.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16, CT.c_void_p]
        __data = (CT.c_uint16 * len(data))(*data)
        retval = self.TestEngineDLL.bccmdMemoryGet(handle, baseAddr, dataLength, CT.byref(__data))
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdMemorySet(uint32 handle, uint16 baseAddr, uint16 dataLength, uint16 const data[]);
    #########################################################
    # TestEngine::bccmdMemorySet
    @fntrace
    def bccmdMemorySet(self, handle, baseAddr, dataLength, data):
        r"""
        Function wrapper for TestEngine::bccmdMemorySet().

        Python API:
            bccmdMemorySet(handle, baseAddr, dataLength, data)
            returns retval

        Generated from C API:

            Function :      int32 bccmdMemorySet(uint32 handle, uint16 baseAddr,
                                                uint16 dataLength, uint16 const * const data)

            Parameters :    handle -
                                Handle to the device.

                            baseAddr -
                                Address to write data to.

                            dataLength -
                                Length (in words) of the array pointed to by data. The
                                function can write a maximum of 48 words.

                            data -
                                Pointer to an array of words to write into memory.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function can be used to write the contents of the BlueCore
                            memory. It should be used with caution as writing to certain
                            memory locations may have unpleasant side effects.
                            <p> Note that the data is specified in terms of 16-bit words and
                            not bytes.
                            <p><b>See also</b> bccmdMemoryGet.

        """
        self.TestEngineDLL.bccmdMemorySet.restype = CT.c_int32
        self.TestEngineDLL.bccmdMemorySet.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16, CT.c_void_p]
        __data = (CT.c_uint16 * len(data))(*data)
        retval = self.TestEngineDLL.bccmdMemorySet(handle, baseAddr, dataLength, CT.byref(__data))
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdGetBuildId(uint32 handle, uint16 * buildid);
    #########################################################
    # TestEngine::bccmdGetBuildId
    @fntrace
    def bccmdGetBuildId(self, handle, _buildid=0):
        r"""
        Function wrapper for TestEngine::bccmdGetBuildId().

        Python API:
            bccmdGetBuildId(handle, _buildid=0)
            returns retval, buildid.value

        Generated from C API:

            Function :      int32 bccmdGetBuildId(uint32 handle, uint16 * buildid)

            Parameters :    handle -
                                Handle to the device

                            buildid -
                                Pointer to a variable to hold the returned firmware build ID

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will retrieve the firmware's build ID

            Example :

                uint32 handle = 0;
                int32 timeout = 0;

                do
                {
                    cout << "Trying to connect..." << endl;
                    handle = openTestEngine(BCSP, "COM1", 115200, 1000, 0);
                    timeout += 1000;
                }while(handle == 0 && timeout < MAX_TIMEOUT);

                if(handle != 0)
                {
                    uint16 temp = 0;
                    cout << "Connected!" << endl;

                    int32 success = bccmdGetBuildId(handle, &temp);
                    if(success == TE_OK)
                    {
                        cout << "Build id = " << temp << endl;
                    }

                    success = bccmdGetChipVersion(handle, &temp);
                    if(success == TE_OK)
                    {
                        cout << "Chip version = " << temp << endl;
                    }

                    success = bccmdGetChipRevision(handle, &temp);
                    if(success == TE_OK)
                    {
                        cout << "Chip revision = " << temp << endl;
                    }

                    char name[MAX_LENGTH];
                    success = bccmdBuildName(handle, name, MAX_LENGTH, &temp);
                    if(success == TE_OK)
                    {
                        cout << "Build name = " << name << endl;
                    }

                    // Perform other testing if necessary

                    closeTestEngine(handle);
                }

        """
        self.TestEngineDLL.bccmdGetBuildId.restype = CT.c_int32
        self.TestEngineDLL.bccmdGetBuildId.argtypes = [CT.c_uint32, CT.c_void_p]
        buildid = CT.c_uint16(_buildid)
        retval = self.TestEngineDLL.bccmdGetBuildId(handle, CT.byref(buildid))
        return retval, buildid.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdBuildName(uint32 handle, char* name, uint16 max_len, uint16 * length);
    #########################################################
    # TestEngine::bccmdBuildName
    @fntrace
    def bccmdBuildName(self, handle, max_len, _name='', _length=0):
        r"""
        Function wrapper for TestEngine::bccmdBuildName().

        Python API:
            bccmdBuildName(handle, max_len, _name='', _length=0)
            returns retval, name.value, length.value

        Generated from C API:

            Function :      int32 bccmdBuildName(uint32 handle, char* name, uint16 max_len,
                                                uint16 * length)

            Parameters :    handle -
                                Handle to the device

                            name -
                                Pointer to an array to hold the returned firmware build ID. A
                                terminating null character will be added. If the array is not
                                large enough then the string will be truncated.

                            max_len -
                                Length of the "name" array.

                            length -
                                Total length of the build name not including any NULL
                                character and assuming the string will not have been truncated.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will retrieve the firmware's build name.

            Example :

                uint32 handle = 0;
                int32 timeout = 0;

                do
                {
                    cout << "Trying to connect..." << endl;
                    handle = openTestEngine(BCSP, "COM1", 115200, 1000, 0);
                    timeout += 1000;
                }while(handle == 0 && timeout < MAX_TIMEOUT);

                if(handle != 0)
                {
                    uint16 temp = 0;
                    cout << "Connected!" << endl;

                    int32 success = bccmdGetBuildId(handle, &temp);
                    if(success == TE_OK)
                    {
                        cout << "Build id = " << temp << endl;
                    }

                    success = bccmdGetChipVersion(handle, &temp);
                    if(success == TE_OK)
                    {
                        cout << "Chip version = " << temp << endl;
                    }

                    success = bccmdGetChipRevision(handle, &temp);
                    if(success == TE_OK)
                    {
                        cout << "Chip revision = " << temp << endl;
                    }

                    char name[MAX_LENGTH];
                    success = bccmdBuildName(handle, name, MAX_LENGTH, &temp);
                    if(success == TE_OK)
                    {
                        cout << "Build name = " << name << endl;
                    }

                    // Perform other testing if necessary

                    closeTestEngine(handle);
                }

        """
        self.TestEngineDLL.bccmdBuildName.restype = CT.c_int32
        self.TestEngineDLL.bccmdBuildName.argtypes = [CT.c_uint32, CT.c_char_p, CT.c_uint16, CT.c_void_p]
        name = CT.create_string_buffer(_name, 255)
        length = CT.c_uint16(_length)
        retval = self.TestEngineDLL.bccmdBuildName(handle, name, max_len, CT.byref(length))
        return retval, name.value, length.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdGetChipVersion(uint32 handle, uint16 * chipver);
    #########################################################
    # TestEngine::bccmdGetChipVersion
    @fntrace
    def bccmdGetChipVersion(self, handle, _chipver=0):
        r"""
        Function wrapper for TestEngine::bccmdGetChipVersion().

        Python API:
            bccmdGetChipVersion(handle, _chipver=0)
            returns retval, chipver.value

        Generated from C API:

            Function :      int32 bccmdGetChipVersion(uint32 handle, uint16 * chipver)

            Parameters :    handle -
                                Handle to the device

                            chipver -
                                Pointer to a variable to hold the returned chip version

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will retrieve the chip major version number

            Example :

                uint32 handle = 0;
                int32 timeout = 0;

                do
                {
                    cout << "Trying to connect..." << endl;
                    handle = openTestEngine(BCSP, "COM1", 115200, 1000, 0);
                    timeout += 1000;
                }while(handle == 0 && timeout < MAX_TIMEOUT);

                if(handle != 0)
                {
                    uint16 temp = 0;
                    cout << "Connected!" << endl;

                    int32 success = bccmdGetBuildId(handle, &temp);
                    if(success == TE_OK)
                    {
                        cout << "Build id = " << temp << endl;
                    }

                    success = bccmdGetChipVersion(handle, &temp);
                    if(success == TE_OK)
                    {
                        cout << "Chip version = " << temp << endl;
                    }

                    success = bccmdGetChipRevision(handle, &temp);
                    if(success == TE_OK)
                    {
                        cout << "Chip revision = " << temp << endl;
                    }

                    char name[MAX_LENGTH];
                    success = bccmdBuildName(handle, name, MAX_LENGTH, &temp);
                    if(success == TE_OK)
                    {
                        cout << "Build name = " << name << endl;
                    }

                    // Perform other testing if necessary

                    closeTestEngine(handle);
                }

        """
        self.TestEngineDLL.bccmdGetChipVersion.restype = CT.c_int32
        self.TestEngineDLL.bccmdGetChipVersion.argtypes = [CT.c_uint32, CT.c_void_p]
        chipver = CT.c_uint16(_chipver)
        retval = self.TestEngineDLL.bccmdGetChipVersion(handle, CT.byref(chipver))
        return retval, chipver.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdGetChipRevision(uint32 handle, uint16 * chiprev);
    #########################################################
    # TestEngine::bccmdGetChipRevision
    @fntrace
    def bccmdGetChipRevision(self, handle, _chiprev=0):
        r"""
        Function wrapper for TestEngine::bccmdGetChipRevision().

        Python API:
            bccmdGetChipRevision(handle, _chiprev=0)
            returns retval, chiprev.value

        Generated from C API:

            Function :      int32 bccmdGetChipRevision(uint32 handle, uint16 * chiprev)

            Parameters :    handle -
                                Handle to the device

                            chiprev -
                                Pointer to a variable to hold the returned chip revision,
                                as follows:
                                <table>
                                    <tr><td>0x43 = BC3-MM
                                    <tr><td>0x15 = BC3-ROM
                                    <tr><td>0xE2 = BC3-Flash
                                    <tr><td>0x26 = BC4-EXT
                                    <tr><td>0x30 = BC4-ROM
                                    <tr><td>0xE1 = BC5-MM
                                    <tr><td>0x41 = BC5-FM
                                    <tr><td>0x11 = BC6-ROM
                                </table>

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will retrieve the chip revision number.
                            bccmdGetChipRevision can be used following bccmdGetChipVersion to
                            identify BlueCore chip variants for all chips up to and including
                            BC6. For later chips, i.e. those for which bccmdGetChipVersion
                            returns a value greater than 6, use bccmdGetChipAnaVer instead.

            Example :

                uint32 handle = 0;
                int32 timeout = 0;

                do
                {
                    cout << "Trying to connect..." << endl;
                    handle = openTestEngine(BCSP, "COM1", 115200, 1000, 0);
                    timeout += 1000;
                }while(handle == 0 && timeout < MAX_TIMEOUT);

                if(handle != 0)
                {
                    uint16 temp = 0;
                    cout << "Connected!" << endl;

                    int32 success = bccmdGetBuildId(handle, &temp);
                    if(success == TE_OK)
                    {
                        cout << "Build id = " << temp << endl;
                    }

                    success = bccmdGetChipVersion(handle, &temp);
                    if(success == TE_OK)
                    {
                        cout << "Chip version = " << temp << endl;
                    }

                    success = bccmdGetChipRevision(handle, &temp);
                    if(success == TE_OK)
                    {
                        cout << "Chip revision = " << temp << endl;
                    }

                    char name[MAX_LENGTH];
                    success = bccmdBuildName(handle, name, MAX_LENGTH, &temp);
                    if(success == TE_OK)
                    {
                        cout << "Build name = " << name << endl;
                    }

                    // Perform other testing if necessary

                    closeTestEngine(handle);
                }

        """
        self.TestEngineDLL.bccmdGetChipRevision.restype = CT.c_int32
        self.TestEngineDLL.bccmdGetChipRevision.argtypes = [CT.c_uint32, CT.c_void_p]
        chiprev = CT.c_uint16(_chiprev)
        retval = self.TestEngineDLL.bccmdGetChipRevision(handle, CT.byref(chiprev))
        return retval, chiprev.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdGetChipAnaVer(uint32 handle, uint8 * major, uint8 * minor, uint8 * vari);
    #########################################################
    # TestEngine::bccmdGetChipAnaVer
    @fntrace
    def bccmdGetChipAnaVer(self, handle, _major=0, _minor=0, _vari=0):
        r"""
        Function wrapper for TestEngine::bccmdGetChipAnaVer().

        Python API:
            bccmdGetChipAnaVer(handle, _major=0, _minor=0, _vari=0)
            returns retval, major.value, minor.value, vari.value

        Generated from C API:

            Function :      int32 bccmdGetChipAnaVer(uint32 handle, uint8 * major,
                                                        uint8 * minor, uint8 * vari)

            Parameters :    handle -
                                Handle to the device

                            major -
                                Pointer to a variable to hold the returned major version value,
                                for example (not a complete list):
                                <table>
                                    <tr><td>0x08 = BC3-MM
                                    <tr><td>0x09 = BC3-ROM
                                    <tr><td>0x0A = BC3-Flash
                                    <tr><td>0x0B = BC4-EXT
                                    <tr><td>0x0C = BC4-ROM
                                    <tr><td>0x0E = BC4-Audio-ROM
                                    <tr><td>0x11 = BC5-MM
                                    <tr><td>0x12 = BC5-FM
                                    <tr><td>0x13 = BC4-Audio-Flash
                                    <tr><td>0x17 = BC6-ROM
                                    <tr><td>0x21 = BC7-FM
                                    <tr><td>0x28 = BC7-MM
                                    <tr><td>0x32 = BC7-ROM
                                </table>

                            minor -
                                Pointer to a variable to hold the returned incremental minor
                                version value.

                            vari -
                                Pointer to a variable to hold the returned variant value.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will retrieve the version information for the chips
                            analogue components. bccmdGetChipAnaVer can be used following
                            bccmdGetChipVersion to identify BlueCore chip variants. The major
                            version number identifies the chip type.

            Example :

                uint32 handle = 0;
                int32 timeout = 0;

                do
                {
                    cout << "Trying to connect..." << endl;
                    handle = openTestEngine(BCSP, "COM1", 115200, 1000, 0);
                    timeout += 1000;
                }while(handle == 0 && timeout < 5000);

                if(handle != 0)
                {
                    cout << "Connected!" << endl;

                    uint16 ver;
                    int32 success = bccmdGetChipVersion(handle, &ver);
                    if(success == TE_OK)
                    {
                        cout << "Chip version = " << ver << endl;
                    }

                    uint8 major, minor, variant;
                    success = bccmdGetChipAnaVer(handle, &major, &minor, &variant);
                    if(success == TE_OK)
                    {
                        cout << "Chip analogue major version = " << (uint16)major << endl;
                        cout << "Chip analogue minor version = " << (uint16)minor << endl;
                        cout << "Chip analogue variant = " << (uint16)variant << endl;
                    }

                    // Perform other testing if necessary

                    closeTestEngine(handle);
                }

        """
        self.TestEngineDLL.bccmdGetChipAnaVer.restype = CT.c_int32
        self.TestEngineDLL.bccmdGetChipAnaVer.argtypes = [CT.c_uint32, CT.c_void_p, CT.c_void_p, CT.c_void_p]
        major = CT.c_uint8(_major)
        minor = CT.c_uint8(_minor)
        vari = CT.c_uint8(_vari)
        retval = self.TestEngineDLL.bccmdGetChipAnaVer(handle, CT.byref(major), CT.byref(minor), CT.byref(vari))
        return retval, major.value, minor.value, vari.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdRouteClock(uint32 handle);
    #########################################################
    # TestEngine::bccmdRouteClock
    @fntrace
    def bccmdRouteClock(self, handle):
        r"""
        Function wrapper for TestEngine::bccmdRouteClock().

        Python API:
            bccmdRouteClock(handle)
            returns retval

        Generated from C API:

            Function :      int32 bccmdRouteClock(uint32 handle)

            Parameters :    handle -
                                Handle to the device.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to issue a BCCMDVARID_ROUTE_CLOCK to the
                            connected BlueCore device.<br>
                            On supported firmware this will route the clock to AIO1.<br>
                            The device must be reset with the bccmdSetColdReset function to
                            stop the clock signal being routed to the AIO pin.

        """
        self.TestEngineDLL.bccmdRouteClock.restype = CT.c_int32
        self.TestEngineDLL.bccmdRouteClock.argtypes = [CT.c_uint32]
        retval = self.TestEngineDLL.bccmdRouteClock(handle)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdRssiAcl(uint32 handle, uint16 connectionHandle, int16 * rssi);
    #########################################################
    # TestEngine::bccmdRssiAcl
    @fntrace
    def bccmdRssiAcl(self, handle, connectionHandle, _rssi=0):
        r"""
        Function wrapper for TestEngine::bccmdRssiAcl().

        Python API:
            bccmdRssiAcl(handle, connectionHandle, _rssi=0)
            returns retval, rssi.value

        Generated from C API:

            Function :      int32 bccmdRssiAcl(uint32 handle, uint16 connectionHandle,
                                                int16 * rssi)

            Parameters :    handle -
                                Handle to the device.

                            connectionHandle -
                                Bluetooth connection handle to remote device.

                            rssi -
                                RSSI of a received signal in dBm.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Retrieves the received signal strength indication (RSSI) for a
                            given ACL connection handle. The RSSI value is interpreted as a
                            signed integer with units of dBm.

        """
        self.TestEngineDLL.bccmdRssiAcl.restype = CT.c_int32
        self.TestEngineDLL.bccmdRssiAcl.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_void_p]
        rssi = CT.c_int16(_rssi)
        retval = self.TestEngineDLL.bccmdRssiAcl(handle, connectionHandle, CT.byref(rssi))
        return retval, rssi.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdSetPio(uint32 handle, uint16 mask, uint16 port);
    #########################################################
    # TestEngine::bccmdSetPio
    @fntrace
    def bccmdSetPio(self, handle, mask, port):
        r"""
        Function wrapper for TestEngine::bccmdSetPio().

        Python API:
            bccmdSetPio(handle, mask, port)
            returns retval

        Generated from C API:

            Function :      int32 bccmdSetPio(uint32 handle, uint16 mask, uint16 port)

            Parameters :    handle -
                                Handle to the device

                            mask -
                                Read / write bit mask of PIO port

                            port -
                                Bit settings of PIO port

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will set the direction mask of the PIO and the
                            state of the pio port.

        """
        self.TestEngineDLL.bccmdSetPio.restype = CT.c_int32
        self.TestEngineDLL.bccmdSetPio.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16]
        retval = self.TestEngineDLL.bccmdSetPio(handle, mask, port)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdGetPio(uint32 handle, uint16 * mask, uint16 * port);
    #########################################################
    # TestEngine::bccmdGetPio
    @fntrace
    def bccmdGetPio(self, handle, _mask=0, _port=0):
        r"""
        Function wrapper for TestEngine::bccmdGetPio().

        Python API:
            bccmdGetPio(handle, _mask=0, _port=0)
            returns retval, mask.value, port.value

        Generated from C API:

            Function :      int32 bccmdGetPio(uint32 handle, uint16 * mask, uint16 * port)

            Parameters :    handle -
                                Handle to the device

                            mask -
                                Direction mask of PIO port

                            port -
                                Bit settings of PIO port

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will retrieve the direction mask of the PIO and the
                            state of the pio port.

        """
        self.TestEngineDLL.bccmdGetPio.restype = CT.c_int32
        self.TestEngineDLL.bccmdGetPio.argtypes = [CT.c_uint32, CT.c_void_p, CT.c_void_p]
        mask = CT.c_uint16(_mask)
        port = CT.c_uint16(_port)
        retval = self.TestEngineDLL.bccmdGetPio(handle, CT.byref(mask), CT.byref(port))
        return retval, mask.value, port.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdMapPio32(uint32 handle, uint32 mask, uint32 pios, uint32* errLines);
    #########################################################
    # TestEngine::bccmdMapPio32
    @fntrace
    def bccmdMapPio32(self, handle, mask, pios, _errLines=0):
        r"""
        Function wrapper for TestEngine::bccmdMapPio32().

        Python API:
            bccmdMapPio32(handle, mask, pios, _errLines=0)
            returns retval, errLines.value

        Generated from C API:

            Function :      int32 bccmdMapPio32(uint32 handle, uint32 mask, uint32 pios,
                                                 uint32* errLines)

            Parameters :    handle -
                                Handle to the device.

                            mask -
                                Bit mask of the mappable pins.

                            pios -
                                Bit field, where a bit being set (1) means that that pin should
                                be configured as a PIO. A bit being cleared (0) means that the
                                pin will be configured for its alternative use.

                            errLines -
                                Pointer to a uint32 bit field indicating which of the specified
                                pins could not be mapped as PIOs.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will map the pins specified in the mask according to
                            the usage specified in the pios parameter. Reports any pins which
                            could not be mapped.

        """
        self.TestEngineDLL.bccmdMapPio32.restype = CT.c_int32
        self.TestEngineDLL.bccmdMapPio32.argtypes = [CT.c_uint32, CT.c_uint32, CT.c_uint32, CT.c_void_p]
        errLines = CT.c_uint32(_errLines)
        retval = self.TestEngineDLL.bccmdMapPio32(handle, mask, pios, CT.byref(errLines))
        return retval, errLines.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdSetPio32(uint32 handle, uint32 mask, uint32 direction, uint32 value, uint32* errLines);
    #########################################################
    # TestEngine::bccmdSetPio32
    @fntrace
    def bccmdSetPio32(self, handle, mask, direction, value, _errLines=0):
        r"""
        Function wrapper for TestEngine::bccmdSetPio32().

        Python API:
            bccmdSetPio32(handle, mask, direction, value, _errLines=0)
            returns retval, errLines.value

        Generated from C API:

            Function :      int32 bccmdSetPio32(uint32 handle, uint32 mask, uint32 direction,
                                                 uint32 value, uint32* errLines)

            Parameters :    handle -
                                Handle to the device.

                            mask -
                                Bit mask of the PIOs to be set.

                            direction -
                                Bit field which sets the PIOs as input (0) or output (1).

                            value -
                                Bit field which sets the values for the PIOs.

                            errLines -
                                Pointer to a uint32 bit field indicating which of the specified
                                pins could not be set.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will set the direction and values for the specified
                            PIOs. The value does not apply to PIOs specified as inputs.
                            <p>If the PIOs to be used include lines which default to alternative
                            uses, it is necessary to call bccmdMapPio32 first to map the lines
                            as PIOs.

            Example :

                uint32 handle = 0;
                uint32 timeout = 0;
                uint32 mapMask = 0x0FFF0000;      // The lines we can map as PIOs
                uint32 pioMask = 0x0FFFFFFF;      // The lines we want to set (dedicated PIOs plus those we mapped)
                uint32 direction = 0x0F0F0F0F;    // Direction - (0) as input (1) as output
                uint32 value = 0x05050505;        // State of the lines to set (applies to output lines only)
                uint32 errLines;

                do
                {
                    cout << "Trying to connect..." << endl;
                    handle = openTestEngine(BCSP, "COM1", 115200, 1000, 0);
                    timeout += 1000;
                }while(handle == 0 && timeout < 5000);

                if(handle != 0)
                {
                    cout << "Connected!" << endl;

                    // Map lines that have other alternate functions as PIOs
                    int32 success = bccmdMapPio32(handle, mapMask, 0xFFFFFFFF, &errLines);

                    if(success != TE_OK)
                    {
                        // Checks if any lines could not be mapped as PIOs
                        if(errLines != 0)
                        {
                            cout << "ERROR: Lines that could not be mapped as PIOs = " << errLines << endl;
                        }
                    }
                    else
                    {
                        // Set the direction and value of the PIO lines.
                        success = bccmdSetPio32(handle, pioMask, direction, value, &errLines);

                        // Checks if any lines could not be mapped as PIOs
                        if(success != TE_OK)
                        {
                            // Checks which PIO lines could not be set as input or output
                            if(errLines != 0)
                            {
                                cout << "ERROR: Lines that could not be set = " << errLines << endl;
                            }
                        }
                    }

                    closeTestEngine(handle);
                }

        """
        self.TestEngineDLL.bccmdSetPio32.restype = CT.c_int32
        self.TestEngineDLL.bccmdSetPio32.argtypes = [CT.c_uint32, CT.c_uint32, CT.c_uint32, CT.c_uint32, CT.c_void_p]
        errLines = CT.c_uint32(_errLines)
        retval = self.TestEngineDLL.bccmdSetPio32(handle, mask, direction, value, CT.byref(errLines))
        return retval, errLines.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdGetPio32(uint32 handle, uint32* direction, uint32* value);
    #########################################################
    # TestEngine::bccmdGetPio32
    @fntrace
    def bccmdGetPio32(self, handle, _direction=0, _value=0):
        r"""
        Function wrapper for TestEngine::bccmdGetPio32().

        Python API:
            bccmdGetPio32(handle, _direction=0, _value=0)
            returns retval, direction.value, value.value

        Generated from C API:

            Function :      int32 bccmdGetPio32(uint32 handle, uint32* direction, uint32* value)

            Parameters :    handle -
                                Handle to the device

                            direction -
                                Pointer to uint32 value which stores the direction of the
                                PIO port lines as a bit field, where a 1 indicates that the
                                PIO is an output, and a 0 means that the PIO is an input.

                            value -
                                Pointer to uint32 value which stores the values of the PIO lines
                                as a bit field.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function gets the direction and state of the PIO lines.
                            If the PIOs to be read include lines which default to alternative
                            uses, it is necessary to call bccmdMapPio32 first to map the lines
                            as PIOs.

            Example :

                uint32 handle = 0;
                uint32 timeout = 0;
                uint32 direction;
                uint32 value;

                do
                {
                    cout << "Trying to connect..." << endl;
                    handle = openTestEngine(BCSP, "COM1", 115200, 1000, 0);
                    timeout += 1000;
                }while(handle == 0 && timeout < 5000);

                if(handle != 0)
                {
                    cout << "Connected!" << endl;

                    // Get the direction and value of the PIO lines.
                    int32 success = bccmdGetPio32(handle, &direction, &value);
                    if(success == TE_OK)
                    {
                        cout << "Direction = " << direction << endl;
                        cout << "Value = " << value << endl;
                    }

                    closeTestEngine(handle);
                }

        """
        self.TestEngineDLL.bccmdGetPio32.restype = CT.c_int32
        self.TestEngineDLL.bccmdGetPio32.argtypes = [CT.c_uint32, CT.c_void_p, CT.c_void_p]
        direction = CT.c_uint32(_direction)
        value = CT.c_uint32(_value)
        retval = self.TestEngineDLL.bccmdGetPio32(handle, CT.byref(direction), CT.byref(value))
        return retval, direction.value, value.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdGetAdc(uint32 handle, uint16 adc, uint16* result);
    #########################################################
    # TestEngine::bccmdGetAdc
    @fntrace
    def bccmdGetAdc(self, handle, adc, _result=0):
        r"""
        Function wrapper for TestEngine::bccmdGetAdc().

        Python API:
            bccmdGetAdc(handle, adc, _result=0)
            returns retval, result.value

        Generated from C API:

            Function :      int32 bccmdGetAdc(uint32 handle, uint16 adc, uint16* result)

            Parameters :    handle -
                                Handle to the device

                            adc -
                                ADC which will be read. The ADCs which can be read are IC
                                dependent.

                            result -
                                Holds the result of the ADC reading. While the ADCs are
                                10-bit, the result is scaled according to any internal
                                potential divider network. Therefore the result can be
                                greater than the 10-bit maximum (1023).

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will perform an ADC reading on the specified ADC.
                            It is only supported in BlueCore firmware from v24.

        """
        self.TestEngineDLL.bccmdGetAdc.restype = CT.c_int32
        self.TestEngineDLL.bccmdGetAdc.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_void_p]
        result = CT.c_uint16(_result)
        retval = self.TestEngineDLL.bccmdGetAdc(handle, adc, CT.byref(result))
        return retval, result.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdGetAio(uint32 handle, uint16 aio, uint16 * result, uint8 * numBits);
    #########################################################
    # TestEngine::bccmdGetAio
    @fntrace
    def bccmdGetAio(self, handle, aio, _result=0, _numBits=0):
        r"""
        Function wrapper for TestEngine::bccmdGetAio().

        Python API:
            bccmdGetAio(handle, aio, _result=0, _numBits=0)
            returns retval, result.value, numBits.value

        Generated from C API:

            Function :      int32 bccmdGetAio(uint32 handle, uint16 aio, uint16 * result,
                                                uint8 * numBits)

            Parameters :    handle -
                                Handle to the device

                            aio -
                                AIO pin which will be read, 0 to 3.

                            result -
                                Holds the result of the ADC reading. This value is from 0 to
                                ADC_MAX, where ADC_MAX = 255 or 1023 for 8 and 10 bit results
                                respectively (the numBits parameter indicates 8 or 10 bit
                                result).
                                <p>0 = 0V and ADC_MAX = VDD, therefore:
                                <p>voltage = (result/ADC_MAX) * VDD
                                <p>VDD can be calculated using a vref ADC reading and the vref
                                constant. See the help for bccmdGetVrefAdc for details.

                            numBits -
                                Holds the number of bits used for the result. In BlueCore
                                firmware from v24, the ADC readings have been changed to return
                                a 10 bit result (8 previously). This output parameter indicates
                                how many bits in the result are valid, 8 or 10, removing the
                                need to check the firmware version when using this function.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will perform an ADC reading on the specified AIO
                            pin. This function supports both the old ADC reading method (8
                            bit results) and the new (10 bit results). BlueCore devices prior
                            to BC5 have 8 bit ADCs. With these devices, and where the
                            firmware used uses the new method (returns 10 bit results), the
                            readings are shifted up by 2 bits.

        """
        self.TestEngineDLL.bccmdGetAio.restype = CT.c_int32
        self.TestEngineDLL.bccmdGetAio.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_void_p, CT.c_void_p]
        result = CT.c_uint16(_result)
        numBits = CT.c_uint8(_numBits)
        retval = self.TestEngineDLL.bccmdGetAio(handle, aio, CT.byref(result), CT.byref(numBits))
        return retval, result.value, numBits.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdBC5MMGetBatteryVoltage(uint32 handle, uint16 * voltage);
    #########################################################
    # TestEngine::bccmdBC5MMGetBatteryVoltage
    @fntrace
    def bccmdBC5MMGetBatteryVoltage(self, handle, _voltage=0):
        r"""
        Function wrapper for TestEngine::bccmdBC5MMGetBatteryVoltage().

        Python API:
            bccmdBC5MMGetBatteryVoltage(handle, _voltage=0)
            returns retval, voltage.value

        Generated from C API:

            Function :      int32 bccmdBC5MMGetBatteryVoltage(uint32 handle, uint16 * voltage)

            Parameters :    handle -
                                Handle to the device

                            voltage -
                                Pointer to 16 bit unsigned int, holds the voltage in
                                milli-volts, calculated from the battery voltage ADC reading.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will read the value of the internal battery voltage
                            monitoring ADC and convert it to a voltage. It can be used with
                            BC5MM chips running firmware version 22d and newer.
                            <p>Using this function with chips other than BC5MM is not
                            supported, and the results are undefined. The same goes for
                            firmware versions older than 22d.
                            <p>Note that the accuracy of this function is limited by the ADC
                            accuracy, resolution and stability. An ADC step of 1 will result
                            in a voltage change of approximately 16-24mV. Testing has shown
                            that the voltage returned is generally within +/-50mV of the
                            voltage measured using a DVM.
                            <p>This function will fail if a radiotest mode is running on the
                            chip (e.g. radiotestTxdata1() was called previously). If this is
                            the case, use a warm reset before calling this function.

        """
        self.TestEngineDLL.bccmdBC5MMGetBatteryVoltage.restype = CT.c_int32
        self.TestEngineDLL.bccmdBC5MMGetBatteryVoltage.argtypes = [CT.c_uint32, CT.c_void_p]
        voltage = CT.c_uint16(_voltage)
        retval = self.TestEngineDLL.bccmdBC5MMGetBatteryVoltage(handle, CT.byref(voltage))
        return retval, voltage.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdGetFirmwareCheckMask(uint32 handle, uint16 * mask);
    #########################################################
    # TestEngine::bccmdGetFirmwareCheckMask
    @fntrace
    def bccmdGetFirmwareCheckMask(self, handle, _mask=0):
        r"""
        Function wrapper for TestEngine::bccmdGetFirmwareCheckMask().

        Python API:
            bccmdGetFirmwareCheckMask(handle, _mask=0)
            returns retval, mask.value

        Generated from C API:

            Function :      int32 bccmdGetFirmwareCheckMask(uint32 handle, uint16 * mask)

            Parameters :    handle -
                                Handle to the device

                            mask -
                                Pointer to mask which states which firmware component is
                                present AND if that component contains a pre-calculated
                                checksum.
                                <table>
                                    <tr><td>Bit  <td>Component
                                    <tr><td>0    <td>Stack
                                    <tr><td>1    <td>Loader
                                    <tr><td>2    <td>VM Application
                                    <tr><td>3    <td>File System
                                </table>

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will check if each firmware component is present and
                            that it contains a valid checksum. This function should be called
                            before bccmdGetFirmwareCheck.

        """
        self.TestEngineDLL.bccmdGetFirmwareCheckMask.restype = CT.c_int32
        self.TestEngineDLL.bccmdGetFirmwareCheckMask.argtypes = [CT.c_uint32, CT.c_void_p]
        mask = CT.c_uint16(_mask)
        retval = self.TestEngineDLL.bccmdGetFirmwareCheckMask(handle, CT.byref(mask))
        return retval, mask.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdGetFirmwareCheck(uint32 handle, uint16 * check);
    #########################################################
    # TestEngine::bccmdGetFirmwareCheck
    @fntrace
    def bccmdGetFirmwareCheck(self, handle, _check=0):
        r"""
        Function wrapper for TestEngine::bccmdGetFirmwareCheck().

        Python API:
            bccmdGetFirmwareCheck(handle, _check=0)
            returns retval, check.value

        Generated from C API:

            Function :      int32 bccmdGetFirmwareCheck(uint32 handle, uint16 * check)

            Parameters :    handle -
                                Handle to the device

                            check -
                                Pointer to a mask which states which firmware component is
                                present AND if that component contains the correct checksum.
                                <table>
                                    <tr><td>Bit  <td>Component
                                    <tr><td>0    <td>Stack
                                    <tr><td>1    <td>Loader
                                    <tr><td>2    <td>VM Application
                                    <tr><td>3    <td>File System
                                </table>

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will check if each firmware component checksum is
                            correct.

        """
        self.TestEngineDLL.bccmdGetFirmwareCheck.restype = CT.c_int32
        self.TestEngineDLL.bccmdGetFirmwareCheck.argtypes = [CT.c_uint32, CT.c_void_p]
        check = CT.c_uint16(_check)
        retval = self.TestEngineDLL.bccmdGetFirmwareCheck(handle, CT.byref(check))
        return retval, check.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdGetExternalClockPeriod(uint32 handle, uint16 * period);
    #########################################################
    # TestEngine::bccmdGetExternalClockPeriod
    @fntrace
    def bccmdGetExternalClockPeriod(self, handle, _period=0):
        r"""
        Function wrapper for TestEngine::bccmdGetExternalClockPeriod().

        Python API:
            bccmdGetExternalClockPeriod(handle, _period=0)
            returns retval, period.value

        Generated from C API:

            Function :      int32 bccmdGetExternalClockPeriod(uint32 handle, uint16 * period)

            Parameters :    handle -
                                Handle to the device

                            period -
                                Pointer to a value in units of 0.25us. Measures the 32kHz
                                clock once it's been divided down to 1kHz. The expected value
                                is therefore about 4000 (or 0xFA0).

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will read the external clock period over the last
                            1kHz cycle from the appropriate pin.

        """
        self.TestEngineDLL.bccmdGetExternalClockPeriod.restype = CT.c_int32
        self.TestEngineDLL.bccmdGetExternalClockPeriod.argtypes = [CT.c_uint32, CT.c_void_p]
        period = CT.c_uint16(_period)
        retval = self.TestEngineDLL.bccmdGetExternalClockPeriod(handle, CT.byref(period))
        return retval, period.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdEnableDeviceConnect(uint32 handle);
    #########################################################
    # TestEngine::bccmdEnableDeviceConnect
    @fntrace
    def bccmdEnableDeviceConnect(self, handle):
        r"""
        Function wrapper for TestEngine::bccmdEnableDeviceConnect().

        Python API:
            bccmdEnableDeviceConnect(handle)
            returns retval

        Generated from C API:

            Function :      int32 bccmdEnableDeviceConnect(uint32 handle)

            Parameters :    handle -
                                Handle to the device

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Causes the device to be discoverable and connectable. This
                            command is effectively the BCCMD equivalent of hciSlave / dmSlave.
                            It will enable the user to set the device into test mode via the
                            SPI interface (a call to bccmdEnableDeviceUnderTestMode following
                            this command is required).
                            <p>Firmware from 23g releases should support this command. It is
                            unsupported in older firmware (function will return 2).
                            <p>Success is determined by:
                            <ol>
                            <li> Valid handle
                            <li> Command complete within timeout
                            </ol>

        """
        self.TestEngineDLL.bccmdEnableDeviceConnect.restype = CT.c_int32
        self.TestEngineDLL.bccmdEnableDeviceConnect.argtypes = [CT.c_uint32]
        retval = self.TestEngineDLL.bccmdEnableDeviceConnect(handle)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdEnableDeviceUnderTestMode(uint32 handle);
    #########################################################
    # TestEngine::bccmdEnableDeviceUnderTestMode
    @fntrace
    def bccmdEnableDeviceUnderTestMode(self, handle):
        r"""
        Function wrapper for TestEngine::bccmdEnableDeviceUnderTestMode().

        Python API:
            bccmdEnableDeviceUnderTestMode(handle)
            returns retval

        Generated from C API:

            Function :      int32 bccmdEnableDeviceUnderTestMode(uint32 handle)

            Parameters :    handle -
                                Handle to the device

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Causes the device to set the enable device under test mode flag
                            using a BCCMD instead of an HCI or DM command. This will enable
                            the user to set the device into testmode via the SPI interface as
                            well as UART and USB.
                            This command will <b>NOT</b> make the device discoverable or
                            connectable. To do this, use bccmdEnableDeviceConnect if
                            supported by the BlueCore firmware (if not, a VM app can be used
                            to do this, or use a host connection and hciSlave / dmSlave).
                            <p>Success is determined by:
                            <ol>
                            <li> Valid handle
                            <li> Command complete within timeout
                            </ol>

            Example :

                uint32 handle = openTestEngineSpi(1, 0, SPI_LPT);

                if(handle != 0)
                {
                    int32 success;
                    do
                    {
                        // Make connectable
                        success = bccmdEnableDeviceConnect(handle);
                        if(success != TE_OK)
                            break;

                        // Enable DUT mode
                        success = bccmdEnableDeviceUnderTestMode(handle);
                        if(success != TE_OK)
                            break;

                        // Run tests on BT tester

                    }while(0);

                    if(success != TE_OK)
                        cout << "TestEngine error" << endl;
                }
                if(handle != 0)
                    closeTestEngine(handle);

        """
        self.TestEngineDLL.bccmdEnableDeviceUnderTestMode.restype = CT.c_int32
        self.TestEngineDLL.bccmdEnableDeviceUnderTestMode.argtypes = [CT.c_uint32]
        retval = self.TestEngineDLL.bccmdEnableDeviceUnderTestMode(handle)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdCheckSqifImageValidationStatus(uint32 handle, uint16* status, uint16 timeout);
    #########################################################
    # TestEngine::bccmdCheckSqifImageValidationStatus
    @fntrace
    def bccmdCheckSqifImageValidationStatus(self, handle, timeout, _status=0):
        r"""
        Function wrapper for TestEngine::bccmdCheckSqifImageValidationStatus().

        Python API:
            bccmdCheckSqifImageValidationStatus(handle, timeout, _status=0)
            returns retval, status.value

        Generated from C API:

            Function :      int32 bccmdCheckSqifImageValidationStatus(uint32 handle,
                                                                      uint16* status,
                                                                      uint16 timeout)

            Parameters :    handle -
                                Handle to the device

                            status -
                                Pointer to a value where the SQIF Image Validation
                                status of the chip will be stored. The following status
                                values may be returned:
                                <table>
                                    <tr><td>0 = Error: validation did not complete
                                    <tr><td>1 = Validation passed: an image
                                                is present.
                                    <tr><td>2 = Validation passed: no image
                                                is present.
                                </table>

                            timeout -
                                Specified in milliseconds. See Description for details.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td> 0 = Error
                                <tr><td> 1 = Success
                                <tr><td> 2 = Unsupported function
                            </table>

            Description :  This function will check the device's SQIF Image
                           Validation status. If validation fails to complete
                           within the specified timeout, the function will return
                           success and an error will be indicated via the status
                           parameter.

        """
        self.TestEngineDLL.bccmdCheckSqifImageValidationStatus.restype = CT.c_int32
        self.TestEngineDLL.bccmdCheckSqifImageValidationStatus.argtypes = [CT.c_uint32, CT.c_void_p, CT.c_uint16]
        status = CT.c_uint16(_status)
        retval = self.TestEngineDLL.bccmdCheckSqifImageValidationStatus(handle, CT.byref(status), timeout)
        return retval, status.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestStereoCodecLB(uint32 handle, uint16 sampleRate, uint16 reroute);
    #########################################################
    # TestEngine::radiotestStereoCodecLB
    @fntrace
    def radiotestStereoCodecLB(self, handle, sampleRate, reroute):
        r"""
        Function wrapper for TestEngine::radiotestStereoCodecLB().

        Python API:
            radiotestStereoCodecLB(handle, sampleRate, reroute)
            returns retval

        Generated from C API:

            Function :      int32 radiotestStereoCodecLB(uint32 handle, uint16 sampleRate,
                                                        uint16 reroute)

            Parameters :    handle -
                                Handle to the device.

                            sampleRate -
                                Sample rate in Hz, which can be one of the following values:
                                <table>
                                    <tr><td>8000
                                    <tr><td>11025
                                    <tr><td>16000
                                    <tr><td>22050
                                    <tr><td>24000
                                    <tr><td>32000
                                    <tr><td>44100
                                </table>

                            reroute -
                                Controls how signal is routed from mic to speaker.
                                <table>
                                    <tr><td>0 - <td>MicL to SpkL, MicR to SpkR
                                    <tr><td>1 - <td>MicR to SpkL and SpkR
                                    <tr><td>2 - <td>MicL to SpkL and SpkR
                                    <tr><td>3 - <td>MicL to SpkR, MicR to SpkL
                                </table>

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to turn on the codec hardware for stereo
                            loopback.

        """
        self.TestEngineDLL.radiotestStereoCodecLB.restype = CT.c_int32
        self.TestEngineDLL.radiotestStereoCodecLB.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16]
        retval = self.TestEngineDLL.radiotestStereoCodecLB(handle, sampleRate, reroute)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestTxstart(uint32 handle, uint16 frequency, uint16 power1, uint16 power2, int16 modulation);
    #########################################################
    # TestEngine::radiotestTxstart
    @fntrace
    def radiotestTxstart(self, handle, frequency, power1, power2, modulation):
        r"""
        Function wrapper for TestEngine::radiotestTxstart().

        Python API:
            radiotestTxstart(handle, frequency, power1, power2, modulation)
            returns retval

        Generated from C API:

            Function :      int32 radiotestTxstart(uint32 handle, uint16 frequency,
                                                   uint16 power1, uint16 power2,
                                                   int16 modulation)

            Parameters :    handle -
                                Handle to the device.

                            frequency -
                                Local Oscillator frequency

                            power1 -
                                For BlueCore ICs this sets the drive level for internal PA.
                                0 to 63 for basic rate packets, 0 to 127 for EDR packets.
                                Typical values are 50 for basic rate and 105 for EDR packet
                                types.
                                <p>For QCC5xxx and QCC3xxx ICs, the LSB (bits 0-7) is used
                                as the LSB of the magnitude value.
                                <p>In all cases the MSB (upper 8 bits) are ignored.

                            power2 -
                                For BlueCore ICs this sets the drive level for external PA.
                                <p>For QCC5xxx and QCC3xxx ICs, bits 0-3 are used
                                as bits 8-11 of the magnitude value.
                                <p>In all cases the MSB (upper 8 bits) are ignored.

                            modulation -
                                Modulation offset - 4096 = 1MHz

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function initiates TXSTART

        """
        self.TestEngineDLL.radiotestTxstart.restype = CT.c_int32
        self.TestEngineDLL.radiotestTxstart.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16, CT.c_uint16, CT.c_int16]
        retval = self.TestEngineDLL.radiotestTxstart(handle, frequency, power1, power2, modulation)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestTxdata1(uint32 handle, uint16 frequency, uint16 power1, uint16 power2);
    #########################################################
    # TestEngine::radiotestTxdata1
    @fntrace
    def radiotestTxdata1(self, handle, frequency, power1, power2):
        r"""
        Function wrapper for TestEngine::radiotestTxdata1().

        Python API:
            radiotestTxdata1(handle, frequency, power1, power2)
            returns retval

        Generated from C API:

            Function :      int32 radiotestTxdata1(uint32 handle, uint16 frequency,
                                                   uint16 power1, uint16 power2)

            Parameters :    handle -
                                Handle to the device

                            frequency -
                                Local Oscillator frequency

                            power1 -
                                For BlueCore ICs this sets the drive level for internal PA.
                                0 to 63 for basic rate packets, 0 to 127 for EDR packets.
                                Typical values are 50 for basic rate and 105 for EDR packet
                                types.
                                <p>For QCC5xxx and QCC3xxx ICs, bits 4-5 specify the exponent
                                power control setting. Bits 0-3 and 6-7 are reserved and
                                should be set to zero. Refer to document CS-00408534-TC for
                                full details.
                                <p>In all cases the MSB (upper 8 bits) are ignored.

                            power2 -
                                For BlueCore ICs this sets the drive level for external PA.
                                <p>For QCC5xxx and QCC3xxx ICs, bits 0-3 specify the magnitude
                                power control setting, and bits 4-7 specify the attenuation
                                setting. The magnitude value is a signed value in the range
                                -8 to 7 (2's complement representation). Refer to document
                                CS-00408534-TC for full details.
                                <p>In all cases the MSB (upper 8 bits) are ignored.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function initiates TXDATA1

            Example :

                uint32 rxHandle = 0;
                uint32 txHandle = 0;
                int32 timeout = 0;
                int32 hopping = 0;

                do
                {
                    cout << "Trying rxHandle..." << endl;
                    rxHandle = openTestEngine(BCSP, "COM1", 115200, 1000, 0);
                    timeout+=1000;
                }while(rxHandle == 0 && timeout<MAX_TIMEOUT);

                timeout = 0;
                do
                {
                    cout << "Trying txHandle..." << endl;
                    txHandle = openTestEngine(USB, "\\\\.\\csr0", 0, 1000, 500);
                    timeout+=1000;
                }while(txHandle == 0 && timeout<MAX_TIMEOUT);

                if((txHandle != 0) && (rxHandle != 0))
                {
                    uint16 dut, ref;
                    int32 success = 0;

                    do
                    {
                        success = bccmdGetBuildId(rxHandle, &dut);
                        if(success == TE_OK)
                            cout << "DUT firmware = " << dut << endl;
                        else
                            break;

                        success = bccmdGetBuildId(txHandle, &ref);
                        if(success == TE_OK)
                            cout << "REF firmware = " << ref << endl;
                        else
                            break;

                        // 1. configure the packet types
                        success = radiotestCfgPkt(rxHandle, 15, 339);
                        if(success != TE_OK)
                            break;

                        success = radiotestCfgPkt(txHandle, 15, 339);
                        if(success != TE_OK)
                            break;

                        // 2. Set the TX/RX interval
                        success = radiotestCfgFreq(rxHandle, 37500, 9375, 1);
                        if(success != TE_OK)
                            break;


                        success = radiotestCfgFreq(txHandle, 37500, 9375, 1);
                        if(success != TE_OK)
                            break;

                        uint16 uiFreq = 2441;
                        uint16 uiIntPA = 50;
                        uint16 uiExtPA = 0;
                        uint16 uiCC = 0;
                        uint8 uiHiside = 0;
                        uint16 uiRxAtt = 0;
                        uint16 uiSampleSize = 10000;

                        // 3. Start the transmitter and receiver
                        if(!hopping)
                        {
                            success = radiotestTxdata1(txHandle, uiFreq, uiIntPA, uiExtPA);
                            if(success != TE_OK)
                                break;

                            success = radiotestBer1(rxHandle, uiFreq, uiHiside, uiRxAtt,
                                                    uiSampleSize);
                            if(success != TE_OK)
                                break;
                        }
                        else
                        {
                            success = radiotestTxdata2(txHandle, uiCC, uiIntPA, uiExtPA);
                            if(success != TE_OK)
                                break;

                            success = radiotestBer2(rxHandle, uiCC, uiHiside, uiRxAtt,
                                                    uiSampleSize);
                            if(success != TE_OK)
                                break;
                        }

                        int32 count = 0;
                        uint32 r[9];

                        // 4. Gather the BER information
                        do
                        {
                            count++;
                            success = hqGetBer(rxHandle, 1000, r);
                        }while(success == TE_ERROR && count<10);

                        if(success == TE_OK)
                        {
                            cout << "Bit count = " << r[0] << endl;
                            cout << "Bit errors = " << r[1] << endl;
                            cout << "Received packets = " << r[3] << endl;
                            cout << "Expected packets = " << r[4] << endl;
                            cout << "Header errors = " << r[5] << endl;
                            cout << "CRC errors = " << r[6] << endl;
                            cout << "Uncorrected errors = " << r[7] << endl;
                            cout << "Sync errors = " << r[8] << endl;
                        }
                        else
                            cout << "****FAIL****" << endl;

                    }while(0);

                    if(success != TE_OK)
                        cout << "TestEngine error" << endl;

                }
                if(txHandle != 0)
                    closeTestEngine(txHandle);
                if(rxHandle != 0)
                    closeTestEngine(rxHandle);

        """
        self.TestEngineDLL.radiotestTxdata1.restype = CT.c_int32
        self.TestEngineDLL.radiotestTxdata1.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16, CT.c_uint16]
        retval = self.TestEngineDLL.radiotestTxdata1(handle, frequency, power1, power2)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestTxdata2(uint32 handle, uint16 countrycode, uint16 power1, uint16 power2);
    #########################################################
    # TestEngine::radiotestTxdata2
    @fntrace
    def radiotestTxdata2(self, handle, countrycode, power1, power2):
        r"""
        Function wrapper for TestEngine::radiotestTxdata2().

        Python API:
            radiotestTxdata2(handle, countrycode, power1, power2)
            returns retval

        Generated from C API:

            Function :      int32 radiotestTxdata2(uint32 handle, uint16 countrycode,
                                                   uint16 power1, uint16 power2)

            Parameters :    handle -
                                Handle to the device

                            countrycode -
                                Must be 0. Setting the countrycode to anything other than
                                zero is deprecated.

                            power1 -
                                For BlueCore ICs this sets the drive level for internal PA.
                                0 to 63 for basic rate packets, 0 to 127 for EDR packets.
                                Typical values are 50 for basic rate and 105 for EDR packet
                                types.
                                <p>For QCC5xxx and QCC3xxx ICs, bits 4-5 specify the exponent
                                power control setting. Bits 0-3 and 6-7 are reserved and
                                should be set to zero. Refer to document CS-00408534-TC for
                                full details.
                                <p>In all cases the MSB (upper 8 bits) are ignored.

                            power2 -
                                For BlueCore ICs this sets the drive level for external PA.
                                <p>For QCC5xxx and QCC3xxx ICs, bits 0-3 specify the magnitude
                                power control setting, and bits 4-7 specify the attenuation
                                setting. The magnitude value is a signed value in the range
                                -8 to 7 (2's complement representation). Refer to document
                                CS-00408534-TC for full details.
                                <p>In all cases the MSB (upper 8 bits) are ignored.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function initiates TXDATA2

            Example :

                uint32 rxHandle = 0;
                uint32 txHandle = 0;
                int32 timeout = 0;
                int32 hopping = 0;

                do
                {
                    cout << "Trying rxHandle..." << endl;
                    rxHandle = openTestEngine(BCSP, "COM1", 115200, 1000, 0);
                    timeout+=1000;
                }while(rxHandle == 0 && timeout<MAX_TIMEOUT);

                timeout = 0;

                do
                {
                    cout << "Trying txHandle..." << endl;
                    txHandle = openTestEngine(USB, "\\\\.\\csr0", 0, 1000, 500);
                    timeout+=1000;
                }while(txHandle == 0 && timeout<MAX_TIMEOUT);

                if((txHandle != 0) && (rxHandle != 0))
                {
                    uint16 dut, ref;
                    int32 success = 0;

                    do
                    {

                        success = bccmdGetBuildId(rxHandle, &dut);
                        if(success == TE_OK)
                            cout << "DUT firmware = " << dut << endl;
                        else
                            break;

                        success = bccmdGetBuildId(txHandle, &ref);
                        if(success == TE_OK)
                            cout << "REF firmware = " << ref << endl;
                        else
                            break;

                        // 1. configure the packet types
                        success = radiotestCfgPkt(rxHandle, 15, 339);
                        if(success != TE_OK)
                            break;

                        success = radiotestCfgPkt(txHandle, 15, 339);
                        if(success != TE_OK)
                            break;

                        // 2. Set the TX/RX interval
                        success = radiotestCfgFreq(rxHandle, 37500, 9375, 1);
                        if(success != TE_OK)
                            break;


                        success = radiotestCfgFreq(txHandle, 37500, 9375, 1);
                        if(success != TE_OK)
                            break;

                        uint16 uiFreq = 2441;
                        uint16 uiIntPA = 50;
                        uint16 uiExtPA = 0;
                        uint16 uiCC = 0;
                        uint8 uiHiside = 0;
                        uint16 uiRxAtt = 0;
                        uint16 uiSampleSize = 10000;

                        // 3. Start the transmitter and receiver
                        if(!hopping)
                        {
                            success = radiotestTxdata1(txHandle, uiFreq, uiIntPA, uiExtPA);
                            if(success != TE_OK)
                                break;

                            success = radiotestBer1(rxHandle, uiFreq, uiHiside, uiRxAtt,
                                                    uiSampleSize);
                            if(success != TE_OK)
                                break;
                        }
                        else
                        {
                            success = radiotestTxdata2(txHandle, uiCC, uiIntPA, uiExtPA);
                            if(success != TE_OK)
                                break;

                            success = radiotestBer2(rxHandle, uiCC, uiHiside, uiRxAtt,
                                                    uiSampleSize);
                            if(success != TE_OK)
                                break;
                        }

                        int32 count = 0;
                        uint32 r[9];

                        // 4. Gather the BER information
                        do
                        {
                            count++;
                            success = hqGetBer(rxHandle, 1000, r);
                        }while(success == TE_ERROR && count<10);

                        if(success == TE_OK)
                        {
                            cout << "Bit count = " << r[0] << endl;
                            cout << "Bit errors = " << r[1] << endl;
                            cout << "Received packets = " << r[3] << endl;
                            cout << "Expected packets = " << r[4] << endl;
                            cout << "Header errors = " << r[5] << endl;
                            cout << "CRC errors = " << r[6] << endl;
                            cout << "Uncorrected errors = " << r[7] << endl;
                            cout << "Sync errors = " << r[8] << endl;
                        }
                        else
                            cout << "****FAIL****" << endl;

                    }while(0);

                    if(success != TE_OK)
                        cout << "TestEngine error" << endl;

                }
                if(txHandle != 0)
                    closeTestEngine(txHandle);
                if(rxHandle != 0)
                    closeTestEngine(rxHandle);

        """
        self.TestEngineDLL.radiotestTxdata2.restype = CT.c_int32
        self.TestEngineDLL.radiotestTxdata2.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16, CT.c_uint16]
        retval = self.TestEngineDLL.radiotestTxdata2(handle, countrycode, power1, power2)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestTxdata3(uint32 handle, uint16 frequency, uint16 power1, uint16 power2);
    #########################################################
    # TestEngine::radiotestTxdata3
    @fntrace
    def radiotestTxdata3(self, handle, frequency, power1, power2):
        r"""
        Function wrapper for TestEngine::radiotestTxdata3().

        Python API:
            radiotestTxdata3(handle, frequency, power1, power2)
            returns retval

        Generated from C API:

            Function :      int32 radiotestTxdata3(uint32 handle, uint16 frequency,
                                                   uint16 power1, uint16 power2)

            Parameters :    handle -
                                Handle to the device

                            frequency -
                                Local Oscillator frequency

                            power1 -
                                For BlueCore ICs this sets the drive level for internal PA.
                                0 to 63 for basic rate packets, 0 to 127 for EDR packets.
                                Typical values are 50 for basic rate and 105 for EDR packet
                                types.
                                <p>For QCC5xxx and QCC3xxx ICs, bits 4-5 specify the exponent
                                power control setting. Bits 0-3 and 6-7 are reserved and
                                should be set to zero. Refer to document CS-00408534-TC for
                                full details.
                                <p>In all cases the MSB (upper 8 bits) are ignored.

                            power2 -
                                For BlueCore ICs this sets the drive level for external PA.
                                <p>For QCC5xxx and QCC3xxx ICs, bits 0-3 specify the magnitude
                                power control setting, and bits 4-7 specify the attenuation
                                setting. The magnitude value is a signed value in the range
                                -8 to 7 (2's complement representation). Refer to document
                                CS-00408534-TC for full details.
                                <p>In all cases the MSB (upper 8 bits) are ignored.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function initiates TXDATA3

        """
        self.TestEngineDLL.radiotestTxdata3.restype = CT.c_int32
        self.TestEngineDLL.radiotestTxdata3.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16, CT.c_uint16]
        retval = self.TestEngineDLL.radiotestTxdata3(handle, frequency, power1, power2)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestTxdata4(uint32 handle, uint16 frequency, uint16 power1, uint16 power2);
    #########################################################
    # TestEngine::radiotestTxdata4
    @fntrace
    def radiotestTxdata4(self, handle, frequency, power1, power2):
        r"""
        Function wrapper for TestEngine::radiotestTxdata4().

        Python API:
            radiotestTxdata4(handle, frequency, power1, power2)
            returns retval

        Generated from C API:

            Function :      int32 radiotestTxdata4(uint32 handle, uint16 frequency,
                                                   uint16 power1, uint16 power2)

            Parameters :    handle -
                                Handle to the device

                            frequency -
                                Local Oscillator frequency

                            power1 -
                                For BlueCore ICs this sets the drive level for internal PA.
                                0 to 63 for basic rate packets, 0 to 127 for EDR packets.
                                Typical values are 50 for basic rate and 105 for EDR packet
                                types.
                                <p>For QCC5xxx and QCC3xxx ICs, bits 4-5 specify the exponent
                                power control setting. Bits 0-3 and 6-7 are reserved and
                                should be set to zero. Refer to document CS-00408534-TC for
                                full details.
                                <p>In all cases the MSB (upper 8 bits) are ignored.

                            power2 -
                                For BlueCore ICs this sets the drive level for external PA.
                                <p>For QCC5xxx and QCC3xxx ICs, bits 0-3 specify the magnitude
                                power control setting, and bits 4-7 specify the attenuation
                                setting. The magnitude value is a signed value in the range
                                -8 to 7 (2's complement representation). Refer to document
                                CS-00408534-TC for full details.
                                <p>In all cases the MSB (upper 8 bits) are ignored.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function initiates TXDATA4

        """
        self.TestEngineDLL.radiotestTxdata4.restype = CT.c_int32
        self.TestEngineDLL.radiotestTxdata4.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16, CT.c_uint16]
        retval = self.TestEngineDLL.radiotestTxdata4(handle, frequency, power1, power2)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestCfgTxPower(uint32 handle, int16 power);
    #########################################################
    # TestEngine::radiotestCfgTxPower
    @fntrace
    def radiotestCfgTxPower(self, handle, power):
        r"""
        Function wrapper for TestEngine::radiotestCfgTxPower().

        Python API:
            radiotestCfgTxPower(handle, power)
            returns retval

        Generated from C API:

            Function :      int32 radiotestCfgTxPower(uint32 handle, int16 power)

            Parameters :    handle -
                                Handle to the device

                            power -
                                Requested power in dBm. This value is used to select the
                                power table entry to use.
                                <p>The power table entry with the closest output power equal
                                to or less than this value is selected.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function utilises the CFG_TXPOWER radiotest function. It is
                            used to specify the output power for subsequent radiotests, e.g.
                            radiotestTxstart. Subsequent radiotests will ignore power
                            parameters (e.g. internal PA level, external PA level) and use
                            the power table values instead.
                            <p>The closest power table entry with output power
                            equal to or less than the power parameter value is selected.
                            Therefore, the actual output power may be less than what is
                            specified. The difference is dependent on the power table
                            configuration for the device.
                            <p>Setting the power value to the int16 representation of 0xdeaf
                            (-8529) switches off the use of the power table for subsequent
                            radiotest operations. After this value is set, power parameters
                            for subsequent radiotests are used instead of the power table
                            values.

        """
        self.TestEngineDLL.radiotestCfgTxPower.restype = CT.c_int32
        self.TestEngineDLL.radiotestCfgTxPower.argtypes = [CT.c_uint32, CT.c_int16]
        retval = self.TestEngineDLL.radiotestCfgTxPower(handle, power)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestRxstart1(uint32 handle, uint16 frequency, uint8 hiside, uint16 rx_attenuation);
    #########################################################
    # TestEngine::radiotestRxstart1
    @fntrace
    def radiotestRxstart1(self, handle, frequency, hiside, rx_attenuation):
        r"""
        Function wrapper for TestEngine::radiotestRxstart1().

        Python API:
            radiotestRxstart1(handle, frequency, hiside, rx_attenuation)
            returns retval

        Generated from C API:

            Function :      int32 radiotestRxstart1(uint32 handle, uint16 frequency,
                                                    uint8 hiside, uint16 rx_attenuation)

            Parameters :    handle -
                                Handle to the device

                            frequency -
                                Local Oscillator frequency in MHz. Should be in the range
                                2402 to 2480.

                            hiside -
                                Set to 0 for low side modulation and 1 for high side
                                modulation.

                            rx_attenuation -
                                Attenuation setting. For ICs other than BC5, this value
                                should be in the range 0 to 15.
                                <p>For BC5 ICs, this value also controls the RX mixer
                                attenuation and I2I level, where the bits are used as
                                follows:
                                <table>
                                    <tr><td>0-3: Attenuation setting
                                    <tr><td>4-5: Mixer attenuation
                                    <tr><td>6-7: Unused
                                    <tr><td>8-9: I2I level
                                </table>
                                Therefore the maximum value for BC5 is 1023.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function initiates RXSTART1.
                            <p>Enables the receiver in continuous reception at a designated
                            frequency (LO Freq) with a choice of low or high side modulation
                            (hi-side) and with a designated attenuation setting (RX
                            Attenuation).
                            <p>Requires a second unit to be running TXSTART.

        """
        self.TestEngineDLL.radiotestRxstart1.restype = CT.c_int32
        self.TestEngineDLL.radiotestRxstart1.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint8, CT.c_uint16]
        retval = self.TestEngineDLL.radiotestRxstart1(handle, frequency, hiside, rx_attenuation)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestRxstart2(uint32 handle, uint16 frequency, uint8 hiside, uint16 rx_attenuation, uint16 sample_size);
    #########################################################
    # TestEngine::radiotestRxstart2
    @fntrace
    def radiotestRxstart2(self, handle, frequency, hiside, rx_attenuation, sample_size):
        r"""
        Function wrapper for TestEngine::radiotestRxstart2().

        Python API:
            radiotestRxstart2(handle, frequency, hiside, rx_attenuation, sample_size)
            returns retval

        Generated from C API:

            Function :      int32 radiotestRxstart2(uint32 handle, uint16 frequency,
                                                    uint8 hiside, uint16 rx_attenuation,
                                                    uint16 sample_size)

            Parameters :    handle -
                                Handle to the device

                            frequency -
                                Local Oscillator frequency in MHz. Should be in the range
                                2402 to 2480.

                            hiside -
                                Set to 0 for low side modulation and 1 for high side
                                modulation.

                            rx_attenuation -
                                Attenuation setting. For ICs other than BC5, this value
                                should be in the range 0 to 15.
                                <p>For BC5 ICs, this value also controls the RX mixer
                                attenuation and I2I level, where the bits are used as
                                follows:
                                <table>
                                    <tr><td>0-3: Attenuation setting
                                    <tr><td>4-5: Mixer attenuation
                                    <tr><td>6-7: Unused
                                    <tr><td>8-9: I2I level
                                </table>
                                Therefore the maximum value for BC5 is 1023.

                            sample_size -
                                Number of RSSI reports to capture (for averaging purposes), up
                                to a maximum value of 100. Values greater than 100 will result
                                in the function returning Error.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function initiates RXSTART2.
                            <p>Enables the receiver in continuous reception, at a designated
                            frequency (LO Freq), with a choice of low or high side modulation
                            (hi-side) and with a designated attenuation setting (RX
                            Attenuation).
                            <p>Requires a second unit to be running TXSTART (see
                            radiotestTxstart).
                            <p>RSSI results can be obtained by calling hqGetRssi. When
                            sample_size results have been obtained, further results are
                            ignored until either the set has been collected with hqGetRssi,
                            or until radiotestRxstart2 is called again.
                            <p>This function is only supported over a SPI connection if HQ
                            over SPI is supported by the device firmware.

        """
        self.TestEngineDLL.radiotestRxstart2.restype = CT.c_int32
        self.TestEngineDLL.radiotestRxstart2.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint8, CT.c_uint16, CT.c_uint16]
        retval = self.TestEngineDLL.radiotestRxstart2(handle, frequency, hiside, rx_attenuation, sample_size)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hqGetRssi(uint32 handle, int32 timeout, uint16 max_size, uint16 r[]);
    #########################################################
    # TestEngine::hqGetRssi
    @fntrace
    def hqGetRssi(self, handle, timeout, max_size, r):
        r"""
        Function wrapper for TestEngine::hqGetRssi().

        Python API:
            hqGetRssi(handle, timeout, max_size, r)
            returns retval

        Generated from C API:

            Function :      int32 hqGetRssi(uint32 handle, int32 timeout, uint16 max_size,
                                            uint16 * r)

            Parameters :    handle -
                                Handle to the device

                            timeout -
                                Time, in ms, which the function will wait for valid results.
                                If set to zero the function will just check for completion.

                            max_size -
                                Size of the array allocated to hold the RSSI samples

                            r -
                                Array to contain the RSSI samples

            Returns :        <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function obtains a set of RSSI results from BlueCore,
                            obtained as a result of calling radiotestRxstart2. The number of
                            results that will be written is dependent on the sample_size
                            parameter for radiotestRxstart2.

        """
        self.TestEngineDLL.hqGetRssi.restype = CT.c_int32
        self.TestEngineDLL.hqGetRssi.argtypes = [CT.c_uint32, CT.c_int32, CT.c_uint16, CT.c_void_p]
        __r = (CT.c_uint16 * len(r))(*r)
        retval = self.TestEngineDLL.hqGetRssi(handle, timeout, max_size, CT.byref(__r))
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestBer1(uint32 handle, uint16 frequency, uint8 hiside, uint16 rx_attenuation, uint32 sampleSize);
    #########################################################
    # TestEngine::radiotestBer1
    @fntrace
    def radiotestBer1(self, handle, frequency, hiside, rx_attenuation, sampleSize):
        r"""
        Function wrapper for TestEngine::radiotestBer1().

        Python API:
            radiotestBer1(handle, frequency, hiside, rx_attenuation, sampleSize)
            returns retval

        Generated from C API:

            Function :      int32 radiotestBer1(uint32 handle, uint16 frequency, uint8 hiside,
                                                uint16 rx_attenuation, uint32 sampleSize)

            Parameters :    handle -
                                Handle to the device

                            frequency -
                                Local Oscillator frequency in MHz. Should be in the range
                                2402 to 2480.

                            hiside -
                                Set to 0 for low side modulation and 1 for high side
                                modulation.

                            rx_attenuation -
                                Attenuation setting. Should be in the range 0 to 15.

                            sampleSize -
                                Number of bits to be sampled.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function initiates BER1.
                            <p>Enables the receiver at a designated frequency (LO Freq) with
                            a choice of low or high side modulation (hi-side) and with a
                            designated attenuation setting (RX Attenuation).
                            <p>This function is only supported over a SPI connection if HQ
                            over SPI is supported by the device firmware.

        """
        self.TestEngineDLL.radiotestBer1.restype = CT.c_int32
        self.TestEngineDLL.radiotestBer1.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint8, CT.c_uint16, CT.c_uint32]
        retval = self.TestEngineDLL.radiotestBer1(handle, frequency, hiside, rx_attenuation, sampleSize)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestBer2(uint32 handle, uint16 country_code, uint8 hiside, uint16 rx_attenuation, uint32 sampleSize);
    #########################################################
    # TestEngine::radiotestBer2
    @fntrace
    def radiotestBer2(self, handle, country_code, hiside, rx_attenuation, sampleSize):
        r"""
        Function wrapper for TestEngine::radiotestBer2().

        Python API:
            radiotestBer2(handle, country_code, hiside, rx_attenuation, sampleSize)
            returns retval

        Generated from C API:

            Function :      int32 radiotestBer2(uint32 handle, uint16 country_code,
                                                uint8 hiside, uint16 rx_attenuation,
                                                uint32 sampleSize)

            Parameters :    handle -
                                Handle to the device

                            country_code -
                                Must be 0. Setting the country_code to anything other than
                                zero is deprecated.

                            hiside -
                                Set to 0 for low side modulation and 1 for high side
                                modulation.

                            rx_attenuation -
                                Attenuation setting. Should be in the range 0 to 15.

                            sampleSize -
                                Number of bits to be sampled.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function initiates BER2.
                            <p>Enables the receiver with hopping, with a choice of low or
                            high side modulation (hiside), and with a designated attenuation
                            setting (rx_attenuation) as for radiotestRxdata2.
                            <p>This function is only supported over a SPI connection if HQ
                            over SPI is supported by the device firmware.

        """
        self.TestEngineDLL.radiotestBer2.restype = CT.c_int32
        self.TestEngineDLL.radiotestBer2.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint8, CT.c_uint16, CT.c_uint32]
        retval = self.TestEngineDLL.radiotestBer2(handle, country_code, hiside, rx_attenuation, sampleSize)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestBerLoopback(uint32 handle, uint16 frequency, uint16 power1, uint16 power2, uint32 sampleSize);
    #########################################################
    # TestEngine::radiotestBerLoopback
    @fntrace
    def radiotestBerLoopback(self, handle, frequency, power1, power2, sampleSize):
        r"""
        Function wrapper for TestEngine::radiotestBerLoopback().

        Python API:
            radiotestBerLoopback(handle, frequency, power1, power2, sampleSize)
            returns retval

        Generated from C API:

            Function :      int32 radiotestBerLoopback(uint32 handle, uint16 frequency,
                                                       uint16 power1, uint16 power2,
                                                       uint32 sampleSize);

            Parameters :    handle -
                                Handle to the device

                            frequency -
                                Local Oscillator frequency

                            power1 -
                                For BlueCore ICs this sets the drive level for internal PA.
                                0 to 63 for basic rate packets, 0 to 127 for EDR packets.
                                Typical values are 50 for basic rate and 105 for EDR packet
                                types.
                                <p>For QCC5xxx and QCC3xxx ICs, bits 4-5 specify the exponent
                                power control setting. Bits 0-3 and 6-7 are reserved and
                                should be set to zero. Refer to document CS-00408534-TC for
                                full details.
                                <p>In all cases the MSB (upper 8 bits) are ignored.

                            power2 -
                                For BlueCore ICs this sets the drive level for external PA.
                                <p>For QCC5xxx and QCC3xxx ICs, bits 0-3 specify the magnitude
                                power control setting, and bits 4-7 specify the attenuation
                                setting. The magnitude value is a signed value in the range
                                -8 to 7 (2's complement representation). Refer to document
                                CS-00408534-TC for full details.
                                <p>In all cases the MSB (upper 8 bits) are ignored.

                            sampleSize -
                                Number of bits to be sampled.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function initiates BER_LOOPBACK.
                            <p>This function is only supported over a SPI connection if HQ
                            over SPI is supported by the device firmware.

            Example :

                uint32 rxHandle = 0;
                uint32 txHandle = 0;
                int32 timeout = 0;

                do
                {
                    cout << "Trying rxHandle..." << endl;
                    rxHandle = openTestEngine(BCSP, "COM1", 115200, 1000, 0);
                    timeout+=1000;
                }while(rxHandle == 0 && timeout<MAX_TIMEOUT);

                timeout = 0;

                do
                {
                    cout << "Trying txHandle..." << endl;
                    txHandle = openTestEngine(USB, "\\\\.\\csr0", 0, 1000, 500);
                    timeout+=1000;
                }while(txHandle == 0 && timeout<MAX_TIMEOUT);

                if((txHandle != 0) && (rxHandle != 0))
                {
                    uint16 dut, ref;
                    int32 success = 0;

                    do
                    {
                        success = bccmdGetBuildId(rxHandle, &dut);
                        if(success == TE_OK)
                            cout << "DUT firmware = " << dut << endl;
                        else
                            break;

                        success = bccmdGetBuildId(txHandle, &ref);
                        if(success == TE_OK)
                            cout << "REF firmware = " << ref << endl;
                        else
                            break;

                        // 1. configure the packet types
                        success = radiotestCfgPkt(rxHandle, 15, 339);
                        if(success != TE_OK)
                            break;

                        success = radiotestCfgPkt(txHandle, 15, 339);
                        if(success != TE_OK)
                            break;

                        // 2. Set the TX/RX interval
                        success = radiotestCfgFreq(rxHandle, 37500, 9375, 1);
                        if(success != TE_OK)
                            break;

                        success = radiotestCfgFreq(txHandle, 37500, 9375, 1);
                        if(success != TE_OK)
                            break;

                        static const uint16 freq(2441);
                        static const uint16 intPA(50);
                        static const uint16 extPA(0);
                        static const uint16 sampleSize(10000);

                        // 3. Start the transmitter and receiver
                        success = radiotestBerLoopback(txHandle, freq, intPA, extPA,
                                                        sampleSize);
                        if(success != TE_OK)
                            break;

                        success = radiotestLoopback(rxHandle, freq, intPA, extPA);
                        if(success != TE_OK)
                            break;

                        int32 count = 0;
                        uint32 r[9];

                        // 4. Gather the BER information
                        do
                        {
                            count++;
                            success = hqGetBer(txHandle, 1000, r);
                        }while(success == TE_ERROR && count<10);

                        if(success == TE_OK)
                        {
                            cout << "Bit count = " << r[0] << endl;
                            cout << "Bit errors = " << r[1] << endl;
                            cout << "Received packets = " << r[3] << endl;
                            cout << "Expected packets = " << r[4] << endl;
                            cout << "Header errors = " << r[5] << endl;
                            cout << "CRC errors = " << r[6] << endl;
                            cout << "Uncorrected errors = " << r[7] << endl;
                            cout << "Sync errors = " << r[8] << endl;
                        }
                        else
                            cout << "****FAIL****" << endl;

                    }while(0);

                    if(success != TE_OK)
                        cout << "TestEngine error" << endl;

                }
                if(txHandle != 0)
                    closeTestEngine(txHandle);
                if(rxHandle != 0)
                    closeTestEngine(rxHandle);

        """
        self.TestEngineDLL.radiotestBerLoopback.restype = CT.c_int32
        self.TestEngineDLL.radiotestBerLoopback.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16, CT.c_uint16, CT.c_uint32]
        retval = self.TestEngineDLL.radiotestBerLoopback(handle, frequency, power1, power2, sampleSize)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestRxLoopback(uint32 handle, uint16 frequency, uint16 intPA, uint16 extPA);
    #########################################################
    # TestEngine::radiotestRxLoopback
    @fntrace
    def radiotestRxLoopback(self, handle, frequency, intPA, extPA):
        r"""
        Function wrapper for TestEngine::radiotestRxLoopback().

        Python API:
            radiotestRxLoopback(handle, frequency, intPA, extPA)
            returns retval

        Generated from C API:

            Function :      int32 radiotestRxLoopback(uint32 handle, uint16 frequency,
                                                      uint16 intPA, uint16 extPA)

            Parameters :    handle -
                                Handle to the device

                            frequency -
                                Local Oscillator frequency

                            intPA -
                                Drive level for internal PA. 0 to 63 for basic rate packets,
                                0 to 127 for EDR packets. Typical values are 50 for basic
                                rate and 105 for EDR packet types.

                            extPA -
                                Drive level for external PA

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function initiates RX_LOOPBACK.
                            <p>This function is only supported over a SPI connection if HQ
                            over SPI is supported by the device firmware.

            Deprecated :

        """
        self.TestEngineDLL.radiotestRxLoopback.restype = CT.c_int32
        self.TestEngineDLL.radiotestRxLoopback.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16, CT.c_uint16]
        retval = self.TestEngineDLL.radiotestRxLoopback(handle, frequency, intPA, extPA)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestLoopback(uint32 handle, uint16 frequency, uint16 power1, uint16 power2);
    #########################################################
    # TestEngine::radiotestLoopback
    @fntrace
    def radiotestLoopback(self, handle, frequency, power1, power2):
        r"""
        Function wrapper for TestEngine::radiotestLoopback().

        Python API:
            radiotestLoopback(handle, frequency, power1, power2)
            returns retval

        Generated from C API:

            Function :      int32 radiotestLoopback(uint32 handle, uint16 frequency,
                                                    uint16 power1, uint16 power2)

            Parameters :    handle -
                                Handle to the device

                            frequency -
                                Local Oscillator frequency

                            power1 -
                                For BlueCore ICs this sets the drive level for internal PA.
                                0 to 63 for basic rate packets, 0 to 127 for EDR packets.
                                Typical values are 50 for basic rate and 105 for EDR packet
                                types.
                                <p>For QCC5xxx and QCC3xxx ICs, bits 4-5 specify the exponent
                                power control setting. Bits 0-3 and 6-7 are reserved and
                                should be set to zero. Refer to document CS-00408534-TC for
                                full details.
                                <p>In all cases the MSB (upper 8 bits) are ignored.

                            power2 -
                                For BlueCore ICs this sets the drive level for external PA.
                                <p>For QCC5xxx and QCC3xxx ICs, bits 0-3 specify the magnitude
                                power control setting, and bits 4-7 specify the attenuation
                                setting. The magnitude value is a signed value in the range
                                -8 to 7 (2's complement representation). Refer to document
                                CS-00408534-TC for full details.
                                <p>In all cases the MSB (upper 8 bits) are ignored.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function initiates LOOPBACK

            Example :

                uint32 rxHandle = 0;
                uint32 txHandle = 0;
                int32 timeout = 0;

                do
                {
                    cout << "Trying rxHandle..." << endl;
                    rxHandle = openTestEngine(BCSP, "COM1", 115200, 1000, 0);
                    timeout+=1000;
                }while(rxHandle == 0 && timeout<MAX_TIMEOUT);

                timeout = 0;

                do
                {
                    cout << "Trying txHandle..." << endl;
                    txHandle = openTestEngine(USB, "\\\\.\\csr0", 0, 1000, 500);
                    timeout+=1000;
                }while(txHandle == 0 && timeout<MAX_TIMEOUT);

                if((txHandle != 0) && (rxHandle != 0))
                {
                    uint16 dut, ref;
                    int32 success = 0;

                    do
                    {
                        success = bccmdGetBuildId(rxHandle, &dut);
                        if(success == TE_OK)
                            cout << "DUT firmware = " << dut << endl;
                        else
                            break;

                        success = bccmdGetBuildId(txHandle, &ref);
                        if(success == TE_OK)
                            cout << "REF firmware = " << ref << endl;
                        else
                            break;

                        // 1. configure the packet types
                        success = radiotestCfgPkt(rxHandle, 15, 339);
                        if(success != TE_OK)
                            break;

                        success = radiotestCfgPkt(txHandle, 15, 339);
                        if(success != TE_OK)
                            break;

                        // 2. Set the TX/RX interval
                        success = radiotestCfgFreq(rxHandle, 37500, 9375, 1);
                        if(success != TE_OK)
                            break;

                        success = radiotestCfgFreq(txHandle, 37500, 9375, 1);
                        if(success != TE_OK)
                            break;

                        static const uint16 freq(2441);
                        static const uint16 intPA(50);
                        static const uint16 extPA(0);
                        static const uint16 sampleSize(10000);

                        // 3. Start the transmitter and receiver
                        success = radiotestBerLoopback(txHandle, freq, intPA, extPA,
                                                        sampleSize);
                        if(success != TE_OK)
                            break;

                        success = radiotestLoopback(rxHandle, freq, intPA, extPA);
                        if(success != TE_OK)
                            break;

                        int32 count = 0;
                        uint32 r[9];

                        // 4. Gather the BER information
                        do
                        {
                            count++;
                            success = hqGetBer(txHandle, 1000, r);
                        }while(success == TE_ERROR && count<10);

                        if(success == TE_OK)
                        {
                            cout << "Bit count = " << r[0] << endl;
                            cout << "Bit errors = " << r[1] << endl;
                            cout << "Received packets = " << r[3] << endl;
                            cout << "Expected packets = " << r[4] << endl;
                            cout << "Header errors = " << r[5] << endl;
                            cout << "CRC errors = " << r[6] << endl;
                            cout << "Uncorrected errors = " << r[7] << endl;
                            cout << "Sync errors = " << r[8] << endl;
                        }
                        else
                            cout << "****FAIL****" << endl;

                    }while(0);

                    if(success != TE_OK)
                        cout << "TestEngine error" << endl;

                }
                if(txHandle != 0)
                    closeTestEngine(txHandle);
                if(rxHandle != 0)
                    closeTestEngine(rxHandle);

        """
        self.TestEngineDLL.radiotestLoopback.restype = CT.c_int32
        self.TestEngineDLL.radiotestLoopback.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16, CT.c_uint16]
        retval = self.TestEngineDLL.radiotestLoopback(handle, frequency, power1, power2)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hqGetBer(uint32 handle, int32 timeout, uint32 r[]);
    #########################################################
    # TestEngine::hqGetBer
    @fntrace
    def hqGetBer(self, handle, timeout, r):
        r"""
        Function wrapper for TestEngine::hqGetBer().

        Python API:
            hqGetBer(handle, timeout, r)
            returns retval

        Generated from C API:

            Function :      int32 hqGetBer(uint32 handle, int32 timeout, uint32 * r)

            Parameters :    handle -
                                Handle to the device

                            timeout -
                                Time, in ms, which the function will wait for valid results.
                                If set to zero the function will just check for completion.

                            r -
                                Array to contain cumulative BER report. Defined as follows:
                                <table>
                                    <tr><td>r[0] = cumulative bit count
                                    <tr><td>r[1] = cumulative bit errors
                                    <tr><td>r[2] = NOT USED
                                    <tr><td>r[3] = cumulative received packets
                                    <tr><td>r[4] = cumulative expected packets
                                    <tr><td>r[5] = cumulative header errors
                                    <tr><td>r[6] = cumulative CRC errors
                                    <tr><td>r[7] = cumulative uncorrected errors
                                    <tr><td>r[8] = cumulative sync errors
                                </table>

            Returns :        <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function obtains the last complete BER results sample from
                            BlueCore.

            Example :

                uint32 rxHandle = 0;
                uint32 txHandle = 0;
                int32 timeout = 0;
                int32 hopping = 0;

                do
                {
                    cout << "Trying rxHandle..." << endl;
                    rxHandle = openTestEngine(BCSP, "COM1", 115200, 1000, 0);
                    timeout+=1000;
                }while(rxHandle == 0 && timeout<MAX_TIMEOUT);

                timeout = 0;

                do
                {
                    cout << "Trying txHandle..." << endl;
                    txHandle = openTestEngine(USB, "\\\\.\\csr0", 0, 1000, 500);
                    timeout+=1000;
                }while(txHandle == 0 && timeout<MAX_TIMEOUT);

                if((txHandle != 0) && (rxHandle != 0))
                {
                    uint16 dut, ref;
                    int32 success = 0;

                    do
                    {

                        success = bccmdGetBuildId(rxHandle, &dut);
                        if(success == TE_OK)
                            cout << "DUT firmware = " << dut << endl;
                        else
                            break;

                        success = bccmdGetBuildId(txHandle, &ref);
                        if(success == TE_OK)
                            cout << "REF firmware = " << ref << endl;
                        else
                            break;

                        // 1. configure the packet types
                        success = radiotestCfgPkt(rxHandle, 15, 339);
                        if(success != TE_OK)
                            break;

                        success = radiotestCfgPkt(txHandle, 15, 339);
                        if(success != TE_OK)
                            break;

                        // 2. Set the TX/RX interval
                        success = radiotestCfgFreq(rxHandle, 37500, 9375, 1);
                        if(success != TE_OK)
                            break;


                        success = radiotestCfgFreq(txHandle, 37500, 9375, 1);
                        if(success != TE_OK)
                            break;

                        uint16 uiFreq = 2441;
                        uint16 uiIntPA = 50;
                        uint16 uiExtPA = 0;
                        uint16 uiCC = 0;
                        uint8 uiHiside = 0;
                        uint16 uiRxAtt = 0;
                        uint16 uiSampleSize = 10000;

                        // 3. Start the transmitter and receiver
                        success = radiotestBerLoopback(txHandle, 2441, uiIntPA, uiExtPA,
                                                        uiSampleSize);
                        if(success != TE_OK)
                            break;

                        success = radiotestLoopback(rxHandle, uiFreq, uiIntPA, uiExtPA);
                        if(success != TE_OK)
                            break;

                        int32 count = 0;
                        uint32 r[9];

                        // 4. Gather the BER information
                        do
                        {
                            count++;
                            success = hqGetBer(txHandle, 1000, r);
                        }while(success == TE_ERROR && count<10);

                        if(success == TE_OK)
                        {
                            cout << "Bit count = " << r[0] << endl;
                            cout << "Bit errors = " << r[1] << endl;
                            cout << "Received packets = " << r[3] << endl;
                            cout << "Expected packets = " << r[4] << endl;
                            cout << "Header errors = " << r[5] << endl;
                            cout << "CRC errors = " << r[6] << endl;
                            cout << "Uncorrected errors = " << r[7] << endl;
                            cout << "Sync errors = " << r[8] << endl;
                        }
                        else
                            cout << "****FAIL****" << endl;

                    }while(0);

                    if(success != TE_OK)
                        cout << "TestEngine error" << endl;

                }
                if(txHandle != 0)
                    closeTestEngine(txHandle);
                if(rxHandle != 0)
                    closeTestEngine(rxHandle);

        """
        self.TestEngineDLL.hqGetBer.restype = CT.c_int32
        self.TestEngineDLL.hqGetBer.argtypes = [CT.c_uint32, CT.c_int32, CT.c_void_p]
        __r = (CT.c_uint32 * len(r))(*r)
        retval = self.TestEngineDLL.hqGetBer(handle, timeout, CT.byref(__r))
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestRxdata1(uint32 handle, uint16 frequency, uint8 hiside, uint16 rx_attenuation);
    #########################################################
    # TestEngine::radiotestRxdata1
    @fntrace
    def radiotestRxdata1(self, handle, frequency, hiside, rx_attenuation):
        r"""
        Function wrapper for TestEngine::radiotestRxdata1().

        Python API:
            radiotestRxdata1(handle, frequency, hiside, rx_attenuation)
            returns retval

        Generated from C API:

            Function :      int32 radiotestRxdata1(uint32 handle, uint16 frequency,
                                                    uint8 hiside, uint16 rx_attenuation)

            Parameters :    handle -
                                Handle to the device.

                            frequency -
                                Local Oscillator (LO) frequency in MHz. Should be in the range
                                2402 to 2480.

                            hiside -
                                Set to 0 for low side modulation or 1 for high side
                                modulation.

                            rx_attenuation -
                                Attenuation setting. Should be in the range 0 to 15.

            Returns :        <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Enables the receiver, at a designated frequency (LO Freq) with
                            a choice of low or high side modulation (hi-side), and with a
                            designated attenuation setting (RX Attenuation). The software
                            counts the number of received packets and the number of payloads
                            with correctable errors.
                            <p>This function is only supported over a SPI connection if HQ
                            over SPI is supported by the device firmware.

        """
        self.TestEngineDLL.radiotestRxdata1.restype = CT.c_int32
        self.TestEngineDLL.radiotestRxdata1.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint8, CT.c_uint16]
        retval = self.TestEngineDLL.radiotestRxdata1(handle, frequency, hiside, rx_attenuation)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestRxdata2(uint32 handle, uint16 countrycode, uint8 hiside, uint16 rx_attenuation);
    #########################################################
    # TestEngine::radiotestRxdata2
    @fntrace
    def radiotestRxdata2(self, handle, countrycode, hiside, rx_attenuation):
        r"""
        Function wrapper for TestEngine::radiotestRxdata2().

        Python API:
            radiotestRxdata2(handle, countrycode, hiside, rx_attenuation)
            returns retval

        Generated from C API:

            Function :      int32 radiotestRxdata2(uint32 handle, uint16 countrycode,
                                                    uint8 hiside, uint16 rx_attenuation)

            Parameters :    handle -
                                Handle to the device.

                            countrycode -
                                Must be 0. Setting the countrycode to anything other than
                                zero is deprecated.

                            hiside -
                                Set to 0 for low side modulation or 1 for high side
                                modulation.

                            rx_attenuation -
                                Attenuation setting. Should be in the range 0 to 15.

            Returns :        <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Enables the receiver with hopping, with a choice of low or high
                            side modulation (hiside) and with a designated attenuation
                            setting (rx_attenuation). The software counts the number of
                            received packets and the number of payloads with correctable
                            errors.
                            <p>This function is only supported over a SPI connection if HQ
                            over SPI is supported by the device firmware.

        """
        self.TestEngineDLL.radiotestRxdata2.restype = CT.c_int32
        self.TestEngineDLL.radiotestRxdata2.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint8, CT.c_uint16]
        retval = self.TestEngineDLL.radiotestRxdata2(handle, countrycode, hiside, rx_attenuation)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hqGetRxdata(uint32 handle, int32 timeout, uint16 r[]);
    #########################################################
    # TestEngine::hqGetRxdata
    @fntrace
    def hqGetRxdata(self, handle, timeout, r):
        r"""
        Function wrapper for TestEngine::hqGetRxdata().

        Python API:
            hqGetRxdata(handle, timeout, r)
            returns retval

        Generated from C API:

            Function :      int32 hqGetRxdata(uint32 handle, int32 timeout, uint16 * r)

            Parameters :    handle -
                                Handle to the device

                            timeout -
                                Time, in ms, which the function will wait for valid results.
                                If set to zero the function will just check for completion.

                            r -
                                Pointer to an array which will receive the following:
                                <table>
                                    <tr><td> r[0] = Number of packets received
                                    <tr><td> r[1] = Number of good packets
                                    <tr><td> r[2] = Number of corrected packets
                                    <tr><td> r[3] = RSSI value
                                    <tr><td> r[4] = Boolean flag showing if RSSI value is valid
                                </table>

            Returns :        <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will return the last packet generated by either
                            RXDATA1 or RXDATA2. Once the packet has been read it cannot be
                            read again until the next packet is generated.

        """
        self.TestEngineDLL.hqGetRxdata.restype = CT.c_int32
        self.TestEngineDLL.hqGetRxdata.argtypes = [CT.c_uint32, CT.c_int32, CT.c_void_p]
        __r = (CT.c_uint16 * len(r))(*r)
        retval = self.TestEngineDLL.hqGetRxdata(handle, timeout, CT.byref(__r))
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestCfgFreq(uint32 handle, uint16 txrxinterval, uint16 loopback, uint16 report);
    #########################################################
    # TestEngine::radiotestCfgFreq
    @fntrace
    def radiotestCfgFreq(self, handle, txrxinterval, loopback, report):
        r"""
        Function wrapper for TestEngine::radiotestCfgFreq().

        Python API:
            radiotestCfgFreq(handle, txrxinterval, loopback, report)
            returns retval

        Generated from C API:

            Function :      int32 radiotestCfgFreq(uint32 handle, uint16 txrxinterval,
                                                    uint16 loopback, uint16 report)

            Parameters :    handle -
                                Handle to the device

                            txrxinterval -
                                Sets the period in microseconds between TX and RX events in
                                RXDATA, TXDATA, BIT ERR and LOOP BACK test modes. Default is
                                1250 (20 slots), maximum 65536. If passed as 0, current value
                                unchanged.

                            loopback -
                                Sets the offset in microseconds between a reception event and
                                retransmission of the data in loopback. Default is 1875 (two
                                slots later), must be less than txrxinterval. If passed as
                                zero current value unchanged.

                            report -
                                Sets the time in seconds between reports to host sent by
                                RXDATA and BIT ERR functions. The default is 1s. If passed 0,
                                the current value is unchanged. The maximum value is 65.
                                Passing a value greater than 65 results in error.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Sets three values used in deciding timing details of tests. This
                            command has been superseded by radiotestCfgFreqMs, which allows
                            finer control of the reporting interval (in milli-seconds).

            Example :

                uint32 rxHandle = 0;
                uint32 txHandle = 0;
                int32 timeout = 0;
                int32 hopping = 0;

                do
                {
                    cout << "Trying rxHandle..." << endl;
                    rxHandle = openTestEngine(BCSP, "COM1", 115200, 1000, 0);
                    timeout+=1000;
                }while(rxHandle == 0 && timeout<MAX_TIMEOUT);

                timeout = 0;

                do
                {
                    cout << "Trying txHandle..." << endl;
                    txHandle = openTestEngine(USB, "\\\\.\\csr0", 0, 1000, 500);
                    timeout+=1000;
                }while(txHandle == 0 && timeout<MAX_TIMEOUT);

                if((txHandle != 0) && (rxHandle != 0))
                {
                    uint16 dut, ref;
                    int32 success = 0;

                    do
                    {

                        success = bccmdGetBuildId(rxHandle, &dut);
                        if(success == TE_OK)
                            cout << "DUT firmware = " << dut << endl;
                        else
                            break;

                        success = bccmdGetBuildId(txHandle, &ref);
                        if(success == TE_OK)
                            cout << "REF firmware = " << ref << endl;
                        else
                            break;

                        // 1. configure the packet types
                        success = radiotestCfgPkt(rxHandle, 15, 339);
                        if(success != TE_OK)
                            break;

                        success = radiotestCfgPkt(txHandle, 15, 339);
                        if(success != TE_OK)
                            break;

                        // 2. Set the TX/RX interval
                        success = radiotestCfgFreq(rxHandle, 37500, 9375, 1);
                        if(success != TE_OK)
                            break;


                        success = radiotestCfgFreq(txHandle, 37500, 9375, 1);
                        if(success != TE_OK)
                            break;

                        uint16 uiFreq = 2441;
                        uint16 uiIntPA = 50;
                        uint16 uiExtPA = 0;
                        uint16 uiCC = 0;
                        uint8 uiHiside = 0;
                        uint16 uiRxAtt = 0;
                        uint16 uiSampleSize = 10000;

                        // 3. Start the transmitter and receiver
                        success = radiotestBerLoopback(txHandle, 2441, uiIntPA, uiExtPA,
                                                        uiSampleSize);
                        if(success != TE_OK)
                            break;

                        success = radiotestLoopback(rxHandle, uiFreq, uiIntPA, uiExtPA);
                        if(success != TE_OK)
                            break;

                        int32 count = 0;
                        uint32 r[9];

                        // 4. Gather the BER information
                        do
                        {
                            count++;
                            success = hqGetBer(txHandle, 1000, r);
                        }while(success == TE_ERROR && count<10);

                        if(success == TE_OK)
                        {
                            cout << "Bit count = " << r[0] << endl;
                            cout << "Bit errors = " << r[1] << endl;
                            cout << "Received packets = " << r[3] << endl;
                            cout << "Expected packets = " << r[4] << endl;
                            cout << "Header errors = " << r[5] << endl;
                            cout << "CRC errors = " << r[6] << endl;
                            cout << "Uncorrected errors = " << r[7] << endl;
                            cout << "Sync errors = " << r[8] << endl;
                        }
                        else
                            cout << "****FAIL****" << endl;

                    }while(0);

                    if(success != TE_OK)
                        cout << "TestEngine error" << endl;

                }
                if(txHandle != 0)
                    closeTestEngine(txHandle);
                if(rxHandle != 0)
                    closeTestEngine(rxHandle);

        """
        self.TestEngineDLL.radiotestCfgFreq.restype = CT.c_int32
        self.TestEngineDLL.radiotestCfgFreq.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16, CT.c_uint16]
        retval = self.TestEngineDLL.radiotestCfgFreq(handle, txrxinterval, loopback, report)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestCfgFreqMs(uint32 handle, uint16 txrxinterval, uint16 loopback, uint16 report);
    #########################################################
    # TestEngine::radiotestCfgFreqMs
    @fntrace
    def radiotestCfgFreqMs(self, handle, txrxinterval, loopback, report):
        r"""
        Function wrapper for TestEngine::radiotestCfgFreqMs().

        Python API:
            radiotestCfgFreqMs(handle, txrxinterval, loopback, report)
            returns retval

        Generated from C API:

            Function :      int32 radiotestCfgFreqMs(uint32 handle, uint16 txrxinterval,
                                                        uint16 loopback, uint16 report)

            Parameters :    handle -
                                Handle to the device

                            txrxinterval -
                                Sets the period in microseconds between TX and RX events in
                                RXDATA, TXDATA, BIT ERR and LOOP BACK test modes. Default is
                                1250 (20 slots), maximum 65536. If passed as 0, current value
                                unchanged.

                            loopback -
                                Sets the offset in microseconds between a reception event and
                                retransmission of the data in loopback. Default is 1875 (two
                                slots later), must be less than txrxinterval. If passed as
                                zero current value unchanged.

                            report -
                                Sets the time in milli-seconds between reports to host sent
                                by RXDATA and BIT ERR functions. The default is 1000mS. If
                                passed 0, the current value is unchanged. The absolute minimum
                                value for this parameter is 100mS. Attempting to set a value
                                greater than 0 and less than 100mS results in error. However,
                                depending on the speed of the transport used, a higher minimum
                                value may be required. It is recommended that for LPT SPI
                                connections, a minimum value of 800mS is used. For all other
                                transports, a 200mS minimum value is recommended. Using values
                                less than the recommended minimum may result in failures to
                                read HQ data, due to the transport not being able to read the
                                data from the chip quickly enough between reports.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Sets three values used in deciding timing details of tests, with
                            the report interval being set in milli-seconds (whereas
                            radiotestCfgFreq sets the interval in seconds). If the firmware
                            version being used does not support this function, it will
                            return error. In this case, use radiotestCfgFreq instead.
                            <p>For BIT ERR functions, reducing the reporting time
                            from the default 1000mS may improve test time for small sample
                            sizes tested with big packet sizes (i.e. if you are getting far
                            more bits in each report than are required for your sample size).

            Example :

                uint32 rxHandle = 0;
                uint32 txHandle = 0;

                cout << "Trying rxHandle..." << endl;
                rxHandle = openTestEngine(BCSP, "COM1", 115200, 5000, 0);

                cout << "Trying txHandle..." << endl;
                txHandle = openTestEngine(USB, "\\\\.\\csr0", 0, 5000, 100);

                if((txHandle != 0) && (rxHandle != 0))
                {
                    int32 success = 0;

                    do
                    {
                        // Configure the packet types
                        success = radiotestCfgPkt(rxHandle, 15, 339);
                        if(success != TE_OK)
                            break;
                        success = radiotestCfgPkt(txHandle, 15, 339);
                        if(success != TE_OK)
                            break;

                        // Set the TX/RX interval
                        success = radiotestCfgFreqMs(rxHandle, 37500, 9375, 200);
                        if(success != TE_OK)
                            break;
                        success = radiotestCfgFreqMs(txHandle, 37500, 9375, 200);
                        if(success != TE_OK)
                            break;

                        static const uint16 freq(2441);
                        static const uint16 intPA(50);
                        static const uint16 extPA(0);
                        static const uint16 sampleSize(10000);

                        // Start the transmitter and receiver
                        success = radiotestBerLoopback(txHandle, freq, intPA, extPA,
                                                        sampleSize);
                        if(success != TE_OK)
                            break;

                        success = radiotestLoopback(rxHandle, freq, intPA, extPA);
                        if(success != TE_OK)
                            break;

                        int32 count = 0;
                        uint32 r[9];

                        // Gather the BER information
                        do
                        {
                            count++;
                            success = hqGetBer(txHandle, 1000, r);
                        }while(success == TE_ERROR && count<10);

                        if(success == TE_OK)
                        {
                            cout << "Bit count = " << r[0] << endl;
                            cout << "Bit errors = " << r[1] << endl;
                            cout << "Received packets = " << r[3] << endl;
                            cout << "Expected packets = " << r[4] << endl;
                            cout << "Header errors = " << r[5] << endl;
                            cout << "CRC errors = " << r[6] << endl;
                            cout << "Uncorrected errors = " << r[7] << endl;
                            cout << "Sync errors = " << r[8] << endl;
                        }
                        else
                            cout << "****FAIL****" << endl;

                    }while(0);

                    if(success != TE_OK)
                        cout << "TestEngine error" << endl;

                }
                if(txHandle != 0)
                    closeTestEngine(txHandle);
                if(rxHandle != 0)
                    closeTestEngine(rxHandle);

        """
        self.TestEngineDLL.radiotestCfgFreqMs.restype = CT.c_int32
        self.TestEngineDLL.radiotestCfgFreqMs.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16, CT.c_uint16]
        retval = self.TestEngineDLL.radiotestCfgFreqMs(handle, txrxinterval, loopback, report)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestCfgPkt(uint32 handle, uint16 type, uint16 size);
    #########################################################
    # TestEngine::radiotestCfgPkt
    @fntrace
    def radiotestCfgPkt(self, handle, type, size):
        r"""
        Function wrapper for TestEngine::radiotestCfgPkt().

        Python API:
            radiotestCfgPkt(handle, type, size)
            returns retval

        Generated from C API:

            Function :      int32 radiotestCfgPkt(uint32 handle, uint16 type, uint16 size)

            Parameters :    handle -
                                Handle to the device

                            type -
                                A standard Bluetooth packet type from the following table:
                                    <table>
                                        <tr><td>DM1 = 3
                                        <tr><td>DH1 = 4
                                        <tr><td>HV1 = 5
                                        <tr><td>HV2 = 6
                                        <tr><td>HV3 = 7
                                        <tr><td>DV = 8
                                        <tr><td>AUX1 = 9
                                        <tr><td>DM3 = 10
                                        <tr><td>DH3 = 11
                                        <tr><td>EV4 = 12
                                        <tr><td>EV5 = 13
                                        <tr><td>DM5 = 14
                                        <tr><td>DH5 = 15
                                        <tr><td>2-DH1 = 20
                                        <tr><td>2-EV3 = 22
                                        <tr><td>3-EV3 = 23
                                        <tr><td>3-DH1 = 24
                                        <tr><td>2-DH3 = 26
                                        <tr><td>3-DH3 = 27
                                        <tr><td>2-EV5 = 28
                                        <tr><td>3-EV5 = 29
                                        <tr><td>2-DH5 = 30
                                        <tr><td>3-DH5 = 31
                                    </table>
                                <p>Any other number sets the default, which is DH1. In this
                                case the size is also set to the maximum for a DH1 packet.

                            size -
                                The size of the data payload in each packet, from 1 to the
                                maximum for the type. The maximum sizes for each type (taken
                                from the BlueTooth specification) are:
                                    <table>
                                        <tr><td>DM1 = 17
                                        <tr><td>DH1 = 27
                                        <tr><td>HV1 = 10
                                        <tr><td>HV2 = 20
                                        <tr><td>HV3 = 30
                                        <tr><td>DV = 19
                                        <tr><td>AUX1 = 29
                                        <tr><td>DM3 = 121
                                        <tr><td>DH3 = 183
                                        <tr><td>EV4 = 120
                                        <tr><td>EV5 = 180
                                        <tr><td>DM5 = 224
                                        <tr><td>DH5 = 339
                                        <tr><td>2-DH1 = 54
                                        <tr><td>2-EV3 = 60
                                        <tr><td>3-EV3 = 90
                                        <tr><td>3-DH1 = 83
                                        <tr><td>2-DH3 = 367
                                        <tr><td>3-DH3 = 552
                                        <tr><td>2-EV5 = 360
                                        <tr><td>3-EV5 = 540
                                        <tr><td>2-DH5 = 679
                                        <tr><td>3-DH5 = 1021
                                    </table>

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Sets packet type and size for transmitter tests. An invalid
                            packet type or size exceeding the maximum for the specified
                            type will result in an error.

            Example :

                uint32 rxHandle = 0;
                uint32 txHandle = 0;
                int32 timeout = 0;
                int32 hopping = 0;

                do
                {
                    cout << "Trying rxHandle..." << endl;
                    rxHandle = openTestEngine(BCSP, "COM1", 115200, 1000, 0);
                    timeout+=1000;
                }while(rxHandle == 0 && timeout<MAX_TIMEOUT);

                timeout = 0;

                do
                {
                    cout << "Trying txHandle..." << endl;
                    txHandle = openTestEngine(USB, "\\\\.\\csr0", 0, 1000, 500);
                    timeout+=1000;
                }while(txHandle == 0 && timeout<MAX_TIMEOUT);

                if((txHandle != 0) && (rxHandle != 0))
                {
                    uint16 dut, ref;
                    int32 success = 0;

                    do
                    {

                        success = bccmdGetBuildId(rxHandle, &dut);
                        if(success == TE_OK)
                            cout << "DUT firmware = " << dut << endl;
                        else
                            break;

                        success = bccmdGetBuildId(txHandle, &ref);
                        if(success == TE_OK)
                            cout << "REF firmware = " << ref << endl;
                        else
                            break;

                        // 1. configure the packet types
                        success = radiotestCfgPkt(rxHandle, 15, 339);
                        if(success != TE_OK)
                            break;

                        success = radiotestCfgPkt(txHandle, 15, 339);
                        if(success != TE_OK)
                            break;

                        // 2. Set the TX/RX interval
                        success = radiotestCfgFreq(rxHandle, 37500, 9375, 1);
                        if(success != TE_OK)
                            break;


                        success = radiotestCfgFreq(txHandle, 37500, 9375, 1);
                        if(success != TE_OK)
                            break;

                        uint16 uiFreq = 2441;
                        uint16 uiIntPA = 50;
                        uint16 uiExtPA = 0;
                        uint16 uiCC = 0;
                        uint8 uiHiside = 0;
                        uint16 uiRxAtt = 0;
                        uint16 uiSampleSize = 10000;

                        // 3. Start the transmitter and receiver
                        success = radiotestBerLoopback(txHandle, 2441, uiIntPA, uiExtPA,
                                                        uiSampleSize);
                        if(success != TE_OK)
                            break;

                        success = radiotestLoopback(rxHandle, uiFreq, uiIntPA, uiExtPA);
                        if(success != TE_OK)
                            break;

                        int32 count = 0;
                        uint32 r[9];

                        // 4. Gather the BER information
                        do
                        {
                            count++;
                            success = hqGetBer(txHandle, 1000, r);
                        }while(success == TE_ERROR && count<10);

                        if(success == TE_OK)
                        {
                            cout << "Bit count = " << r[0] << endl;
                            cout << "Bit errors = " << r[1] << endl;
                            cout << "Received packets = " << r[3] << endl;
                            cout << "Expected packets = " << r[4] << endl;
                            cout << "Header errors = " << r[5] << endl;
                            cout << "CRC errors = " << r[6] << endl;
                            cout << "Uncorrected errors = " << r[7] << endl;
                            cout << "Sync errors = " << r[8] << endl;
                        }
                        else
                            cout << "****FAIL****" << endl;

                    }while(0);

                    if(success != TE_OK)
                        cout << "TestEngine error" << endl;

                }
                if(txHandle != 0)
                    closeTestEngine(txHandle);
                if(rxHandle != 0)
                    closeTestEngine(rxHandle);

        """
        self.TestEngineDLL.radiotestCfgPkt.restype = CT.c_int32
        self.TestEngineDLL.radiotestCfgPkt.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16]
        retval = self.TestEngineDLL.radiotestCfgPkt(handle, type, size)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestCfgBitError(uint32 handle, uint32 sampleSize, uint8 reset);
    #########################################################
    # TestEngine::radiotestCfgBitError
    @fntrace
    def radiotestCfgBitError(self, handle, sampleSize, reset):
        r"""
        Function wrapper for TestEngine::radiotestCfgBitError().

        Python API:
            radiotestCfgBitError(handle, sampleSize, reset)
            returns retval

        Generated from C API:

            Function :      int32 radiotestCfgBitError(uint32 handle, uint32 sampleSize,
                                                        uint8 reset)

            Parameters :    handle -
                                Handle to the device

                            sampleSize -
                                If non-zero, the target for total counters is set to this and
                                total count resets at this value. If passed as 0 the current
                                value is unchanged.

                            reset -
                                Boolean value (0 = False, anything else = True). If True, and
                                if Ber1, Ber2, or BerLoopback is active, immediately resets
                                the counters for the total statistics, but not over the last
                                report period.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Can be used to set the sample size used in bit error measurements,
                            and reset the counters if required.
                            <p>Note that the radiotestBer1, radiotestBer2, and
                            radiotestBerLoopback functions also take a sampleSize parameter
                            and set the value before running the test. Therefore there is
                            normally no need to call this function, unless a reset of the
                            counters is required.

        """
        self.TestEngineDLL.radiotestCfgBitError.restype = CT.c_int32
        self.TestEngineDLL.radiotestCfgBitError.argtypes = [CT.c_uint32, CT.c_uint32, CT.c_uint8]
        retval = self.TestEngineDLL.radiotestCfgBitError(handle, sampleSize, reset)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestCfgTxPaAtten(uint32 handle, uint16 atten);
    #########################################################
    # TestEngine::radiotestCfgTxPaAtten
    @fntrace
    def radiotestCfgTxPaAtten(self, handle, atten):
        r"""
        Function wrapper for TestEngine::radiotestCfgTxPaAtten().

        Python API:
            radiotestCfgTxPaAtten(handle, atten)
            returns retval

        Generated from C API:

            Function :      int32 radiotestCfgTxPaAtten(uint32 handle, uint16 atten)

            Parameters :    handle -
                                Handle to the device

                            atten -
                                Attenuation value to set.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Sets the TX PA attenuation value used during transmission.

        """
        self.TestEngineDLL.radiotestCfgTxPaAtten.restype = CT.c_int32
        self.TestEngineDLL.radiotestCfgTxPaAtten.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.radiotestCfgTxPaAtten(handle, atten)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestCfgXtalFtrim(uint32 handle, uint16 ftrim);
    #########################################################
    # TestEngine::radiotestCfgXtalFtrim
    @fntrace
    def radiotestCfgXtalFtrim(self, handle, ftrim):
        r"""
        Function wrapper for TestEngine::radiotestCfgXtalFtrim().

        Python API:
            radiotestCfgXtalFtrim(handle, ftrim)
            returns retval

        Generated from C API:

            Function :      int32 radiotestCfgXtalFtrim(uint32 handle, uint16 ftrim)

            Parameters :    handle -
                                Handle to the device

                            ftrim -
                                A number between 0 and 63 inclusive. This is not a permanent
                                change.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Timing for BlueCore is controlled by a crystal. This requires
                            trimming for new hardware. This command can be used to set a new
                            trim value either before a radiotest command is started or while
                            a test is already in operation; the change takes effect
                            immediately.
                            <p>This is supported for BlueCore ICs only - for other ICs use
                            teMcSetXtalFreqTrim.

        """
        self.TestEngineDLL.radiotestCfgXtalFtrim.restype = CT.c_int32
        self.TestEngineDLL.radiotestCfgXtalFtrim.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.radiotestCfgXtalFtrim(handle, ftrim)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestCalcXtalOffset(float64 nominalFreqMhz, float64 actualFreqMhz, int16 * offset);
    #########################################################
    # TestEngine::radiotestCalcXtalOffset
    @fntrace
    def radiotestCalcXtalOffset(self, nominalFreqMhz, actualFreqMhz, _offset=0):
        r"""
        Function wrapper for TestEngine::radiotestCalcXtalOffset().

        Python API:
            radiotestCalcXtalOffset(nominalFreqMhz, actualFreqMhz, _offset=0)
            returns retval, offset.value

        Generated from C API:

            Function :      int32 radiotestCalcXtalOffset(float64 nominalFreqMhz,
                                                          float64 actualFreqMhz,
                                                          int16 * offset)

            Parameters :    nominalFreqMhz -
                                The nominal frequency in MHz.

                            actualFreqMhz -
                                The measured frequency in MHz.

                            offset -
                                Location where the resulting offset value will be stored.

            Returns :       <table>
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                            </table>

            Description :   This function will calculate the frequency offset value from the
                            given parameters. The resulting value can be written to the
                            persistent store using psWriteXtalOffset.
                            <p>The value of the PSKEY is an int16 value in parts per 2^20.
                            The calculation used is:
                            <p>((actualFreqKhz/nominalFreqKhz) - 1) * 2^20 = offset
                            <p>The resulting offset value is then rounded to the nearest
                            integer value. This value must be between -300 and 300, otherwise
                            an error is returned (the frequency offset is too large to be
                            compensated for). An error is also returned if either frequency
                            parameter is zero.
                            <p>The frequency measurement is usually taken after starting CW
                            transmission with radiotestTxstart. It is important that the
                            chip booted with an offset value of 0 (default) when taking the
                            frequency measurement for offset calibration, otherwise the
                            resulting value will be incorrect. The current offset value can
                            be read using psReadXtalOffset, and set using psWriteXtalOffset
                            (after which a reset is required for the value to take effect).
                            <p>This function can be used as part of per-device crystal
                            calibration for some BC7 and later BlueCore ICs, which use a
                            frequency offset value rather than a crystal trim value.

        """
        self.TestEngineDLL.radiotestCalcXtalOffset.restype = CT.c_int32
        self.TestEngineDLL.radiotestCalcXtalOffset.argtypes = [CT.c_double, CT.c_double, CT.c_void_p]
        offset = CT.c_int16(_offset)
        retval = self.TestEngineDLL.radiotestCalcXtalOffset(nominalFreqMhz, actualFreqMhz, CT.byref(offset))
        return retval, offset.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestCfgUapLap(uint32 handle, uint16 uap, uint32 lap);
    #########################################################
    # TestEngine::radiotestCfgUapLap
    @fntrace
    def radiotestCfgUapLap(self, handle, uap, lap):
        r"""
        Function wrapper for TestEngine::radiotestCfgUapLap().

        Python API:
            radiotestCfgUapLap(handle, uap, lap)
            returns retval

        Generated from C API:

            Function :      int32 radiotestCfgUapLap(uint32 handle, uint16 uap, uint32 lap)

            Parameters :    handle -
                                Handle to the device.

                            uap -
                                UAP portion of the device address (0 to 0xff).

                            lap -
                                LAP portion of the device address (0 to 0xffffff).

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Sets the UAP and LAP to be used in radio tests. BlueCore usually
                            uses its own Bluetooth Device address to determine the access sync
                            code, as if it is master of a piconet.
                            The UAP and LAP are the only parts used.

        """
        self.TestEngineDLL.radiotestCfgUapLap.restype = CT.c_int32
        self.TestEngineDLL.radiotestCfgUapLap.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint32]
        retval = self.TestEngineDLL.radiotestCfgUapLap(handle, uap, lap)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestCfgIqTrim(uint32 handle, uint16 trim);
    #########################################################
    # TestEngine::radiotestCfgIqTrim
    @fntrace
    def radiotestCfgIqTrim(self, handle, trim):
        r"""
        Function wrapper for TestEngine::radiotestCfgIqTrim().

        Python API:
            radiotestCfgIqTrim(handle, trim)
            returns retval

        Generated from C API:

            Function :      int32 radiotestCfgIqTrim(uint32 handle, uint16 trim)

            Parameters :    handle -
                                Handle to the device

                            trim -
                                Value to be written to the register, ANA_IQ_TRIM.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   The IQ block may be trimmed by setting a register, ANA_IQ_TRIM.
                            This allows the variable to be set for the test modes.  The value
                            "trim", 0 to 511 inclusive, takes effect immediately, is not
                            saved, and must be reset on each test, as the IQ is calibrated at
                            the start.

        """
        self.TestEngineDLL.radiotestCfgIqTrim.restype = CT.c_int32
        self.TestEngineDLL.radiotestCfgIqTrim.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.radiotestCfgIqTrim(handle, trim)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestCfgTxIf(uint32 handle, int16 offset);
    #########################################################
    # TestEngine::radiotestCfgTxIf
    @fntrace
    def radiotestCfgTxIf(self, handle, offset):
        r"""
        Function wrapper for TestEngine::radiotestCfgTxIf().

        Python API:
            radiotestCfgTxIf(handle, offset)
            returns retval

        Generated from C API:

            Function :      int32 radiotestCfgTxIf(uint32 handle, int16 offset)

            Parameters :    handle -
                                Handle to the device

                            offset -
                                Value to be written to the register, ANA_IQ_TRIM.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   The bc01 transmitter uses an additional offset in the IF part of
                            the transmitter, which needs to be configured for optimum
                            performance. The target is to be able to set this to zero.
                            However, the stack currently uses an offset of 1MHz 1.5MHz.
                            "offset_half_mhz" sets the value of this in units of 0.5MHz for
                            the radiotests. The default is the value used in standard
                            operation, currently 3 units, i.e. 1.5MHz offset.  Note this is a
                            signed integer.  The range is -5 to +5. This must be set before
                            the start of a radio transmitter test.

        """
        self.TestEngineDLL.radiotestCfgTxIf.restype = CT.c_int32
        self.TestEngineDLL.radiotestCfgTxIf.argtypes = [CT.c_uint32, CT.c_int16]
        retval = self.TestEngineDLL.radiotestCfgTxIf(handle, offset)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestCfgTxTrim(uint32 handle, uint16 am_addr);
    #########################################################
    # TestEngine::radiotestCfgTxTrim
    @fntrace
    def radiotestCfgTxTrim(self, handle, am_addr):
        r"""
        Function wrapper for TestEngine::radiotestCfgTxTrim().

        Python API:
            radiotestCfgTxTrim(handle, am_addr)
            returns retval

        Generated from C API:

            Function :      int32 radiotestCfgTxTrim(uint32 handle, uint16 am_addr)

            Parameters :    handle -
                                Handle to the device

                            am_addr -
                                Active member address.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Sets the AM_ADDR used in the header of test transmissions, and
                            expected in receiving test transmissions.  It is a number between
                            0 and 7, inclusive.  The default value is 7.  This takes effect
                            immediately and lasts until a new test is restarted, at which
                            point a default read from persistent store is used.  The defaults
                            are: PSKEY_RADIOTEST_TX_TRIM1 for PRBS data (which is assumed
                            for all reception routines), PSKEY_RADIOTEST_TX_TRIM2 for alternating
                            bits (1010...), PSKEY_RADIOTEST_TX_TRIM3 for alternating nibbles
                            (11110000).

        """
        self.TestEngineDLL.radiotestCfgTxTrim.restype = CT.c_int32
        self.TestEngineDLL.radiotestCfgTxTrim.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.radiotestCfgTxTrim(handle, am_addr)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestCfgLoLvl(uint32 handle, uint16 lo_lvl);
    #########################################################
    # TestEngine::radiotestCfgLoLvl
    @fntrace
    def radiotestCfgLoLvl(self, handle, lo_lvl):
        r"""
        Function wrapper for TestEngine::radiotestCfgLoLvl().

        Python API:
            radiotestCfgLoLvl(handle, lo_lvl)
            returns retval

        Generated from C API:

            Function :      int32 radiotestCfgLoLvl(uint32 handle, uint16 lo_lvl)

            Parameters :    handle -
                                Handle to the device

                            lo_lvl -
                                Analogue LO output level

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This command sets the value of the Analogue Local Oscillator
                            output level to lvl, over-riding the value calculated by the
                            internal calibration algorithm.
                            <p>Important Note:
                                Use of this command is unnecessary for all normal users.

        """
        self.TestEngineDLL.radiotestCfgLoLvl.restype = CT.c_int32
        self.TestEngineDLL.radiotestCfgLoLvl.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.radiotestCfgLoLvl(handle, lo_lvl)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestCfgHoppingSeq(uint32 handle, uint16 channels[]);
    #########################################################
    # TestEngine::radiotestCfgHoppingSeq
    @fntrace
    def radiotestCfgHoppingSeq(self, handle, channels):
        r"""
        Function wrapper for TestEngine::radiotestCfgHoppingSeq().

        Python API:
            radiotestCfgHoppingSeq(handle, channels)
            returns retval

        Generated from C API:

            Function :      int32 radiotestCfgHoppingSeq(uint32 handle, uint16* channels)

            Parameters :    handle -
                                Handle to the device

                            channels -
                                Pointer to an array of 5 uint16 values. The 5 values are bit
                                fields used to enable / disable each of the 79 channels. If a
                                bit = 1, the channel is used, otherwise it is not used. The
                                array elements map to channels as follows:
                                <table>
                                    <tr><td>0 = Channels 0-15
                                    <tr><td>1 = Channels 16-31
                                    <tr><td>2 = Channels 32-47
                                    <tr><td>3 = Channels 48-63
                                    <tr><td>4 = Channels 64-78 (bit 15 unused)
                                </table>
                                For each uint16 element, bit 0 (LSB) is for the lowest channel
                                number in the range, i.e. bit 0 of the first uint16 is for
                                channel 0.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Configures the set of hop frequencies to be used with
                            radiotestRxdata2, radiotestTxdata2 and radiotestBer2. The
                            functionality is analogous to the hciSetAfhHostChannelClass
                            function. The adjusted hopping sequence allows use of these
                            radiotest modes while simulating a reduced hop set.
                            <p>The hopping algorithm is the same as that used when all
                            frequencies are in use, but only on the selected frequencies.
                            This guarantees that the radio spends equal amounts of time on
                            each selected frequency.

            Example :

                uint32 handle = 0;
                int32 timeout = 0;

                do
                {
                    cout << "Attempting to open connection..." << endl;
                    handle = openTestEngine(BCSP, "COM1", 115200, 1000, 0);
                    timeout+=1000;
                }while(handle == 0 && timeout<MAX_TIMEOUT);

                if(handle != 0)
                {
                    int32 success = 0;

                    do
                    {
                        // Configure the packet type
                        success = radiotestCfgPkt(handle, 15, 339);
                        if(success != TE_OK)
                            break;

                        // Set the TX/RX interval
                        success = radiotestCfgFreq(handle, 37500, 9375, 1);
                        if(success != TE_OK)
                            break;

                        // Use lowest 20 channels only
                        uint16 channels[5] = { 0xffff, 0x000f, 0x0000, 0x0000, 0x0000 };
                        success = radiotestCfgHoppingSeq(handle, channels);
                        if(success != TE_OK)
                            break;

                        // Start transmission
                        success = radiotestTxdata2(handle, 0, 50, 0);
                        if(success != TE_OK)
                            break;

                        // Perform tests

                    }while(0);

                    if(success != TE_OK)
                        cout << "TestEngine error" << endl;

                }
                if(handle != 0)
                    closeTestEngine(handle);

        """
        self.TestEngineDLL.radiotestCfgHoppingSeq.restype = CT.c_int32
        self.TestEngineDLL.radiotestCfgHoppingSeq.argtypes = [CT.c_uint32, CT.c_void_p]
        __channels = (CT.c_uint16 * len(channels))(*channels)
        retval = self.TestEngineDLL.radiotestCfgHoppingSeq(handle, CT.byref(__channels))
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestSettle(uint32 handle, uint16 start, uint16 end);
    #########################################################
    # TestEngine::radiotestSettle
    @fntrace
    def radiotestSettle(self, handle, start, end):
        r"""
        Function wrapper for TestEngine::radiotestSettle().

        Python API:
            radiotestSettle(handle, start, end)
            returns retval

        Generated from C API:

            Function :      int32 radiotestSettle(uint32 handle, uint16 start, uint16 end)

            Parameters :    handle -
                                Handle to the device.

                            start -
                                Channel to start stepping from. 0 to 78.

                            end -
                                Channel to finish stepping at. 0 to 78.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Builds the radio's channel LO_TRIM frequency lookup table (LUT),
                            then does a step from the start channel to the end channel, while
                            the synthesiser is running. It digitises the synthesiser (LO_TUNE)
                            error voltage at intervals of 10 | 20us over the next 200us.
                            <p>The results can be retrieved using hqGetSettle.
                            <p>This function is only supported over a SPI connection if HQ
                            over SPI is supported by the device firmware.

            Deprecated :

        """
        self.TestEngineDLL.radiotestSettle.restype = CT.c_int32
        self.TestEngineDLL.radiotestSettle.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16]
        retval = self.TestEngineDLL.radiotestSettle(handle, start, end)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hqGetSettle(uint32 handle, uint16 r[]);
    #########################################################
    # TestEngine::hqGetSettle
    @fntrace
    def hqGetSettle(self, handle, r):
        r"""
        Function wrapper for TestEngine::hqGetSettle().

        Python API:
            hqGetSettle(handle, r)
            returns retval

        Generated from C API:

            Function :      int32 hqGetSettle(uint32 handle, uint16 * r)

            Parameters :    handle -
                                Handle to the device.

                            r -
                                Pointer to a pre-allocated 9 element array to receive the
                                results.

            Returns :        <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to retrieve the results made from a call to
                            radiotestSettle.

            Deprecated :

        """
        self.TestEngineDLL.hqGetSettle.restype = CT.c_int32
        self.TestEngineDLL.hqGetSettle.argtypes = [CT.c_uint32, CT.c_void_p]
        __r = (CT.c_uint16 * len(r))(*r)
        retval = self.TestEngineDLL.hqGetSettle(handle, CT.byref(__r))
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) get_freq_offset(uint32 handle, float64 * offset, int32 sample_size);
    #########################################################
    # TestEngine::get_freq_offset
    @fntrace
    def get_freq_offset(self, handle, sample_size, _offset=0):
        r"""
        Function wrapper for TestEngine::get_freq_offset().

        Python API:
            get_freq_offset(handle, sample_size, _offset=0)
            returns retval, offset.value

        Generated from C API:

            Function :      int32 get_freq_offset(uint32 handle, float64 * offset,
                                                    int32 sample_size)

            Parameters :    handle -
                                Handle to the device

                            offset -
                                Pointer to the offset. Measured in parts per million (PPM),
                                this value is an average of sample_size readings from the
                                BlueCore receiver.

                            sample_size -
                                Determines how many readings of offset are averaged. The
                                software will only accept values in the range of 20 to 200,
                                any values outside this range will be forced up to 20 or 200.
                                The choice of value will determine the accuracy and speed.
                                The lower the value the quicker but less accurate the results,
                                the higher the value the more accurate but slower the function.
                                The optimal value is 60 to 100.

            Returns :       -1 on error, 1 on success, 0 on fail

            Description :   This command will read the frequency offset of a received signal.

        """
        self.TestEngineDLL.get_freq_offset.restype = CT.c_int32
        self.TestEngineDLL.get_freq_offset.argtypes = [CT.c_uint32, CT.c_void_p, CT.c_int32]
        offset = CT.c_float64(_offset)
        retval = self.TestEngineDLL.get_freq_offset(handle, CT.byref(offset), sample_size)
        return retval, offset.value
    #########################################################
    PS_STORES_FR = (0x0006)
    PS_STORES_IF = (0x0003)
    PS_STORES_IFR = (0x0007)
    PS_STORES_IFAR = (0x0017)

    #########################################################
    # TESTENGINE_API(int32) bccmdSetEeprom(uint32 handle, uint16 log2bytes, uint16 addrMask);
    #########################################################
    # TestEngine::bccmdSetEeprom
    @fntrace
    def bccmdSetEeprom(self, handle, log2bytes, addrMask):
        r"""
        Function wrapper for TestEngine::bccmdSetEeprom().

        Python API:
            bccmdSetEeprom(handle, log2bytes, addrMask)
            returns retval

        Generated from C API:

            Function :      int32 bccmdSetEeprom(uint32 handle, uint16 log2bytes,
                                                uint16 addrMask)

            Parameters :    handle -
                                Handle to the device

                            log2bytes -
                                Specifies the size of the EEPROM (log2 bytes). That is the
                                number of bits required to address any byte of the memory
                                array.

                            addrMask -
                                A bitmap specifying which, if any, of the slave address bits
                                are used to specify high-order bits of the array address.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Writes the header to the I2C EEPROM to allow the device to be used
                            to store PS values and VM applications.

        """
        self.TestEngineDLL.bccmdSetEeprom.restype = CT.c_int32
        self.TestEngineDLL.bccmdSetEeprom.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16]
        retval = self.TestEngineDLL.bccmdSetEeprom(handle, log2bytes, addrMask)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psReadBdAddr(uint32 handle, uint32 * lap, uint8 * uap, uint16 * nap);
    #########################################################
    # TestEngine::psReadBdAddr
    @fntrace
    def psReadBdAddr(self, handle, _lap=0, _uap=0, _nap=0):
        r"""
        Function wrapper for TestEngine::psReadBdAddr().

        Python API:
            psReadBdAddr(handle, _lap=0, _uap=0, _nap=0)
            returns retval, lap.value, uap.value, nap.value

        Generated from C API:

            Function :      int32 psReadBdAddr(uint32 handle, uint32 * lap, uint8 * uap,
                                                uint16 * nap)

            Parameters :    handle -
                                Handle to the device

                            lap -
                                Pointer to a uint32 to hold the LAP portion of the BD_ADDR

                            uap -
                                Pointer to a uint8 to hold the UAP portion of the BD_ADDR

                            nap -
                                Pointer to a uint16 to hold the NAP portion of the BD_ADDR

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Function to read the BD_ADDR from the persistent store.

        """
        self.TestEngineDLL.psReadBdAddr.restype = CT.c_int32
        self.TestEngineDLL.psReadBdAddr.argtypes = [CT.c_uint32, CT.c_void_p, CT.c_void_p, CT.c_void_p]
        lap = CT.c_uint32(_lap)
        uap = CT.c_uint8(_uap)
        nap = CT.c_uint16(_nap)
        retval = self.TestEngineDLL.psReadBdAddr(handle, CT.byref(lap), CT.byref(uap), CT.byref(nap))
        return retval, lap.value, uap.value, nap.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psRead(uint32 handle, uint16 psKey, uint16 store, uint16 arrayLen, uint16 data[], uint16 * len);
    #########################################################
    # TestEngine::psRead
    @fntrace
    def psRead(self, handle, psKey, store, arrayLen, data, _len=0):
        r"""
        Function wrapper for TestEngine::psRead().

        Python API:
            psRead(handle, psKey, store, arrayLen, data, _len=0)
            returns retval, len.value

        Generated from C API:

            Function :      int32 psRead(uint32 handle, uint16 psKey, uint16 store,
                                            uint16 arrayLen, uint16 * data, uint16 * len)

            Parameters :    handle -
                                Handle to the device

                            psKey -
                                PS key ID of the value to be read.

                            store -
                                Bit mask to identify the PS store layer to write to. See
                                comments above under PS_STORES.

                            arrayLen -
                                Size of the array allocated to hold the PS value

                            data -
                                Array of 16-bit words of size "arraylen" to hold the PS value

                            len -
                                Pointer to a 16-bit word to hold to length of the PS key

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Used to read the value of the PS key specified

        """
        self.TestEngineDLL.psRead.restype = CT.c_int32
        self.TestEngineDLL.psRead.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16, CT.c_uint16, CT.c_void_p, CT.c_void_p]
        __data = (CT.c_uint16 * len(data))(*data)
        len = CT.c_uint16(_len)
        retval = self.TestEngineDLL.psRead(handle, psKey, store, arrayLen, CT.byref(__data), CT.byref(len))
        return retval, len.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psClear(uint32 handle, uint16 psKey, uint16 store);
    #########################################################
    # TestEngine::psClear
    @fntrace
    def psClear(self, handle, psKey, store):
        r"""
        Function wrapper for TestEngine::psClear().

        Python API:
            psClear(handle, psKey, store)
            returns retval

        Generated from C API:

            Function :      int32 psClear(uint32 handle, uint16 psKey, uint16 store)

            Parameters :    handle -
                                Handle to the device

                            psKey -
                                PS key value of the entry to be removed

                            store -
                                Bit mask to identify the PS store layer(s) from which the PS
                                entry will be erased.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function removes from the ps the value stored in the layer
                            specified by the stores parameter. Clearing values from the ps
                            can have subtle effects. The description of the Layered Model of
                            the ps stores above shows how clearing a value from the
                            PS_STORES_I layer can (re)expose a value in the PS_STORES_F or
                            PS_STORES_R layer.

        """
        self.TestEngineDLL.psClear.restype = CT.c_int32
        self.TestEngineDLL.psClear.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16]
        retval = self.TestEngineDLL.psClear(handle, psKey, store)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psClearAll(uint32 handle, uint16 store);
    #########################################################
    # TestEngine::psClearAll
    @fntrace
    def psClearAll(self, handle, store):
        r"""
        Function wrapper for TestEngine::psClearAll().

        Python API:
            psClearAll(handle, store)
            returns retval

        Generated from C API:

            Function :      int32 psClearAll(uint32 handle, uint16 store)

            Parameters :    handle -
                                Handle to the device

                            store -
                                Bit mask to identify the PS store layer(s) from which the PS
                                entries will be erased.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This removes all values from the stores specified; only values in
                            the psrom layer remain. This command should be used with caution.

        """
        self.TestEngineDLL.psClearAll.restype = CT.c_int32
        self.TestEngineDLL.psClearAll.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.psClearAll(handle, store)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psFactorySet(uint32 handle);
    #########################################################
    # TestEngine::psFactorySet
    @fntrace
    def psFactorySet(self, handle):
        r"""
        Function wrapper for TestEngine::psFactorySet().

        Python API:
            psFactorySet(handle)
            returns retval

        Generated from C API:

            Function :      int32 psFactorySet(uint32 handle)

            Parameters :    handle -
                                Handle to the device

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This moves all values in the PS_STORES_I layer to the PS_STORES_F
                            layer.

        """
        self.TestEngineDLL.psFactorySet.restype = CT.c_int32
        self.TestEngineDLL.psFactorySet.argtypes = [CT.c_uint32]
        retval = self.TestEngineDLL.psFactorySet(handle)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psFactoryRestoreAll(uint32 handle);
    #########################################################
    # TestEngine::psFactoryRestoreAll
    @fntrace
    def psFactoryRestoreAll(self, handle):
        r"""
        Function wrapper for TestEngine::psFactoryRestoreAll().

        Python API:
            psFactoryRestoreAll(handle)
            returns retval

        Generated from C API:

            Function :      int32 psFactoryRestoreAll(uint32 handle)

            Parameters :    handle -
                                Handle to the device

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This command removes all values from the PS_STORES_I store, so
                            restoring the chip?s ps to the state it was in when the
                            PS_FACTORY_SET command was invoked.

        """
        self.TestEngineDLL.psFactoryRestoreAll.restype = CT.c_int32
        self.TestEngineDLL.psFactoryRestoreAll.argtypes = [CT.c_uint32]
        retval = self.TestEngineDLL.psFactoryRestoreAll(handle)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psFactoryRestore(uint32 handle);
    #########################################################
    # TestEngine::psFactoryRestore
    @fntrace
    def psFactoryRestore(self, handle):
        r"""
        Function wrapper for TestEngine::psFactoryRestore().

        Python API:
            psFactoryRestore(handle)
            returns retval

        Generated from C API:

            Function :      int32 psFactoryRestore(uint32 handle)

            Parameters :    handle -
                                Handle to the device

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This is identical to PS_FACTORY_RESTORE_ALL except that values in
                            the PS_STORES_I layer that do not obscure values in the
                            PS_STORES_F and/or PS_STORES_R layers are left in place.

        """
        self.TestEngineDLL.psFactoryRestore.restype = CT.c_int32
        self.TestEngineDLL.psFactoryRestore.argtypes = [CT.c_uint32]
        retval = self.TestEngineDLL.psFactoryRestore(handle)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psSize(uint32 handle, uint16 psKey, uint16 store, uint16 * size);
    #########################################################
    # TestEngine::psSize
    @fntrace
    def psSize(self, handle, psKey, store, _size=0):
        r"""
        Function wrapper for TestEngine::psSize().

        Python API:
            psSize(handle, psKey, store, _size=0)
            returns retval, size.value

        Generated from C API:

            Function :      int32 psSize(uint32 handle, uint16 psKey, uint16 store,
                                            uint16 * size)

            Parameters :    handle -
                                Handle to the device

                            psKey -
                                PS entry to return the size of.

                            store -
                                PS store to access

                            size -
                                Reference to size of PS entry.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Returns the internal amount of data, in uint16's, held by a
                            particular PS entry. Failure may indicate that the store mask
                            selected does not contain a valid PS entry.

        """
        self.TestEngineDLL.psSize.restype = CT.c_int32
        self.TestEngineDLL.psSize.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16, CT.c_void_p]
        size = CT.c_uint16(_size)
        retval = self.TestEngineDLL.psSize(handle, psKey, store, CT.byref(size))
        return retval, size.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psWrite(uint32 handle, uint16 psKey, uint16 store, uint16 length, const uint16 data[]);
    #########################################################
    # TestEngine::psWrite
    @fntrace
    def psWrite(self, handle, psKey, store, length, data):
        r"""
        Function wrapper for TestEngine::psWrite().

        Python API:
            psWrite(handle, psKey, store, length, data)
            returns retval

        Generated from C API:

            Function :      int32 psWrite(uint32 handle, uint16 psKey, uint16 store,
                                        uint16 length, const uint16 * data)

            Parameters :    handle -
                                Handle to the device

                            psKey -
                                PS key value of the entry to be written

                            store -
                                PS store to access

                            length -
                                Length of the data to be written.

                            data -
                                Pointer to an array to hold the key contents

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Writes the PS setting "psKey" array "data" of length "length"
                            to the persistent store layer identified by "store".

        """
        self.TestEngineDLL.psWrite.restype = CT.c_int32
        self.TestEngineDLL.psWrite.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16, CT.c_uint16, CT.c_void_p]
        __data = (CT.c_uint16 * len(data))(*data)
        retval = self.TestEngineDLL.psWrite(handle, psKey, store, length, CT.byref(__data))
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psWriteVerify(uint32 handle, uint16 psKey, uint16 store, uint16 length, const uint16 data[]);
    #########################################################
    # TestEngine::psWriteVerify
    @fntrace
    def psWriteVerify(self, handle, psKey, store, length, data):
        r"""
        Function wrapper for TestEngine::psWriteVerify().

        Python API:
            psWriteVerify(handle, psKey, store, length, data)
            returns retval

        Generated from C API:

            Function :      int32 psWriteVerify(uint32 handle, uint16 psKey, uint16 store,
                                                uint16 length, const uint16 * data)

            Parameters :    handle -
                                Handle to the device

                            psKey -
                                PS key value of the entry to be written

                            store -
                                PS store to access

                            length -
                                Length of the data to be written.

                            data -
                                Pointer to an array to hold the key contents

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Writes the PS setting "psKey" array "data" of length "length"
                            to the persistent store layer PS_STORES_I or PS_STORES_F if one
                            those values are identified by "store". The function will then
                            perform a PS read and verify that the PS key has been written
                            correctly.
                            <p>This function will fail if a store mask does not contain
                            PS_STORES_I or PS_STORES_F.

        """
        self.TestEngineDLL.psWriteVerify.restype = CT.c_int32
        self.TestEngineDLL.psWriteVerify.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16, CT.c_uint16, CT.c_void_p]
        __data = (CT.c_uint16 * len(data))(*data)
        retval = self.TestEngineDLL.psWriteVerify(handle, psKey, store, length, CT.byref(__data))
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psWriteBdAddr(uint32 handle, uint32 lap, uint32 uap, uint32 nap);
    #########################################################
    # TestEngine::psWriteBdAddr
    @fntrace
    def psWriteBdAddr(self, handle, lap, uap, nap):
        r"""
        Function wrapper for TestEngine::psWriteBdAddr().

        Python API:
            psWriteBdAddr(handle, lap, uap, nap)
            returns retval

        Generated from C API:

            Function :      int32 psWriteBdAddr(uint32 handle, uint32 lap, uint32 uap,
                                                uint32 nap)

            Parameters :    handle -
                                Handle to the device.

                            lap -
                                LAP portion of the address to write to the persistent store.

                            uap -
                                UAP portion of the address to write to the persistent store.

                            nap -
                                NAP portion of the address to write to the persistent store.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to write the BlueTooth address of the
                            device.

        """
        self.TestEngineDLL.psWriteBdAddr.restype = CT.c_int32
        self.TestEngineDLL.psWriteBdAddr.argtypes = [CT.c_uint32, CT.c_uint32, CT.c_uint32, CT.c_uint32]
        retval = self.TestEngineDLL.psWriteBdAddr(handle, lap, uap, nap)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psReadModuleId(uint32 handle, uint32 * moduleId);
    #########################################################
    # TestEngine::psReadModuleId
    @fntrace
    def psReadModuleId(self, handle, _moduleId=0):
        r"""
        Function wrapper for TestEngine::psReadModuleId().

        Python API:
            psReadModuleId(handle, _moduleId=0)
            returns retval, moduleId.value

        Generated from C API:

            Function :      int32 psReadModuleId(uint32 handle, uint32 * moduleId)

            Parameters :    handle -
                                Handle to the device

                            moduleId -
                                Pointer to a variable that will hold the returned module ID.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will attempt to read the module ID stored in
                            BlueCore. The default for this value is 0 (zero)

        """
        self.TestEngineDLL.psReadModuleId.restype = CT.c_int32
        self.TestEngineDLL.psReadModuleId.argtypes = [CT.c_uint32, CT.c_void_p]
        moduleId = CT.c_uint32(_moduleId)
        retval = self.TestEngineDLL.psReadModuleId(handle, CT.byref(moduleId))
        return retval, moduleId.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psReadXtalFtrim(uint32 handle, uint16 * fTrim);
    #########################################################
    # TestEngine::psReadXtalFtrim
    @fntrace
    def psReadXtalFtrim(self, handle, _fTrim=0):
        r"""
        Function wrapper for TestEngine::psReadXtalFtrim().

        Python API:
            psReadXtalFtrim(handle, _fTrim=0)
            returns retval, fTrim.value

        Generated from C API:

            Function :      int32 psReadXtalFtrim(uint32 handle, uint16 * fTrim)

            Parameters :    handle -
                                Handle to the device

                            fTrim -
                                Pointer to a variable that will hold the returned xtal ftrim.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will attempt to read the xtal ftrim from the
                            persistent store.
                            <p>For BC7 and later BlueCore ICs, use psReadXtalOffset to read
                            the frequency offset value instead.

        """
        self.TestEngineDLL.psReadXtalFtrim.restype = CT.c_int32
        self.TestEngineDLL.psReadXtalFtrim.argtypes = [CT.c_uint32, CT.c_void_p]
        fTrim = CT.c_uint16(_fTrim)
        retval = self.TestEngineDLL.psReadXtalFtrim(handle, CT.byref(fTrim))
        return retval, fTrim.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psWriteXtalFtrim(uint32 handle, uint16 fTrim);
    #########################################################
    # TestEngine::psWriteXtalFtrim
    @fntrace
    def psWriteXtalFtrim(self, handle, fTrim):
        r"""
        Function wrapper for TestEngine::psWriteXtalFtrim().

        Python API:
            psWriteXtalFtrim(handle, fTrim)
            returns retval

        Generated from C API:

            Function :      int32 psWriteXtalFtrim(uint32 handle, uint16 fTrim)

            Parameters :    handle -
                                Handle to the device

                            fTrim -
                                Crystal frequency trim

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will write the crystal trim value to the persistent
                            store.
                            <p>For BC7 and later BlueCore ICs, use psWriteXtalOffset to write
                            a frequency offset value rather than a crystal trim value.

        """
        self.TestEngineDLL.psWriteXtalFtrim.restype = CT.c_int32
        self.TestEngineDLL.psWriteXtalFtrim.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.psWriteXtalFtrim(handle, fTrim)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psReadXtalOffset(uint32 handle, int16 * offset);
    #########################################################
    # TestEngine::psReadXtalOffset
    @fntrace
    def psReadXtalOffset(self, handle, _offset=0):
        r"""
        Function wrapper for TestEngine::psReadXtalOffset().

        Python API:
            psReadXtalOffset(handle, _offset=0)
            returns retval, offset.value

        Generated from C API:

            Function :      int32 psReadXtalOffset(uint32 handle, int16 * offset)

            Parameters :    handle -
                                Handle to the device

                            offset -
                                Pointer to a variable that will hold the returned xtal
                                offset value. The value is in parts per 2^20.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will attempt to read the crystal offset from the
                            persistent store.
                            <p>This function is for use with some BC7 and later BlueCore ICs
                            which use a frequency offset value for crystal trimming. For
                            earlier ICs, use psReadXtalFtrim to read the crystal trim value
                            instead.

        """
        self.TestEngineDLL.psReadXtalOffset.restype = CT.c_int32
        self.TestEngineDLL.psReadXtalOffset.argtypes = [CT.c_uint32, CT.c_void_p]
        offset = CT.c_int16(_offset)
        retval = self.TestEngineDLL.psReadXtalOffset(handle, CT.byref(offset))
        return retval, offset.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psWriteXtalOffset(uint32 handle, int16 offset);
    #########################################################
    # TestEngine::psWriteXtalOffset
    @fntrace
    def psWriteXtalOffset(self, handle, offset):
        r"""
        Function wrapper for TestEngine::psWriteXtalOffset().

        Python API:
            psWriteXtalOffset(handle, offset)
            returns retval

        Generated from C API:

            Function :      int32 psWriteXtalOffset(uint32 handle, int16 offset)

            Parameters :    handle -
                                Handle to the device

                            offset -
                                The offset value to write (can be calculated using
                                radiotestCalcXtalOffset).

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will write the crystal frequency offset value to
                            the persistent store.
                            <p>The value of the PSKEY is an int16 value in parts per 2^20,
                            and can be calculated using radiotestCalcXtalOffset. See the
                            documentation for radiotestCalcXtalOffset for details of the
                            calculation used and value range.
                            <p>This function is for use with some BC7 and later BlueCore ICs
                            which use a frequency offset value for crystal trimming. For
                            earlier ICs, use psWriteXtalFtrim to write a crystal trim value
                            instead.

        """
        self.TestEngineDLL.psWriteXtalOffset.restype = CT.c_int32
        self.TestEngineDLL.psWriteXtalOffset.argtypes = [CT.c_uint32, CT.c_int16]
        retval = self.TestEngineDLL.psWriteXtalOffset(handle, offset)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psWriteModuleSecurityCode(uint32 handle, const uint16 code[]);
    #########################################################
    # TestEngine::psWriteModuleSecurityCode
    @fntrace
    def psWriteModuleSecurityCode(self, handle, code):
        r"""
        Function wrapper for TestEngine::psWriteModuleSecurityCode().

        Python API:
            psWriteModuleSecurityCode(handle, code)
            returns retval

        Generated from C API:

            Function :      int32 psWriteModuleSecurityCode(uint32 handle, const uint16 * code)

            Parameters :    handle -
                                Handle to the device

                            code -
                                Pointer to 8 member unsigned 16 bit integer array holding the
                                128 bit module security code.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will write the module security code to the
                            persistent store.

        """
        self.TestEngineDLL.psWriteModuleSecurityCode.restype = CT.c_int32
        self.TestEngineDLL.psWriteModuleSecurityCode.argtypes = [CT.c_uint32, CT.c_void_p]
        __code = (CT.c_uint16 * len(code))(*code)
        retval = self.TestEngineDLL.psWriteModuleSecurityCode(handle, CT.byref(__code))
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psWriteModuleId(uint32 handle, uint32 moduleId);
    #########################################################
    # TestEngine::psWriteModuleId
    @fntrace
    def psWriteModuleId(self, handle, moduleId):
        r"""
        Function wrapper for TestEngine::psWriteModuleId().

        Python API:
            psWriteModuleId(handle, moduleId)
            returns retval

        Generated from C API:

            Function :      int32 psWriteModuleId(uint32 handle, uint32 moduleId)

            Parameters :    handle -
                                Handle to the device

                            moduleId -
                                Module ID to be stored in BlueCore persistent store

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will attempt to write the module ID to BlueCore.
                            The default for this value is 0 (zero).

        """
        self.TestEngineDLL.psWriteModuleId.restype = CT.c_int32
        self.TestEngineDLL.psWriteModuleId.argtypes = [CT.c_uint32, CT.c_uint32]
        retval = self.TestEngineDLL.psWriteModuleId(handle, moduleId)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psWriteBaudrate(uint32 handle, uint16 value);
    #########################################################
    # TestEngine::psWriteBaudrate
    @fntrace
    def psWriteBaudrate(self, handle, value):
        r"""
        Function wrapper for TestEngine::psWriteBaudrate().

        Python API:
            psWriteBaudrate(handle, value)
            returns retval

        Generated from C API:

            Function :      int32 psWriteBaudrate(uint32 handle, uint16 value)

            Parameters :    handle -
                                Handle to the device

                            value -
                                Used determine the baudrate where:
                                    baudrate = value/0.004096

                                <p>Some common values are:
                                <table>
                                    <tr><td> 38k4    baud    -   157   (0x009d)
                                    <tr><td> 57k6    baud    -   236   (0x00ec)
                                    <tr><td> 115k2   baud    -   472   (0x01d8)
                                    <tr><td> 230k4   baud    -   944   (0x03b0)
                                    <tr><td> 460k8   baud    -   1887  (0x075f)
                                    <tr><td> 921k6   baud    -   3775  (0x0ebf)
                                    <tr><td> 1382k4  baud    -   5662  (0x161e)
                                </table>
                                The maximum rated speed for the UART hardware is 1.5 Mbaud,
                                although this key can be set to a higher value.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will write a value corresponding to the baudrate to
                            the persistent store. This function can only be used in builds
                            later than 18.x. For earlier builds the
                            PSKEY_HOSTIO_UART_PS_BLOCK should be modified.

        """
        self.TestEngineDLL.psWriteBaudrate.restype = CT.c_int32
        self.TestEngineDLL.psWriteBaudrate.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.psWriteBaudrate(handle, value)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psWriteRadiotestFirstTrimTime(uint32 handle, uint32 time);
    #########################################################
    # TestEngine::psWriteRadiotestFirstTrimTime
    @fntrace
    def psWriteRadiotestFirstTrimTime(self, handle, time):
        r"""
        Function wrapper for TestEngine::psWriteRadiotestFirstTrimTime().

        Python API:
            psWriteRadiotestFirstTrimTime(handle, time)
            returns retval

        Generated from C API:

            Function :      int32 psWriteRadiotestFirstTrimTime(uint32 handle, uint32 time)

            Parameters :    handle -
                                Handle to the device

                            time -
                                The time from the start of a test till the first IQ trim
                                auto-calibration is done.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will write the time to the persistent store. A value
                            of 0 disables the automatic calibration completely during the
                            test. A calibration is still done before the test starts.

        """
        self.TestEngineDLL.psWriteRadiotestFirstTrimTime.restype = CT.c_int32
        self.TestEngineDLL.psWriteRadiotestFirstTrimTime.argtypes = [CT.c_uint32, CT.c_uint32]
        retval = self.TestEngineDLL.psWriteRadiotestFirstTrimTime(handle, time)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psReadRadiotestFirstTrimTime(uint32 handle, uint32 * time);
    #########################################################
    # TestEngine::psReadRadiotestFirstTrimTime
    @fntrace
    def psReadRadiotestFirstTrimTime(self, handle, _time=0):
        r"""
        Function wrapper for TestEngine::psReadRadiotestFirstTrimTime().

        Python API:
            psReadRadiotestFirstTrimTime(handle, _time=0)
            returns retval, time.value

        Generated from C API:

            Function :      int32 psReadRadiotestFirstTrimTime(uint32 handle, uint32 * time)

            Parameters :    handle -
                                Handle to the device

                            time -
                                The time from the start of a test till the first IQ trim
                                auto-calibration is done.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will read the trim time from the persistent store.

        """
        self.TestEngineDLL.psReadRadiotestFirstTrimTime.restype = CT.c_int32
        self.TestEngineDLL.psReadRadiotestFirstTrimTime.argtypes = [CT.c_uint32, CT.c_void_p]
        time = CT.c_uint32(_time)
        retval = self.TestEngineDLL.psReadRadiotestFirstTrimTime(handle, CT.byref(time))
        return retval, time.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psWriteRadiotestLoLvlTrimEnable(uint32 handle, uint16 state);
    #########################################################
    # TestEngine::psWriteRadiotestLoLvlTrimEnable
    @fntrace
    def psWriteRadiotestLoLvlTrimEnable(self, handle, state):
        r"""
        Function wrapper for TestEngine::psWriteRadiotestLoLvlTrimEnable().

        Python API:
            psWriteRadiotestLoLvlTrimEnable(handle, state)
            returns retval

        Generated from C API:

            Function :      int32 psWriteRadiotestLoLvlTrimEnable(uint32 handle, uint16 state)

            Parameters :    handle -
                                Handle to the device

                            state -
                                Whether IQ trim is enabled. Basically this is a boolean
                                where 1 = true and 0 = false.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function enables trimming of the value for the register
                            ANA_LO_ENABLE; otherwise, has no effect. This trimming takes place
                            before the start of each test.

        """
        self.TestEngineDLL.psWriteRadiotestLoLvlTrimEnable.restype = CT.c_int32
        self.TestEngineDLL.psWriteRadiotestLoLvlTrimEnable.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.psWriteRadiotestLoLvlTrimEnable(handle, state)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psReadRadiotestLoLvlTrimEnable(uint32 handle, uint16 * state);
    #########################################################
    # TestEngine::psReadRadiotestLoLvlTrimEnable
    @fntrace
    def psReadRadiotestLoLvlTrimEnable(self, handle, _state=0):
        r"""
        Function wrapper for TestEngine::psReadRadiotestLoLvlTrimEnable().

        Python API:
            psReadRadiotestLoLvlTrimEnable(handle, _state=0)
            returns retval, state.value

        Generated from C API:

            Function :      int32 psReadRadiotestLoLvlTrimEnable(uint32 handle, uint16 * state)

            Parameters :    handle -
                                Handle to the device

                            state -
                                Whether IQ trim is enabled. Basically this is a boolean
                                where 1 = true and 0 = false.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Reads value of PS setting

        """
        self.TestEngineDLL.psReadRadiotestLoLvlTrimEnable.restype = CT.c_int32
        self.TestEngineDLL.psReadRadiotestLoLvlTrimEnable.argtypes = [CT.c_uint32, CT.c_void_p]
        state = CT.c_uint16(_state)
        retval = self.TestEngineDLL.psReadRadiotestLoLvlTrimEnable(handle, CT.byref(state))
        return retval, state.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psWriteRadiotestSubsequentTrimTime(uint32 handle, uint32 time);
    #########################################################
    # TestEngine::psWriteRadiotestSubsequentTrimTime
    @fntrace
    def psWriteRadiotestSubsequentTrimTime(self, handle, time):
        r"""
        Function wrapper for TestEngine::psWriteRadiotestSubsequentTrimTime().

        Python API:
            psWriteRadiotestSubsequentTrimTime(handle, time)
            returns retval

        Generated from C API:

            Function :      int32 psWriteRadiotestSubsequentTrimTime(uint32 handle, uint32 time)

            Parameters :    handle -
                                Handle to the device

                            time -
                                The time between each IQ trim auto-calibration.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Once the first IQ trim auto calibration is done in test mode, then
                            subsequent trims are done at regular intervals with the period
                            being given by this key. If this key is 0 then only the first trim
                            is done. If PSKEY_RADIOTEST_FIRST_TRIM_TIME is 0 then no trims are
                            done during the test regardless of the setting of this key.

        """
        self.TestEngineDLL.psWriteRadiotestSubsequentTrimTime.restype = CT.c_int32
        self.TestEngineDLL.psWriteRadiotestSubsequentTrimTime.argtypes = [CT.c_uint32, CT.c_uint32]
        retval = self.TestEngineDLL.psWriteRadiotestSubsequentTrimTime(handle, time)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psReadRadiotestSubsequentTrimTime(uint32 handle, uint32 * time);
    #########################################################
    # TestEngine::psReadRadiotestSubsequentTrimTime
    @fntrace
    def psReadRadiotestSubsequentTrimTime(self, handle, _time=0):
        r"""
        Function wrapper for TestEngine::psReadRadiotestSubsequentTrimTime().

        Python API:
            psReadRadiotestSubsequentTrimTime(handle, _time=0)
            returns retval, time.value

        Generated from C API:

            Function :      int32 psReadRadiotestSubsequentTrimTime(uint32 handle,
                                                                    uint32 * time)

            Parameters :    handle -
                                Handle to the device

                            time -
                                The time between each IQ trim auto-calibration.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Read the PS setting

        """
        self.TestEngineDLL.psReadRadiotestSubsequentTrimTime.restype = CT.c_int32
        self.TestEngineDLL.psReadRadiotestSubsequentTrimTime.argtypes = [CT.c_uint32, CT.c_void_p]
        time = CT.c_uint32(_time)
        retval = self.TestEngineDLL.psReadRadiotestSubsequentTrimTime(handle, CT.byref(time))
        return retval, time.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psWriteHostInterface(uint32 handle, uint16 transport);
    #########################################################
    # TestEngine::psWriteHostInterface
    @fntrace
    def psWriteHostInterface(self, handle, transport):
        r"""
        Function wrapper for TestEngine::psWriteHostInterface().

        Python API:
            psWriteHostInterface(handle, transport)
            returns retval

        Generated from C API:

            Function :      int32 psWriteHostInterface(uint32 handle, uint16 transport)

            Parameters :    handle -
                                Handle to the device

                            transport -
                                The choice of physical bus. Valid values are:
                                <table>
                                    <tr><td> 1 : BCSP
                                    <tr><td> 2 : H2 - USB
                                    <tr><td> 3 : H4 - UART
                                    <tr><td> 4 : Raw UART (VM)
                                    <tr><td> 6 : H5
                                    <tr><td> 7 : H4DS
                                </table>

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Select the interface type that is to be used to pass information
                            to and from the host. Only one of these can be used at a time,
                            not least because the USB interface uses some of the same chip
                            pins as the UART. If this key specifies use of USB and the chip
                            has no USB hardware support then it reverts to using BCSP. Much
                            of the chip's support for BCSP/H5 is performed by hardware - this
                            allows interrupt routines to deal with complete BCSP/H5 packets.
                            When the chip uses its UART in a more naive fashion, e.g., for
                            H4, this support must be disabled. The UART's hardware support
                            for BCSP/H5 must be enabled via PSKEY_UART_CONFIG_BCSP/H5. The H4
                            specification requires the use of UART hardware flow control and
                            no use of parity. These must also be set via PSKEY_UART_CONFIG_H4.

        """
        self.TestEngineDLL.psWriteHostInterface.restype = CT.c_int32
        self.TestEngineDLL.psWriteHostInterface.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.psWriteHostInterface(handle, transport)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psReadHostInterface(uint32 handle, uint16 * transport);
    #########################################################
    # TestEngine::psReadHostInterface
    @fntrace
    def psReadHostInterface(self, handle, _transport=0):
        r"""
        Function wrapper for TestEngine::psReadHostInterface().

        Python API:
            psReadHostInterface(handle, _transport=0)
            returns retval, transport.value

        Generated from C API:

            Function :      int32 psReadHostInterface(uint32 handle, uint16 * transport)

            Parameters :    handle -
                                Handle to the device

                            transport -
                                The selected physical bus. Valid values are:
                                <table>
                                    <tr><td> 1 : BCSP
                                    <tr><td> 2 : H2 - USB
                                    <tr><td> 3 : H4 - UART
                                    <tr><td> 4 : Raw UART (VM)
                                    <tr><td> 6 : H5
                                    <tr><td> 7 : H4DS
                                </table>

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Reads the Host interface PS setting

        """
        self.TestEngineDLL.psReadHostInterface.restype = CT.c_int32
        self.TestEngineDLL.psReadHostInterface.argtypes = [CT.c_uint32, CT.c_void_p]
        transport = CT.c_uint16(_transport)
        retval = self.TestEngineDLL.psReadHostInterface(handle, CT.byref(transport))
        return retval, transport.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psWriteUsbAttributes(uint32 handle, uint16 bmAttributes);
    #########################################################
    # TestEngine::psWriteUsbAttributes
    @fntrace
    def psWriteUsbAttributes(self, handle, bmAttributes):
        r"""
        Function wrapper for TestEngine::psWriteUsbAttributes().

        Python API:
            psWriteUsbAttributes(handle, bmAttributes)
            returns retval

        Generated from C API:

            Function :      int32 psWriteUsbAttributes(uint32 handle, uint16 bmAttributes)

            Parameters :    handle -
                                Handle to the device

                            bmAttributes -
                                The bmAttributes of the Standard Configuration Descriptor,
                                described in Table 9.8 of USB specification, version 1.1.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Configuration characteristics
                            <table>
                                <tr><td> D7: Reserved (set to one)
                                <tr><td> D6: Self-powered
                                <tr><td> D5: Remote Wakeup
                                <tr><td> D4...0: Reserved (reset to zero)
                            </table>
                            D7 is reserved and must be set to one for historical reasons. A
                            device configuration that uses power from the bus and a local
                            source reports a non-zero value in MaxPower to indicate the
                            amount of bus power required and sets D6. The actual power
                            source at runtime may be determined using the GetStatus(DEVICE)
                            request (see Section 9.4.5 of USB specification 1.1). If a device
                            configuration supports remote wakeup, D5 is set to one.

        """
        self.TestEngineDLL.psWriteUsbAttributes.restype = CT.c_int32
        self.TestEngineDLL.psWriteUsbAttributes.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.psWriteUsbAttributes(handle, bmAttributes)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psWriteDPlusPullup(uint32 handle, uint16 pioPin);
    #########################################################
    # TestEngine::psWriteDPlusPullup
    @fntrace
    def psWriteDPlusPullup(self, handle, pioPin):
        r"""
        Function wrapper for TestEngine::psWriteDPlusPullup().

        Python API:
            psWriteDPlusPullup(handle, pioPin)
            returns retval

        Generated from C API:

            Function :      int32 psWriteDPlusPullup(uint32 handle, uint16 pioPin)

            Parameters :    handle -
                                Handle to the device

                            pioPin -
                                The PIO line used to control the pull-up resistor on the USB
                                D+ line.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   The absence of this key in the ps indicates that this feature is
                            not in use. This value is only used if the chip is presenting its
                            USB interface. See the description of PSKEY_HOST_INTERFACE.

        """
        self.TestEngineDLL.psWriteDPlusPullup.restype = CT.c_int32
        self.TestEngineDLL.psWriteDPlusPullup.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.psWriteDPlusPullup(handle, pioPin)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psWriteUsbMaxPower(uint32 handle, uint16 maxPower);
    #########################################################
    # TestEngine::psWriteUsbMaxPower
    @fntrace
    def psWriteUsbMaxPower(self, handle, maxPower):
        r"""
        Function wrapper for TestEngine::psWriteUsbMaxPower().

        Python API:
            psWriteUsbMaxPower(handle, maxPower)
            returns retval

        Generated from C API:

            Function :      int32 psWriteUsbMaxPower(uint32 handle, uint16 maxPower)

            Parameters :    handle -
                                Handle to the device

                            maxPower -
                                The MaxPower field of the local USB device, as defined in
                                Table 9.8 of version 1.1 of the USB specification. Bizarrely,
                                this value is given in units of 2mA.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Function to write the pskey PSKEY_USB_MAX_POWER to the persistent
                            store. This value is only used if the chip is presenting its USB
                            interface. See the description of PSKEY_HOST_INTERFACE.

        """
        self.TestEngineDLL.psWriteUsbMaxPower.restype = CT.c_int32
        self.TestEngineDLL.psWriteUsbMaxPower.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.psWriteUsbMaxPower(handle, maxPower)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psWritePioProtectMask(uint32 handle, uint16 mask);
    #########################################################
    # TestEngine::psWritePioProtectMask
    @fntrace
    def psWritePioProtectMask(self, handle, mask):
        r"""
        Function wrapper for TestEngine::psWritePioProtectMask().

        Python API:
            psWritePioProtectMask(handle, mask)
            returns retval

        Generated from C API:

            Function :      int32 psWritePioProtectMask(uint32 handle, uint16 mask)

            Parameters :    handle -
                                Handle to the device

                            mask -
                                PIO mask to protect PIO lines from unwanted access. If a bit
                                of the pskey's value is set high then application code cannot
                                change the value of the corresponding PIO port pin.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   The chip's PIO port can function as a general purpose input and
                            output port. However, various optional hardware and software
                            configurations require some PIO pins for their own needs, e.g., an
                            optional external PA/LNA block is driven using PIO pins 0 and 1.
                            This pskey prevents applications changing PIO pins that are
                            required for lower level functions. This blocking action applies
                            to host-based applications requesting PIO changes via BCCMD and to
                            VM applications running on the chip. Bits 0 to 7 of this uint16
                            pskey map to PIO0 to PIO7 respectively. If a bit of the pskey's
                            value is set high then application code cannot change the value of
                            the corresponding PIO port pin. The default value, 0x03, suits
                            Casira as this has an external PA/LNA.

        """
        self.TestEngineDLL.psWritePioProtectMask.restype = CT.c_int32
        self.TestEngineDLL.psWritePioProtectMask.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.psWritePioProtectMask(handle, mask)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psReadPioProtectMask(uint32 handle, uint16 * mask);
    #########################################################
    # TestEngine::psReadPioProtectMask
    @fntrace
    def psReadPioProtectMask(self, handle, _mask=0):
        r"""
        Function wrapper for TestEngine::psReadPioProtectMask().

        Python API:
            psReadPioProtectMask(handle, _mask=0)
            returns retval, mask.value

        Generated from C API:

            Function :      int32 psReadPioProtectMask(uint32 handle, uint16 * mask)

            Parameters :    handle -
                                Handle to the device

                            mask -
                                PIO mask to protect PIO lines from unwanted access. If a bit
                                of the pskey's value is set high then application code cannot
                                change the value of the corresponding PIO port pin.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Reads the PIO protect mask PS setting

        """
        self.TestEngineDLL.psReadPioProtectMask.restype = CT.c_int32
        self.TestEngineDLL.psReadPioProtectMask.argtypes = [CT.c_uint32, CT.c_void_p]
        mask = CT.c_uint16(_mask)
        retval = self.TestEngineDLL.psReadPioProtectMask(handle, CT.byref(mask))
        return retval, mask.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psWriteTxOffsetHalfMhz(uint32 handle, int16 offset);
    #########################################################
    # TestEngine::psWriteTxOffsetHalfMhz
    @fntrace
    def psWriteTxOffsetHalfMhz(self, handle, offset):
        r"""
        Function wrapper for TestEngine::psWriteTxOffsetHalfMhz().

        Python API:
            psWriteTxOffsetHalfMhz(handle, offset)
            returns retval

        Generated from C API:

            Function :      int32 psWriteTxOffsetHalfMhz(uint32 handle, int16 offset)

            Parameters :    handle -
                                Handle to the device

                            offset -
                                Transmit offset in units of 500kHz. For example -2 == -1MHz

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Sets the transmit offset.

        """
        self.TestEngineDLL.psWriteTxOffsetHalfMhz.restype = CT.c_int32
        self.TestEngineDLL.psWriteTxOffsetHalfMhz.argtypes = [CT.c_uint32, CT.c_int16]
        retval = self.TestEngineDLL.psWriteTxOffsetHalfMhz(handle, offset)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psReadTxOffsetHalfMhz(uint32 handle, int16 * offset);
    #########################################################
    # TestEngine::psReadTxOffsetHalfMhz
    @fntrace
    def psReadTxOffsetHalfMhz(self, handle, _offset=0):
        r"""
        Function wrapper for TestEngine::psReadTxOffsetHalfMhz().

        Python API:
            psReadTxOffsetHalfMhz(handle, _offset=0)
            returns retval, offset.value

        Generated from C API:

            Function :      int32 psReadTxOffsetHalfMhz(uint32 handle, int16 * offset)

            Parameters :    handle -
                                Handle to the device

                            offset -
                                Transmit offset in units of 500kHz. For example -2 == -1MHz

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Reads the transmit offset.

        """
        self.TestEngineDLL.psReadTxOffsetHalfMhz.restype = CT.c_int32
        self.TestEngineDLL.psReadTxOffsetHalfMhz.argtypes = [CT.c_uint32, CT.c_void_p]
        offset = CT.c_int16(_offset)
        retval = self.TestEngineDLL.psReadTxOffsetHalfMhz(handle, CT.byref(offset))
        return retval, offset.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psWriteUsrValue(uint32 handle, uint16 userNo, uint16 length, const uint16 data[]);
    #########################################################
    # TestEngine::psWriteUsrValue
    @fntrace
    def psWriteUsrValue(self, handle, userNo, length, data):
        r"""
        Function wrapper for TestEngine::psWriteUsrValue().

        Python API:
            psWriteUsrValue(handle, userNo, length, data)
            returns retval

        Generated from C API:

            Function :      int32 psWriteUsrValue(uint32 handle, uint16 userNo, uint16 length,
                                                    const uint16 * data)

            Parameters :    handle -
                                Handle to the device

                            userNo -
                                Value between 0 and 49 which refers to 'n' in PSKEY_USRn

                            length -
                                Length of PS key value in uint16's

                            data -
                                Pointer to an array containing the data to be written to the
                                PS.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Writes to PSKEY_USRn.
                            <p>This function is for use with BlueCore ICs only. For other
                            devices see tePsWrite.

        """
        self.TestEngineDLL.psWriteUsrValue.restype = CT.c_int32
        self.TestEngineDLL.psWriteUsrValue.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16, CT.c_void_p]
        __data = (CT.c_uint16 * len(data))(*data)
        retval = self.TestEngineDLL.psWriteUsrValue(handle, userNo, length, CT.byref(__data))
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psReadUsrValue(uint32 handle, uint16 userNo, uint16 maxLen, uint16 * length, uint16 data[]);
    #########################################################
    # TestEngine::psReadUsrValue
    @fntrace
    def psReadUsrValue(self, handle, userNo, maxLen, data, _length=0):
        r"""
        Function wrapper for TestEngine::psReadUsrValue().

        Python API:
            psReadUsrValue(handle, userNo, maxLen, data, _length=0)
            returns retval, length.value

        Generated from C API:

            Function :      int32 psReadUsrValue(uint32 handle, uint16 userNo, uint16 maxLen,
                                                uint16 * length, uint16 * data)

            Parameters :    handle -
                                Handle to the device

                            userNo -
                                Value between 0 and 49 which refers to 'n' in PSKEY_USRn

                            maxLen -
                                Length of the array to hold the returned value

                            length -
                                Pointer to the length of the returned ps entry

                            data -
                                Pointer to an array to hold the returned PS value

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Reads the PSKEY_USRn. If the length of PSKEY_USRn is greater than
                            maxLen, Error is returned, with the length parameter set to the
                            length of the key's data.
                            <p>This function is for use with BlueCore ICs only. For other
                            devices see tePsWrite.

        """
        self.TestEngineDLL.psReadUsrValue.restype = CT.c_int32
        self.TestEngineDLL.psReadUsrValue.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16, CT.c_void_p, CT.c_void_p]
        length = CT.c_uint16(_length)
        __data = (CT.c_uint16 * len(data))(*data)
        retval = self.TestEngineDLL.psReadUsrValue(handle, userNo, maxLen, CT.byref(length), CT.byref(__data))
        return retval, length.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psWritePowerTable(uint32 handle, uint16 numEntries, const uint8 intPA[] , const uint8 extPA[], const int32 power[]);
    #########################################################
    # TestEngine::psWritePowerTable
    @fntrace
    def psWritePowerTable(self, handle, numEntries, intPA, extPA, power):
        r"""
        Function wrapper for TestEngine::psWritePowerTable().

        Python API:
            psWritePowerTable(handle, numEntries, intPA, extPA, power)
            returns retval

        Generated from C API:

            Function :      int32 psWritePowerTable(uint32 handle, uint16 numEntries,
                                                    const uint8 * intPA, const uint8 * extPA,
                                                    const int32 * power)

            Parameters :    handle -
                                Handle to the device

                            numEntries -
                                Number of entries in the power table

                            intPA -
                                Pointer to an array of unsigned 8 bit integers of size
                                numEntries containing the intPA entries for the power table.

                            extPA -
                                Pointer to an array of unsigned 8 bit integers of size
                                numEntries containing the extPA entries for the power table.

                            power -
                                Pointer to an array of signed integers of size numEntries
                                containing the Output power entries, in dBm, for the power
                                table.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Writes to PSKEY_LC_POWER_TABLE
                            <p>This function is supported for BlueCore ICs only.

        """
        self.TestEngineDLL.psWritePowerTable.restype = CT.c_int32
        self.TestEngineDLL.psWritePowerTable.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_void_p, CT.c_void_p, CT.c_void_p]
        __intPA = (CT.c_uint8 * len(intPA))(*intPA)
        __extPA = (CT.c_uint8 * len(extPA))(*extPA)
        __power = (CT.c_int32 * len(power))(*power)
        retval = self.TestEngineDLL.psWritePowerTable(handle, numEntries, CT.byref(__intPA), CT.byref(__extPA), CT.byref(__power))
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psReadPowerTable(uint32 handle, int32 maxSize, int32 * numEntries, uint8 intPA[], uint8 extPA[], int32 power[]);
    #########################################################
    # TestEngine::psReadPowerTable
    @fntrace
    def psReadPowerTable(self, handle, maxSize, intPA, extPA, power, _numEntries=0):
        r"""
        Function wrapper for TestEngine::psReadPowerTable().

        Python API:
            psReadPowerTable(handle, maxSize, intPA, extPA, power, _numEntries=0)
            returns retval, numEntries.value

        Generated from C API:

            Function :      int32 psReadPowerTable(uint32 handle, int32 maxSize,
                                                    int32 * numEntries, uint8 * intPA,
                                                    uint8 * extPA, int32 * power)

            Parameters :    handle -
                                Handle to the device

                            maxSize -
                                Size of the arrays to hold the intPA, extPA and power

                            numEntries -
                                Pointer to the number of entries in the power table

                            intPA -
                                Pointer to an array of unsigned 8 bit integers of size
                                maxSize containing the intPA entries for the power table.

                            extPA -
                                Pointer to an array of unsigned 8 bit integers of size
                                maxSize containing the extPA entries for the power table.

                            power -
                                Pointer to an array of signed integers of size maxSize
                                containing the Output power entries, in dBm, for the power
                                table.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Reads the PSKEY_LC_POWER_TABLE
                            <p>This function is supported for BlueCore ICs only.

        """
        self.TestEngineDLL.psReadPowerTable.restype = CT.c_int32
        self.TestEngineDLL.psReadPowerTable.argtypes = [CT.c_uint32, CT.c_int32, CT.c_void_p, CT.c_void_p, CT.c_void_p, CT.c_void_p]
        numEntries = CT.c_int32(_numEntries)
        __intPA = (CT.c_uint8 * len(intPA))(*intPA)
        __extPA = (CT.c_uint8 * len(extPA))(*extPA)
        __power = (CT.c_int32 * len(power))(*power)
        retval = self.TestEngineDLL.psReadPowerTable(handle, maxSize, CT.byref(numEntries), CT.byref(__intPA), CT.byref(__extPA), CT.byref(__power))
        return retval, numEntries.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psWriteVmDisable(uint32 handle, uint8 vmDisable);
    #########################################################
    # TestEngine::psWriteVmDisable
    @fntrace
    def psWriteVmDisable(self, handle, vmDisable):
        r"""
        Function wrapper for TestEngine::psWriteVmDisable().

        Python API:
            psWriteVmDisable(handle, vmDisable)
            returns retval

        Generated from C API:

            Function :      int32 psWriteVmDisable(uint32 handle, uint8 vmDisable)

            Parameters :    handle -
                                Handle to the device

                            vmDisable -
                                Boolean value where non-zero indicates that the VM will be
                                disabled after the next reset.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Writes to PSKEY_VM_DISABLE
                            <p>This function is for use with BlueCore ICs only.

        """
        self.TestEngineDLL.psWriteVmDisable.restype = CT.c_int32
        self.TestEngineDLL.psWriteVmDisable.argtypes = [CT.c_uint32, CT.c_uint8]
        retval = self.TestEngineDLL.psWriteVmDisable(handle, vmDisable)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psReadVmDisable(uint32 handle, uint8 * vmDisable);
    #########################################################
    # TestEngine::psReadVmDisable
    @fntrace
    def psReadVmDisable(self, handle, _vmDisable=0):
        r"""
        Function wrapper for TestEngine::psReadVmDisable().

        Python API:
            psReadVmDisable(handle, _vmDisable=0)
            returns retval, vmDisable.value

        Generated from C API:

            Function :      int32 psReadVmDisable(uint32 handle, uint8 * vmDisable)

            Parameters :    handle -
                                Handle to the device

                            vmDisable -
                                Boolean value where non-zero indicates that the VM is
                                disabled.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Writes to PSKEY_VM_DISABLE
                            <p>This function is for use with BlueCore ICs only.

        """
        self.TestEngineDLL.psReadVmDisable.restype = CT.c_int32
        self.TestEngineDLL.psReadVmDisable.argtypes = [CT.c_uint32, CT.c_void_p]
        vmDisable = CT.c_uint8(_vmDisable)
        retval = self.TestEngineDLL.psReadVmDisable(handle, CT.byref(vmDisable))
        return retval, vmDisable.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) psMergeFromFile(uint32 handle, const char *filePath);
    #########################################################
    # TestEngine::psMergeFromFile
    @fntrace
    def psMergeFromFile(self, handle, filePath):
        r"""
        Function wrapper for TestEngine::psMergeFromFile().

        Python API:
            psMergeFromFile(handle, filePath)
            returns retval

        Generated from C API:

            Function :      int32 psMergeFromFile(uint32 handle, const char * filePath)

            Parameters :    handle -
                                Handle to the device

                            filePath -
                                Full path of the persistent store file (*.psr) to merge

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                            </table>

            Description :   This function can be used to merge a Qualcomm *.psr format
                            persistent store file into the persistent store of a BlueCore
                            chip. Patch files for ROM chips in *.psr format can be loaded
                            using this function.
                            <p>If there are any format errors in the file, psMergeFromFile
                            fails. The PSTool or PSCli applications can be used to find the
                            cause of the failure by attempting to merge the file using one of
                            the tools (errors are reported to the user).
                            <p>This function is for use with BlueCore ICs only.

        """
        self.TestEngineDLL.psMergeFromFile.restype = CT.c_int32
        self.TestEngineDLL.psMergeFromFile.argtypes = [CT.c_uint32, CT.c_char_p]
        retval = self.TestEngineDLL.psMergeFromFile(handle, filePath)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciSlave(uint32 handle);
    #########################################################
    # TestEngine::hciSlave
    @fntrace
    def hciSlave(self, handle):
        r"""
        Function wrapper for TestEngine::hciSlave().

        Python API:
            hciSlave(handle)
            returns retval

        Generated from C API:

            Function :      int32 hciSlave(uint32 handle)

            Parameters :    handle -
                                Handle to the device

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Performs four steps:
                            1. Sets the event filter to auto accept a connection request.
                            Implements the command HCI_Set_Event_Filter with the following
                            parameters:
                            <table>
                                <tr><td> 0x02 - Connection  setup filter type
                                <tr><td> 0x00 - Allow connections from all devices
                                <tr><td> 0x02 - Auto accept the connection
                            </table>

                            2. Implements the command HCI_WriteScan_Enable with the following
                            parameter:
                                0x03 - Inquiry scan enabled, Page scan enabled.

            Example :

                uint32 iHandle = 0;
                int32 iTimeout = 0;

                do
                {
                    cout << "Trying to connect..." << endl;
                    iHandle = openTestEngine(BCSP, "COM1", 115200, 1000, 0);
                    iTimeout += 1000;
                }while(iHandle == 0 && iTimeout < MAX_TIMEOUT);

                if(iHandle != 0)
                {
                    int32 iResetError = 0;

                    do
                    {
                        cout << "Connected!" << endl;

                        int32 iSuccess = psWriteVmDisable(iHandle, 1);

                        if(iSuccess != TE_OK)
                        {
                            cout << "psWriteVmDisable error" << endl;
                            break;
                        }

                        iSuccess = bccmdSetColdReset(iHandle, 0);

                        if(iSuccess != TE_OK)
                        {
                            iResetError = 1;
                            cout << "bccmdSetColdReset error" << endl;
                            break;
                        }

                        iSuccess = hciSlave(iHandle);

                        if(iSuccess != TE_OK)
                        {
                            cout << "hciSlave error" << endl;
                            if(iSuccess == TE_UNSUPPORTED_FUNCTION)
                            {
                                cout << "Unsupported command - if using RFCOMM build use dmSlave instead"
                                     << endl;
                            }
                            break;
                        }

                        iSuccess = hciEnableDeviceUnderTestMode(iHandle);

                        if(iSuccess != TE_OK)
                        {
                            cout << "hciEnableDeviceUnderTestMode error" << endl;
                            if(iSuccess == TE_UNSUPPORTED_FUNCTION)
                            {
                                cout << "Unsupported command - if using RFCOMM build use dmEnableDeviceUnderTestMode instead"
                                     << endl;
                            }
                            break;
                        }

                        // PERFORM TESTING HERE.....
                        cout << "TESTING.........." << endl;

                        iSuccess = psWriteVmDisable(iHandle, 0);

                        if(iSuccess != TE_OK)
                        {
                            cout << "psWriteVmDisable error" << endl;
                            break;
                        }

                        iSuccess = bccmdSetColdReset(iHandle, 0);

                        if(iSuccess != TE_OK)
                        {
                            iResetError = 1;
                            cout << "bccmdSetColdReset error" << endl;
                            break;
                        }

                    }while(0);

                    if(!iResetError)
                    {
                        closeTestEngine(iHandle);
                    }
                }


        """
        self.TestEngineDLL.hciSlave.restype = CT.c_int32
        self.TestEngineDLL.hciSlave.argtypes = [CT.c_uint32]
        retval = self.TestEngineDLL.hciSlave(handle)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciSetAfhHostChannelClass(uint32 handle, uint8 const cClass[]);
    #########################################################
    # TestEngine::hciSetAfhHostChannelClass
    @fntrace
    def hciSetAfhHostChannelClass(self, handle, cClass):
        r"""
        Function wrapper for TestEngine::hciSetAfhHostChannelClass().

        Python API:
            hciSetAfhHostChannelClass(handle, cClass)
            returns retval

        Generated from C API:

            Function :      int32 hciSetAfhHostChannelClass(uint32 handle,
                                                            uint8 const * const cClass)

            Parameters :    handle -
                                Handle to the device.

                            cClass -
                                Pointer to a 10 byte array containing channel classification
                                information. The most significant bit must be 0.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function allow the Bluetooth host to specify a channel
                            classification based on its local information.
                            <p>When called this function will block until it receives a
                            command complete event or until a preset time has elapsed
                            (~ 3 secs).

        """
        self.TestEngineDLL.hciSetAfhHostChannelClass.restype = CT.c_int32
        self.TestEngineDLL.hciSetAfhHostChannelClass.argtypes = [CT.c_uint32, CT.c_void_p]
        __cClass = (CT.c_uint8 * len(cClass))(*cClass)
        retval = self.TestEngineDLL.hciSetAfhHostChannelClass(handle, CT.byref(__cClass))
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciReadAfhChannelMap(uint32 handle, uint16 aclHandle, uint8* mode, uint8 cMap[]);
    #########################################################
    # TestEngine::hciReadAfhChannelMap
    @fntrace
    def hciReadAfhChannelMap(self, handle, aclHandle, cMap, _mode=0):
        r"""
        Function wrapper for TestEngine::hciReadAfhChannelMap().

        Python API:
            hciReadAfhChannelMap(handle, aclHandle, cMap, _mode=0)
            returns retval, mode.value

        Generated from C API:

            Function :      int32 hciReadAfhChannelMap(uint32 handle, uint16 aclHandle,
                                                        uint8* mode, uint8* cMap)

            Parameters :    handle -
                                Handle to the device.

                            aclHandle -
                                Valid ACL connection handle.

                            mode -
                                Pointer to a single byte which the function will set to
                                define whether AFH is enabled or disabled for the given
                                connection.

                            cMap -
                                Pointer to a 10 byte array that will contain channel map
                                information for the given connection.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function can be used to indicate the state of the hop
                            sequence specified by the most recent LMP_Set_AFH message for a
                            given connection handle.
                            <p>When called this function will block until it receives a
                            command complete event or until a preset time has elapsed
                            (~ 3 secs).

        """
        self.TestEngineDLL.hciReadAfhChannelMap.restype = CT.c_int32
        self.TestEngineDLL.hciReadAfhChannelMap.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_void_p, CT.c_void_p]
        mode = CT.c_uint8(_mode)
        __cMap = (CT.c_uint8 * len(cMap))(*cMap)
        retval = self.TestEngineDLL.hciReadAfhChannelMap(handle, aclHandle, CT.byref(mode), CT.byref(__cMap))
        return retval, mode.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciSetEventFilterAutoacceptConnection(uint32 handle);
    #########################################################
    # TestEngine::hciSetEventFilterAutoacceptConnection
    @fntrace
    def hciSetEventFilterAutoacceptConnection(self, handle):
        r"""
        Function wrapper for TestEngine::hciSetEventFilterAutoacceptConnection().

        Python API:
            hciSetEventFilterAutoacceptConnection(handle)
            returns retval

        Generated from C API:

            Function :      int32 hciSetEventFilterAutoacceptConnection(uint32 handle)

            Parameters :    handle -
                                Handle to the device

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Sets the event filter to auto accept a connection request.
                            Implements the command HCI_Set_Event_Filter with the following
                            parameters:
                            <table>
                                <tr><td> 0x02 - Connection  setup filter type
                                <tr><td> 0x00 - Allow connections from all devices
                                <tr><td> 0x02 - Auto accept the connection
                            </table>

        """
        self.TestEngineDLL.hciSetEventFilterAutoacceptConnection.restype = CT.c_int32
        self.TestEngineDLL.hciSetEventFilterAutoacceptConnection.argtypes = [CT.c_uint32]
        retval = self.TestEngineDLL.hciSetEventFilterAutoacceptConnection(handle)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciWriteInquiryScanActivity(uint32 handle, uint16 inquiryscan_interval, uint16 inquiryscan_window);
    #########################################################
    # TestEngine::hciWriteInquiryScanActivity
    @fntrace
    def hciWriteInquiryScanActivity(self, handle, inquiryscan_interval, inquiryscan_window):
        r"""
        Function wrapper for TestEngine::hciWriteInquiryScanActivity().

        Python API:
            hciWriteInquiryScanActivity(handle, inquiryscan_interval, inquiryscan_window)
            returns retval

        Generated from C API:

            Function :      int32 hciWriteInquiryScanActivity(uint32 handle,
                                                            uint16 inquiryscan_interval,
                                                            uint16 inquiryscan_window)

            Parameters :    handle -
                                Handle to the device

                            inquiryscan_interval -
                                Defines the amount of time between consecutive inquiry scans.

                            inquiryscan_window -
                                Defines the amount of time for the duration of the inquiry
                                scan.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Implements the command HCI_Write_Inquiryscan_activity.

        """
        self.TestEngineDLL.hciWriteInquiryScanActivity.restype = CT.c_int32
        self.TestEngineDLL.hciWriteInquiryScanActivity.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16]
        retval = self.TestEngineDLL.hciWriteInquiryScanActivity(handle, inquiryscan_interval, inquiryscan_window)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciWritePageScanActivity(uint32 handle, uint16 pagescan_interval, uint16 pagescan_window);
    #########################################################
    # TestEngine::hciWritePageScanActivity
    @fntrace
    def hciWritePageScanActivity(self, handle, pagescan_interval, pagescan_window):
        r"""
        Function wrapper for TestEngine::hciWritePageScanActivity().

        Python API:
            hciWritePageScanActivity(handle, pagescan_interval, pagescan_window)
            returns retval

        Generated from C API:

            Function :      int32 hciWritePageScanActivity(uint32 handle,
                                                            uint16 pagescan_interval,
                                                            uint16 pagescan_window)

            Parameters :    handle -
                                Handle to the device

                            pagescan_interval -
                                Defines the amount of time between consecutive page scans.

                            pagescan_window -
                                Defines the amount of time for the duration of the page scan.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Implements the command HCI_Write_Pagescan_activity.

        """
        self.TestEngineDLL.hciWritePageScanActivity.restype = CT.c_int32
        self.TestEngineDLL.hciWritePageScanActivity.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16]
        retval = self.TestEngineDLL.hciWritePageScanActivity(handle, pagescan_interval, pagescan_window)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciWriteScanEnable(uint32 handle, uint8 scan_enable);
    #########################################################
    # TestEngine::hciWriteScanEnable
    @fntrace
    def hciWriteScanEnable(self, handle, scan_enable):
        r"""
        Function wrapper for TestEngine::hciWriteScanEnable().

        Python API:
            hciWriteScanEnable(handle, scan_enable)
            returns retval

        Generated from C API:

            Function :      int32 hciWriteScanEnable(uint32 handle, uint8 scan_enable)

            Parameters :    handle -
                                Handle to the device

                            scan_enable -
                                Controls whether or not the Bluetooth device will periodically
                                scan for page attempts and/or inquiry requests from other
                                Bluetooth devices.
                                <table>
                                    <tr><td> 0x00 = No Scans enabled.
                                    <tr><td> 0x01 = Inquiry Scan enabled. Page Scan disabled.
                                    <tr><td> 0x02 = Inquiry Scan disabled. Page Scan enabled.
                                    <tr><td> 0x03 = Inquiry Scan enabled. Page Scan enabled.
                                </table>

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Implements the command HCI_WriteScan_Enable.

        """
        self.TestEngineDLL.hciWriteScanEnable.restype = CT.c_int32
        self.TestEngineDLL.hciWriteScanEnable.argtypes = [CT.c_uint32, CT.c_uint8]
        retval = self.TestEngineDLL.hciWriteScanEnable(handle, scan_enable)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciInquiry(uint32 handle, uint8 inquiryLength, uint8 numResponses, uint32 lap);
    #########################################################
    # TestEngine::hciInquiry
    @fntrace
    def hciInquiry(self, handle, inquiryLength, numResponses, lap):
        r"""
        Function wrapper for TestEngine::hciInquiry().

        Python API:
            hciInquiry(handle, inquiryLength, numResponses, lap)
            returns retval

        Generated from C API:

            Function :      int32 hciInquiry(uint32 handle, uint8 inquiryLength,
                                            uint8 numResponses, uint32 lap)

            Parameters :    handle -
                                Handle to the device

                            inquiryLength -
                                Maximum amount of time before the inquiry is halted, as
                                defined by the BT spec.

                            numResponses -
                                Maximum number of responses before inquiry is halted, as
                                defined in the BT spec. 0x00 is unlimited.

                            lap -
                                The LAP from which the inquiry access code is derived, as
                                defined in the BT spec.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Causes the device to enter inquiry mode.
                            This function will NOT wait for any inquiry results.

        """
        self.TestEngineDLL.hciInquiry.restype = CT.c_int32
        self.TestEngineDLL.hciInquiry.argtypes = [CT.c_uint32, CT.c_uint8, CT.c_uint8, CT.c_uint32]
        retval = self.TestEngineDLL.hciInquiry(handle, inquiryLength, numResponses, lap)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciGetInquiryResults(uint32 handle, uint32 lap[], uint8 uap[], uint16 nap[], uint16 clockOffset[], uint32 maxLen, uint32 * len);
    #########################################################
    # TestEngine::hciGetInquiryResults
    @fntrace
    def hciGetInquiryResults(self, handle, lap, uap, nap, clockOffset, maxLen, _len=0):
        r"""
        Function wrapper for TestEngine::hciGetInquiryResults().

        Python API:
            hciGetInquiryResults(handle, lap, uap, nap, clockOffset, maxLen, _len=0)
            returns retval, len.value

        Generated from C API:

            Function :      int32 hciGetInquiryResults(uint32 handle, uint32 * lap,
                                                        uint8 * uap, uint16 * nap,
                                                        uint16 * clockOffset, uint32 maxLen,
                                                        uint32 * len)

            Parameters :    handle -
                                Handle to the device

                            lap -
                                Pointer to the array of LAP's as defined by the returned
                                value.

                            uap -
                                Pointer to the array of UAP's as defined by the returned
                                value.

                            nap -
                                Pointer to the array of NAP's as defined by the returned
                                value.

                            clockOffset -
                                Pointer to the array of clock offsets as defined by the
                                returned value.

                            maxLen -
                                Size of the arrays allocated for the LAP, UAP and NAP.

                            len -
                                Pointer to a variable to hold the actual data returned with
                                the LAP, UAP and NAP.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Retrieves the bd_addrs and clock offsets of the devices found
                            during the last completed inquiry. If an inquiry is in progress
                            then this function will wait for a time based on the inquiryLength
                            parameter passed to hciInquiry().

        """
        self.TestEngineDLL.hciGetInquiryResults.restype = CT.c_int32
        self.TestEngineDLL.hciGetInquiryResults.argtypes = [CT.c_uint32, CT.c_void_p, CT.c_void_p, CT.c_void_p, CT.c_void_p, CT.c_uint32, CT.c_void_p]
        __lap = (CT.c_uint32 * len(lap))(*lap)
        __uap = (CT.c_uint8 * len(uap))(*uap)
        __nap = (CT.c_uint16 * len(nap))(*nap)
        __clockOffset = (CT.c_uint16 * len(clockOffset))(*clockOffset)
        len = CT.c_uint32(_len)
        retval = self.TestEngineDLL.hciGetInquiryResults(handle, CT.byref(__lap), CT.byref(__uap), CT.byref(__nap), CT.byref(__clockOffset), maxLen, CT.byref(len))
        return retval, len.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciInquiryCancel(uint32 handle);
    #########################################################
    # TestEngine::hciInquiryCancel
    @fntrace
    def hciInquiryCancel(self, handle):
        r"""
        Function wrapper for TestEngine::hciInquiryCancel().

        Python API:
            hciInquiryCancel(handle)
            returns retval

        Generated from C API:

            Function :      int32 hciInquiryCancel(uint32 handle)

            Parameters :    handle -
                                Handle to the device

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Causes the device to exit inquiry mode. The DLL will check if the
                            device is currently in inquiry before issuing this. Failure on
                            invalid handle or DM commands enabled.

        """
        self.TestEngineDLL.hciInquiryCancel.restype = CT.c_int32
        self.TestEngineDLL.hciInquiryCancel.argtypes = [CT.c_uint32]
        retval = self.TestEngineDLL.hciInquiryCancel(handle)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciCreateConnection(uint32 handle, uint32 lap, uint8 uap, uint16 nap, uint16 pkt_type, uint16 * connectionHandle);
    #########################################################
    # TestEngine::hciCreateConnection
    @fntrace
    def hciCreateConnection(self, handle, lap, uap, nap, pkt_type, _connectionHandle=0):
        r"""
        Function wrapper for TestEngine::hciCreateConnection().

        Python API:
            hciCreateConnection(handle, lap, uap, nap, pkt_type, _connectionHandle=0)
            returns retval, connectionHandle.value

        Generated from C API:

            Function :      int32 hciCreateConnection(uint32 handle, uint32 lap, uint8 uap,
                                                    uint16 nap, uint16 pkt_type,
                                                    uint16 * connectionHandle)

            Parameters :    handle -
                                Handle to the device

                            lap -
                                LAP of BD_ADDR. Must be one of the BD_ADDR's returned by
                                get_inquiry_results.

                            uap -
                                UAP of BD_ADDR. Must be one of the BD_ADDR's returned by
                                get_inquiry_results.

                            nap -
                                NAP of BD_ADDR. Must be one of the BD_ADDR's returned by
                                get_inquiry_results.

                            pkt_type -
                                Or-ed combination of packet types as defined in the Bluetooth
                                specification. A bit being set may indicate that a packet type
                                is allowed or disallowed, as indicated in the following table:
                                <table>
                                    <tr><td>0x0002 = 2-DH1 may not be used
                                    <tr><td>0x0004 = 3-DH1 may not be used
                                    <tr><td>0x0008 = DM1 may be used
                                    <tr><td>0x0010 = DH1 may be used
                                    <tr><td>0x0100 = 2-DH3 may not be used
                                    <tr><td>0x0200 = 3-DH3 may not be used
                                    <tr><td>0x0400 = DM3 may be used
                                    <tr><td>0x0800 = DH3 may be used
                                    <tr><td>0x1000 = 2-DH5 may not be used
                                    <tr><td>0x2000 = 3-DH5 may not be used
                                    <tr><td>0x4000 = DM5 may be used
                                    <tr><td>0x8000 = DH5 may be used
                                </table>
                                <p>Normally this value is set to allow all packet types, which
                                allows BlueCore to use its own algorithm to select the best
                                rate depending on the link quality.
                                The value which would allow all of the above packets to be
                                used is: 0xCC18 = (DM1 | DH1 | DM3 | DH3 | DM5 | DH5).

                            connectionHandle -
                                Connection handle to a Bluetooth connection.
                                NOTE: The connection handle is local to a device and not a
                                link. Therefore when you create a connection each member could
                                have a different connection handle for that link so the value
                                returned by this function should only be used for this device.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Causes the device to attempt to create a connection with a device
                            discovered during the inquiry phase. This function will wait for a
                            connection complete event with a 3 second time out. Success is
                            determined by:
                            <ol>
                                <li> Valid handle
                                <li> Successful connection complete event
                                <li> Invalid BD_ADDR (not part of the inquiry results list)
                            </ol>

        """
        self.TestEngineDLL.hciCreateConnection.restype = CT.c_int32
        self.TestEngineDLL.hciCreateConnection.argtypes = [CT.c_uint32, CT.c_uint32, CT.c_uint8, CT.c_uint16, CT.c_uint16, CT.c_void_p]
        connectionHandle = CT.c_uint16(_connectionHandle)
        retval = self.TestEngineDLL.hciCreateConnection(handle, lap, uap, nap, pkt_type, CT.byref(connectionHandle))
        return retval, connectionHandle.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciCreateConnectionNoInquiry(uint32 handle, uint32 lap, uint8 uap, uint16 nap, uint16 pkt_type, uint8 page_scan_rep_mode, uint8 page_scan_mode, uint16 clock_offset, uint8 allow_role_switch, uint16 * connectionHandle);
    #########################################################
    # TestEngine::hciCreateConnectionNoInquiry
    @fntrace
    def hciCreateConnectionNoInquiry(self, handle, lap, uap, nap, pkt_type, page_scan_rep_mode, page_scan_mode, clock_offset, allow_role_switch, _connectionHandle=0):
        r"""
        Function wrapper for TestEngine::hciCreateConnectionNoInquiry().

        Python API:
            hciCreateConnectionNoInquiry(handle, lap, uap, nap, pkt_type, page_scan_rep_mode, page_scan_mode, clock_offset, allow_role_switch, _connectionHandle=0)
            returns retval, connectionHandle.value

        Generated from C API:

            Function :      int32 hciCreateConnectionNoInquiry(uint32 handle, uint32 lap,
                                                                uint8 uap, uint16 nap,
                                                                uint16 pkt_type,
                                                                uint8 page_scan_rep_mode,
                                                                uint8 page_scan_mode,
                                                                uint16 clock_offset,
                                                                uint8 allow_role_switch,
                                                                uint16 * connectionHandle)

            Parameters :    handle -
                                Handle to the device

                            lap -
                                LAP of BD_ADDR.

                            uap -
                                UAP of BD_ADDR.

                            nap -
                                NAP of BD_ADDR.

                            pkt_type -
                                As for hciCreateConnection(...)

                            page_scan_repetition_mode -
                                Page scan repetition mode supported by the slave device
                                (0 = R0, 1 = R1, 2 = R2).

                            page_scan_mode -
                                BT Spec. requires this to be set to 0.

                            clock_offset -
                                The clock offset between the devices.

                            allow_role_switch -
                                Specifies if the local device accepts the request of a
                                master/slave role switch. 0 to not allow, 1 to allow.

                            connectionHandle -
                                Connection handle to a Bluetooth connection.
                                NOTE: The connection handle is local to a device and not a
                                link. Therefore when you create a connection each member could
                                have a different connection handle for that link so the value
                                returned by this function should only be used for this device.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Causes the device to attempt to create a connection with a remote
                            device. This function will wait for a connection complete event
                            with a 5 second time out. Success is determined by:
                            <ol>
                                <li> Valid handle
                                <li> Successful connection complete event
                                <li> Invalid BD_ADDR (not part of the inquiry results list)
                            </ol>

        """
        self.TestEngineDLL.hciCreateConnectionNoInquiry.restype = CT.c_int32
        self.TestEngineDLL.hciCreateConnectionNoInquiry.argtypes = [CT.c_uint32, CT.c_uint32, CT.c_uint8, CT.c_uint16, CT.c_uint16, CT.c_uint8, CT.c_uint8, CT.c_uint16, CT.c_uint8, CT.c_void_p]
        connectionHandle = CT.c_uint16(_connectionHandle)
        retval = self.TestEngineDLL.hciCreateConnectionNoInquiry(handle, lap, uap, nap, pkt_type, page_scan_rep_mode, page_scan_mode, clock_offset, allow_role_switch, CT.byref(connectionHandle))
        return retval, connectionHandle.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciCreateScoConnection(uint32 handle, uint16 aclConnectionHandle, uint16 pkt_type, uint16 * scoConnectionHandle);
    #########################################################
    # TestEngine::hciCreateScoConnection
    @fntrace
    def hciCreateScoConnection(self, handle, aclConnectionHandle, pkt_type, _scoConnectionHandle=0):
        r"""
        Function wrapper for TestEngine::hciCreateScoConnection().

        Python API:
            hciCreateScoConnection(handle, aclConnectionHandle, pkt_type, _scoConnectionHandle=0)
            returns retval, scoConnectionHandle.value

        Generated from C API:

            Function :      int32 hciCreateScoConnection(uint32 handle,
                                                            uint16 aclConnectionHandle,
                                                            uint16 pkt_type,
                                                            uint16 * scoConnectionHandle)

            Parameters :    handle -
                                Handle to the device

                            aclConnectionHandle -
                                ACL connection handle.

                            pkt_type -
                                Or-ed combination of packet types as defined in the Bluetooth
                                specification for the "Add_Sco_Connection" HCI command. A bit
                                being set indicates that a packet type is allowed. The
                                following table lists the packet types which can be specified:
                                <table>
                                    <tr><td>0x0020 = HV1 may be used
                                    <tr><td>0x0040 = HV2 may be used
                                    <tr><td>0x0080 = HV3 may be used
                                </table>
                                <p>Normally this value is set to allow all packet types, which
                                allows BlueCore to use its own algorithm to select the best
                                rate depending on the link quality.
                                The value which would allow all of the above packets to be
                                used is: 0x00E0 = (HV1 | HV2 | HV3).

                            scoConnectionHandle -
                                Pointer to a variable to hold the returned SCO connection
                                handle.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This command will cause the link manager to create a SCO
                            connection using the ACL connection specified by the
                            aclConnectionHandle command parameter. Success is determined by:
                            <ol>
                                <li> Valid handle
                                <li> Successful connection complete event
                            </ol>
                            <p>This function is deprecated due to the HCI_Add_Sco_Connection
                            command being deprecated in the BT spec from v1.2. Use
                            hciSetupScoConnection instead for v1.2 onwards.

            Deprecated :

        """
        self.TestEngineDLL.hciCreateScoConnection.restype = CT.c_int32
        self.TestEngineDLL.hciCreateScoConnection.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16, CT.c_void_p]
        scoConnectionHandle = CT.c_uint16(_scoConnectionHandle)
        retval = self.TestEngineDLL.hciCreateScoConnection(handle, aclConnectionHandle, pkt_type, CT.byref(scoConnectionHandle))
        return retval, scoConnectionHandle.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciSetupScoConnection(uint32 handle, uint16 aclConnectionHandle, uint32 txBandwidth, uint32 rxBandwidth, uint16 maxLatency, uint16 voiceSetting, uint8 retransEffort, uint16 pktType, uint16 * scoConnectionHandle);
    #########################################################
    # TestEngine::hciSetupScoConnection
    @fntrace
    def hciSetupScoConnection(self, handle, aclConnectionHandle, txBandwidth, rxBandwidth, maxLatency, voiceSetting, retransEffort, pktType, _scoConnectionHandle=0):
        r"""
        Function wrapper for TestEngine::hciSetupScoConnection().

        Python API:
            hciSetupScoConnection(handle, aclConnectionHandle, txBandwidth, rxBandwidth, maxLatency, voiceSetting, retransEffort, pktType, _scoConnectionHandle=0)
            returns retval, scoConnectionHandle.value

        Generated from C API:

            Function :      int32 hciSetupScoConnection(uint32 handle,
                                                        uint16 aclConnectionHandle,
                                                        uint32 txBandwidth,
                                                        uint32 rxBandwidth,
                                                        uint16 maxLatency,
                                                        uint16 voiceSetting,
                                                        uint8 retransEffort,
                                                        uint16 pktType,
                                                        uint16 * scoConnectionHandle)

            Parameters :    handle -
                                Handle to the device

                            aclConnectionHandle -
                                ACL connection handle.

                            txBandwidth -
                                The transmit bandwidth requirement in octets per second.

                            rxBandwidth -
                                The receive bandwidth requirement in octets per second.

                            maxLatency -
                                A value, in milli-seconds, representing the upper limit of the
                                sum of the synchronous interval and the size of the eSCO
                                window.

                            voiceSetting -
                                Also referred to as the content format. Specifies settings for
                                voice connections. Setting this parameter to anything other than
                                the current device(s) voice setting will result in failure. Use
                                hciReadVoiceSetting to get the setting, and hciWriteVoiceSetting
                                to alter it, if required, before calling this function.

                            retransEffort -
                                Specifies the retransmission behaviour as follows:
                                <table>
                                    <tr><td>0x00 = No retransmissions
                                    <tr><td>0x01 = At least 1 retransmission, optimise for
                                                   power consumption
                                    <tr><td>0x02 = At least 1 retransmission, optimise for
                                                   link quality
                                    <tr><td>0xFF = Don't care
                                    <tr><td>0x03 -> 0xFE = Reserved
                                </table>

                            pktType -
                                Or-ed combination of packet types. A bit being set may
                                indicate that a packet type is allowed or disallowed, as
                                indicated in the following table:
                                <table>
                                    <tr><td>0x0001 = HV1 may be used
                                    <tr><td>0x0002 = HV2 may be used
                                    <tr><td>0x0004 = HV3 may be used
                                    <tr><td>0x0008 = EV3 may be used
                                    <tr><td>0x0010 = EV4 may be used
                                    <tr><td>0x0020 = EV5 may be used
                                    <tr><td>0x0040 = 2-EV3 may not be used
                                    <tr><td>0x0080 = 3-EV3 may not be used
                                    <tr><td>0x0100 = 2-EV5 may not be used
                                    <tr><td>0x0200 = 3-EV5 may not be used
                                </table>
                                <p>Normally this value is set to allow all packet types, which
                                allows BlueCore to use its own algorithm to select the best
                                rate depending on the link quality.
                                The value which would allow all of the above packets to be
                                used is: 0x003F = (HV1 | HV2 | HV3 | EV3 | EV4 | EV5).

                            scoConnectionHandle -
                                Pointer to a variable to hold the returned SCO connection
                                handle.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This command will cause the link manager to setup a SCO/eSCO
                            connection using the ACL connection specified by the
                            aclConnectionHandle command parameter. Success is determined by:
                            <ol>
                                <li> Valid handle
                                <li> Successful connection complete event
                            </ol>
                            <p>For more information about the parameters and their ranges,
                            see the BlueTooth specification for the
                            "Setup_Synchronous_Connection" HCI command.
                            <p>Note that this function does not support the modification
                            of an existing SCO/eSCO connection.

            Example :

                static const uint16 SETUP_SCO_ALL_PKTS = 0x003F;
                static const int32 MAX_TIMEOUT = 5000;

                uint32 masterHandle;
                uint32 slaveHandle;
                int32 timeout = 0;

                do
                {
                    cout << "Trying to connect to master..." << endl;
                    masterHandle = openTestEngine(BCSP, "COM1", 115200, 1000, 0);
                    timeout+=1000;
                }while(masterHandle == 0 && timeout<MAX_TIMEOUT);

                timeout = 0;

                do
                {
                    cout << "Trying to connect to slave..." << endl;
                    slaveHandle = openTestEngine(USB, "\\\\.\\csr0", 0, 1000, 500);
                    timeout+=1000;
                }while(slaveHandle == 0 && timeout<MAX_TIMEOUT);

                if((masterHandle != 0) && (slaveHandle != 0))
                {
                    // Enable the slave device to auto accept incoming connections
                    // Implement the command HCI_WriteScan_Enable with the parameter 0x03,
                    // Inquiry scan enabled, Page scan enabled.
                    hciSlave(slaveHandle);

                    // Create an ACL connection to the Slave device from the master
                    uint16 aclHandle;
                    if(hciCreateConnectionNoInquiry(masterHandle, 0xf502, 0x5b, 0x2, 0xcc18, 0, 0, 0, 1, &aclHandle) != TE_OK)
                    {
                        std::cout << "Failed to connect to device" << std::endl;
                        closeTestEngine(masterHandle);
                        closeTestEngine(slaveHandle);
                        return -1;
                    }
                    std::cout << "ACL Handle = " << aclHandle << std::endl;

                    // Check connection status
                    if(hciConnectionStatus(masterHandle, aclHandle) != TE_OK)
                    {
                        std::cout << "Failed connection status" << std::endl;
                        closeTestEngine(masterHandle);
                        closeTestEngine(slaveHandle);
                        return -1;
                    }

                    // Check link quality
                    uint8 linkQuality;
                    if(hciGetLinkQuality(masterHandle, aclHandle, &linkQuality) != TE_OK)
                    {
                        std::cout << "Failed link quality" << std::endl;
                        closeTestEngine(masterHandle);
                        closeTestEngine(slaveHandle);
                        return -1;

                    }
                    std::cout << "link quality = " << (uint16)linkQuality << std::endl;

                    // Map SCO over PCM for audio test
                    if(bccmdSetMapScoPcm(masterHandle, 1) != TE_OK || bccmdSetMapScoPcm(slaveHandle, 1) != TE_OK)
                    {
                        std::cout << "Failed to map SCO over PCM" << std::endl;
                        closeTestEngine(masterHandle);
                        closeTestEngine(slaveHandle);
                        return -1;
                    }

                    // Set the voice setting if required
                    const uint16 voiceSetting(0x0060);
                    if(hciWriteVoiceSetting(masterHandle, voiceSetting) != TE_OK || hciWriteVoiceSetting(slaveHandle, voiceSetting) != TE_OK)
                    {
                        std::cout << "Failed to set voice settings" << std::endl;
                        closeTestEngine(masterHandle);
                        closeTestEngine(slaveHandle);
                        return -1;
                    }

                    // Setup SCO connection
                    uint16 scoConnectionHandle;
                    if(hciSetupScoConnection(masterHandle, aclHandle, 8000, 8000, 32, voiceSetting, 0, SETUP_SCO_ALL_PKTS, &scoConnectionHandle) != TE_OK)
                    {
                        std::cout << "Failed to establish SCO link" << std::endl;
                        closeTestEngine(masterHandle);
                        closeTestEngine(slaveHandle);
                        return -1;
                    }
                    else
                    {
                        // Created SCO connection successfully

                        //...
                        // Do SCO testing here
                        //...

                        hciDisconnect(masterHandle, scoConnectionHandle);
                    }

                    // Disconnect ACL link
                    hciDisconnect(masterHandle, aclHandle);

                    // Close handles to master and slave devices
                    closeTestEngine(masterHandle);
                    closeTestEngine(slaveHandle);
                }
                else
                {
                    std::cout << "Failed to get handles to one or both devices" << std::endl;
                    return -1;
                }

                return 0;

        """
        self.TestEngineDLL.hciSetupScoConnection.restype = CT.c_int32
        self.TestEngineDLL.hciSetupScoConnection.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint32, CT.c_uint32, CT.c_uint16, CT.c_uint16, CT.c_uint8, CT.c_uint16, CT.c_void_p]
        scoConnectionHandle = CT.c_uint16(_scoConnectionHandle)
        retval = self.TestEngineDLL.hciSetupScoConnection(handle, aclConnectionHandle, txBandwidth, rxBandwidth, maxLatency, voiceSetting, retransEffort, pktType, CT.byref(scoConnectionHandle))
        return retval, scoConnectionHandle.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciReadVoiceSetting(uint32 handle, uint16* voiceSetting);
    #########################################################
    # TestEngine::hciReadVoiceSetting
    @fntrace
    def hciReadVoiceSetting(self, handle, _voiceSetting=0):
        r"""
        Function wrapper for TestEngine::hciReadVoiceSetting().

        Python API:
            hciReadVoiceSetting(handle, _voiceSetting=0)
            returns retval, voiceSetting.value

        Generated from C API:

            Function :      int32 hciReadVoiceSetting(uint32 handle, uint16* voiceSetting)

            Parameters :    handle -
                                Handle to the device

                            voiceSetting -
                                The location where the value specifying the settings for voice
                                connections will be stored. Also referred to as the content
                                format.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This command will read the Voice_Setting configuration parameter.
                            Success is determined by:
                            <ol>
                                <li> Valid handle
                                <li> Successful command complete event
                            </ol>
                            <p>For more information about the parameter, see the BlueTooth
                            specification for the "HCI_Read_Voice_Setting" HCI command.
                            <p>This function can be used to get the default or current voice
                            setting to be passed as a parameter in the hciSetupScoConnection
                            function.

        """
        self.TestEngineDLL.hciReadVoiceSetting.restype = CT.c_int32
        self.TestEngineDLL.hciReadVoiceSetting.argtypes = [CT.c_uint32, CT.c_void_p]
        voiceSetting = CT.c_uint16(_voiceSetting)
        retval = self.TestEngineDLL.hciReadVoiceSetting(handle, CT.byref(voiceSetting))
        return retval, voiceSetting.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciWriteVoiceSetting(uint32 handle, uint16 voiceSetting);
    #########################################################
    # TestEngine::hciWriteVoiceSetting
    @fntrace
    def hciWriteVoiceSetting(self, handle, voiceSetting):
        r"""
        Function wrapper for TestEngine::hciWriteVoiceSetting().

        Python API:
            hciWriteVoiceSetting(handle, voiceSetting)
            returns retval

        Generated from C API:

            Function :      int32 hciWriteVoiceSetting(uint32 handle, uint16 voiceSetting)

            Parameters :    handle -
                                Handle to the device

                            voiceSetting -
                                Also referred to as the content format. Specifies settings for
                                voice connections.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This command will write the Voice_Setting configuration parameter.
                            Success is determined by:
                            <ol>
                                <li> Valid handle
                                <li> Successful command complete event
                            </ol>
                            <p>For more information about the parameter, see the BlueTooth
                            specification for the "HCI_Write_Voice_Setting" HCI command.
                            <p>Note that hciSetupScoConnection will fail if the voice setting
                            parameter is different to that set on the devices to be connected.
                            Therefore, if a voice setting other than the default is required,
                            this function must be called to write the voice setting to the
                            devices before hciSetupScoConnection is called.

        """
        self.TestEngineDLL.hciWriteVoiceSetting.restype = CT.c_int32
        self.TestEngineDLL.hciWriteVoiceSetting.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.hciWriteVoiceSetting(handle, voiceSetting)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciWriteLinkPolicySettings(uint32 handle, uint16 connection_handle, uint16 link_policy_settings);
    #########################################################
    # TestEngine::hciWriteLinkPolicySettings
    @fntrace
    def hciWriteLinkPolicySettings(self, handle, connection_handle, link_policy_settings):
        r"""
        Function wrapper for TestEngine::hciWriteLinkPolicySettings().

        Python API:
            hciWriteLinkPolicySettings(handle, connection_handle, link_policy_settings)
            returns retval

        Generated from C API:

            Function :      int32 hciWriteLinkPolicySettings(uint32 handle,
                                                                uint16 connection_handle,
                                                                uint16 link_policy_settings)

            Parameters :    handle -
                                Handle to the device

                            connection_handle -
                                Bluetooth connection handle to remote device

                            link_policy_settings -
                                Determines the behaviour of the local Link Manager when it
                                receives a request from a remote device or it determines
                                itself to change the master-slave role or to enter the hold,
                                sniff, or park mode.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Causes the device to attempt to write the link policy settings.
                            Success determined by:
                            <ol>
                                <li> Valid handle
                                <li> Valid connection handle
                            </ol>

        """
        self.TestEngineDLL.hciWriteLinkPolicySettings.restype = CT.c_int32
        self.TestEngineDLL.hciWriteLinkPolicySettings.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16]
        retval = self.TestEngineDLL.hciWriteLinkPolicySettings(handle, connection_handle, link_policy_settings)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciSendAclFile(uint32 handle, uint16 connHandle, const char* fileName);
    #########################################################
    # TestEngine::hciSendAclFile
    @fntrace
    def hciSendAclFile(self, handle, connHandle, fileName):
        r"""
        Function wrapper for TestEngine::hciSendAclFile().

        Python API:
            hciSendAclFile(handle, connHandle, fileName)
            returns retval

        Generated from C API:

            Function :      int32 hciSendAclFile(uint32 handle, uint16 connHandle,
                                                    const char* fileName)

            Parameters :    handle -
                                Handle to the device.

                            connHandle -
                                ACL connection handle as returned by hciCreateConnection or
                                hciCreateConnectionNoInquiry.

                            fileName -
                                Name of the file whose contents will be transmitted as ACL
                                packets.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to transmit the contents of a file as ACL
                            data to another device. Before this function is called a valid
                            ACL connection must have been established with a call to either
                            hciCreateConnection or hciCreateConnectionNoInquiry.
                            <p>Before transmitting data a call to hciResetAclState must be
                            made on the receiving device.
                            <p>When a file is sent the receiving device must be polled using
                            the hciGetAclState function for a result of ACL_RECEIVED (3) or
                            ACL_ERROR (4) at which point the entire file should have been
                            received unless there was a problem.
                            <p>The received file will be saved in the system's temporary files
                            directory regardless of where it was transmitted from. The
                            received file name may be different to the original. The
                            hciGetAclFileName function should be used to find the path
                            that the received data was saved to.
                            <p>This function has been tested with file sizes up to 1 Mbyte,
                            though typically smaller file sizes should be used (< 100 Kbyte)
                            to preserve memory resources required by the software in
                            transmitting the file.

        """
        self.TestEngineDLL.hciSendAclFile.restype = CT.c_int32
        self.TestEngineDLL.hciSendAclFile.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_char_p]
        retval = self.TestEngineDLL.hciSendAclFile(handle, connHandle, fileName)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciSendAclData(uint32 handle, uint16 connHandle, const uint8 data[], const uint16 length);
    #########################################################
    # TestEngine::hciSendAclData
    @fntrace
    def hciSendAclData(self, handle, connHandle, data, length):
        r"""
        Function wrapper for TestEngine::hciSendAclData().

        Python API:
            hciSendAclData(handle, connHandle, data, length)
            returns retval

        Generated from C API:

            Function :      int32 hciSendAclData(uint32 handle, uint16 connHandle,
                                                    const uint8* data, const uint16 length)

            Parameters :    handle -
                                Handle to the device.

                            connHandle -
                                ACL connection handle as returned by hciCreateConnection or
                                hciCreateConnectionNoInquiry.

                            data -
                                Pointer to array of bytes to be transmitted.

                            length -
                                Length of the array (in bytes) pointed to by data. Maximum
                                length is 65535.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to transmit an array of bytes as ACL data
                            to another device. Before this function is called a valid ACL
                            connection must have been established with a call to either
                            hciCreateConnection or hciCreateConnectionNoInquiry.
                            <p>Before transmitting data a call to hciResetAclState must be
                            made on the receiving device.
                            <p>When data is sent the receiving device must be polled using
                            the hciGetAclState function for a result of ACL_RECEIVED (3) at
                            which point all data should have been received.
                            <p>The received data can be read using the hciGetAclData
                            function.

        """
        self.TestEngineDLL.hciSendAclData.restype = CT.c_int32
        self.TestEngineDLL.hciSendAclData.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_void_p, CT.c_uint16]
        __data = (CT.c_uint8 * len(data))(*data)
        retval = self.TestEngineDLL.hciSendAclData(handle, connHandle, CT.byref(__data), length)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciGetAclData(uint32 handle, uint8 data[], uint32* numBytes);
    #########################################################
    # TestEngine::hciGetAclData
    @fntrace
    def hciGetAclData(self, handle, data, _numBytes=0):
        r"""
        Function wrapper for TestEngine::hciGetAclData().

        Python API:
            hciGetAclData(handle, data, _numBytes=0)
            returns retval, numBytes.value

        Generated from C API:

            Function :      int32 hciGetAclData(uint32 handle, uint8* data, uint32* numBytes)

            Parameters :    handle -
                                Handle to the device.

                            data -
                                Pointer to a pre-allocated byte array to receive ACL data
                                packet. If set to 0 no data will be copied.

                            numBytes -
                                Pointer to a value where the number of bytes in the packet
                                will be stored.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function can be used to retrieve the contents of a received
                            ACL packet.

        """
        self.TestEngineDLL.hciGetAclData.restype = CT.c_int32
        self.TestEngineDLL.hciGetAclData.argtypes = [CT.c_uint32, CT.c_void_p, CT.c_void_p]
        __data = (CT.c_uint8 * len(data))(*data)
        numBytes = CT.c_uint32(_numBytes)
        retval = self.TestEngineDLL.hciGetAclData(handle, CT.byref(__data), CT.byref(numBytes))
        return retval, numBytes.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciGetAclBytesRead(uint32 handle, uint32* numBytes);
    #########################################################
    # TestEngine::hciGetAclBytesRead
    @fntrace
    def hciGetAclBytesRead(self, handle, _numBytes=0):
        r"""
        Function wrapper for TestEngine::hciGetAclBytesRead().

        Python API:
            hciGetAclBytesRead(handle, _numBytes=0)
            returns retval, numBytes.value

        Generated from C API:

            Function :      int32 hciGetAclBytesRead(uint32 handle, uint32* numBytes)

            Parameters :    handle -
                                Handle to the device.

                            numBytes -
                                Pointer to a value where the number of bytes read so far will
                                be stored.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function can be used to retrieve the number of bytes read so
                            far by the recipient of some ACL data transferred as a result of
                            a call to hciSendAclData or hciSendAclFile.

        """
        self.TestEngineDLL.hciGetAclBytesRead.restype = CT.c_int32
        self.TestEngineDLL.hciGetAclBytesRead.argtypes = [CT.c_uint32, CT.c_void_p]
        numBytes = CT.c_uint32(_numBytes)
        retval = self.TestEngineDLL.hciGetAclBytesRead(handle, CT.byref(numBytes))
        return retval, numBytes.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciGetAclFileName(uint32 handle, char * fileName, uint32* length);
    #########################################################
    # TestEngine::hciGetAclFileName
    @fntrace
    def hciGetAclFileName(self, handle, _fileName='', _length=0):
        r"""
        Function wrapper for TestEngine::hciGetAclFileName().

        Python API:
            hciGetAclFileName(handle, _fileName='', _length=0)
            returns retval, fileName.value, length.value

        Generated from C API:

            Function :      int32 hciGetAclFileName(uint32 handle, char * fileName,
                                                    uint32* length)

            Parameters :    handle -
                                Handle to the device.

                            fileName -
                                A pre-allocated string to receive the name (full path) of the
                                file. This parameter can be set to 0, in which case the
                                function will just set the length parameter to the number of
                                characters (NOT including a null terminator) required to hold
                                the file name.

                            length -
                                Pointer to a value where the file name length (not including
                                null terminator) will be stored.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   The file name of received ACL data is not necessarily the same as
                            the one transmitted, because of one of the following reasons:
                            <ol>
                                <li> File already exists
                                <li> File was not in the system temporary folder
                            </ol>
                            This function is therefore used to find the name of the file
                            containing received ACL data. The name is returned in the
                            fileName argument which should be a pointer to a pre-allocated
                            character string large enough to accept the full path for the
                            file. The required string size can be found by calling this
                            function with the fileName argument set to 0.

        """
        self.TestEngineDLL.hciGetAclFileName.restype = CT.c_int32
        self.TestEngineDLL.hciGetAclFileName.argtypes = [CT.c_uint32, CT.c_char_p, CT.c_void_p]
        fileName = CT.create_string_buffer(_fileName, 255)
        length = CT.c_uint32(_length)
        retval = self.TestEngineDLL.hciGetAclFileName(handle, fileName, CT.byref(length))
        return retval, fileName.value, length.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciGetAclState(uint32 handle, int32* state);
    #########################################################
    # TestEngine::hciGetAclState
    @fntrace
    def hciGetAclState(self, handle, _state=0):
        r"""
        Function wrapper for TestEngine::hciGetAclState().

        Python API:
            hciGetAclState(handle, _state=0)
            returns retval, state.value

        Generated from C API:

            Function :      int32 hciGetAclState(uint32 handle, int32* state)

            Parameters :    handle -
                                Handle to the device.

                            state -
                                Pointer to a value where the state will be stored. This can
                                be one of the following values:
                                <table>
                                    <tr><td> ACL_IDLE<td>0<td>No transfer taking place.
                                    <tr><td> ACL_FILENAME<td>1<td>Receiving file name packet.
                                    <tr><td> ACL_RECEIVING<td>2<td>Receiving data packets.
                                    <tr><td> ACL_RECEIVED<td>3<td>File received.
                                    <tr><td> ACL_ERROR<td>4<td>Error occurred.
                                </table>

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to check the state of a transmission in
                            progress. It should be called for the device that is RECEIVING
                            data.

        """
        self.TestEngineDLL.hciGetAclState.restype = CT.c_int32
        self.TestEngineDLL.hciGetAclState.argtypes = [CT.c_uint32, CT.c_void_p]
        state = CT.c_int32(_state)
        retval = self.TestEngineDLL.hciGetAclState(handle, CT.byref(state))
        return retval, state.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciResetAclState(uint32 handle);
    #########################################################
    # TestEngine::hciResetAclState
    @fntrace
    def hciResetAclState(self, handle):
        r"""
        Function wrapper for TestEngine::hciResetAclState().

        Python API:
            hciResetAclState(handle)
            returns retval

        Generated from C API:

            Function :      int32 hciResetAclState(uint32 handle)

            Parameters :    handle -
                                Handle to the device.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function should be called by the RECEIVING device before a
                            call to hclSendAclData.

        """
        self.TestEngineDLL.hciResetAclState.restype = CT.c_int32
        self.TestEngineDLL.hciResetAclState.argtypes = [CT.c_uint32]
        retval = self.TestEngineDLL.hciResetAclState(handle)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciReset(uint32 handle);
    #########################################################
    # TestEngine::hciReset
    @fntrace
    def hciReset(self, handle):
        r"""
        Function wrapper for TestEngine::hciReset().

        Python API:
            hciReset(handle)
            returns retval

        Generated from C API:

            Function :      int32 hciReset(uint32 handle)

            Parameters :    handle -
                                Handle to the device.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function issues an HCI_Reset and waits for the appropriate
                            command complete.

        """
        self.TestEngineDLL.hciReset.restype = CT.c_int32
        self.TestEngineDLL.hciReset.argtypes = [CT.c_uint32]
        retval = self.TestEngineDLL.hciReset(handle)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdGetAnaXtalFtrim(uint32 handle, uint16 * ftrim);
    #########################################################
    # TestEngine::bccmdGetAnaXtalFtrim
    @fntrace
    def bccmdGetAnaXtalFtrim(self, handle, _ftrim=0):
        r"""
        Function wrapper for TestEngine::bccmdGetAnaXtalFtrim().

        Python API:
            bccmdGetAnaXtalFtrim(handle, _ftrim=0)
            returns retval, ftrim.value

        Generated from C API:

            Function :      int32 bccmdGetAnaXtalFtrim(uint32 handle, uint16 * ftrim)

            Parameters :    handle -
                                Handle to the device.

                            ftrim -
                                Variable to receive value of ftrim.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to retrieve the BlueCore crystal trim value.
                            It can be used to get the crystal trim value on devices where the
                            firmware does not have radiotest functionality.
                            It can only be used on firmware that supports
                            BCCMD_ANA_FTRIM_READWRITE. Some recent BlueCore ICs use a
                            frequency offset value instead of a trim value. In these cases
                            this function is not supported.

        """
        self.TestEngineDLL.bccmdGetAnaXtalFtrim.restype = CT.c_int32
        self.TestEngineDLL.bccmdGetAnaXtalFtrim.argtypes = [CT.c_uint32, CT.c_void_p]
        ftrim = CT.c_uint16(_ftrim)
        retval = self.TestEngineDLL.bccmdGetAnaXtalFtrim(handle, CT.byref(ftrim))
        return retval, ftrim.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdSetAnaXtalFtrim(uint32 handle, uint16 ftrim);
    #########################################################
    # TestEngine::bccmdSetAnaXtalFtrim
    @fntrace
    def bccmdSetAnaXtalFtrim(self, handle, ftrim):
        r"""
        Function wrapper for TestEngine::bccmdSetAnaXtalFtrim().

        Python API:
            bccmdSetAnaXtalFtrim(handle, ftrim)
            returns retval

        Generated from C API:

            Function :      int32 bccmdSetAnaXtalFtrim(uint32 handle, uint16 ftrim)

            Parameters :    handle -
                                Handle to the device.

                            ftrim -
                                Value of ftrim to set

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to set the BlueCore crystal trim value.
                            It is equivalent to radiotestCfgXtalFtrim but can be used to set
                            the trim value on devices where the firmware does not have
                            radiotest functionality.
                            It can only be used on firmware that supports
                            BCCMD_ANA_FTRIM_READWRITE. Some recent BlueCore ICs use a
                            frequency offset value instead of a trim value. In these cases
                            this function is not supported.

        """
        self.TestEngineDLL.bccmdSetAnaXtalFtrim.restype = CT.c_int32
        self.TestEngineDLL.bccmdSetAnaXtalFtrim.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.bccmdSetAnaXtalFtrim(handle, ftrim)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciSniffMode(uint32 handle, uint16 connection_handle, uint16 sniff_max_interval, uint16 sniff_min_interval, uint16 sniff_attempt, uint16 sniff_timeout);
    #########################################################
    # TestEngine::hciSniffMode
    @fntrace
    def hciSniffMode(self, handle, connection_handle, sniff_max_interval, sniff_min_interval, sniff_attempt, sniff_timeout):
        r"""
        Function wrapper for TestEngine::hciSniffMode().

        Python API:
            hciSniffMode(handle, connection_handle, sniff_max_interval, sniff_min_interval, sniff_attempt, sniff_timeout)
            returns retval

        Generated from C API:

            Function :      int32 hciSniffMode(uint32 handle, uint16 connection_handle,
                                                uint16 sniff_max_interval,
                                                uint16 sniff_min_interval,
                                                uint16 sniff_attempt, uint16 sniff_timeout)

            Parameters :    handle -
                                Handle to the device

                            connection_handle -
                                Bluetooth connection handle to remote device.

                            sniff_max_interval -
                                Maximum acceptable number of Baseband slots between each sniff
                                period.

                            sniff_min_interval -
                                Minimum acceptable number of Baseband slots between each sniff
                                period.

                            sniff_attempt -
                                Number of Baseband receive slots for sniff attempt.

                            sniff_timeout -
                                Number of Baseband receive slots for sniff timeout.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Causes the device to attempt to enter Sniff Mode with a remote
                            device. Success determined by:
                            <ol>
                                <li> Valid handle
                                <li> Valid connection handle
                            </ol>

        """
        self.TestEngineDLL.hciSniffMode.restype = CT.c_int32
        self.TestEngineDLL.hciSniffMode.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16, CT.c_uint16, CT.c_uint16, CT.c_uint16]
        retval = self.TestEngineDLL.hciSniffMode(handle, connection_handle, sniff_max_interval, sniff_min_interval, sniff_attempt, sniff_timeout)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciExitSniff(uint32 handle, uint16 connection_handle);
    #########################################################
    # TestEngine::hciExitSniff
    @fntrace
    def hciExitSniff(self, handle, connection_handle):
        r"""
        Function wrapper for TestEngine::hciExitSniff().

        Python API:
            hciExitSniff(handle, connection_handle)
            returns retval

        Generated from C API:

            Function :      int32 hciExitSniff(uint32 handle, uint16 connection_handle)

            Parameters :    handle -
                                Handle to the device

                            connection_handle -
                                Bluetooth connection handle to remote device.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Causes the device to attempt to exit Sniff Mode with a remote
                            device. Success determined by:
                            <ol>
                                <li> Valid handle
                                <li> Valid connection handle
                                <li> DM commands not in use
                            </ol>

        """
        self.TestEngineDLL.hciExitSniff.restype = CT.c_int32
        self.TestEngineDLL.hciExitSniff.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.hciExitSniff(handle, connection_handle)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciDisconnect(uint32 handle, uint16 connection_handle);
    #########################################################
    # TestEngine::hciDisconnect
    @fntrace
    def hciDisconnect(self, handle, connection_handle):
        r"""
        Function wrapper for TestEngine::hciDisconnect().

        Python API:
            hciDisconnect(handle, connection_handle)
            returns retval

        Generated from C API:

            Function :      int32 hciDisconnect(uint32 handle, uint16 connection_handle)

            Parameters :    handle -
                                Handle to the device

                            connection_handle -
                                Bluetooth connection handle to remote device.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Causes the device to request a disconnect from a remote Bluetooth.
                            The DLL will wait, with a 5 second timeout, for the disconnect
                            event to be returned. The memory for all disconnected devices will
                            be freed for this handle. If the remote device is also connected
                            via TestEngine it is important to run
                            hciPollDisconnectComplete(..) to confirm the correct operation
                            and to free any memory on that handle. Success is determined by:
                            <ol>
                                <li> Valid handle
                                <li> Valid active connection_handle
                                <li> Successful completion of disconnect.
                            </ol>

        """
        self.TestEngineDLL.hciDisconnect.restype = CT.c_int32
        self.TestEngineDLL.hciDisconnect.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.hciDisconnect(handle, connection_handle)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciGetConnectionHandle(uint32 handle, uint32 lap, uint8 uap, uint16 nap, uint16 * connectionHandle);
    #########################################################
    # TestEngine::hciGetConnectionHandle
    @fntrace
    def hciGetConnectionHandle(self, handle, lap, uap, nap, _connectionHandle=0):
        r"""
        Function wrapper for TestEngine::hciGetConnectionHandle().

        Python API:
            hciGetConnectionHandle(handle, lap, uap, nap, _connectionHandle=0)
            returns retval, connectionHandle.value

        Generated from C API:

            Function :      int32 hciGetConnectionHandle(uint32 handle, uint32 lap, uint8 uap,
                                                        uint16 nap, uint16 * connectionHandle)

            Parameters :    handle -
                                Handle to the device.

                            lap -
                                LAP of BD_ADDR.

                            uap -
                                UAP of BD_ADDR.

                            nap -
                                NAP of BD_ADDR.

                            connectionHandle -
                                Pointer to a variable which will hold the returned connection
                                handle.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Returns the connection handle, if existing, between the device
                            identified by the "handle" parameter and the device identified by
                            the Bluetooth address. Failure on invalid handle and DM commands
                            enabled.

        """
        self.TestEngineDLL.hciGetConnectionHandle.restype = CT.c_int32
        self.TestEngineDLL.hciGetConnectionHandle.argtypes = [CT.c_uint32, CT.c_uint32, CT.c_uint8, CT.c_uint16, CT.c_void_p]
        connectionHandle = CT.c_uint16(_connectionHandle)
        retval = self.TestEngineDLL.hciGetConnectionHandle(handle, lap, uap, nap, CT.byref(connectionHandle))
        return retval, connectionHandle.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciConnectionStatus(uint32 handle, uint16 connection_handle);
    #########################################################
    # TestEngine::hciConnectionStatus
    @fntrace
    def hciConnectionStatus(self, handle, connection_handle):
        r"""
        Function wrapper for TestEngine::hciConnectionStatus().

        Python API:
            hciConnectionStatus(handle, connection_handle)
            returns retval

        Generated from C API:

            Function :      int32 hciConnectionStatus(uint32 handle, uint16 connection_handle)


            Parameters :    handle -
                                Handle to the device

                            connection_handle -
                                Bluetooth connection handle to remote device.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Checks whether the connection handle points to a valid, active
                            Bluetooth connection. Failure on invalid handle and DM commands
                            enabled.

        """
        self.TestEngineDLL.hciConnectionStatus.restype = CT.c_int32
        self.TestEngineDLL.hciConnectionStatus.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.hciConnectionStatus(handle, connection_handle)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciEnableDeviceUnderTestMode(uint32 handle);
    #########################################################
    # TestEngine::hciEnableDeviceUnderTestMode
    @fntrace
    def hciEnableDeviceUnderTestMode(self, handle):
        r"""
        Function wrapper for TestEngine::hciEnableDeviceUnderTestMode().

        Python API:
            hciEnableDeviceUnderTestMode(handle)
            returns retval

        Generated from C API:

            Function :      int32 hciEnableDeviceUnderTestMode(uint32 handle)

            Parameters :    handle -
                                Handle to the device

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Causes the device to send the enable device under test mode.
                            Success is determined by:
                            <ol>
                                <li> Valid handle
                                <li> Command complete within timeout
                            </ol>

            Example :

                uint32 iHandle = 0;
                int32 iTimeout = 0;

                do
                {
                    cout << "Trying to connect..." << endl;
                    iHandle = openTestEngine(BCSP, "COM1", 115200, 1000, 0);
                    iTimeout += 1000;
                }while(iHandle == 0 && iTimeout < MAX_TIMEOUT);

                if(iHandle != 0)
                {
                    int32 iResetError = 0;

                    do
                    {
                        cout << "Connected!" << endl;

                        int32 iSuccess = psWriteVmDisable(iHandle, 1);

                        if(iSuccess != TE_OK)
                        {
                            cout << "psWriteVmDisable error" << endl;
                            break;
                        }

                        iSuccess = bccmdSetColdReset(iHandle, 0);

                        if(iSuccess != TE_OK)
                        {
                            iResetError = 1;
                            cout << "bccmdSetColdReset error" << endl;
                            break;
                        }

                        iSuccess = hciSlave(iHandle);

                        if(iSuccess != TE_OK)
                        {
                            cout << "hciSlave error" << endl;
                            if(iSuccess == TE_UNSUPPORTED_FUNCTION)
                            {
                                cout << "Unsupported command - if using RFCOMM build use dmSlave instead"
                                     << endl;
                            }
                            break;
                        }

                        iSuccess = hciEnableDeviceUnderTestMode(iHandle);

                        if(iSuccess != TE_OK)
                        {
                            cout << "hciEnableDeviceUnderTestMode error" << endl;
                            if(iSuccess == TE_UNSUPPORTED_FUNCTION)
                            {
                                cout << "Unsupported command - if using RFCOMM build use dmEnableDeviceUnderTestMode instead"
                                     << endl;
                            }
                            break;
                        }

                        // PERFORM TESTING HERE.....
                        cout << "TESTING.........." << endl;

                        iSuccess = psWriteVmDisable(iHandle, 0);

                        if(iSuccess != TE_OK)
                        {
                            cout << "psWriteVmDisable error" << endl;
                            break;
                        }

                        iSuccess = bccmdSetColdReset(iHandle, 0);

                        if(iSuccess != TE_OK)
                        {
                            iResetError = 1;
                            cout << "bccmdSetColdReset error" << endl;
                            break;
                        }

                    }while(0);

                    if(!iResetError)
                    {
                        closeTestEngine(iHandle);
                    }
                }

        """
        self.TestEngineDLL.hciEnableDeviceUnderTestMode.restype = CT.c_int32
        self.TestEngineDLL.hciEnableDeviceUnderTestMode.argtypes = [CT.c_uint32]
        retval = self.TestEngineDLL.hciEnableDeviceUnderTestMode(handle)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciGetLinkQuality(uint32 handle, uint16 connection_handle, uint8 * quality);
    #########################################################
    # TestEngine::hciGetLinkQuality
    @fntrace
    def hciGetLinkQuality(self, handle, connection_handle, _quality=0):
        r"""
        Function wrapper for TestEngine::hciGetLinkQuality().

        Python API:
            hciGetLinkQuality(handle, connection_handle, _quality=0)
            returns retval, quality.value

        Generated from C API:

            Function :      int32 hciGetLinkQuality(uint32 handle, uint16 connection_handle,
                                                    uint8 * quality)

            Parameters :    handle -
                                Handle to the device

                            connection_handle -
                                Bluetooth connection handle to remote device.

                            quality -
                                Pointer to the link quality parameter returned by this
                                function.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Reads the link quality

        """
        self.TestEngineDLL.hciGetLinkQuality.restype = CT.c_int32
        self.TestEngineDLL.hciGetLinkQuality.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_void_p]
        quality = CT.c_uint8(_quality)
        retval = self.TestEngineDLL.hciGetLinkQuality(handle, connection_handle, CT.byref(quality))
        return retval, quality.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciReadRemoteVersionInformation(uint32 handle, uint16 connection_handle, uint32 r[]);
    #########################################################
    # TestEngine::hciReadRemoteVersionInformation
    @fntrace
    def hciReadRemoteVersionInformation(self, handle, connection_handle, r):
        r"""
        Function wrapper for TestEngine::hciReadRemoteVersionInformation().

        Python API:
            hciReadRemoteVersionInformation(handle, connection_handle, r)
            returns retval

        Generated from C API:

            Function :      int32 hciReadRemoteVersionInformation(uint32 handle,
                                                                    uint16 connection_handle,
                                                                    uint32 * r)

            Parameters :    handle -
                                Handle to the device

                            connection_handle -
                                Bluetooth connection handle to remote device.

                            r -
                                A 4 member array of unsigned 32 bit integers. On success r is:
                                <table>
                                    <tr><td> r[0] = connection_handle
                                    <tr><td> r[1] = lmp_version, as defined by the Bluetooth spec.
                                    <tr><td> r[2] = Manufacturer name.
                                    <tr><td> r[3] = lmp_subversion
                                </table>
                                With BlueCore the lmp_subversion is the firmware build ID.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Reads the remote version information.

        """
        self.TestEngineDLL.hciReadRemoteVersionInformation.restype = CT.c_int32
        self.TestEngineDLL.hciReadRemoteVersionInformation.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_void_p]
        __r = (CT.c_uint32 * len(r))(*r)
        retval = self.TestEngineDLL.hciReadRemoteVersionInformation(handle, connection_handle, CT.byref(__r))
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciReadRemoteNameRequest(uint32 handle, uint32 lap, uint8 uap, uint16 nap, uint8 page_scan_repetition_mode, uint8 page_scan_offset, uint16 clock_offset, char * r);
    #########################################################
    # TestEngine::hciReadRemoteNameRequest
    @fntrace
    def hciReadRemoteNameRequest(self, handle, lap, uap, nap, page_scan_repetition_mode, page_scan_offset, clock_offset, _r=''):
        r"""
        Function wrapper for TestEngine::hciReadRemoteNameRequest().

        Python API:
            hciReadRemoteNameRequest(handle, lap, uap, nap, page_scan_repetition_mode, page_scan_offset, clock_offset, _r='')
            returns retval, r.value

        Generated from C API:

            Function :      int32 hciReadRemoteNameRequest(uint32 handle, uint32 lap, uint8 uap,
                                                            uint16 nap,
                                                            uint8 page_scan_repetition_mode,
                                                            uint8 page_scan_offset,
                                                            uint16 clock_offset, char * r)

            Parameters :    handle -
                                Handle to the device

                            lap -
                                LAP of BD_ADDR.

                            uap -
                                UAP of BD_ADDR.

                            nap -
                                NAP of BD_ADDR.

                            page_scan_repetition_mode -
                                Page scan mode supported by the remote device.

                            page_scan_offset -
                                Page scan mode supported by the remote device.

                            clock_offset -
                                The clock offset between the devices.

                            r -
                                Pointer to the remote name returned by the command. This
                                needs to be pre-allocated to at least 249 bytes.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Reads the remote name.

        """
        self.TestEngineDLL.hciReadRemoteNameRequest.restype = CT.c_int32
        self.TestEngineDLL.hciReadRemoteNameRequest.argtypes = [CT.c_uint32, CT.c_uint32, CT.c_uint8, CT.c_uint16, CT.c_uint8, CT.c_uint8, CT.c_uint16, CT.c_char_p]
        r = CT.create_string_buffer(_r, 255)
        retval = self.TestEngineDLL.hciReadRemoteNameRequest(handle, lap, uap, nap, page_scan_repetition_mode, page_scan_offset, clock_offset, r)
        return retval, r.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) dmRegisterReq(uint32 handle);
    #########################################################
    # TestEngine::dmRegisterReq
    @fntrace
    def dmRegisterReq(self, handle):
        r"""
        Function wrapper for TestEngine::dmRegisterReq().

        Python API:
            dmRegisterReq(handle)
            returns retval

        Generated from C API:

            Function :      int32 dmRegisterReq(uint32 handle)

            Parameters :    handle -
                                Handle to the device

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Causes the device to register the application manager queue
                            command. No other DM commands will function unless this is called.
                            Success is determined by:
                            <ol>
                                <li> Valid handle
                                <li> Command complete within timeout
                                <li> non-HCI firmware
                            </ol>

        """
        self.TestEngineDLL.dmRegisterReq.restype = CT.c_int32
        self.TestEngineDLL.dmRegisterReq.argtypes = [CT.c_uint32]
        retval = self.TestEngineDLL.dmRegisterReq(handle)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) dmSlave(uint32 handle);
    #########################################################
    # TestEngine::dmSlave
    @fntrace
    def dmSlave(self, handle):
        r"""
        Function wrapper for TestEngine::dmSlave().

        Python API:
            dmSlave(handle)
            returns retval

        Generated from C API:

            Function :      int32 dmSlave(uint32 handle)

            Parameters :    handle -
                                Handle to the device

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Performs two steps:
                            <ol>
                                <li>Sets the event filter to auto accept a connection request.
                                    Implements the command DM_HCI_Set_Event_Filter with the
                                    following parameters:
                                    <table>
                                        <tr><td> 0x02 - Connection setup filter type
                                        <tr><td> 0x00 - Allow connections from all devices
                                        <tr><td> 0x02 - Auto accept the connection
                                    </table>

                                <li>Implements the command DM_HCI_WriteScan_Enable with the
                                    following parameter:
                                    <p>0x03 - Inquiry scan enabled, Page scan enabled.
                            </ol>

        """
        self.TestEngineDLL.dmSlave.restype = CT.c_int32
        self.TestEngineDLL.dmSlave.argtypes = [CT.c_uint32]
        retval = self.TestEngineDLL.dmSlave(handle)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) dmEnableDeviceUnderTestMode(uint32 handle);
    #########################################################
    # TestEngine::dmEnableDeviceUnderTestMode
    @fntrace
    def dmEnableDeviceUnderTestMode(self, handle):
        r"""
        Function wrapper for TestEngine::dmEnableDeviceUnderTestMode().

        Python API:
            dmEnableDeviceUnderTestMode(handle)
            returns retval

        Generated from C API:

            Function :      int32 dmEnableDeviceUnderTestMode(uint32 handle)

            Parameters :    handle -
                                Handle to the device

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Causes the device to send the enable device under test mode.
                            Success is determined by:
                            <ol>
                                <li> Valid handle
                                <li> Command complete within timeout
                            </ol>

        """
        self.TestEngineDLL.dmEnableDeviceUnderTestMode.restype = CT.c_int32
        self.TestEngineDLL.dmEnableDeviceUnderTestMode.argtypes = [CT.c_uint32]
        retval = self.TestEngineDLL.dmEnableDeviceUnderTestMode(handle)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) dmWritePageScanActivity(uint32 handle, uint16 pagescan_interval, uint16 pagescan_window);
    #########################################################
    # TestEngine::dmWritePageScanActivity
    @fntrace
    def dmWritePageScanActivity(self, handle, pagescan_interval, pagescan_window):
        r"""
        Function wrapper for TestEngine::dmWritePageScanActivity().

        Python API:
            dmWritePageScanActivity(handle, pagescan_interval, pagescan_window)
            returns retval

        Generated from C API:

            Function :      int32 dmWritePageScanActivity(uint32 handle,
                                                            uint16 pagescan_interval,
                                                            uint16 pagescan_window)

            Parameters :    handle -
                                Handle to the device

                            pagescan_interval -
                                Defines the amount of time between consecutive page scans.

                            pagescan_window -
                                Defines the amount of time for the duration of the page scan.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Implements the command DM_HCI_Write_Pagescan_activity.

        """
        self.TestEngineDLL.dmWritePageScanActivity.restype = CT.c_int32
        self.TestEngineDLL.dmWritePageScanActivity.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16]
        retval = self.TestEngineDLL.dmWritePageScanActivity(handle, pagescan_interval, pagescan_window)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) dmWriteInquiryScanActivity(uint32 handle, uint16 inquiryscan_interval, uint16 inquiryscan_window);
    #########################################################
    # TestEngine::dmWriteInquiryScanActivity
    @fntrace
    def dmWriteInquiryScanActivity(self, handle, inquiryscan_interval, inquiryscan_window):
        r"""
        Function wrapper for TestEngine::dmWriteInquiryScanActivity().

        Python API:
            dmWriteInquiryScanActivity(handle, inquiryscan_interval, inquiryscan_window)
            returns retval

        Generated from C API:

            Function :      int32 dmWriteInquiryScanActivity(uint32 handle,
                                                                uint16 inquiryscan_interval,
                                                                uint16 inquiryscan_window)

            Parameters :    handle -
                                Handle to the device

                            inquiryscan_interval -
                                Defines the amount of time between consecutive inquiry scans.

                            inquiryscan_window -
                                Defines the amount of time for the duration of the inquiry
                                scan.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Implements the command DM_HCI_Write_Inquiryscan_activity.

        """
        self.TestEngineDLL.dmWriteInquiryScanActivity.restype = CT.c_int32
        self.TestEngineDLL.dmWriteInquiryScanActivity.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16]
        retval = self.TestEngineDLL.dmWriteInquiryScanActivity(handle, inquiryscan_interval, inquiryscan_window)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) dmWriteScanEnable(uint32 handle, uint8 scan_enable);
    #########################################################
    # TestEngine::dmWriteScanEnable
    @fntrace
    def dmWriteScanEnable(self, handle, scan_enable):
        r"""
        Function wrapper for TestEngine::dmWriteScanEnable().

        Python API:
            dmWriteScanEnable(handle, scan_enable)
            returns retval

        Generated from C API:

            Function :      int32 dmWriteScanEnable(uint32 handle, uint8 scan_enable)

            Parameters :    handle -
                                Handle to the device

                            scan_enable -
                                Controls whether or not the Bluetooth device will periodically
                                scan for page attempts and/or inquiry requests from other
                                Bluetooth devices.
                                <table>
                                    <tr><td> 0x00 = No Scans enabled.
                                    <tr><td> 0x01 = Inquiry Scan enabled. Page Scan disabled.
                                    <tr><td> 0x02 = Inquiry Scan disabled. Page Scan enabled.
                                    <tr><td> 0x03 = Inquiry Scan enabled. Page Scan enabled.
                                </table>

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Implements the command DM_HCI_WriteScan_Enable.

        """
        self.TestEngineDLL.dmWriteScanEnable.restype = CT.c_int32
        self.TestEngineDLL.dmWriteScanEnable.argtypes = [CT.c_uint32, CT.c_uint8]
        retval = self.TestEngineDLL.dmWriteScanEnable(handle, scan_enable)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) dmSetEventFilterAutoacceptConnection(uint32 handle);
    #########################################################
    # TestEngine::dmSetEventFilterAutoacceptConnection
    @fntrace
    def dmSetEventFilterAutoacceptConnection(self, handle):
        r"""
        Function wrapper for TestEngine::dmSetEventFilterAutoacceptConnection().

        Python API:
            dmSetEventFilterAutoacceptConnection(handle)
            returns retval

        Generated from C API:

            Function :      int32 dmSetEventFilterAutoacceptConnection(uint32 handle)

            Parameters :    handle -
                                Handle to the device

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Sets the event filter to auto accept a connection request.
                            Implements the command DM_HCI_Set_Event_Filter with the following
                            parameters:
                            <table>
                                <tr><td> 0x02 - Connection setup filter type
                                <tr><td> 0x00 - Allow connections from all devices
                                <tr><td> 0x02 - Auto accept the connection
                            </table>

        """
        self.TestEngineDLL.dmSetEventFilterAutoacceptConnection.restype = CT.c_int32
        self.TestEngineDLL.dmSetEventFilterAutoacceptConnection.argtypes = [CT.c_uint32]
        retval = self.TestEngineDLL.dmSetEventFilterAutoacceptConnection(handle)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdBc3PsuTrim(uint32 handle, uint16 data);
    #########################################################
    # TestEngine::bccmdBc3PsuTrim
    @fntrace
    def bccmdBc3PsuTrim(self, handle, data):
        r"""
        Function wrapper for TestEngine::bccmdBc3PsuTrim().

        Python API:
            bccmdBc3PsuTrim(handle, data)
            returns retval

        Generated from C API:

            Function :      int32 bccmdBc3PsuTrim(uint32 handle, uint16 data)

            Parameters :    handle -
                                Handle to the device.

                            data -
                                Data to send with the PDU.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will write a BCCMD with VARID of BCCMDVARID_BC3PSU
                            and a function of BCCMDPDU_BC3PSU_PSU_TRIM to the connected
                            BlueCore device.

            Deprecated :

        """
        self.TestEngineDLL.bccmdBc3PsuTrim.restype = CT.c_int32
        self.TestEngineDLL.bccmdBc3PsuTrim.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.bccmdBc3PsuTrim(handle, data)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdChargerPsuTrim(uint32 handle, uint16 trim);
    #########################################################
    # TestEngine::bccmdChargerPsuTrim
    @fntrace
    def bccmdChargerPsuTrim(self, handle, trim):
        r"""
        Function wrapper for TestEngine::bccmdChargerPsuTrim().

        Python API:
            bccmdChargerPsuTrim(handle, trim)
            returns retval

        Generated from C API:

            Function :      int32 bccmdChargerPsuTrim(uint32 handle, uint16 trim)

            Parameters :    handle -
                                Handle to the device.

                            trim -
                                Charger trim value to be written.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to modify the charger trim register in
                            order to alter the behaviour of BlueCore's built-in battery
                            charger.

        """
        self.TestEngineDLL.bccmdChargerPsuTrim.restype = CT.c_int32
        self.TestEngineDLL.bccmdChargerPsuTrim.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.bccmdChargerPsuTrim(handle, trim)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdBc3BuckReg(uint32 handle, uint16 data);
    #########################################################
    # TestEngine::bccmdBc3BuckReg
    @fntrace
    def bccmdBc3BuckReg(self, handle, data):
        r"""
        Function wrapper for TestEngine::bccmdBc3BuckReg().

        Python API:
            bccmdBc3BuckReg(handle, data)
            returns retval

        Generated from C API:

            Function :      int32 bccmdBc3BuckReg(uint32 handle, uint16 data)

            Parameters :    handle -
                                Handle to the device.

                            data -
                                Data to send with the PDU.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will write a BCCMD with VARID of BCCMDVARID_BC3PSU
                            and a function of BCCMDPDU_BC3PSU_BUCK_REG to the connected
                            BlueCore device.

            Deprecated :

        """
        self.TestEngineDLL.bccmdBc3BuckReg.restype = CT.c_int32
        self.TestEngineDLL.bccmdBc3BuckReg.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.bccmdBc3BuckReg(handle, data)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdPsuSmpsEnable(uint32 handle, uint16 reg);
    #########################################################
    # TestEngine::bccmdPsuSmpsEnable
    @fntrace
    def bccmdPsuSmpsEnable(self, handle, reg):
        r"""
        Function wrapper for TestEngine::bccmdPsuSmpsEnable().

        Python API:
            bccmdPsuSmpsEnable(handle, reg)
            returns retval

        Generated from C API:

            Function :      int32 bccmdPsuSmpsEnable(uint32 handle, uint16 reg)

            Parameters :    handle -
                                Handle to the device.

                            reg -
                                Value to enable (1) or disable the switch-mode regulator.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to enable or disable the switch-mode
                            regulator.

        """
        self.TestEngineDLL.bccmdPsuSmpsEnable.restype = CT.c_int32
        self.TestEngineDLL.bccmdPsuSmpsEnable.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.bccmdPsuSmpsEnable(handle, reg)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdBc3MicEn(uint32 handle, uint16 data);
    #########################################################
    # TestEngine::bccmdBc3MicEn
    @fntrace
    def bccmdBc3MicEn(self, handle, data):
        r"""
        Function wrapper for TestEngine::bccmdBc3MicEn().

        Python API:
            bccmdBc3MicEn(handle, data)
            returns retval

        Generated from C API:

            Function :      int32 bccmdBc3MicEn(uint32 handle, uint16 data)

            Parameters :    handle -
                                Handle to the device.

                            data -
                                Data to send with the PDU.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will write a BCCMD with VARID of BCCMDVARID_BC3PSU
                            and a function of BCCMDPDU_BC3PSU_MIC_EN to the connected
                            BlueCore device.

            Deprecated :

        """
        self.TestEngineDLL.bccmdBc3MicEn.restype = CT.c_int32
        self.TestEngineDLL.bccmdBc3MicEn.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.bccmdBc3MicEn(handle, data)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdPsuHvLinearEnable(uint32 handle, uint16 reg);
    #########################################################
    # TestEngine::bccmdPsuHvLinearEnable
    @fntrace
    def bccmdPsuHvLinearEnable(self, handle, reg):
        r"""
        Function wrapper for TestEngine::bccmdPsuHvLinearEnable().

        Python API:
            bccmdPsuHvLinearEnable(handle, reg)
            returns retval

        Generated from C API:

            Function :      int32 bccmdPsuHvLinearEnable(uint32 handle, uint16 reg)

            Parameters :    handle -
                                Handle to the device.

                            reg -
                                Value to enable (1) or disable the linear regulator.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to enable or disable the linear regulator.
                            <p>For BC5 chips, use bccmdSetMicBias instead.

        """
        self.TestEngineDLL.bccmdPsuHvLinearEnable.restype = CT.c_int32
        self.TestEngineDLL.bccmdPsuHvLinearEnable.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.bccmdPsuHvLinearEnable(handle, reg)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdBc3Led0(uint32 handle, uint16 data);
    #########################################################
    # TestEngine::bccmdBc3Led0
    @fntrace
    def bccmdBc3Led0(self, handle, data):
        r"""
        Function wrapper for TestEngine::bccmdBc3Led0().

        Python API:
            bccmdBc3Led0(handle, data)
            returns retval

        Generated from C API:

            Function :      int32 bccmdBc3Led0(uint32 handle, uint16 data)

            Parameters :    handle -
                                Handle to the device.

                            data -
                                Data to send with the PDU.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will write a BCCMD with VARID of BCCMDVARID_BC3PSU
                            and a function of BCCMDPDU_BC3PSU_LED0 to the connected BlueCore
                            device.

            Deprecated :

        """
        self.TestEngineDLL.bccmdBc3Led0.restype = CT.c_int32
        self.TestEngineDLL.bccmdBc3Led0.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.bccmdBc3Led0(handle, data)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdLedEnable(uint32 handle, uint16 led, uint16 state);
    #########################################################
    # TestEngine::bccmdLedEnable
    @fntrace
    def bccmdLedEnable(self, handle, led, state):
        r"""
        Function wrapper for TestEngine::bccmdLedEnable().

        Python API:
            bccmdLedEnable(handle, led, state)
            returns retval

        Generated from C API:

            Function :      int32 bccmdLedEnable(uint32 handle, uint16 led, uint16 state)

            Parameters :    handle -
                                Handle to the device.

                            led -
                                A zero-indexed identifier for the LED

                            state -
                                Value to enable (1) or disable (0) the LED

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to set the state of a single LED.
                            <p>This function can be used with some recent BlueCore ICs, where
                            the firmware supports the LED_CONFIG BCCMD for configuring any
                            LED. For older BlueCore ICs (supporting only 2 LEDs), use
                            bccmdLed0Enable or bccmdLed1Enable.

        """
        self.TestEngineDLL.bccmdLedEnable.restype = CT.c_int32
        self.TestEngineDLL.bccmdLedEnable.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16]
        retval = self.TestEngineDLL.bccmdLedEnable(handle, led, state)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdLed0Enable(uint32 handle, uint16 state);
    #########################################################
    # TestEngine::bccmdLed0Enable
    @fntrace
    def bccmdLed0Enable(self, handle, state):
        r"""
        Function wrapper for TestEngine::bccmdLed0Enable().

        Python API:
            bccmdLed0Enable(handle, state)
            returns retval

        Generated from C API:

            Function :      int32 bccmdLed0Enable(uint32 handle, uint16 state)

            Parameters :    handle -
                                Handle to the device.

                            state -
                                Value to enable (1) or disable (0) LED0

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to set the state of LED0.
                            <p>For some recent BlueCore ICs, where the firmware supports the
                            LED_CONFIG BCCMD for configuring any LED, use bccmdLedEnable
                            instead.

        """
        self.TestEngineDLL.bccmdLed0Enable.restype = CT.c_int32
        self.TestEngineDLL.bccmdLed0Enable.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.bccmdLed0Enable(handle, state)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdBc3Led1(uint32 handle, uint16 data);
    #########################################################
    # TestEngine::bccmdBc3Led1
    @fntrace
    def bccmdBc3Led1(self, handle, data):
        r"""
        Function wrapper for TestEngine::bccmdBc3Led1().

        Python API:
            bccmdBc3Led1(handle, data)
            returns retval

        Generated from C API:

            Function :      int32 bccmdBc3Led1(uint32 handle, uint16 data)

            Parameters :    handle -
                                Handle to the device.

                            data -
                                Data to send with the PDU.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will write a BCCMD with VARID of BCCMDVARID_BC3PSU
                            and a function of BCCMDPDU_BC3PSU_LED1 to the connected BlueCore
                            device.

            Deprecated :

        """
        self.TestEngineDLL.bccmdBc3Led1.restype = CT.c_int32
        self.TestEngineDLL.bccmdBc3Led1.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.bccmdBc3Led1(handle, data)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdLed1Enable(uint32 handle, uint16 state);
    #########################################################
    # TestEngine::bccmdLed1Enable
    @fntrace
    def bccmdLed1Enable(self, handle, state):
        r"""
        Function wrapper for TestEngine::bccmdLed1Enable().

        Python API:
            bccmdLed1Enable(handle, state)
            returns retval

        Generated from C API:

            Function :      int32 bccmdLed1Enable(uint32 handle, uint16 state)

            Parameters :    handle -
                                Handle to the device.

                            state -
                                Value to enable (1) or disable (0) LED1

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to set the state of LED1.
                            <p>For some recent BlueCore ICs, where the firmware supports the
                            LED_CONFIG BCCMD for configuring any LED, use bccmdLedEnable
                            instead.

        """
        self.TestEngineDLL.bccmdLed1Enable.restype = CT.c_int32
        self.TestEngineDLL.bccmdLed1Enable.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.bccmdLed1Enable(handle, state)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdChargerStatus(uint32 handle, uint16 * state);
    #########################################################
    # TestEngine::bccmdChargerStatus
    @fntrace
    def bccmdChargerStatus(self, handle, _state=0):
        r"""
        Function wrapper for TestEngine::bccmdChargerStatus().

        Python API:
            bccmdChargerStatus(handle, _state=0)
            returns retval, state.value

        Generated from C API:

            Function :      int32 bccmdChargerStatus(uint32 handle, uint16 * state)

            Parameters :    handle -
                                Handle to the device.

                            state -
                                Pointer to a 16-bit unsigned integer to hold the returned
                                state of the charger.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to read the state of built-in battery
                            charger.

        """
        self.TestEngineDLL.bccmdChargerStatus.restype = CT.c_int32
        self.TestEngineDLL.bccmdChargerStatus.argtypes = [CT.c_uint32, CT.c_void_p]
        state = CT.c_uint16(_state)
        retval = self.TestEngineDLL.bccmdChargerStatus(handle, CT.byref(state))
        return retval, state.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdChargerDisable(uint32 handle, uint16 state);
    #########################################################
    # TestEngine::bccmdChargerDisable
    @fntrace
    def bccmdChargerDisable(self, handle, state):
        r"""
        Function wrapper for TestEngine::bccmdChargerDisable().

        Python API:
            bccmdChargerDisable(handle, state)
            returns retval

        Generated from C API:

            Function :      int32 bccmdChargerDisable(uint32 handle, uint16 state)

            Parameters :    handle -
                                Handle to the device.

                            state -
                                Value to disable (1) or enable (0) the built-in battery
                                charger.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to disable or enable the built-in battery
                            charger.

        """
        self.TestEngineDLL.bccmdChargerDisable.restype = CT.c_int32
        self.TestEngineDLL.bccmdChargerDisable.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.bccmdChargerDisable(handle, state)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdChargerSuppressLed0(uint32 handle, uint16 state);
    #########################################################
    # TestEngine::bccmdChargerSuppressLed0
    @fntrace
    def bccmdChargerSuppressLed0(self, handle, state):
        r"""
        Function wrapper for TestEngine::bccmdChargerSuppressLed0().

        Python API:
            bccmdChargerSuppressLed0(handle, state)
            returns retval

        Generated from C API:

            Function :      int32 bccmdChargerSuppressLed0(uint32 handle, uint16 state)

            Parameters :    handle -
                                Handle to the device.

                            state -
                                Value to suppress (0) or allow (1) LED0 to light when charging.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to enable or disable LED0 on changing.

        """
        self.TestEngineDLL.bccmdChargerSuppressLed0.restype = CT.c_int32
        self.TestEngineDLL.bccmdChargerSuppressLed0.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.bccmdChargerSuppressLed0(handle, state)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciCreateConnectionNoWait(uint32 handle, uint32 lap, uint8 uap, uint16 nap, uint16 pktType, uint8 pageScanRepMode, uint8 pageScanMode, uint16 clockOffset, uint8 allowRoleSwitch);
    #########################################################
    # TestEngine::hciCreateConnectionNoWait
    @fntrace
    def hciCreateConnectionNoWait(self, handle, lap, uap, nap, pktType, pageScanRepMode, pageScanMode, clockOffset, allowRoleSwitch):
        r"""
        Function wrapper for TestEngine::hciCreateConnectionNoWait().

        Python API:
            hciCreateConnectionNoWait(handle, lap, uap, nap, pktType, pageScanRepMode, pageScanMode, clockOffset, allowRoleSwitch)
            returns retval

        Generated from C API:

            Function :      int32 hciCreateConnectionNoWait(uint32 handle, uint32 lap,
                                                            uint8 uap, uint16 nap,
                                                            uint16 pktType,
                                                            uint8 pageScanRepMode,
                                                            uint8 pageScanMode,
                                                            uint16 clockOffset,
                                                            uint8 allowRoleSwitch)

            Parameters :    handle -
                                Handle to the device.

                            lap -
                                The LAP portion of the Bluetooth address of the device to
                                request a connection with.

                            uap -
                                The UAP portion of the Bluetooth address of the device to
                                request a connection with.

                            nap -
                                The NAP portion of the Bluetooth address of the device to
                                request a connection with.

                            pktType -
                                As for hciCreateConnection(...)

                            pageScanRepMode -
                                The page scan repetition mode supported by the remote device
                                with the given Bluetooth address.

                            pageScanMode -
                                Indicates the page scan mode that is used for default page
                                scan.

                            clockOffset -
                                Difference between the clocks of the local device and the
                                remote device with the given Bluetooth address.

                            allowRoleSwitch -
                                Specifies whether the local device accepts or rejects the
                                request of master-slave role switch.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function can be used to request a connection with a given
                            device.<br>
                            It is similar to the hciCreateConnectionNoInquiry function, the
                            difference being that this function does not wait for a
                            'Command Complete' or a 'Connection Complete' event.<br>
                            This is useful for creating connections involving encryption and
                            link-key requests where other operations need to be performed
                            before these events are generated.

        """
        self.TestEngineDLL.hciCreateConnectionNoWait.restype = CT.c_int32
        self.TestEngineDLL.hciCreateConnectionNoWait.argtypes = [CT.c_uint32, CT.c_uint32, CT.c_uint8, CT.c_uint16, CT.c_uint16, CT.c_uint8, CT.c_uint8, CT.c_uint16, CT.c_uint8]
        retval = self.TestEngineDLL.hciCreateConnectionNoWait(handle, lap, uap, nap, pktType, pageScanRepMode, pageScanMode, clockOffset, allowRoleSwitch)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciWriteAuthenticationEnable(uint32 handle, uint8 enable);
    #########################################################
    # TestEngine::hciWriteAuthenticationEnable
    @fntrace
    def hciWriteAuthenticationEnable(self, handle, enable):
        r"""
        Function wrapper for TestEngine::hciWriteAuthenticationEnable().

        Python API:
            hciWriteAuthenticationEnable(handle, enable)
            returns retval

        Generated from C API:

            Function :      int32 hciWriteAuthenticationEnable(uint32 handle, uint8 enable)

            Parameters :    handle -
                                Handle to the device.

                            enable -
                                Set to 1 when authentication required for connections,
                                otherwise 0.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will write the value for the Authentication_Enable
                            configuration parameter.

        """
        self.TestEngineDLL.hciWriteAuthenticationEnable.restype = CT.c_int32
        self.TestEngineDLL.hciWriteAuthenticationEnable.argtypes = [CT.c_uint32, CT.c_uint8]
        retval = self.TestEngineDLL.hciWriteAuthenticationEnable(handle, enable)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciLinkKeyRequestNegativeReply(uint32 handle, uint32 lap, uint8 uap, uint16 nap);
    #########################################################
    # TestEngine::hciLinkKeyRequestNegativeReply
    @fntrace
    def hciLinkKeyRequestNegativeReply(self, handle, lap, uap, nap):
        r"""
        Function wrapper for TestEngine::hciLinkKeyRequestNegativeReply().

        Python API:
            hciLinkKeyRequestNegativeReply(handle, lap, uap, nap)
            returns retval

        Generated from C API:

            Function :      int32 hciLinkKeyRequestNegativeReply(uint32 handle, uint32 lap,
                                                                uint8 uap, uint16 nap)

            Parameters :    handle -
                                Handle to the device.

                            lap -
                                The LAP portion of the Bluetooth address of the device that
                                there is no stored link key for.

                            uap -
                                The UAP portion of the Bluetooth address of the device that
                                there is no stored link key for.

                            nap -
                                The NAP portion of the Bluetooth address of the device that
                                there is no stored link key for.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to reply to a Link Key Request from the
                            controller if the local device does not have a stored link key
                            for the device with the given Bluetooth address.

        """
        self.TestEngineDLL.hciLinkKeyRequestNegativeReply.restype = CT.c_int32
        self.TestEngineDLL.hciLinkKeyRequestNegativeReply.argtypes = [CT.c_uint32, CT.c_uint32, CT.c_uint8, CT.c_uint16]
        retval = self.TestEngineDLL.hciLinkKeyRequestNegativeReply(handle, lap, uap, nap)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciWaitForConnectionComplete(uint32 handle, uint16* connHandle);
    #########################################################
    # TestEngine::hciWaitForConnectionComplete
    @fntrace
    def hciWaitForConnectionComplete(self, handle, _connHandle=0):
        r"""
        Function wrapper for TestEngine::hciWaitForConnectionComplete().

        Python API:
            hciWaitForConnectionComplete(handle, _connHandle=0)
            returns retval, connHandle.value

        Generated from C API:

            Function :      int32 hciWaitForConnectionComplete(uint32 handle,
                                                                uint16* connHandle)

            Parameters :    handle -
                                Handle to the device.

                            connHandle -
                                Pointer to connection handle of the device if successful. Set
                                to 0xffff on failure.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to wait for a 'Connection Complete'
                            event.<br>
                            If the event is not triggered within 2 seconds of invocation it
                            will return failure and the value of the connHandle parameter
                            will be set to 0xffff.

        """
        self.TestEngineDLL.hciWaitForConnectionComplete.restype = CT.c_int32
        self.TestEngineDLL.hciWaitForConnectionComplete.argtypes = [CT.c_uint32, CT.c_void_p]
        connHandle = CT.c_uint16(_connHandle)
        retval = self.TestEngineDLL.hciWaitForConnectionComplete(handle, CT.byref(connHandle))
        return retval, connHandle.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciWaitForLinkKeyRequest(uint32 handle, uint32* lap, uint8* uap, uint16* nap);
    #########################################################
    # TestEngine::hciWaitForLinkKeyRequest
    @fntrace
    def hciWaitForLinkKeyRequest(self, handle, _lap=0, _uap=0, _nap=0):
        r"""
        Function wrapper for TestEngine::hciWaitForLinkKeyRequest().

        Python API:
            hciWaitForLinkKeyRequest(handle, _lap=0, _uap=0, _nap=0)
            returns retval, lap.value, uap.value, nap.value

        Generated from C API:

            Function :      int32 hciWaitForLinkKeyRequest(uint32 handle, uint32* lap,
                                                            uint8* uap, uint16* nap)

            Parameters :    handle -
                                Handle to the device.

                            lap -
                                The LAP portion of the Bluetooth address of the device for
                                which a stored link key is being requested.

                            uap -
                                The UAP portion of the Bluetooth address of the device for
                                which a stored link key is being requested.

                            nap -
                                The NAP portion of the Bluetooth address of the device for
                                which a stored link key is being requested.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function can be used to wait for a 'Link Key Request' event.
                            A link key request event will be generated when setting up a
                            connection when authentication has been enabled (see
                            hciWriteAuthenticationEnable) prior to calling
                            hciCreateConnectionNoWait.<br>
                            If the event is not triggered within 2 seconds of invocation it
                            will return failure.

        """
        self.TestEngineDLL.hciWaitForLinkKeyRequest.restype = CT.c_int32
        self.TestEngineDLL.hciWaitForLinkKeyRequest.argtypes = [CT.c_uint32, CT.c_void_p, CT.c_void_p, CT.c_void_p]
        lap = CT.c_uint32(_lap)
        uap = CT.c_uint8(_uap)
        nap = CT.c_uint16(_nap)
        retval = self.TestEngineDLL.hciWaitForLinkKeyRequest(handle, CT.byref(lap), CT.byref(uap), CT.byref(nap))
        return retval, lap.value, uap.value, nap.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciWaitForPinCodeRequest(uint32 handle, uint32* lap, uint8* uap, uint16* nap);
    #########################################################
    # TestEngine::hciWaitForPinCodeRequest
    @fntrace
    def hciWaitForPinCodeRequest(self, handle, _lap=0, _uap=0, _nap=0):
        r"""
        Function wrapper for TestEngine::hciWaitForPinCodeRequest().

        Python API:
            hciWaitForPinCodeRequest(handle, _lap=0, _uap=0, _nap=0)
            returns retval, lap.value, uap.value, nap.value

        Generated from C API:

            Function :      int32 hciWaitForPinCodeRequest(uint32 handle, uint32* lap,
                                                            uint8* uap, uint16* nap)

            Parameters :    handle -
                                Handle to the device.

                            lap -
                                The LAP portion of the Bluetooth address of the device which
                                a new link key is being created for.

                            uap -
                                The UAP portion of the Bluetooth address of the device which
                                a new link key is being created for.

                            nap -
                                The NAP portion of the Bluetooth address of the device which
                                a new link key is being created for.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function can be used to wait for a 'PIN Code Request' event.
                            A PIN code request event will be generated when setting up a
                            connection when authentication has been enabled (see
                            hciWriteAuthenticationEnable) prior to calling
                            hciCreateConnectionNoWait.<br>
                            The recipient must respond by calling hciPinCodeRequestReply.<br>
                            If the event is not triggered within 2 seconds of invocation it
                            will return failure.

        """
        self.TestEngineDLL.hciWaitForPinCodeRequest.restype = CT.c_int32
        self.TestEngineDLL.hciWaitForPinCodeRequest.argtypes = [CT.c_uint32, CT.c_void_p, CT.c_void_p, CT.c_void_p]
        lap = CT.c_uint32(_lap)
        uap = CT.c_uint8(_uap)
        nap = CT.c_uint16(_nap)
        retval = self.TestEngineDLL.hciWaitForPinCodeRequest(handle, CT.byref(lap), CT.byref(uap), CT.byref(nap))
        return retval, lap.value, uap.value, nap.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciWaitForEncryptionChange(uint32 handle, uint8* enable);
    #########################################################
    # TestEngine::hciWaitForEncryptionChange
    @fntrace
    def hciWaitForEncryptionChange(self, handle, _enable=0):
        r"""
        Function wrapper for TestEngine::hciWaitForEncryptionChange().

        Python API:
            hciWaitForEncryptionChange(handle, _enable=0)
            returns retval, enable.value

        Generated from C API:

            Function :      int32 hciWaitForEncryptionChange(uint32 handle, uint8* enable)

            Parameters :    handle -
                                Handle to the device.

                            enable -
                                Defines whether encryption is enabled/disabled.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function can be used to wait for an 'Encryption Change'
                            event. An encryption change event will be generated following a
                            call to hciSetConnectionEncryption. The event will be generated
                            for both devices on the connection<br>
                            If the event is not triggered within 2 seconds of invocation it
                            will return failure.

        """
        self.TestEngineDLL.hciWaitForEncryptionChange.restype = CT.c_int32
        self.TestEngineDLL.hciWaitForEncryptionChange.argtypes = [CT.c_uint32, CT.c_void_p]
        enable = CT.c_uint8(_enable)
        retval = self.TestEngineDLL.hciWaitForEncryptionChange(handle, CT.byref(enable))
        return retval, enable.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciPinCodeRequestReply(uint32 handle, uint32 lap, uint8 uap, uint16 nap, uint8 pinCodeLength, uint8 pinCode[]);
    #########################################################
    # TestEngine::hciPinCodeRequestReply
    @fntrace
    def hciPinCodeRequestReply(self, handle, lap, uap, nap, pinCodeLength, pinCode):
        r"""
        Function wrapper for TestEngine::hciPinCodeRequestReply().

        Python API:
            hciPinCodeRequestReply(handle, lap, uap, nap, pinCodeLength, pinCode)
            returns retval

        Generated from C API:

            Function :      int32 hciPinCodeRequestReply(uint32 handle, uint32 lap, uint8 uap,
                                                            uint16 nap, uint8 pinCodeLength,
                                                            uint8* pinCode)

            Parameters :    handle -
                                Handle to the device.

                            lap -
                                The LAP portion of the Bluetooth address of the device for
                                which the PIN code is for.

                            uap -
                                The UAP portion of the Bluetooth address of the device for
                                which the PIN code is for.

                            nap -
                                The NAP portion of the Bluetooth address of the device for
                                which the PIN code is for.

                            pinCodeLength -
                                The length (in bytes) of the PIN code to be used. Range 1-16.

                            pinCode -
                                PIN code for the device that is to be connected. This should
                                be an array of bytes equal in length to the given
                                pinCodeLength parameter.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function can be used to respond to a 'PIN Code Request' event
                            (see hciWaitForPinCodeRequest) and specifies the PIN code to use
                            for a connection.

        """
        self.TestEngineDLL.hciPinCodeRequestReply.restype = CT.c_int32
        self.TestEngineDLL.hciPinCodeRequestReply.argtypes = [CT.c_uint32, CT.c_uint32, CT.c_uint8, CT.c_uint16, CT.c_uint8, CT.c_void_p]
        __pinCode = (CT.c_uint8 * len(pinCode))(*pinCode)
        retval = self.TestEngineDLL.hciPinCodeRequestReply(handle, lap, uap, nap, pinCodeLength, CT.byref(__pinCode))
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hciSetConnectionEncryption(uint32 handle, uint16 connHandle, uint8 enable);
    #########################################################
    # TestEngine::hciSetConnectionEncryption
    @fntrace
    def hciSetConnectionEncryption(self, handle, connHandle, enable):
        r"""
        Function wrapper for TestEngine::hciSetConnectionEncryption().

        Python API:
            hciSetConnectionEncryption(handle, connHandle, enable)
            returns retval

        Generated from C API:

            Function :      int32 hciSetConnectionEncryption(uint32 handle, uint16 connHandle,
                                                                uint8 enable)

            Parameters :    handle -
                                Handle to the device.

                            connHandle -
                                Connection handle to enable/disable link layer encryption.

                            enable -
                                Set to 1 to enable link layer encryption, otherwise set to 0.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function can be used to enable or disable link level
                            encryption.

        """
        self.TestEngineDLL.hciSetConnectionEncryption.restype = CT.c_int32
        self.TestEngineDLL.hciSetConnectionEncryption.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint8]
        retval = self.TestEngineDLL.hciSetConnectionEncryption(handle, connHandle, enable)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) vmWrite(uint32 handle, const uint16 data[]);
    #########################################################
    # TestEngine::vmWrite
    @fntrace
    def vmWrite(self, handle, data):
        r"""
        Function wrapper for TestEngine::vmWrite().

        Python API:
            vmWrite(handle, data)
            returns retval

        Generated from C API:

            Function :      int32 vmWrite(uint32 handle, const uint16 * data)

            Parameters :    handle -
                                Handle to the device.

                            data -
                                A pointer to an array of 16-bit unsigned integers. The first
                                element must be the length of the array.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to write to a VM application running on a
                            BlueCore IC.
                            <p>When using a host (HCI) transport, it uses BCSP channel 13,
                            which is tunnelled, using manufacturer specific extensions
                            through the transport.
                            <p>When using a debug transport, a BCCMD is used to write the
                            data to the VM. If the firmware does not support the relevant
                            BCCMD, TE_UNSUPPORTED_FUNCTION is returned.
                            <p>A successful status does NOT indicate that the packet has
                            reached BlueCore. The user should implement the appropriate hand
                            shaking both in the on-chip application and the application
                            sitting above TestEngine.
                            <p>This function is supported for BlueCore ICs only - see
                            teAppWrite for other IC types.

        """
        self.TestEngineDLL.vmWrite.restype = CT.c_int32
        self.TestEngineDLL.vmWrite.argtypes = [CT.c_uint32, CT.c_void_p]
        __data = (CT.c_uint16 * len(data))(*data)
        retval = self.TestEngineDLL.vmWrite(handle, CT.byref(__data))
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) vmRead(uint32 handle, uint16 data[], uint16 timeout);
    #########################################################
    # TestEngine::vmRead
    @fntrace
    def vmRead(self, handle, data, timeout):
        r"""
        Function wrapper for TestEngine::vmRead().

        Python API:
            vmRead(handle, data, timeout)
            returns retval

        Generated from C API:

            Function :      int32 vmRead(uint32 handle, uint16 * data, uint16 timeout)

            Parameters :    handle -
                                Handle to the device.

                            data -
                                A pointer to an array of 16-bit unsigned integers. The first
                                element must be the length of the array.

                            timeout -
                                A timeout, in ms, of the time to wait for the packet. A value
                                of 0 means the function will just poll.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to read any data returned from a VM
                            application running on a BlueCore IC. If no data has been read
                            within the timeout period, TE_ERROR will be returned and data[0]
                            will be set to zero.
                            <p>When using a host (HCI) transport, it uses BCSP channel 13,
                            which is tunnelled, using manufacturer specific extensions
                            through the transport. The function will fetch VM printf()
                            messages as well as other VM host messages.
                            <p>When using a debug transport, a BCCMD is used to read data
                            from the VM. If the firmware does not support the relevant
                            BCCMD, TE_UNSUPPORTED_FUNCTION is returned. VM printf() messages
                            are not fetched when using a debug transport.
                            <p>This function is supported for BlueCore ICs only - see
                            teAppWrite for other IC types.

        """
        self.TestEngineDLL.vmRead.restype = CT.c_int32
        self.TestEngineDLL.vmRead.argtypes = [CT.c_uint32, CT.c_void_p, CT.c_uint16]
        __data = (CT.c_uint16 * len(data))(*data)
        retval = self.TestEngineDLL.vmRead(handle, CT.byref(__data), timeout)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdSetSingleChan(uint32 handle, uint16 channel);
    #########################################################
    # TestEngine::bccmdSetSingleChan
    @fntrace
    def bccmdSetSingleChan(self, handle, channel):
        r"""
        Function wrapper for TestEngine::bccmdSetSingleChan().

        Python API:
            bccmdSetSingleChan(handle, channel)
            returns retval

        Generated from C API:

            Function :      int32 bccmdSetSingleChan(uint32 handle, uint16 channel)

            Parameters :    handle -
                                Handle to the device.

                            channel -
                                The single hop channel the radio should use.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to configure the BlueCore radio to transmit
                            and receive on a single hop channel. This disables hopping and
                            whitening.
                            <p>To re-enable hopping use the bccmdSetHoppingOn function.
                            <p>For more information see the BlueCore BCCMD commands document.

        """
        self.TestEngineDLL.bccmdSetSingleChan.restype = CT.c_int32
        self.TestEngineDLL.bccmdSetSingleChan.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.bccmdSetSingleChan(handle, channel)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdSetHoppingOn(uint32 handle);
    #########################################################
    # TestEngine::bccmdSetHoppingOn
    @fntrace
    def bccmdSetHoppingOn(self, handle):
        r"""
        Function wrapper for TestEngine::bccmdSetHoppingOn().

        Python API:
            bccmdSetHoppingOn(handle)
            returns retval

        Generated from C API:

            Function :      int32 bccmdSetHoppingOn(uint32 handle)

            Parameters :    handle -
                                Handle to the device.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to enable hopping after previously being
                            disabled as a result of calling bccmdSetSingleChan.
                            <p>For more information see the BlueCore BCCMD commands document.

        """
        self.TestEngineDLL.bccmdSetHoppingOn.restype = CT.c_int32
        self.TestEngineDLL.bccmdSetHoppingOn.argtypes = [CT.c_uint32]
        retval = self.TestEngineDLL.bccmdSetHoppingOn(handle)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdFmSwitchPower(uint32 handle, uint8 powerOn);
    #########################################################
    # TestEngine::bccmdFmSwitchPower
    @fntrace
    def bccmdFmSwitchPower(self, handle, powerOn):
        r"""
        Function wrapper for TestEngine::bccmdFmSwitchPower().

        Python API:
            bccmdFmSwitchPower(handle, powerOn)
            returns retval

        Generated from C API:

            Function :      int32 bccmdFmSwitchPower(uint32 handle, uint8 powerOn)

            Parameters :    handle -
                                Handle to the device.

                            powerOn -
                                Power state to set. Set to 1 to power on, 0 to power off.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to switch on power to the FM radio part of
                            the chip.
                            <p>If powerOn is set to 1, RDS reception is switched on after
                            the radio is powered up.
                            <p>NOTE: radiotest and FM control functions should be run either
                            side of a warm reset to ensure that the firmware is in the correct
                            state in each case.

            Example :

                uint32 iHandle = 0;
                int32 iTimeout = 0;

                do
                {
                    cout << "Trying to connect..." << endl;
                    iHandle = openTestEngine(BCSP, "COM1", 115200, 1000, 0);
                    iTimeout += 1000;
                }while(iHandle == 0 && iTimeout < MAX_TIMEOUT);

                if(iHandle != 0)
                {
                    do
                    {
                        int32 iSuccess = bccmdFmSwitchPower(iHandle, 1);
                        if(iSuccess != TE_OK)
                        {
                            cout << "bccmdFmSwitchPower error" << endl;
                            break;
                        }

                        uint8 status;
                        iSuccess = bccmdFmGetStatus(iHandle, &status);
                        if(iSuccess != TE_OK)
                        {
                            cout << "bccmdFmGetStatus error" << endl;
                            break;
                        }
                        // Check status as expected (tuner should be powered, but not tuned)
                        if((status & 0x07) != 1)
                        {
                            cout << "bccmdFmGetStatus indicates incorrect tuner state" << endl;
                            break;
                        }

                        iSuccess = bccmdFmSetFreq(iHandle, 103000);
                        if(iSuccess != TE_OK)
                        {
                            cout << "bccmdFmSetFreq error" << endl;
                            break;
                        }

                        int8 rssi;
                        iSuccess = bccmdFmGetRssi(iHandle, &rssi);
                        if(iSuccess != TE_OK)
                        {
                            cout << "bccmdFmGetRssi error" << endl;
                            break;
                        }
                        cout << "RSSI = " << (int16)rssi << endl;

                        int16 snr;
                        iSuccess = bccmdFmGetSnr(iHandle, &snr);
                        if(iSuccess != TE_OK)
                        {
                            cout << "bccmdFmGetSnr error" << endl;
                            break;
                        }
                        cout << "SNR = " << snr << endl;

                        int16 offset;
                        iSuccess = bccmdFmGetIfOffset(iHandle, &offset);
                        if(iSuccess != TE_OK)
                        {
                            cout << "bccmdFmGetIfOffset error" << endl;
                            break;
                        }
                        cout << "IF offset = " << offset << endl;

                        iSuccess = bccmdFmSetupAudio(iHandle, 0, 16, 2);
                        if(iSuccess != TE_OK)
                        {
                            cout << "bccmdFmSetupAudio error" << endl;
                            break;
                        }

                        // TEST AUDIO HERE

                        // If using BC7FM or later, should disconnect audio
                        iSuccess = bccmdDisconnectAudio(iHandle);
                        if(iSuccess != TE_OK)
                        {
                            cout << "bccmdDisconnectAudio error" << endl;
                            break;
                        }

                        // May need a delay to ensure the RDS PI has been received
                        Sleep(3000);
                        uint8 matched;
                        iSuccess = bccmdFmVerifyRDSPi(iHandle, 0xCF86, 3000, &matched);
                        if(iSuccess != TE_OK)
                        {
                            cout << "bccmdFmVerifyRDSPi error" << endl;
                            break;
                        }
                        if(matched != 0x01)
                        {
                            cout << "RDS PI did not match within the timeout period" << endl;
                        }
                    }
                    while(0);

                    closeTestEngine(iHandle);
                }

        """
        self.TestEngineDLL.bccmdFmSwitchPower.restype = CT.c_int32
        self.TestEngineDLL.bccmdFmSwitchPower.argtypes = [CT.c_uint32, CT.c_uint8]
        retval = self.TestEngineDLL.bccmdFmSwitchPower(handle, powerOn)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdFmSetFreq(uint32 handle, uint32 freqKHz);
    #########################################################
    # TestEngine::bccmdFmSetFreq
    @fntrace
    def bccmdFmSetFreq(self, handle, freqKHz):
        r"""
        Function wrapper for TestEngine::bccmdFmSetFreq().

        Python API:
            bccmdFmSetFreq(handle, freqKHz)
            returns retval

        Generated from C API:

            Function :      int32 bccmdFmSetFreq(uint32 handle, uint32 freqKHz)

            Parameters :    handle -
                                Handle to the device.

                            freqKHz -
                                Frequency to set in KHz.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to tune the FM radio part of the chip to
                            the desired frequency.
                            <p>Preconditions for success are:
                                <ol>
                                    <li> Valid handle
                                    <li> Valid frequency (freqKHz in range)
                                </ol>

        """
        self.TestEngineDLL.bccmdFmSetFreq.restype = CT.c_int32
        self.TestEngineDLL.bccmdFmSetFreq.argtypes = [CT.c_uint32, CT.c_uint32]
        retval = self.TestEngineDLL.bccmdFmSetFreq(handle, freqKHz)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdFmGetRssi(uint32 handle, int8* rssi);
    #########################################################
    # TestEngine::bccmdFmGetRssi
    @fntrace
    def bccmdFmGetRssi(self, handle, _rssi=0):
        r"""
        Function wrapper for TestEngine::bccmdFmGetRssi().

        Python API:
            bccmdFmGetRssi(handle, _rssi=0)
            returns retval, rssi.value

        Generated from C API:

            Function :      int32 bccmdFmGetRssi(uint32 handle, int8* rssi)

            Parameters :    handle -
                                Handle to the device.

                            rssi -
                                The measured RSSI value will be stored at this address

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function gets an RSSI (Received Signal Strength Indication)
                            measurement from the radio, measured in dBm.
                            <p>Preconditions for success are:
                                <ol>
                                    <li> Valid handle
                                    <li> Radio powered up and tuned to a valid frequency
                                </ol>

        """
        self.TestEngineDLL.bccmdFmGetRssi.restype = CT.c_int32
        self.TestEngineDLL.bccmdFmGetRssi.argtypes = [CT.c_uint32, CT.c_void_p]
        rssi = CT.c_int8(_rssi)
        retval = self.TestEngineDLL.bccmdFmGetRssi(handle, CT.byref(rssi))
        return retval, rssi.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdFmGetSnr(uint32 handle, int16* snr);
    #########################################################
    # TestEngine::bccmdFmGetSnr
    @fntrace
    def bccmdFmGetSnr(self, handle, _snr=0):
        r"""
        Function wrapper for TestEngine::bccmdFmGetSnr().

        Python API:
            bccmdFmGetSnr(handle, _snr=0)
            returns retval, snr.value

        Generated from C API:

            Function :      int32 bccmdFmGetSnr(uint32 handle, int16* snr)

            Parameters :    handle -
                                Handle to the device.

                            snr -
                                The measured SNR value will be stored at this address

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function gets a SNR (Signal to Noise Ratio) measurement
                            from the radio, measured in dB.
                            <p>Preconditions for success are:
                                <ol>
                                    <li> Valid handle
                                    <li> Radio powered up and tuned to a valid frequency
                                </ol>

        """
        self.TestEngineDLL.bccmdFmGetSnr.restype = CT.c_int32
        self.TestEngineDLL.bccmdFmGetSnr.argtypes = [CT.c_uint32, CT.c_void_p]
        snr = CT.c_int16(_snr)
        retval = self.TestEngineDLL.bccmdFmGetSnr(handle, CT.byref(snr))
        return retval, snr.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdFmGetIfOffset(uint32 handle, int16* offset);
    #########################################################
    # TestEngine::bccmdFmGetIfOffset
    @fntrace
    def bccmdFmGetIfOffset(self, handle, _offset=0):
        r"""
        Function wrapper for TestEngine::bccmdFmGetIfOffset().

        Python API:
            bccmdFmGetIfOffset(handle, _offset=0)
            returns retval, offset.value

        Generated from C API:

            Function :      int32 bccmdFmGetIfOffset(uint32 handle, int16* offset)

            Parameters :    handle -
                                Handle to the device.

                            offset -
                                The measured IF offset value will be stored at this address

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function gets an IF (Intermediate Frequency) offset
                            measurement from the radio, measured in KHz.
                            <p>Preconditions for success are:
                                <ol>
                                    <li> Valid handle
                                    <li> Radio powered up and tuned to a valid frequency
                                </ol>

        """
        self.TestEngineDLL.bccmdFmGetIfOffset.restype = CT.c_int32
        self.TestEngineDLL.bccmdFmGetIfOffset.argtypes = [CT.c_uint32, CT.c_void_p]
        offset = CT.c_int16(_offset)
        retval = self.TestEngineDLL.bccmdFmGetIfOffset(handle, CT.byref(offset))
        return retval, offset.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdFmGetStatus(uint32 handle, uint8* status);
    #########################################################
    # TestEngine::bccmdFmGetStatus
    @fntrace
    def bccmdFmGetStatus(self, handle, _status=0):
        r"""
        Function wrapper for TestEngine::bccmdFmGetStatus().

        Python API:
            bccmdFmGetStatus(handle, _status=0)
            returns retval, status.value

        Generated from C API:

            Function :      int32 bccmdFmGetStatus(uint32 handle, uint8* status)

            Parameters :    handle -
                                Handle to the device.

                            status -
                                The fetched status value will be stored at this address.
                                <p>The status is a bit field with the following format:
                                <ol>
                                    <li>bit[0] = TS[0] (tuner state bit 0 *)
                                    <li>bit[1] = TS[1] (tuner state bit 1 *)
                                    <li>bit[2] = TS[2] (tuner state bit 2 *)
                                    <li>bit[3] = RDA   (RDS Data Available indicator)
                                    <li>bit[4] = STR   (Stereo signal detected indicator)
                                    <li>bit[5] = IFR   (IF frequency in range indicator)
                                    <li>bit[6] = RSR   (RSSI in range indicator)
                                    <li>bit[7] = Not used
                                </ol>
                                <p>Values for TS indicate:
                                <ol>
                                    <li>0 = Radio is not powered up, or has been reset
                                    <li>1 = Radio is not tuned to a valid frequency
                                    <li>2 = A tuning process is in progress
                                    <li>3 = Radio is tuned but RDS Rx. is powered down
                                    <li>4 = Radio is tuned but no RDS is present
                                    <li>5 = Radio is tuned but RDS is not synchronised
                                    <li>6 = Radio is tuned and RDS is being received
                                </ol>

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function reads the status register of the FM radio.

        """
        self.TestEngineDLL.bccmdFmGetStatus.restype = CT.c_int32
        self.TestEngineDLL.bccmdFmGetStatus.argtypes = [CT.c_uint32, CT.c_void_p]
        status = CT.c_uint8(_status)
        retval = self.TestEngineDLL.bccmdFmGetStatus(handle, CT.byref(status))
        return retval, status.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdFmSetupAudio(uint32 handle, uint8 route, uint8 gain, uint8 channel);
    #########################################################
    # TestEngine::bccmdFmSetupAudio
    @fntrace
    def bccmdFmSetupAudio(self, handle, route, gain, channel):
        r"""
        Function wrapper for TestEngine::bccmdFmSetupAudio().

        Python API:
            bccmdFmSetupAudio(handle, route, gain, channel)
            returns retval

        Generated from C API:

            Function :      int32 bccmdFmSetupAudio(uint32 handle, uint8 route, uint8 gain,
                                                    uint8 channel)

            Parameters :    handle -
                                Handle to the device.

                            route -
                                Audio output route for FM RX, where:
                                <table>
                                    <tr><td>0 = Route audio to CODEC
                                    <tr><td>1 = Route audio to PCM port 1 (PCM Mode)
                                    <tr><td>2 = Route audio to PCM port 2 (PCM Mode)
                                    <tr><td>3 = Route audio to PCM port 1 (I2S Master Mode)
                                    <tr><td>4 = Route audio to PCM port 2 (I2S Master Mode)
                                </table>
                                <p>Note: Default voice slots are used in each case

                            gain -
                                Audio gain value to set, between 0 and maxValue, where
                                maxValue depends on the "route" parameter as follows:
                                <table>
                                    <tr><td>route = 0, maxValue = 22
                                    <tr><td>route = 1-4, maxValue = 7 (BC5FM only)
                                </table>
                                <p>The gain parameter for PCM audio is ignored for chips
                                other than BC5FM.

                            channel -
                                Channel selection for CODEC routing, where:
                                <table>
                                    <tr><td>0 = Left only
                                    <tr><td>1 = Right only
                                    <tr><td>2 = Left and Right channels
                                </table>
                                <p>If a single channel is selected, the other channel will be
                                muted.
                                <p>Channel selection is for CODEC output only (PCM outputs
                                always use both channels).

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to setup the audio routing and gain for
                            the FM RX output.

        """
        self.TestEngineDLL.bccmdFmSetupAudio.restype = CT.c_int32
        self.TestEngineDLL.bccmdFmSetupAudio.argtypes = [CT.c_uint32, CT.c_uint8, CT.c_uint8, CT.c_uint8]
        retval = self.TestEngineDLL.bccmdFmSetupAudio(handle, route, gain, channel)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdFmVerifyRDSPi(uint32 handle, uint16 pi, uint16 timeoutMs, uint8* matched);
    #########################################################
    # TestEngine::bccmdFmVerifyRDSPi
    @fntrace
    def bccmdFmVerifyRDSPi(self, handle, pi, timeoutMs, _matched=0):
        r"""
        Function wrapper for TestEngine::bccmdFmVerifyRDSPi().

        Python API:
            bccmdFmVerifyRDSPi(handle, pi, timeoutMs, _matched=0)
            returns retval, matched.value

        Generated from C API:

            Function :      int32 bccmdFmVerifyRDSPi(uint32 handle, uint16 pi, uint16 timeoutMs,
                                                    uint8* matched)

            Parameters :    handle -
                                Handle to the device.

                            pi -
                                PI (Program Identity) value to match against.

                            timeoutMs -
                                Timeout value in milliseconds. The function will poll for a
                                matching PI value for the duration of the timeout before
                                returning.

                            matched -
                                The status of the match check will be stored at this address,
                                where 0 = not matched, and 1 = matched.

          Returns :         <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to verify that an expected PI value has
                            been received and decoded by the FM radio.
                            <p>Preconditions for success are:
                                <ol>
                                    <li> Valid handle
                                    <li> Radio powered up and tuned to a valid frequency
                                    <li> RDS data being received
                                </ol>

        """
        self.TestEngineDLL.bccmdFmVerifyRDSPi.restype = CT.c_int32
        self.TestEngineDLL.bccmdFmVerifyRDSPi.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16, CT.c_void_p]
        matched = CT.c_uint8(_matched)
        retval = self.TestEngineDLL.bccmdFmVerifyRDSPi(handle, pi, timeoutMs, CT.byref(matched))
        return retval, matched.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdFmTxSwitchPower(uint32 handle, uint8 powerOn);
    #########################################################
    # TestEngine::bccmdFmTxSwitchPower
    @fntrace
    def bccmdFmTxSwitchPower(self, handle, powerOn):
        r"""
        Function wrapper for TestEngine::bccmdFmTxSwitchPower().

        Python API:
            bccmdFmTxSwitchPower(handle, powerOn)
            returns retval

        Generated from C API:

            Function :      int32 bccmdFmTxSwitchPower(uint32 handle, uint8 powerOn)

            Parameters :    handle -
                                Handle to the device.

                            powerOn -
                                Power state to set. Set to 1 to power on, 0 to power off.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to switch on power to the FM TX part of
                            the chip.
                            <p>If powerOn is set to 1, FM TX with RDS is switched on.
                            <p>NOTE: FM TX and RX testing cannot run in parallel.

            Example :

                uint32 iHandle = 0;
                int32 iTimeout = 0;

                do
                {
                    cout << "Trying to connect..." << endl;
                    iHandle = openTestEngine(BCSP, "COM1", 115200, 1000, 0);
                    iTimeout += 1000;
                }while(iHandle == 0 && iTimeout < MAX_TIMEOUT);

                if(iHandle != 0)
                {
                    do
                    {
                        int32 iSuccess = bccmdFmTxSwitchPower(iHandle, 1);
                        if(iSuccess != TE_OK)
                        {
                            cout << "bccmdFmTxSwitchPower error" << endl;
                            break;
                        }

                        iSuccess = bccmdFmTxSetFreq(iHandle, 103000);
                        if(iSuccess != TE_OK)
                        {
                            cout << "bccmdFmTxSetFreq error" << endl;
                            break;
                        }

                        iSuccess = bccmdFmTxSetPowerLevel(iHandle, -1);
                        if(iSuccess != TE_OK)
                        {
                            cout << "bccmdFmTxSetPowerLevel error" << endl;
                            break;
                        }

                        iSuccess = bccmdFmTxSetupAudio(iHandle, 0, 10);
                        if(iSuccess != TE_OK)
                        {
                            cout << "bccmdFmTxSetupAudio error" << endl;
                            break;
                        }

                        // TEST AUDIO HERE

                        iSuccess = bccmdDisconnectAudio(iHandle);
                        if(iSuccess != TE_OK)
                        {
                            cout << "bccmdDisconnectAudio error" << endl;
                            break;
                        }
                    }
                    while(0);

                    closeTestEngine(iHandle);
                }

        """
        self.TestEngineDLL.bccmdFmTxSwitchPower.restype = CT.c_int32
        self.TestEngineDLL.bccmdFmTxSwitchPower.argtypes = [CT.c_uint32, CT.c_uint8]
        retval = self.TestEngineDLL.bccmdFmTxSwitchPower(handle, powerOn)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdFmTxSetFreq(uint32 handle, uint32 freqKHz);
    #########################################################
    # TestEngine::bccmdFmTxSetFreq
    @fntrace
    def bccmdFmTxSetFreq(self, handle, freqKHz):
        r"""
        Function wrapper for TestEngine::bccmdFmTxSetFreq().

        Python API:
            bccmdFmTxSetFreq(handle, freqKHz)
            returns retval

        Generated from C API:

            Function :      int32 bccmdFmTxSetFreq(uint32 handle, uint32 freqKHz)

            Parameters :    handle -
                                Handle to the device.

                            freqKHz -
                                Frequency to set in KHz.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to tune the FM TX part of the chip to
                            the desired frequency.
                            <p>Preconditions for success are:
                                <ol>
                                    <li> Valid handle
                                    <li> Valid frequency (freqKHz in range)
                                </ol>

        """
        self.TestEngineDLL.bccmdFmTxSetFreq.restype = CT.c_int32
        self.TestEngineDLL.bccmdFmTxSetFreq.argtypes = [CT.c_uint32, CT.c_uint32]
        retval = self.TestEngineDLL.bccmdFmTxSetFreq(handle, freqKHz)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdFmTxSetPowerLevel(uint32 handle, int16 powerLevel);
    #########################################################
    # TestEngine::bccmdFmTxSetPowerLevel
    @fntrace
    def bccmdFmTxSetPowerLevel(self, handle, powerLevel):
        r"""
        Function wrapper for TestEngine::bccmdFmTxSetPowerLevel().

        Python API:
            bccmdFmTxSetPowerLevel(handle, powerLevel)
            returns retval

        Generated from C API:

            Function :      int32 bccmdFmTxSetPowerLevel(uint32 handle, int16 powerLevel)

            Parameters :    handle -
                                Handle to the device.

                            powerLevel -
                                FM power output value to set, between -24 and 0, where 0
                                gives the highest output power. 0 is the default value.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to set the FM power level for FM TX.

        """
        self.TestEngineDLL.bccmdFmTxSetPowerLevel.restype = CT.c_int32
        self.TestEngineDLL.bccmdFmTxSetPowerLevel.argtypes = [CT.c_uint32, CT.c_int16]
        retval = self.TestEngineDLL.bccmdFmTxSetPowerLevel(handle, powerLevel)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdFmTxSetupAudio(uint32 handle, uint8 route, uint16 audioGain);
    #########################################################
    # TestEngine::bccmdFmTxSetupAudio
    @fntrace
    def bccmdFmTxSetupAudio(self, handle, route, audioGain):
        r"""
        Function wrapper for TestEngine::bccmdFmTxSetupAudio().

        Python API:
            bccmdFmTxSetupAudio(handle, route, audioGain)
            returns retval

        Generated from C API:

            Function :      int32 bccmdFmTxSetupAudio(uint32 handle, uint8 route,
                                                        uint16 audioGain)

            Parameters :    handle -
                                Handle to the device.

                            route -
                                Audio output route for FM TX, where:
                                <table>
                                    <tr><td>0 = Route audio from CODEC
                                    <tr><td>1 = Route audio from PCM port 1 (PCM Mode)
                                    <tr><td>2 = Route audio from PCM port 2 (PCM Mode)
                                    <tr><td>3 = Route audio from PCM port 1 (I2S Master Mode)
                                    <tr><td>4 = Route audio from PCM port 2 (I2S Master Mode)
                                </table>
                                <p>Note: Default voice slots are used in each case

                            audioGain -
                                Analogue audio input gain value to set, between 0 and 22.
                                <p>The following key-points are given for approximately 80%
                                full-scale on the audio input:
                                <table>
                                    <tr><td>100mV input = 12
                                    <tr><td>200mV input = 10
                                    <tr><td>300mV input = 9
                                </table>

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to setup the audio routing and input level
                            for FM TX.

        """
        self.TestEngineDLL.bccmdFmTxSetupAudio.restype = CT.c_int32
        self.TestEngineDLL.bccmdFmTxSetupAudio.argtypes = [CT.c_uint32, CT.c_uint8, CT.c_uint16]
        retval = self.TestEngineDLL.bccmdFmTxSetupAudio(handle, route, audioGain)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdDisconnectAudio(uint32 handle);
    #########################################################
    # TestEngine::bccmdDisconnectAudio
    @fntrace
    def bccmdDisconnectAudio(self, handle):
        r"""
        Function wrapper for TestEngine::bccmdDisconnectAudio().

        Python API:
            bccmdDisconnectAudio(handle)
            returns retval

        Generated from C API:

            Function :      int32 bccmdDisconnectAudio(uint32 handle)

            Parameters :    handle -
                                Handle to the device.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to disconnect any connected audio streams,
                            such as may be created when calling bccmdFmSetupAudio with BC7
                            and later chips.
                            <p>Connections are disconnected, and the stream endpoints are
                            closed.

        """
        self.TestEngineDLL.bccmdDisconnectAudio.restype = CT.c_int32
        self.TestEngineDLL.bccmdDisconnectAudio.argtypes = [CT.c_uint32]
        retval = self.TestEngineDLL.bccmdDisconnectAudio(handle)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdAudioGetSource(uint32 handle, uint16 device, uint16 iface, uint16 channel, uint16* sid);
    #########################################################
    # TestEngine::bccmdAudioGetSource
    @fntrace
    def bccmdAudioGetSource(self, handle, device, iface, channel, _sid=0):
        r"""
        Function wrapper for TestEngine::bccmdAudioGetSource().

        Python API:
            bccmdAudioGetSource(handle, device, iface, channel, _sid=0)
            returns retval, sid.value

        Generated from C API:

            Function :      int32 bccmdAudioGetSource(uint32 handle, uint16 device,
                                                      uint16 iface, uint16 channel,
                                                      uint16* sid)

            Parameters :    handle -
                                Handle to the device.

                            device -
                                The stream device identifier, where:
                                <table>
                                    <tr><td>1 = PCM
                                    <tr><td>2 = I2S
                                    <tr><td>3 = CODEC
                                    <tr><td>4 = FM
                                    <tr><td>5 = SPDIF
                                    <tr><td>6 = DIGITAL_MIC
                                </table>

                            iface -
                                The hardware interface instance, e.g. 0, 1, etc.

                            channel -
                                Channel selection for CODEC device, where:
                                <table>
                                    <tr><td>0 = Channel A
                                    <tr><td>1 = Channel B
                                    <tr><td>2 = Channels A and B
                                </table>
                                <p>If a single channel is selected, the other channel will be
                                muted.

                            sid -
                                Pointer to where the source ID will be stored.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to get the ID for an audio stream endpoint
                            source device. This sid can be used when configuring audio
                            configuration parameters using bccmdAudioConfigure.

        """
        self.TestEngineDLL.bccmdAudioGetSource.restype = CT.c_int32
        self.TestEngineDLL.bccmdAudioGetSource.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16, CT.c_uint16, CT.c_void_p]
        sid = CT.c_uint16(_sid)
        retval = self.TestEngineDLL.bccmdAudioGetSource(handle, device, iface, channel, CT.byref(sid))
        return retval, sid.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdAudioGetSink(uint32 handle, uint16 device, uint16 iface, uint16 channel, uint16* sid);
    #########################################################
    # TestEngine::bccmdAudioGetSink
    @fntrace
    def bccmdAudioGetSink(self, handle, device, iface, channel, _sid=0):
        r"""
        Function wrapper for TestEngine::bccmdAudioGetSink().

        Python API:
            bccmdAudioGetSink(handle, device, iface, channel, _sid=0)
            returns retval, sid.value

        Generated from C API:

            Function :      int32 bccmdAudioGetSink(uint32 handle, uint16 device,
                                                    uint16 iface, uint16 channel,
                                                    uint16* sid)

            Parameters :    handle -
                                Handle to the device.

                            device -
                                The stream device identifier, where:
                                <table>
                                    <tr><td>1 = PCM
                                    <tr><td>2 = I2S
                                    <tr><td>3 = CODEC
                                    <tr><td>4 = FM
                                    <tr><td>5 = SPDIF
                                    <tr><td>6 = DIGITAL_MIC
                                </table>

                            iface -
                                The hardware interface instance, e.g. 0, 1, etc.

                            channel -
                                Channel selection for CODEC device, where:
                                <table>
                                    <tr><td>0 = Channel A
                                    <tr><td>1 = Channel B
                                    <tr><td>2 = Channels A and B
                                </table>
                                <p>If a single channel is selected, the other channel will be
                                muted.

                            sid -
                                Pointer to where the sink ID will be stored.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to get the ID for an audio stream endpoint
                            sink device. This sid can be used when configuring audio
                            configuration parameters using bccmdAudioConfigure.

        """
        self.TestEngineDLL.bccmdAudioGetSink.restype = CT.c_int32
        self.TestEngineDLL.bccmdAudioGetSink.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16, CT.c_uint16, CT.c_void_p]
        sid = CT.c_uint16(_sid)
        retval = self.TestEngineDLL.bccmdAudioGetSink(handle, device, iface, channel, CT.byref(sid))
        return retval, sid.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdAudioConfigure(uint32 handle, uint16 sid, uint16 key, uint32 value);
    #########################################################
    # TestEngine::bccmdAudioConfigure
    @fntrace
    def bccmdAudioConfigure(self, handle, sid, key, value):
        r"""
        Function wrapper for TestEngine::bccmdAudioConfigure().

        Python API:
            bccmdAudioConfigure(handle, sid, key, value)
            returns retval

        Generated from C API:

            Function :      int32 bccmdAudioConfigure(uint32 handle, uint16 sid, uint16 key,
                                                      uint32 value)

            Parameters :    handle -
                                Handle to the device.

                            sid -
                                Source/Sink ID to configure.

                            key -
                                Audio configuration key to set.

                            value -
                                The value to set for the audio configuration key.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to set an audio configuration key value
                            for a given audio source / sink. The sid parameter can be obtained
                            using bccmdAudioGetSource / bccmdAudioGetSink.

        """
        self.TestEngineDLL.bccmdAudioConfigure.restype = CT.c_int32
        self.TestEngineDLL.bccmdAudioConfigure.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16, CT.c_uint32]
        retval = self.TestEngineDLL.bccmdAudioConfigure(handle, sid, key, value)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdDirectChargerPsuTrim(uint32 handle, uint16 trim);
    #########################################################
    # TestEngine::bccmdDirectChargerPsuTrim
    @fntrace
    def bccmdDirectChargerPsuTrim(self, handle, trim):
        r"""
        Function wrapper for TestEngine::bccmdDirectChargerPsuTrim().

        Python API:
            bccmdDirectChargerPsuTrim(handle, trim)
            returns retval

        Generated from C API:

            Function :      int32 bccmdDirectChargerPsuTrim(uint32 handle, uint16 trim)

            Parameters :    handle -
                                Handle to the device.

                            trim -
                                Charger trim value to be written.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to modify the charger trim register in
                            order to alter the behaviour of BlueCore's built-in battery
                            charger. The function is for use initially with BC4 and BC5
                            devices, in order to bypass routines in the firmware which cause
                            a delay in setting the register value.

        """
        self.TestEngineDLL.bccmdDirectChargerPsuTrim.restype = CT.c_int32
        self.TestEngineDLL.bccmdDirectChargerPsuTrim.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.bccmdDirectChargerPsuTrim(handle, trim)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teSupportsHq(uint32 handle, uint8* hqSupported);
    #########################################################
    # TestEngine::teSupportsHq
    @fntrace
    def teSupportsHq(self, handle, _hqSupported=0):
        r"""
        Function wrapper for TestEngine::teSupportsHq().

        Python API:
            teSupportsHq(handle, _hqSupported=0)
            returns retval, hqSupported.value

        Generated from C API:

            Function :      int32 teSupportsHq(uint32 handle, uint8* hqSupported)

            Parameters :    handle -
                                Handle to the device.

                            hqSupported -
                                If HQCMDs are supported by the transport used for the
                                specified handle, this value is set to 1. Otherwise the value
                                is set to 0.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                            </table>

            Description :   This function gets the status of HQCMD support. Host transports
                            such as BCSP/H4/H4DS/H5 (UART) and USB support HQCMDs. SPI
                            transports may support HQCMD if the BlueCore firmware supports HQ
                            over SPI.
                            <p>Certain radiotests and other BCCMDs generate HQCMDs. This
                            function can be used to determine whether using such commands
                            is appropriate for the transport used.
                            <p>All functions with names beginning hq... require HQCMD support.
                            The following radiotest / bccmd functions also require HQ support:
                            <table>
                                <tr><td>radiotestRxstart2
                                <tr><td>radiotestBer1
                                <tr><td>radiotestBer2
                                <tr><td>radiotestBerLoopback
                                <tr><td>radiotestRadioStatus
                                <tr><td>radiotestRadioStatusArray
                                <tr><td>radiotestRxdata1
                                <tr><td>radiotestRxdata2
                                <tr><td>radiotestRxLoopback (deprecated)
                                <tr><td>radiotestSettle (deprecated)
                                <tr><td>bccmdProvokeFault
                            </table>
                            <p>See the BCCMD specification document for further information.

        """
        self.TestEngineDLL.teSupportsHq.restype = CT.c_int32
        self.TestEngineDLL.teSupportsHq.argtypes = [CT.c_uint32, CT.c_void_p]
        hqSupported = CT.c_uint8(_hqSupported)
        retval = self.TestEngineDLL.teSupportsHq(handle, CT.byref(hqSupported))
        return retval, hqSupported.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdSetAuxDac(uint32 handle, uint8 enable, uint8 level);
    #########################################################
    # TestEngine::bccmdSetAuxDac
    @fntrace
    def bccmdSetAuxDac(self, handle, enable, level):
        r"""
        Function wrapper for TestEngine::bccmdSetAuxDac().

        Python API:
            bccmdSetAuxDac(handle, enable, level)
            returns retval

        Generated from C API:

            Function :      int32 bccmdSetAuxDac(uint32 handle, uint8 enable, uint8 level)

            Parameters :    handle -
                                Handle to the device.

                            enable -
                                Flag defining whether the aux dac output should be enabled or
                                disabled. Set the value to 1 to enable, 0 to disable.

                            level -
                                Value to write to the AUX_DAC output (if enable = 1).
                                Range is 0...255.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to control the AUX_DAC line on BlueCore.
                            The line can be enabled with a given level written, or be
                            disabled.

        """
        self.TestEngineDLL.bccmdSetAuxDac.restype = CT.c_int32
        self.TestEngineDLL.bccmdSetAuxDac.argtypes = [CT.c_uint32, CT.c_uint8, CT.c_uint8]
        retval = self.TestEngineDLL.bccmdSetAuxDac(handle, enable, level)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdSetMicBiasIf(uint32 handle, uint8 instance, uint8 enable, uint8 voltage);
    #########################################################
    # TestEngine::bccmdSetMicBiasIf
    @fntrace
    def bccmdSetMicBiasIf(self, handle, instance, enable, voltage):
        r"""
        Function wrapper for TestEngine::bccmdSetMicBiasIf().

        Python API:
            bccmdSetMicBiasIf(handle, instance, enable, voltage)
            returns retval

        Generated from C API:

        Function :          int32 bccmdSetMicBiasIf(uint32 handle, uint8 instance, uint8 enable,
                                                      uint8 voltage);

            Parameters :    handle -
                                Handle to the device.

                            instance -
                                Selects the mic bias line to set (zero indexed).

                            enable -
                                Value indicating whether the mic bias output should be enabled
                                or disabled. Set the value to 1 to enable, 0 to disable.

                            voltage -
                                Value which maps to an approximate output voltage for the line,
                                e.g.:
                                <table>
                                    <tr><td>0 = 1.80 V
                                    <tr><td>1 = 2.60 V
                                </table>

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to control MIC bias lines for some BC7
                            (those supporting multiple MIC_BIAS outputs) and later BlueCore
                            chips. Use bccmdSetMicBias for older chips.

        """
        self.TestEngineDLL.bccmdSetMicBiasIf.restype = CT.c_int32
        self.TestEngineDLL.bccmdSetMicBiasIf.argtypes = [CT.c_uint32, CT.c_uint8, CT.c_uint8, CT.c_uint8]
        retval = self.TestEngineDLL.bccmdSetMicBiasIf(handle, instance, enable, voltage)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdSetMicBias(uint32 handle, uint8 enable, uint8 voltage, uint8 current, uint8 enableLowPowerMode);
    #########################################################
    # TestEngine::bccmdSetMicBias
    @fntrace
    def bccmdSetMicBias(self, handle, enable, voltage, current, enableLowPowerMode):
        r"""
        Function wrapper for TestEngine::bccmdSetMicBias().

        Python API:
            bccmdSetMicBias(handle, enable, voltage, current, enableLowPowerMode)
            returns retval

        Generated from C API:

            Function :      int32 bccmdSetMicBias(uint32 handle, uint8 enable, uint8 voltage,
                                                    uint8 current, uint8 enableLowPowerMode)

            Parameters :    handle -
                                Handle to the device.

                            enable -
                                Flag defining whether the mic bias output should be enabled
                                or disabled. Set the value to 1 to enable, 0 to disable.

                            voltage -
                                Value which maps to a voltage set for the line, where for
                                BC5-MM, the mapping is:
                                <table>
                                    <tr><td>0 = 1.72 V
                                    <tr><td>1 = 1.77 V
                                    <tr><td>2 = 1.83 V
                                    <tr><td>3 = 1.89 V
                                    <tr><td>4 = 1.97 V
                                    <tr><td>5 = 2.03 V
                                    <tr><td>6 = 2.12 V
                                    <tr><td>7 = 2.20 V
                                    <tr><td>8 = 2.34 V
                                    <tr><td>9 = 2.44 V
                                    <tr><td>10 = 2.58 V
                                    <tr><td>11 = 2.71 V
                                    <tr><td>12 = 2.92 V
                                    <tr><td>13 = 3.10 V
                                    <tr><td>14 = 3.34 V
                                    <tr><td>15 = 3.60 V
                                </table>

                            current -
                                Value which maps to a current set for the line, where for
                                BC5-MM, the mapping is:
                                <table>
                                    <tr><td>0 = 0.32 mA
                                    <tr><td>1 = 0.40 mA
                                    <tr><td>2 = 0.48 mA
                                    <tr><td>3 = 0.56 mA
                                    <tr><td>4 = 0.64 mA
                                    <tr><td>5 = 0.72 mA
                                    <tr><td>6 = 0.80 mA
                                    <tr><td>7 = 0.88 mA
                                    <tr><td>8 = 0.97 mA
                                    <tr><td>9 = 1.05 mA
                                    <tr><td>10 = 1.13 mA
                                    <tr><td>11 = 1.21 mA
                                    <tr><td>12 = 1.29 mA
                                    <tr><td>13 = 1.37 mA
                                    <tr><td>14 = 1.45 mA
                                    <tr><td>15 = 1.53 mA
                                </table>

                            enableLowPowerMode -
                                Flag defining whether the mic bias circuit low power mode
                                is enabled or disabled. Set the value to 1 to enable, 0 to
                                disable.
                                <p>NOTE: Whilst saving power, the low power mode does result
                                in greater noise on the line. For most production test
                                purposes, this should be irrelevant.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to control the MIC_BIAS line on BlueCore.
                            The line can be enabled with specified voltage and current levels,
                            or be disabled. The built in low power mode for mic_bias can also
                            be controlled.
                            <p>This function and the associated BCCMD in the firmware has
                            been implemented initially for BC5-MM.
                            <p>For BC7 chips with multiple MIC_BIAS lines and later chips,
                            use bccmdSetMicBiasIf.

        """
        self.TestEngineDLL.bccmdSetMicBias.restype = CT.c_int32
        self.TestEngineDLL.bccmdSetMicBias.argtypes = [CT.c_uint32, CT.c_uint8, CT.c_uint8, CT.c_uint8, CT.c_uint8]
        retval = self.TestEngineDLL.bccmdSetMicBias(handle, enable, voltage, current, enableLowPowerMode)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teGetAvailableSpiPorts(uint16* maxLen, char* ports, char* trans, uint16* count);
    #########################################################
    # TestEngine::teGetAvailableSpiPorts
    @fntrace
    def teGetAvailableSpiPorts(self, _maxLen=0, _ports='', _trans='', _count=0):
        r"""
        Function wrapper for TestEngine::teGetAvailableSpiPorts().

        Python API:
            teGetAvailableSpiPorts(_maxLen=0, _ports='', _trans='', _count=0)
            returns retval, maxLen.value, ports.value, trans.value, count.value

        Generated from C API:

            Function :      int32 teGetAvailableSpiPorts(uint16* maxLen, char* ports,
                                                            char* trans, uint16* count)

            Parameters :    maxLen -
                                Size of the arrays pointed to by the ports and trans
                                parameters. If this parameter indicates that the ports or
                                trans arrays are too small to store the complete strings,
                                then the value is set to the size required (and error is
                                returned).
                                If any other error occurs, this value is set to zero.

                            ports -
                                Pointer to an array of ASCII chars where the comma separated
                                list of available SPI port names will be stored. These are
                                readable names which could be used for user selection.

                            trans -
                                Pointer to an array of ASCII chars where the comma separated
                                list of SPI transport options for each of the available ports
                                will be stored. The transport options for a port can be
                                passed directly into the openTestEngineSpiTrans function to
                                open the port.

                            count -
                                This value is set to the number of available ports found.

            Returns :       <table>
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                            </table>

            Description :   This function is used to get a list of available SPI ports. A
                            char array, pointed to by the ports parameter, is filled with
                            a comma separated list of port names. A further char array,
                            pointed to by the trans parameter, is filled with a comma
                            separated list of the relevant transport option strings that
                            specify each available SPI port.
                            <p>If the maxLen parameter indicates that either char array is not
                            large enough to contain the strings, Error is returned and the
                            maxLen parameter is set to the size required for the arrays.
                            <p>If any other error occurs, the maxLen parameter is set to 0.
                            <p>This function can be used by an application to get a list
                            of available SPI ports with which to populate a drop down list
                            or other means of selection. The strings returned in the ports
                            parameter are in human readable format for display / selection.
                            The strings returned in the trans parameter can be passed directly
                            to the openTestEngineSpiTrans function to open the port.

            Example :

                uint32 iHandle(0);
                uint16 maxLen(256);
                uint16 count(0);
                char* portsStr = new char[maxLen];
                char* transStr = new char[maxLen];
                vector<string> ports; // The human readable port strings (e.g. "LPT1")
                vector<string> trans; // The transport option strings (e.g. "SPITRANS=LPT SPIPORT=1")

                int32 status = teGetAvailableSpiPorts(&maxLen, portsStr, transStr, &count);
                if( status != TE_OK && maxLen != 0 )
                {
                    // Not enough space - resize the storage
                    portsStr = new char[maxLen];
                    transStr = new char[maxLen];
                    status = teGetAvailableSpiPorts(&maxLen, portsStr, transStr, &count);
                }
                if( status != TE_OK || count == 0 )
                {
                    cout << "Error getting SPI ports, or none found" << endl;
                    delete[] portsStr;
                    delete[] transStr;
                    return;
                }

                // Split up the comma separated strings of ports / transport options
                split(ports, portsStr, ','); // Use these for user selection (e.g. drop down list)
                split(trans, transStr, ','); // Use these to open a transport

                // Open the SPI port using the trans string
                // For the purposes of this example, we're just using the first in the list
                iHandle = openTestEngineSpiTrans(trans.at(0).c_str(), 0);

                if(iHandle != 0)
                {
                    cout << "Connected!" << endl;

                    // Perform all your testing here

                    closeTestEngine(iHandle);
                }

                delete[] portsStr;
                delete[] transStr;

                return;

        """
        self.TestEngineDLL.teGetAvailableSpiPorts.restype = CT.c_int32
        self.TestEngineDLL.teGetAvailableSpiPorts.argtypes = [CT.c_void_p, CT.c_char_p, CT.c_char_p, CT.c_void_p]
        maxLen = CT.c_uint16(_maxLen)
        ports = CT.create_string_buffer(_ports, 255)
        trans = CT.create_string_buffer(_trans, 255)
        count = CT.c_uint16(_count)
        retval = self.TestEngineDLL.teGetAvailableSpiPorts(CT.byref(maxLen), ports, trans, CT.byref(count))
        return retval, maxLen.value, ports.value, trans.value, count.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdProvokeFault(uint32 handle, uint16 faultCode);
    #########################################################
    # TestEngine::bccmdProvokeFault
    @fntrace
    def bccmdProvokeFault(self, handle, faultCode):
        r"""
        Function wrapper for TestEngine::bccmdProvokeFault().

        Python API:
            bccmdProvokeFault(handle, faultCode)
            returns retval

        Generated from C API:

            Function :      int32 bccmdProvokeFault(uint32 handle, uint16 faultCode)

            Parameters :    handle -
                                Handle to the device.

                            faultCode -
                                Fault code to provoke.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function provokes a fault within the chip. Once the command
                            is issued BlueCore will need to be polled for the fault report
                            using the hqGetFaultReport(..) function.
                            <p>This function will clear any existing logged fault reports
                            before sending the BCCMD to provoke a fault. Therefore, it may be
                            appropriate to fetch any existing fault reports first (using
                            hqGetFaultReports).
                            <p>Fault reports can be disabled by PSKEY, in which case
                            hqGetFaultReport() will timeout and fail, despite the fact that
                            bccmdProvokeFault succeeded.
                            <p>This function is only supported over a SPI connection if HQ
                            over SPI is supported by the device firmware.

        """
        self.TestEngineDLL.bccmdProvokeFault.restype = CT.c_int32
        self.TestEngineDLL.bccmdProvokeFault.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.bccmdProvokeFault(handle, faultCode)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hqGetFaultReports(uint32 handle, uint16 maxReports, uint16 codes[], uint32 timestamps[], uint16 repeats[], uint16 * numReports, int32 timeout);
    #########################################################
    # TestEngine::hqGetFaultReports
    @fntrace
    def hqGetFaultReports(self, handle, maxReports, codes, timestamps, repeats, timeout, _numReports=0):
        r"""
        Function wrapper for TestEngine::hqGetFaultReports().

        Python API:
            hqGetFaultReports(handle, maxReports, codes, timestamps, repeats, timeout, _numReports=0)
            returns retval, numReports.value

        Generated from C API:

            Function :      int32 hqGetFaultReports(uint32 handle, uint16 maxReports,
                                                    uint16 * codes, uint32 * timestamps,
                                                    uint16 * repeats, uint16 * numReports,
                                                    int32 timeout)

            Parameters :    handle -
                                Handle to the device

                            maxReports -
                                Maximum number of fault reports to fetch. The size of the
                                arrays pointed to by the codes, timeStamps and repeats
                                parameters must be >= maxReports.

                            codes -
                                Pointer to an array where the fault codes will be stored. The
                                size of the array should be >= maxReports.

                            timestamps -
                                Pointer to an array where the fault timestamps (in
                                milliseconds) will be stored. The size of the array should
                                be >= maxReports. This parameter can be set to zero if the
                                timestamps are not required.

                            repeats -
                                Pointer to an array where the fault repeat counts will be
                                stored. The repeat count specifies how many fault with the
                                same code were stored by the firmware before sending the fault
                                report. The size of the array should be >= maxReports. This
                                parameter can be set to zero if the timestamps are not
                                required.

                            numReports -
                                Pointer to a value where the actual number of fault reports
                                fetched will be stored.

                            timeout -
                                Used to specify a timeout, in ms, which the software will wait
                                for HQ fault reports to be returned from BlueCore.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function returns a number of fault reports. If there are
                            existing logged faults, the function will immediately fetch the
                            fault report fields. If there are no logged fault reports, then
                            the function will wait for the specified timeout period for a
                            fault report from BlueCore.
                            <p>Fault reports can be received at any time after communications
                            are established with BlueCore. This function can be called to
                            check for logged faults.
                            <p>Faults are stored in the order in which they were received,
                            i.e. codes[0] will contain the first fault code in the list.
                            The most recent fault being codes[numReports - 1].
                            <p>This function can be called following a call to
                            bccmdProvokeFault, which will clear any existing fault reports
                            and provoke a specified fault. In this case, it is possible that
                            a fault report other than that provoked is received prior to
                            receiving the provoked fault. This will cause the function to
                            return reports without the inclusion the provoked fault. In this
                            case, calling hqGetFaultReports again after a small delay should
                            return reports including the provoked fault.
                            <p>Fault reports can be disabled by PSKEY, in which case
                            hqGetFaultReport() will timeout and fail.

        """
        self.TestEngineDLL.hqGetFaultReports.restype = CT.c_int32
        self.TestEngineDLL.hqGetFaultReports.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_void_p, CT.c_void_p, CT.c_void_p, CT.c_void_p, CT.c_int32]
        __codes = (CT.c_uint16 * len(codes))(*codes)
        __timestamps = (CT.c_uint32 * len(timestamps))(*timestamps)
        __repeats = (CT.c_uint16 * len(repeats))(*repeats)
        numReports = CT.c_uint16(_numReports)
        retval = self.TestEngineDLL.hqGetFaultReports(handle, maxReports, CT.byref(__codes), CT.byref(__timestamps), CT.byref(__repeats), CT.byref(numReports), timeout)
        return retval, numReports.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teGetFaultDesc(uint32 handle, uint16 faultCode, char * desc);
    #########################################################
    # TestEngine::teGetFaultDesc
    @fntrace
    def teGetFaultDesc(self, handle, faultCode, _desc=''):
        r"""
        Function wrapper for TestEngine::teGetFaultDesc().

        Python API:
            teGetFaultDesc(handle, faultCode, _desc='')
            returns retval, desc.value

        Generated from C API:

            Function :      int32 teGetFaultDesc(uint32 handle, uint16 faultCode, char * desc)

            Parameters :    handle -
                                Handle to the device

                            faultCode -
                                Fault code to get the description for.

                            desc -
                                Pointer to a char array where the fault description for the
                                code will be stored. The array should be at least 128 bytes
                                long.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                            </table>

            Description :   This function gets a fault description string for the
                            specified BlueCore fault code.
                            <p>Fault reports sent from BlueCore can be fetched using
                            hqGetFaultReports.

        """
        self.TestEngineDLL.teGetFaultDesc.restype = CT.c_int32
        self.TestEngineDLL.teGetFaultDesc.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_char_p]
        desc = CT.create_string_buffer(_desc, 255)
        retval = self.TestEngineDLL.teGetFaultDesc(handle, faultCode, desc)
        return retval, desc.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdSetMapScoPcm(uint32 handle, uint8 enable);
    #########################################################
    # TestEngine::bccmdSetMapScoPcm
    @fntrace
    def bccmdSetMapScoPcm(self, handle, enable):
        r"""
        Function wrapper for TestEngine::bccmdSetMapScoPcm().

        Python API:
            bccmdSetMapScoPcm(handle, enable)
            returns retval

        Generated from C API:

            Function :      int32 bccmdSetMapScoPcm(uint32 handle, uint8 enable)

            Parameters :    handle -
                                Handle to the device

                            enable -
                                Value to enable (1) or disable (0) SCO over PCM.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to enable or disable SCO over PCM. The
                            value set with this command overrides PSKEY_MAP_SCO_PCM for the
                            next SCO connection created using hciSetupScoConnection.

        """
        self.TestEngineDLL.bccmdSetMapScoPcm.restype = CT.c_int32
        self.TestEngineDLL.bccmdSetMapScoPcm.argtypes = [CT.c_uint32, CT.c_uint8]
        retval = self.TestEngineDLL.bccmdSetMapScoPcm(handle, enable)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdGetVrefConstant(uint32 handle, uint16* vref);
    #########################################################
    # TestEngine::bccmdGetVrefConstant
    @fntrace
    def bccmdGetVrefConstant(self, handle, _vref=0):
        r"""
        Function wrapper for TestEngine::bccmdGetVrefConstant().

        Python API:
            bccmdGetVrefConstant(handle, _vref=0)
            returns retval, vref.value

        Generated from C API:

            Function :      int32 bccmdGetVrefConstant(uint32 handle, uint16* vref)

            Parameters :    handle -
                                Handle to the device

                            vref -
                                Pointer to a variable to hold the returned Vref voltage
                                (milli-volts).

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to get the VREF constant voltage for the
                            chip. This value can be used when calculating voltages from ADC
                            values (see the help for bccmdGetVrefAdc).

        """
        self.TestEngineDLL.bccmdGetVrefConstant.restype = CT.c_int32
        self.TestEngineDLL.bccmdGetVrefConstant.argtypes = [CT.c_uint32, CT.c_void_p]
        vref = CT.c_uint16(_vref)
        retval = self.TestEngineDLL.bccmdGetVrefConstant(handle, CT.byref(vref))
        return retval, vref.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdGetVrefAdc(uint32 handle, uint16 * result, uint8 * numBits);
    #########################################################
    # TestEngine::bccmdGetVrefAdc
    @fntrace
    def bccmdGetVrefAdc(self, handle, _result=0, _numBits=0):
        r"""
        Function wrapper for TestEngine::bccmdGetVrefAdc().

        Python API:
            bccmdGetVrefAdc(handle, _result=0, _numBits=0)
            returns retval, result.value, numBits.value

        Generated from C API:

            Function :      int32 bccmdGetVrefAdc(uint32 handle, uint16 * result,
                                                    uint8 * numBits)

            Parameters :    handle -
                                Handle to the device

                            result -
                                Holds the result of the ADC reading. This value is from 0 to
                                ADC_MAX, where ADC_MAX = 255 or 1023 for 8 and 10 bit results
                                respectively (the numBits parameter indicates 8 or 10 bit
                                result).

                            numBits -
                                Holds the number of bits used for the result. In BlueCore
                                firmware from v24, the ADC readings have been changed to return
                                a 10 bit result (8 previously). This output parameter indicates
                                how many bits in the result are valid, 8 or 10, removing the
                                need to check the firmware version when using this function.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will read the vref ADC. This function supports both
                            the old ADC reading method (8 bit results) and the new (10 bit
                            results). BlueCore devices prior to BC5 have 8 bit ADCs. With
                            these devices, and where the firmware used uses the new method
                            (returns 10 bit results), the readings are shifted up by 2 bits.
                            <p>The vref ADC reading can be used to calculate VDD, which can
                            then be used when calculating voltages from other ADC readings,
                            e.g. from bccmdGetAio. Calculate VDD (in milli-volts) as follows:
                            <p>VDD = (vref_constant * ADC_MAX) / result
                            <p>The vref_constant can be found using bccmdGetVrefConstant if
                            supported. Otherwise use the fixed value 1250mV.

        """
        self.TestEngineDLL.bccmdGetVrefAdc.restype = CT.c_int32
        self.TestEngineDLL.bccmdGetVrefAdc.argtypes = [CT.c_uint32, CT.c_void_p, CT.c_void_p]
        result = CT.c_uint16(_result)
        numBits = CT.c_uint8(_numBits)
        retval = self.TestEngineDLL.bccmdGetVrefAdc(handle, CT.byref(result), CT.byref(numBits))
        return retval, result.value, numBits.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdBC5FMGetI2CState(uint32 handle, uint8 * sda, uint8 * scl);
    #########################################################
    # TestEngine::bccmdBC5FMGetI2CState
    @fntrace
    def bccmdBC5FMGetI2CState(self, handle, _sda=0, _scl=0):
        r"""
        Function wrapper for TestEngine::bccmdBC5FMGetI2CState().

        Python API:
            bccmdBC5FMGetI2CState(handle, _sda=0, _scl=0)
            returns retval, sda.value, scl.value

        Generated from C API:

            Function :      int32 bccmdBC5FMGetI2CState(uint32 handle, uint8 * sda,
                                                        uint8 * scl)

            Parameters :    handle -
                                Handle to the device

                            sda -
                                Pointer to 8 bit unsigned int, used to store the state of the
                                SDA (I2C_DATA) line. The value will be either 0 (low) or
                                1 (high).

                            scl -
                                Pointer to 8 bit unsigned int, used to store the state of the
                                SCL (I2C_CLK) line. The value will be either 0 (low) or
                                1 (high).

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will read the status of the I2C interface lines
                            on a BC5-FM BlueCore chip. It can therefore be used to test for
                            open and short circuits on the I2C interface using appropriate
                            external hardware.
                            <p>The lines are pulled up, therefore with nothing connected,
                            the values of sda and scl will be 1.
                            <p>This function is for use with BC5-FM only. The results of
                            calling this function with other chip types is undefined.

        """
        self.TestEngineDLL.bccmdBC5FMGetI2CState.restype = CT.c_int32
        self.TestEngineDLL.bccmdBC5FMGetI2CState.argtypes = [CT.c_uint32, CT.c_void_p, CT.c_void_p]
        sda = CT.c_uint8(_sda)
        scl = CT.c_uint8(_scl)
        retval = self.TestEngineDLL.bccmdBC5FMGetI2CState(handle, CT.byref(sda), CT.byref(scl))
        return retval, sda.value, scl.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) refEpGetRssiDbm(uint32 handle, uint16 freqMHz, float64 rssiChip, float64 * rssiDbm);
    #########################################################
    # TestEngine::refEpGetRssiDbm
    @fntrace
    def refEpGetRssiDbm(self, handle, freqMHz, rssiChip, _rssiDbm=0):
        r"""
        Function wrapper for TestEngine::refEpGetRssiDbm().

        Python API:
            refEpGetRssiDbm(handle, freqMHz, rssiChip, _rssiDbm=0)
            returns retval, rssiDbm.value

        Generated from C API:

            Function :      int32 refEpGetRssiDbm(uint32 handle, uint16 freqMHz,
                                                    float64 rssiChip, float64 * rssiDbm)

            Parameters :    handle -
                                Handle to the device.

                            freqMHz -
                                The frequency in MHz for which the RSSI value is required.

                            rssiChip -
                                The rssi value, returned from the chip (using hqGetRssi) to
                                be converted into an RSSI value in dBm. The value is a
                                floating point value because the values returned from the chip
                                are usually averaged (as are the calibration values of the
                                reference endpoint).

                            rssiDbm -
                                Pointer to a value where the RSSI value in dBm will be stored.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function can be used with reference endpoint hardware to
                            convert RSSI values obtained from the chip to a dBm value, using
                            the calibration values stored in the reference endpoint.
                            <p>The reference endpoint is calibrated at the following
                            frequencies:
                            <table>
                                <tr><td>2402 MHz
                                <tr><td>2441 MHz
                                <tr><td>2480 MHz
                            </table>
                            If the freqMHz parameter value is not equal to one of these
                            frequencies, the closest of the above frequencies will be used.
                            Frequencies outside of the BlueTooth range produce result in an
                            error being returned.
                            <p>If the rssiChip parameter value is outside of the range
                            covered by the reference endpoint's calibration table, an error
                            code will be returned.
                            <p>Linear interpolation is performed on the calibration data
                            when calculating the rssiDbm value.
                            <p>Note that if the calibration data has not been pre-loaded
                            using refEpLoadCalDataFile, the first call to this function will
                            take a few seconds to complete. This is due to reading the
                            calibration data from the device. Subsequent calls will not incur
                            this overhead.

        """
        self.TestEngineDLL.refEpGetRssiDbm.restype = CT.c_int32
        self.TestEngineDLL.refEpGetRssiDbm.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_double, CT.c_void_p]
        rssiDbm = CT.c_float64(_rssiDbm)
        retval = self.TestEngineDLL.refEpGetRssiDbm(handle, freqMHz, rssiChip, CT.byref(rssiDbm))
        return retval, rssiDbm.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) refEpGetPaLevel(uint32 handle, uint16 freqMHz, float64 targetPowerDbm, uint16 * intPa, float64 * powerDbm);
    #########################################################
    # TestEngine::refEpGetPaLevel
    @fntrace
    def refEpGetPaLevel(self, handle, freqMHz, targetPowerDbm, _intPa=0, _powerDbm=0):
        r"""
        Function wrapper for TestEngine::refEpGetPaLevel().

        Python API:
            refEpGetPaLevel(handle, freqMHz, targetPowerDbm, _intPa=0, _powerDbm=0)
            returns retval, intPa.value, powerDbm.value

        Generated from C API:

            Function :      int32 refEpGetPaLevel(uint32 handle, uint16 freqMHz,
                                                    float64 targetPowerDbm, uint16 * intPa,
                                                    float64 * powerDbm)

            Parameters :    handle -
                                Handle to the device.

                            freqMHz -
                                The frequency in MHz for which the internal PA drive level is
                                required.

                            targetPowerDbm -
                                The power in dBm for which the internal PA drive level is
                                required.

                            intPa -
                                Pointer to a value where the internal PA drive level will be
                                stored.

                            powerDbm -
                                Pointer to a value where the actual power in dBm for the
                                internal PA drive level will be stored.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function can be used with reference endpoint hardware to
                            convert a required output power, specified in dBm, to the
                            corresponding internal PA drive level (taken from
                            the calibration values stored in the reference endpoint). The
                            intPa value is used with radiotestTx... functions.
                            <p>Some TestEngine transmit functions take both internal and
                            external PA drive levels as parameters. The external PA drive
                            level is irrelevant for the reference endpoint hardware (set to
                            zero).
                            <p>The reference endpoint is calibrated at the following
                            frequencies:
                            <table>
                                <tr><td>2402 MHz
                                <tr><td>2441 MHz
                                <tr><td>2480 MHz
                            </table>
                            If the freqMHz parameter value is not equal to one of these
                            frequencies, the closest of the above frequencies will be used.
                            Frequencies outside of the BlueTooth range produce result in an
                            error being returned.
                            <p>The internal PA gain value for the power closest to the
                            targetPowerDbm parameter value will be returned, along with the
                            actual recorded power for that gain value. If the targetPowerDbm
                            parameter value is outside of the range covered by the reference
                            endpoint's calibration table, an error code will be returned.
                            <p>Note that if the calibration data has not been pre-loaded
                            using refEpLoadCalDataFile, the first call to this function will
                            take a few seconds to complete. This is due to reading the
                            calibration data from the device. Subsequent calls will not incur
                            this overhead.

        """
        self.TestEngineDLL.refEpGetPaLevel.restype = CT.c_int32
        self.TestEngineDLL.refEpGetPaLevel.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_double, CT.c_void_p, CT.c_void_p]
        intPa = CT.c_uint16(_intPa)
        powerDbm = CT.c_float64(_powerDbm)
        retval = self.TestEngineDLL.refEpGetPaLevel(handle, freqMHz, targetPowerDbm, CT.byref(intPa), CT.byref(powerDbm))
        return retval, intPa.value, powerDbm.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) refEpWriteCalDataFile(uint32 handle, const char * filePath);
    #########################################################
    # TestEngine::refEpWriteCalDataFile
    @fntrace
    def refEpWriteCalDataFile(self, handle, filePath):
        r"""
        Function wrapper for TestEngine::refEpWriteCalDataFile().

        Python API:
            refEpWriteCalDataFile(handle, filePath)
            returns retval

        Generated from C API:

            Function :      int32 refEpWriteCalDataFile(uint32 handle, const char * filePath)

            Parameters :    handle -
                                Handle to the device.

                            filePath -
                                This specifies the name of the save file.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function can be used with reference endpoint hardware to
                            save the calibration data to a file. For subsequent tests,
                            this file can be loaded using the refEpLoadCalDataFile function
                            before refEpGetPaLevel or refEpGetRssiDbm are used. This saves
                            time due to not having to read the calibration data from the
                            reference endpoint.
                            <p>If filePath is NULL, or if the file specified cannot be
                            written, an error is returned. An error is also returned if the
                            data cannot be read from the reference endpoint.
                            <p>If the calibration data has already been loaded, e.g. due to a
                            call to refEpGetPaLevel or refEpGetRssiDbm, this function will
                            simply write the data to the file. If the calibration data hasn't
                            been loaded, it must be read from the reference endpoint, which
                            will take a few seconds to complete.

        """
        self.TestEngineDLL.refEpWriteCalDataFile.restype = CT.c_int32
        self.TestEngineDLL.refEpWriteCalDataFile.argtypes = [CT.c_uint32, CT.c_char_p]
        retval = self.TestEngineDLL.refEpWriteCalDataFile(handle, filePath)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) refEpLoadCalDataFile(uint32 handle, const char * filePath);
    #########################################################
    # TestEngine::refEpLoadCalDataFile
    @fntrace
    def refEpLoadCalDataFile(self, handle, filePath):
        r"""
        Function wrapper for TestEngine::refEpLoadCalDataFile().

        Python API:
            refEpLoadCalDataFile(handle, filePath)
            returns retval

        Generated from C API:

            Function :      int32 refEpLoadCalDataFile(uint32 handle, const char * filePath)

            Parameters :    handle -
                                Handle to the device.

                            filePath -
                                This specifies the name of the file containing the calibration
                                data.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function can be used to pre-load reference endpoint
                            calibration data from a file previously created using
                            refEpWriteCalDataFile. This saves time due to not having to read
                            the calibration data from the reference endpoint when
                            refEpGetPaLevel or refEpGetRssiDbm are called.
                            <p>If filePath is NULL, or if the file specified cannot be
                            read, an error is returned. An error is also returned if the file
                            is not correctly formatted (i.e. if it has been changed from what
                            was written by refEpWriteCalDataFile).
                            <p>If the calibration data has already been loaded, e.g. due to a
                            call to refEpGetPaLevel or refEpGetRssiDbm, this function will
                            overwrite the calibration data.

        """
        self.TestEngineDLL.refEpLoadCalDataFile.restype = CT.c_int32
        self.TestEngineDLL.refEpLoadCalDataFile.argtypes = [CT.c_uint32, CT.c_char_p]
        retval = self.TestEngineDLL.refEpLoadCalDataFile(handle, filePath)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdGetVmStatus(uint32 handle, uint16 * status, uint16 * exitCode);
    #########################################################
    # TestEngine::bccmdGetVmStatus
    @fntrace
    def bccmdGetVmStatus(self, handle, _status=0, _exitCode=0):
        r"""
        Function wrapper for TestEngine::bccmdGetVmStatus().

        Python API:
            bccmdGetVmStatus(handle, _status=0, _exitCode=0)
            returns retval, status.value, exitCode.value

        Generated from C API:

            Function :      int32 bccmdGetVmStatus(uint32 handle, uint16 * status,
                                                    uint16 * exitCode)

            Parameters :    handle -
                                Handle to the device

                            status -
                                Pointer to a value where the VM status will be stored. The
                                following status values may be returned:
                                <table>
                                    <tr><td>0 = VM_STATUS_BOOT (Not initialised yet)
                                    <tr><td>1 = VM_STATUS_FAIL (Failed to initialise)
                                    <tr><td>2 = VM_STATUS_RUN (Running)
                                    <tr><td>3 = VM_STATUS_PANIC (A panic occurred)
                                    <tr><td>4 = VM_STATUS_EXIT (Normal termination)
                                </table>

                            exitCode -
                                Pointer to a value where the VM application exit code will
                                be stored, if the application terminates normally (status
                                is VM_STATUS_EXIT).
                                <p>If the application is the CVC license checker application,
                                then the exit codes are as follows:
                                <table>
                                    <tr><td>1 = CVC_PRODTEST_PASS
                                    <tr><td>2 = CVC_PRODTEST_FAIL
                                    <tr><td>3 = CVC_PRODTEST_NO_CHECK
                                    <tr><td>4 = CVC_PRODTEST_FILE_NOT_FOUND
                                </table>

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will read the VM status. The exit code from a VM
                            application is returned if such an application terminated
                            normally. This function can be used to perform a CVC license check.

            Example :

                // Perform CVC license check

                static const uint16 PSKEY_INITIAL_BOOTMODE = 973;
                static const uint16 BOOT_MODE_CVC_TEST = 4;  // boot mode for CVC license key testing
                static const uint16 BOOT_MODE_NORMAL = 1;    // normal boot mode

                uint32 dutHandle = openTestEngineSpi(1, 0, SPI_LPT);
                if(dutHandle == 0)
                {
                    cout << "Failed to connect to device under test" << endl;
                    return;
                }

                // Set boot mode for CVC
                uint16 bootMode = BOOT_MODE_CVC_TEST;
                if(psWrite(dutHandle, PSKEY_INITIAL_BOOTMODE, PS_STORES_I, 1, &bootMode) != TE_OK)
                {
                    cout << "Failed to set boot mode" << endl;
                    closeTestEngine(dutHandle);
                    return;
                }

                if(bccmdSetColdReset(dutHandle, 0) != TE_OK)
                {
                    cout << "Cold reset failed" << endl;
                    closeTestEngine(dutHandle);
                    return;
                }

                // CVC license key check
                uint16 status;
                uint16 exitCode;
                static const uint16 MAX_ATTEMPTS = 100; // With loop delay 100ms, gives approx 10s timeout
                uint16 attempts(0);
                do
                {
                    if (bccmdGetVmStatus(dutHandle, &status, &exitCode) != TE_OK)
                    {
                        cout << "bccmdGetVmStatus failed" << endl;
                        closeTestEngine(dutHandle);
                        return;
                    }
                    Sleep(100);
                    ++attempts;

                }while((status == VM_STATUS_BOOT || status == VM_STATUS_RUN) && (attempts < MAX_ATTEMPTS));

                // Check returned status and exit code
                if ((status == VM_STATUS_EXIT) && (exitCode == CVC_PRODTEST_PASS))
                {
                    cout << "CVC Production Test PASS" << endl;
                }
                else
                {
                    cout << "CVC Production Test FAIL" << endl;
                }

                // Set boot mode back to normal
                bootMode = BOOT_MODE_NORMAL;
                if(psWrite(dutHandle, PSKEY_INITIAL_BOOTMODE, PS_STORES_I, 1, &bootMode) != TE_OK)
                {
                    cout << "Failed to set boot mode" << endl;
                    closeTestEngine(dutHandle);
                    return;
                }

                closeTestEngine(dutHandle);

        """
        self.TestEngineDLL.bccmdGetVmStatus.restype = CT.c_int32
        self.TestEngineDLL.bccmdGetVmStatus.argtypes = [CT.c_uint32, CT.c_void_p, CT.c_void_p]
        status = CT.c_uint16(_status)
        exitCode = CT.c_uint16(_exitCode)
        retval = self.TestEngineDLL.bccmdGetVmStatus(handle, CT.byref(status), CT.byref(exitCode))
        return retval, status.value, exitCode.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdI2CTransfer(uint32 handle, uint16 slaveAddr, uint16 txOctets, uint16 rxOctets, uint8 restart, uint8 data[], uint16 *octets);
    #########################################################
    # TestEngine::bccmdI2CTransfer
    @fntrace
    def bccmdI2CTransfer(self, handle, slaveAddr, txOctets, rxOctets, restart, data, _octets=0):
        r"""
        Function wrapper for TestEngine::bccmdI2CTransfer().

        Python API:
            bccmdI2CTransfer(handle, slaveAddr, txOctets, rxOctets, restart, data, _octets=0)
            returns retval, octets.value

        Generated from C API:

            Function :      int32 bccmdI2CTransfer(uint32 handle, uint16 slaveAddr,
                                                    uint16 txOctets, uint16 rxOctets,
                                                    uint8 restart, uint8 * data, uint16 * octets)

            Parameters :    handle -
                                Handle to the device

                            slaveAddr -
                                Slave address of the I2C device.

                            txOctets -
                                Number of bytes to write to the I2C device. The data array
                                should contain this many bytes.

                            rxOctets -
                                Number of bytes to read from the I2C device. The data array
                                should be large enough to hold the number of bytes specified.

                            restart -
                                Boolean value (0 = False, anything else = True). If True,
                                inserts a repeated start condition between writes and reads.
                                This should always be '1' for devices that comply with the I2C
                                specification.

                            data -
                                Pointer to an array of bytes to write, or read from memory.
                                This parameter is an input/output parameter. In the case of a
                                write operation, data should contain the bytes to write.
                                On return, data will contain the bytes which have been written.
                                In the case of a read operation, or a composite write + read
                                operation, data should be at least as large as rxOctets.
                                On return, data will contain the bytes read from the device.

                            octets -
                                Holds the value of the number of octets transferred to or from
                                the I2C device.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function performs writes to and/or reads from an I2C device.

        """
        self.TestEngineDLL.bccmdI2CTransfer.restype = CT.c_int32
        self.TestEngineDLL.bccmdI2CTransfer.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16, CT.c_uint16, CT.c_uint8, CT.c_void_p, CT.c_void_p]
        __data = (CT.c_uint8 * len(data))(*data)
        octets = CT.c_uint16(_octets)
        retval = self.TestEngineDLL.bccmdI2CTransfer(handle, slaveAddr, txOctets, rxOctets, restart, CT.byref(__data), CT.byref(octets))
        return retval, octets.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) radiotestBle(uint32 handle, uint16 command, uint8 channel, uint8 txLength, uint8 txPayload);
    #########################################################
    # TestEngine::radiotestBle
    @fntrace
    def radiotestBle(self, handle, command, channel, txLength, txPayload):
        r"""
        Function wrapper for TestEngine::radiotestBle().

        Python API:
            radiotestBle(handle, command, channel, txLength, txPayload)
            returns retval

        Generated from C API:

            Function :      int32 radiotestBle(uint32 handle, uint16 command, uint8 channel,
                                               uint8 txLength, uint8 txPayload)

            Parameters :    handle -
                                Handle to the device

                            command -
                                The BLE radio test command, where:
                                <table>
                                    <tr><td>0 = END
                                    <tr><td>1 = RECEIVER
                                    <tr><td>2 = TRANSMITTER
                                </table>

                            channel -
                                The BLE channel (0 - 39).
                                <p>channel = (freqMHz ? 2402) / 2
                                <p>This parameter is relevant to the RECEIVER and TRANSMITTER
                                test commands only.

                            txLength -
                                The length in bytes (0 - 37), of the payload data in each
                                packet.
                                <p>This parameter is relevant to the TRANSMITTER test command
                                only.

                            txPayload -
                                The transmit packet payload, where:
                                <table>
                                    <tr><td>0 = Pseudo-Random bit sequence 9
                                    <tr><td>1 = Pattern of alternating bits "11110000"
                                    <tr><td>2 = Pattern of alternating bits "10101010"
                                    <tr><td>3 = Pseudo-Random bit sequence 15
                                    <tr><td>4 = Pattern of all "1" bits
                                    <tr><td>5 = Pattern of all "0" bits
                                    <tr><td>6 = Pattern of alternating bits "00001111"
                                    <tr><td>7 = Pattern of alternating bits "01010101"
                                </table>
                                <p>This parameter is relevant to the TRANSMITTER test command
                                only.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function sends Bluetooth Low Energy (BLE) radio test
                            commands to the chip.
                            <p>Only one of the RECEIVER and TRANSMITTER test modes can be
                            running at any one time - an error is returned if one of these
                            modes is specified when RECEIVER / TRANSMITTER test is running.
                            It is necessary to END each mode before running another test
                            mode (or changing parameters for the same mode).
                            <p>When the END command is sent following the RECEIVER command,
                            an HQ message is sent containing the received packet count. Use
                            hqGetBleRxPktCount to retrieve the received packet count value.

            Example :

                static const uint16 BLE_TEST_CMD_END = 0;
                static const uint16 BLE_TEST_CMD_RX = 1;
                static const uint16 BLE_TEST_CMD_TX = 2;

                static const uint8 BLE_TEST_CHANNEL = 1;
                static const uint8 BLE_TX_LENGTH = 37; // 37 = Max
                static const uint8 BLE_TX_PAYLOAD = 0; // 0 = PRBS9

                static const uint32 RX_TEST_DURATION_MS = 1000;

                uint32 dutHandle = openTestEngineSpi(1, 0, SPI_LPT);
                if(dutHandle == 0)
                {
                    cout << "Failed to connect to device under test" << endl;
                    return;
                }

                // Test TX
                if(radiotestBle(dutHandle, BLE_TEST_CMD_TX, BLE_TEST_CHANNEL, BLE_TX_LENGTH, BLE_TX_PAYLOAD) != TE_OK)
                {
                    cout << "radiotestBle failed (starting TX)" << endl;
                    closeTestEngine(dutHandle);
                    return;
                }

                // Perform TX measurements using test equipment here

                // End TX test
                if(radiotestBle(dutHandle, BLE_TEST_CMD_END, 0, 0, 0) != TE_OK)
                {
                    cout << "radiotestBle failed (stopping TX)" << endl;
                    closeTestEngine(dutHandle);
                    return;
                }

                // Test RX

                // Switch on test equipment transmission here

                // Start receiving packets
                if(radiotestBle(dutHandle, BLE_TEST_CMD_RX, BLE_TEST_CHANNEL, 0, 0) != TE_OK)
                {
                    cout << "radiotestBle failed (starting RX)" << endl;
                    closeTestEngine(dutHandle);
                    return;
                }

                // Wait for test duration
                Sleep(RX_TEST_DURATION_MS);

                // Stop receiving
                if(radiotestBle(dutHandle, BLE_TEST_CMD_END, 0, 0, 0) != TE_OK)
                {
                    cout << "radiotestBle failed (stopping RX)" << endl;
                    closeTestEngine(dutHandle);
                    return;
                }

                // Retrieve the packet count
                uint16 pktCount = 0;
                if(hqGetBleRxPktCount(dutHandle, 1000, &pktCount) != TE_OK)
                {
                    cout << "hqGetBleRxPktCount failed" << endl;
                }
                else
                {
                    cout << "Packet count = " << pktCount << endl;
                }

                closeTestEngine(dutHandle);

        """
        self.TestEngineDLL.radiotestBle.restype = CT.c_int32
        self.TestEngineDLL.radiotestBle.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint8, CT.c_uint8, CT.c_uint8]
        retval = self.TestEngineDLL.radiotestBle(handle, command, channel, txLength, txPayload)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) hqGetBleRxPktCount(uint32 handle, int32 timeout, uint16 * pktCount);
    #########################################################
    # TestEngine::hqGetBleRxPktCount
    @fntrace
    def hqGetBleRxPktCount(self, handle, timeout, _pktCount=0):
        r"""
        Function wrapper for TestEngine::hqGetBleRxPktCount().

        Python API:
            hqGetBleRxPktCount(handle, timeout, _pktCount=0)
            returns retval, pktCount.value

        Generated from C API:

            Function :      int32 hqGetBleRxPktCount(uint32 handle, int32 timeout,
                                                     uint16 * pktCount)

            Parameters :    handle -
                                Handle to the device

                            timeout -
                                Time in ms, for which the function will wait for a result.
                                If set to zero the function will just check for completion.

                            pktCount -
                                Pointer to a uint16 value where the number of received packets
                                will be stored.

            Returns :        <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function obtains the number of received Bluetooth Low Energy
                            (BLE) packets received during the last BLE receiver test. The
                            radiotestBle function must be called first to start the RECEIVER
                            test mode, and again to END the receiver test after the desired
                            test duration has elapsed (see radiotestBle for more details).
                            <p>The pktCount value will wrap once the limit of its type (65535)
                            is reached. This can occur if the receiver test duration is
                            greater than approximately 42 seconds.

        """
        self.TestEngineDLL.hqGetBleRxPktCount.restype = CT.c_int32
        self.TestEngineDLL.hqGetBleRxPktCount.argtypes = [CT.c_uint32, CT.c_int32, CT.c_void_p]
        pktCount = CT.c_uint16(_pktCount)
        retval = self.TestEngineDLL.hqGetBleRxPktCount(handle, timeout, CT.byref(pktCount))
        return retval, pktCount.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdGetChargerTrims(uint32 handle, uint16 * chgRefTrim, uint16 * hVrefTrim, uint16 * rTrim, uint16 * iTrim, uint16 * iExtTrim, uint16 * iTermTrim, uint16 * vFastTrim, uint16 * hystTrim);
    #########################################################
    # TestEngine::bccmdGetChargerTrims
    @fntrace
    def bccmdGetChargerTrims(self, handle, _chgRefTrim=0, _hVrefTrim=0, _rTrim=0, _iTrim=0, _iExtTrim=0, _iTermTrim=0, _vFastTrim=0, _hystTrim=0):
        r"""
        Function wrapper for TestEngine::bccmdGetChargerTrims().

        Python API:
            bccmdGetChargerTrims(handle, _chgRefTrim=0, _hVrefTrim=0, _rTrim=0, _iTrim=0, _iExtTrim=0, _iTermTrim=0, _vFastTrim=0, _hystTrim=0)
            returns retval, chgRefTrim.value, hVrefTrim.value, rTrim.value, iTrim.value, iExtTrim.value, iTermTrim.value, vFastTrim.value, hystTrim.value

        Generated from C API:

            Function :      int32 bccmdGetChargerTrims(uint32 handle, uint16 * chgRefTrim,
                                                        uint16 * hVrefTrim, uint16 * rTrim,
                                                        uint16 * iTrim, uint16 * iExtTrim,
                                                        uint16 * iTermTrim, uint16 * vFastTrim,
                                                        uint16 * hystTrim);

            Parameters :    handle -
                                Handle to the device

                            chgRefTrim -
                                Pointer to a uint16 value where the voltage reference trim for
                                the charger will be stored.

                            hVrefTrim -
                                Pointer to a uint16 value where the voltage reference trim for
                                high voltage parts is stored.

                            rTrim -
                                Pointer to a uint16 value where the float voltage trim is stored.

                            iTrim -
                                Pointer to a uint16 value where the charger current trim for the
                                internal pass transistor is stored.

                            iExtTrim -
                                Pointer to a uint16 value where the charger current trim for an
                                external pass transistor is stored.

                            iTermTrim -
                                Pointer to a uint16 value where the trim to adjust the termination
                                current is stored.

                            vFastTrim -
                                Pointer to a uint16 value where the trickle->fast charge transition
                                voltage trim is stored.

                            hystTrim -
                                Pointer to a uint16 value where the hysteresis trim for standby->fast
                                charge transition is stored.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function reads charger trim values stored in the flash information
                            block in some BlueCore ICs.

        """
        self.TestEngineDLL.bccmdGetChargerTrims.restype = CT.c_int32
        self.TestEngineDLL.bccmdGetChargerTrims.argtypes = [CT.c_uint32, CT.c_void_p, CT.c_void_p, CT.c_void_p, CT.c_void_p, CT.c_void_p, CT.c_void_p, CT.c_void_p, CT.c_void_p]
        chgRefTrim = CT.c_uint16(_chgRefTrim)
        hVrefTrim = CT.c_uint16(_hVrefTrim)
        rTrim = CT.c_uint16(_rTrim)
        iTrim = CT.c_uint16(_iTrim)
        iExtTrim = CT.c_uint16(_iExtTrim)
        iTermTrim = CT.c_uint16(_iTermTrim)
        vFastTrim = CT.c_uint16(_vFastTrim)
        hystTrim = CT.c_uint16(_hystTrim)
        retval = self.TestEngineDLL.bccmdGetChargerTrims(handle, CT.byref(chgRefTrim), CT.byref(hVrefTrim), CT.byref(rTrim), CT.byref(iTrim), CT.byref(iExtTrim), CT.byref(iTermTrim), CT.byref(vFastTrim), CT.byref(hystTrim))
        return retval, chgRefTrim.value, hVrefTrim.value, rTrim.value, iTrim.value, iExtTrim.value, iTermTrim.value, vFastTrim.value, hystTrim.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdCapacitiveSensorRead(uint32 handle, uint16 mask, uint16 values[]);
    #########################################################
    # TestEngine::bccmdCapacitiveSensorRead
    @fntrace
    def bccmdCapacitiveSensorRead(self, handle, mask, values):
        r"""
        Function wrapper for TestEngine::bccmdCapacitiveSensorRead().

        Python API:
            bccmdCapacitiveSensorRead(handle, mask, values)
            returns retval

        Generated from C API:

            Function :      int32 bccmdCapacitiveSensorRead(uint32 handle, uint16 mask,
                                                            uint16 * values)

            Parameters :    handle -
                                Handle to the device

                            mask -
                                The mask specifying which pads to read, where bit 0 being
                                set means pad 0 will be read.

                            values -
                                Pointer to an array of uint16 values. These are the relative
                                values of capacitance read from the pads in units of
                                approximately 1fF.
                                There must be at least enough space in the array coresponding
                                to the highest entry in the mask. For each position in the
                                mask, the corresponding array entry is populated with the
                                current value of the capacitative sense pad.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function reads the capacitance values of the specified
                            capacitative sense pads.
                            <p>
                            The capacitive touch sensors must be calibrated before use.
                            Calibration needs to be done only once for any given product
                            design (taking into account the length and location of wires
                            etc.).
                            To initiate this calibration, set PSKEY_CAP_SENSE_CALIBRATE
                            to the mask indicating which pads are to be used.
                            During boot up, if this value is non-zero, the chip will
                            calibrate the specified pads, write the calibration constants
                            to PSKEY_CAP_SENSE_PRELOAD and reset the
                            PSKEY_CAP_SENSE_CALIBRATE value to zero.
                            The psWrite / psWriteVerify and bccmdSetWarmReset functions
                            can be used to initiate calibration.
                            If required, psReadVerify can be used to check the calibration
                            constants.
                            If the PSKEY_CAP_SENSE_PRELOAD constants are incorporated
                            into the image, it is not necessary to perform calibration on
                            each and every unit.

        """
        self.TestEngineDLL.bccmdCapacitiveSensorRead.restype = CT.c_int32
        self.TestEngineDLL.bccmdCapacitiveSensorRead.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_void_p]
        __values = (CT.c_uint16 * len(values))(*values)
        retval = self.TestEngineDLL.bccmdCapacitiveSensorRead(handle, mask, CT.byref(__values))
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdSetSpiLockCustomerKey(uint32 handle, uint32 custKey[]);
    #########################################################
    # TestEngine::bccmdSetSpiLockCustomerKey
    @fntrace
    def bccmdSetSpiLockCustomerKey(self, handle, custKey):
        r"""
        Function wrapper for TestEngine::bccmdSetSpiLockCustomerKey().

        Python API:
            bccmdSetSpiLockCustomerKey(handle, custKey)
            returns retval

        Generated from C API:

            Function :      int32 bccmdSetSpiLockCustomerKey(uint32 handle, uint32* custKey)

            Parameters :    handle -
                                Handle to the device

                            custKey -
                                Pointer to an array of 4 uint32 values that make up the
                                128-bit customer key.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function writes the 128 bit SPI lock customer key. The
                            customer key written using this function can be used with the
                            spiunlock tool to unlock the SPI port if the SPI port had been
                            previously locked (see bccmdSpiLockInitiateLock). Therefore, the
                            customer key should be set before the SPI port is locked. Use
                            bccmdGetSpiLockStatus to check the SPI Lock status.
                            <p>Note that the customer key can be incorporated into the
                            firmware image loaded to flash memory. In this case, it is not
                            necessary to set the key using this function.

        """
        self.TestEngineDLL.bccmdSetSpiLockCustomerKey.restype = CT.c_int32
        self.TestEngineDLL.bccmdSetSpiLockCustomerKey.argtypes = [CT.c_uint32, CT.c_void_p]
        __custKey = (CT.c_uint32 * len(custKey))(*custKey)
        retval = self.TestEngineDLL.bccmdSetSpiLockCustomerKey(handle, CT.byref(__custKey))
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdGetSpiLockStatus(uint32 handle, uint16* status);
    #########################################################
    # TestEngine::bccmdGetSpiLockStatus
    @fntrace
    def bccmdGetSpiLockStatus(self, handle, _status=0):
        r"""
        Function wrapper for TestEngine::bccmdGetSpiLockStatus().

        Python API:
            bccmdGetSpiLockStatus(handle, _status=0)
            returns retval, status.value

        Generated from C API:

            Function :      int32 bccmdGetSpiLockStatus(uint32 handle, uint16* status)

            Parameters :    handle -
                                Handle to the device

                            status -
                                Pointer to a value where the SPI lock status of the chip will
                                be stored. The following status values may be returned:
                                <table>
                                    <tr><td>0 = SPI unlocked, customer key not set
                                    <tr><td>1 = SPI locked, customer key not set
                                    <tr><td>2 = SPI unlocked, customer key set
                                    <tr><td>3 = SPI locked, customer key set
                                </table>

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function reads the SPI Lock status, i.e. whether the
                            customer key is set and the SPI port is locked. If the SPI is
                            unlocked, the customer key can be set using
                            bccmdSetSpiLockCustomerKey. If the customer key is set,
                            bccmdSpiLockInitiateLock can be used to lock the SPI port.

        """
        self.TestEngineDLL.bccmdGetSpiLockStatus.restype = CT.c_int32
        self.TestEngineDLL.bccmdGetSpiLockStatus.argtypes = [CT.c_uint32, CT.c_void_p]
        status = CT.c_uint16(_status)
        retval = self.TestEngineDLL.bccmdGetSpiLockStatus(handle, CT.byref(status))
        return retval, status.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) bccmdSpiLockInitiateLock(uint32 handle);
    #########################################################
    # TestEngine::bccmdSpiLockInitiateLock
    @fntrace
    def bccmdSpiLockInitiateLock(self, handle):
        r"""
        Function wrapper for TestEngine::bccmdSpiLockInitiateLock().

        Python API:
            bccmdSpiLockInitiateLock(handle)
            returns retval

        Generated from C API:

            Function :      int32 bccmdSpiLockInitiateLock(uint32 handle)

            Parameters :    handle -
                                Handle to the device

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function initiates locking of the SPI port on devices which
                            support SPI locking.
                            <p>The customer key should be set before calling this function
                            (see bccmdSetSpiLockCustomerKey). If the customer key is not set,
                            the SPI lock will not be set, and an error will be returned. If
                            the SPI lock is already set, an error will be returned. Use
                            bccmdGetSpiLockStatus to check the SPI Lock status.
                            <p>Once the SPI lock is set, the SPI port will be locked
                            out at boot. The spiunlock tool must then be used to unlock the
                            SPI port if necessary, using the customer key.

        """
        self.TestEngineDLL.bccmdSpiLockInitiateLock.restype = CT.c_int32
        self.TestEngineDLL.bccmdSpiLockInitiateLock.argtypes = [CT.c_uint32]
        retval = self.TestEngineDLL.bccmdSpiLockInitiateLock(handle)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teGetChipDisplayName(uint32 handle, uint32 maxLen, char *name);
    #########################################################
    # TestEngine::teGetChipDisplayName
    @fntrace
    def teGetChipDisplayName(self, handle, maxLen, _name=''):
        r"""
        Function wrapper for TestEngine::teGetChipDisplayName().

        Python API:
            teGetChipDisplayName(handle, maxLen, _name='')
            returns retval, name.value

        Generated from C API:

            Function :      int32 teGetChipDisplayName(uint32 handle, uint32 maxLen, char *name)

            Parameters :    handle -
                                Handle to the device.

                            maxLen -
                                The maximum number of characters which will be copied to the
                                name location.

                            name -
                                Pointer to the location where the display name will be stored.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Obtains the display name for the connected chip. The name will be
                            truncated if it is longer than maxLen.

        """
        self.TestEngineDLL.teGetChipDisplayName.restype = CT.c_int32
        self.TestEngineDLL.teGetChipDisplayName.argtypes = [CT.c_uint32, CT.c_uint32, CT.c_char_p]
        name = CT.create_string_buffer(_name, 255)
        retval = self.TestEngineDLL.teGetChipDisplayName(handle, maxLen, name)
        return retval, name.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teGetChipFamily(uint32 handle, uint8* family);
    #########################################################
    # TestEngine::teGetChipFamily
    @fntrace
    def teGetChipFamily(self, handle, _family=0):
        r"""
        Function wrapper for TestEngine::teGetChipFamily().

        Python API:
            teGetChipFamily(handle, _family=0)
            returns retval, family.value

        Generated from C API:

            Function :      int32 teGetChipFamily(uint32 handle, uint8* family)

            Parameters :    handle -
                                Handle to the device.

                            family -
                                Pointer to the location where the chip family code will be
                                stored. Possible values are:
                                <table>
                                    <tr><td>TE_CHIP_FAMILY_UNKNOWN = Unknown
                                    <tr><td>TE_CHIP_FAMILY_BLUECORE = BlueCore chip
                                    <tr><td>TE_CHIP_FAMILY_MULTI = Multi-core chip
                                </table>

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Obtains the chip family code for the connected chip.

        """
        self.TestEngineDLL.teGetChipFamily.restype = CT.c_int32
        self.TestEngineDLL.teGetChipFamily.argtypes = [CT.c_uint32, CT.c_void_p]
        family = CT.c_uint8(_family)
        retval = self.TestEngineDLL.teGetChipFamily(handle, CT.byref(family))
        return retval, family.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teGetPtapHifs( uint16* length, char* hifIds, char* types, char* names, uint16* count);
    #########################################################
    # TestEngine::teGetPtapHifs
    @fntrace
    def teGetPtapHifs(self, _length=0, _hifIds='', _types='', _names='', _count=0):
        r"""
        Function wrapper for TestEngine::teGetPtapHifs().

        Python API:
            teGetPtapHifs(_length=0, _hifIds='', _types='', _names='', _count=0)
            returns retval, length.value, hifIds.value, types.value, names.value, count.value

        Generated from C API:

            Function :      int32 teGetPtapHifs(
                                uint16* length,
                                char* hifIds,
                                char* types,
                                char* names,
                                uint16* count)

            Parameters :    length -
                                Size of the char arrays pointed to by each of the char*
                                parameters. If this parameter indicates that any of the char
                                arrays are too small to store the complete strings, then the
                                value is set to the size required and Error is returned. If
                                any other error occurs, this value is set to zero.

                            hifIds -
                                Pointer to an array of ASCII chars where the comma-separated
                                list of host interface ids for each of the host interfaces
                                will be stored.

                            types -
                                Pointer to an array of ASCII chars where the comma-separated
                                list of types for each of the host interfaces will be stored.

                            names -
                                Pointer to an array of ASCII chars where the comma-separated
                                list of names for each of the host interfaces will be stored.

                            count -
                                This value is set to the number of host interfaces found.

            Returns :       <table>
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                            </table>

            Description :   This function is used to get a list of PTAP host interfaces. The
                            char array pointed to by the hifIds parameter is filled with a
                            comma-separated list of host interface ids. The char array
                            pointed to by the types parameter is filled with a comma-
                            separated list of the corresponding types. The char array pointed
                            to by the names parameter is filled with a comma-separated list
                            of the corresponding names.
                            <p>If the length parameter indicates that any char array is not
                            large enough to contain the strings, Error is returned and the
                            length parameter is set to the size required for the arrays.
                            <p>If any other error occurs, the length parameter is set to zero.
                            <p>This function can be used by an application to get a list of
                            host interfaces with which to populate a drop down list or other
                            means of selection.

        """
        self.TestEngineDLL.teGetPtapHifs.restype = CT.c_int32
        self.TestEngineDLL.teGetPtapHifs.argtypes = [CT.c_void_p, CT.c_char_p, CT.c_char_p, CT.c_char_p, CT.c_void_p]
        length = CT.c_uint16(_length)
        hifIds = CT.create_string_buffer(_hifIds, 255)
        types = CT.create_string_buffer(_types, 255)
        names = CT.create_string_buffer(_names, 255)
        count = CT.c_uint16(_count)
        retval = self.TestEngineDLL.teGetPtapHifs(CT.byref(length), hifIds, types, names, CT.byref(count))
        return retval, length.value, hifIds.value, types.value, names.value, count.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teMcSetXtalFreqTrim(uint32 handle, uint16 trimVal, int16* mibVal);
    #########################################################
    # TestEngine::teMcSetXtalFreqTrim
    @fntrace
    def teMcSetXtalFreqTrim(self, handle, trimVal, _mibVal=0):
        r"""
        Function wrapper for TestEngine::teMcSetXtalFreqTrim().

        Python API:
            teMcSetXtalFreqTrim(handle, trimVal, _mibVal=0)
            returns retval, mibVal.value

        Generated from C API:

            Function :      int32 teMcSetXtalFreqTrim(uint32 handle, uint16 trimVal,
                                                      int16* mibVal)

            Parameters :    handle -
                                Handle to the device.

                            trimVal -
                                Frequency trim value to write. For CSRC9xxx ICs this value is
                                a register value. For CSRA681xx and later ICs this value is
                                effectively the XtalFreqTrim MIB value, written in 2's
                                complement form (range -16 to 15).

                            mibVal -
                                The equivalent frequency trim MIB value for the given trim
                                value (output parameter).

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Sets the active crystal frequency trim value, and returns the
                            equivalent MIB value. For CSRA681xx and later ICs, this is a fine
                            frequency trim adjustment, with teMcSetXtalLoadCapacitance
                            providing a coarse trim adjustment.
                            <p>This function is not supported for BlueCore ICs - use
                            radiotestCfgXtalFtrim or radiotestCalcXtalOffset instead.

        """
        self.TestEngineDLL.teMcSetXtalFreqTrim.restype = CT.c_int32
        self.TestEngineDLL.teMcSetXtalFreqTrim.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_void_p]
        mibVal = CT.c_int16(_mibVal)
        retval = self.TestEngineDLL.teMcSetXtalFreqTrim(handle, trimVal, CT.byref(mibVal))
        return retval, mibVal.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teMcSetXtalLoadCapacitance(uint32 handle, uint16 value);
    #########################################################
    # TestEngine::teMcSetXtalLoadCapacitance
    @fntrace
    def teMcSetXtalLoadCapacitance(self, handle, value):
        r"""
        Function wrapper for TestEngine::teMcSetXtalLoadCapacitance().

        Python API:
            teMcSetXtalLoadCapacitance(handle, value)
            returns retval

        Generated from C API:

            Function :      int32 teMcSetXtalLoadCapacitance(uint32 handle, uint16 value)

            Parameters :    handle -
                                Handle to the device.

                            value -
                                Xtal load capacitance value to write (active
                                XtalLoadCapacitance MIB value).

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Sets the active crystal load capacitance value. For CSRA681xx and
                            later ICs, this is effectively a coarse frequency trim adjustment,
                            with teMcSetXtalFreqTrim providing a fine trim adjustment.
                            <p>This function is not supported for BlueCore and CSRC9xxx ICs.

        """
        self.TestEngineDLL.teMcSetXtalLoadCapacitance.restype = CT.c_int32
        self.TestEngineDLL.teMcSetXtalLoadCapacitance.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.teMcSetXtalLoadCapacitance(handle, value)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teGetBuildId(uint32 handle, uint8 subsystem, uint32* buildId);
    #########################################################
    # TestEngine::teGetBuildId
    @fntrace
    def teGetBuildId(self, handle, subsystem, _buildId=0):
        r"""
        Function wrapper for TestEngine::teGetBuildId().

        Python API:
            teGetBuildId(handle, subsystem, _buildId=0)
            returns retval, buildId.value

        Generated from C API:

            Function :      int32 teGetBuildId(uint32 handle, uint8 subsystem,
                                               uint16* buildId)

            Parameters :    handle -
                                Handle to the device

                            subsystem -
                                Subsystem ID, where:
                                <table>
                                    <tr><td>TE_SUBSYSTEM_BT = Bluetooth
                                    <tr><td>TE_SUBSYSTEM_AUDIO = Audio
                                    <tr><td>TE_SUBSYSTEM_APPS0 = Application 0
                                </table>

                            buildId -
                                Pointer to a variable to hold the returned firmware build ID

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will retrieve the firmware build ID for a
                            given subsystem. This function is supported for Multi-core
                            ICs only (for BlueCore ICs, use bccmdGetBuildId).

        """
        self.TestEngineDLL.teGetBuildId.restype = CT.c_int32
        self.TestEngineDLL.teGetBuildId.argtypes = [CT.c_uint32, CT.c_uint8, CT.c_void_p]
        buildId = CT.c_uint32(_buildId)
        retval = self.TestEngineDLL.teGetBuildId(handle, subsystem, CT.byref(buildId))
        return retval, buildId.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) tePioMap(uint32 handle, uint16 bank, uint32 mask, uint32 pios, uint32* errLines);
    #########################################################
    # TestEngine::tePioMap
    @fntrace
    def tePioMap(self, handle, bank, mask, pios, _errLines=0):
        r"""
        Function wrapper for TestEngine::tePioMap().

        Python API:
            tePioMap(handle, bank, mask, pios, _errLines=0)
            returns retval, errLines.value

        Generated from C API:

            Function :      int32 tePioMap(uint32 handle, uint16 bank, uint32 mask,
                                           uint32 pios, uint32* errLines)

            Parameters :    handle -
                                Handle to the device.

                            bank -
                                PIO bank.

                            mask -
                                Bit mask of the mappable pins.

                            pios -
                                Bit field, where a bit being set (1) means that that pin should
                                be configured as a PIO. A bit being cleared (0) means that the
                                pin will be configured for its alternative use.

                            errLines -
                                Pointer to a uint32 bit field indicating which of the specified
                                pins could not be mapped as PIOs.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will map the pins specified by the bank and mask
                            according to the usage specified in the pios parameter. Reports
                            any pins which could not be mapped.

        """
        self.TestEngineDLL.tePioMap.restype = CT.c_int32
        self.TestEngineDLL.tePioMap.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint32, CT.c_uint32, CT.c_void_p]
        errLines = CT.c_uint32(_errLines)
        retval = self.TestEngineDLL.tePioMap(handle, bank, mask, pios, CT.byref(errLines))
        return retval, errLines.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) tePioSet(uint32 handle, uint16 bank, uint32 mask, uint32 direction, uint32 value, uint32* errLines);
    #########################################################
    # TestEngine::tePioSet
    @fntrace
    def tePioSet(self, handle, bank, mask, direction, value, _errLines=0):
        r"""
        Function wrapper for TestEngine::tePioSet().

        Python API:
            tePioSet(handle, bank, mask, direction, value, _errLines=0)
            returns retval, errLines.value

        Generated from C API:

            Function :      int32 tePioSet(uint32 handle, uint16 bank, uint32 mask,
                                           uint32 direction, uint32 value, uint32* errLines)

            Parameters :    handle -
                                Handle to the device.

                            bank -
                                PIO bank.

                            mask -
                                Bit mask of the PIOs to be set.

                            direction -
                                Bit field which sets the PIOs as input (0) or output (1).

                            value -
                                Bit field, where a bit being set (1) means that that pin should
                                be set high. A bit being cleared (0) means that the pin will be
                                set low.

                            errLines -
                                Pointer to a uint32 bit field indicating which of the specified
                                pins could not be set.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will set the state for the specified PIOs.
                            <p>If the PIOs to be used include lines which default to alternative
                            uses, it is necessary to call tePioMap first to map the lines as
                            PIOs.

            Example :

                uint16 pioBank = 2;              // The PIO bank we want to use
                uint32 mapMask = 0x00000400;     // The line(s) we want to map as PIOs
                uint32 pioMask = 0x00000400;     // The line(s) we want to set
                uint32 direction = 0x00000400;   // Direction - (0) as input (1) as output
                uint32 value = 0x00000400;       // State of the lines to set (applies to output lines only)
                uint32 errLines;

                cout << "Trying to connect..." << endl;
                uint32 handle = openTestEngine(TRB, "1", 0, 5000, 0);

                if (handle != 0)
                {
                    cout << "Connected!" << endl;

                    // Map lines as PIOs
                    int32 success = tePioMap(handle, pioBank, mapMask, pioMask, &errLines);

                    if (success != TE_OK)
                    {
                        // Checks if any lines could not be mapped as PIOs
                        if (errLines != 0)
                        {
                            cout << "ERROR: Lines that could not be mapped as PIOs = "
                                 << hex << "0x" << errLines << endl;
                        }
                    }
                    else
                    {
                        // Set the direction and value of the PIO lines.
                        success = tePioSet(handle, pioBank, pioMask, direction, value, &errLines);

                        // Checks if any lines could not be set
                        if (success != TE_OK)
                        {
                            if (errLines != 0)
                            {
                                cout << "ERROR: Lines that could not be set = "
                                     << hex << "0x" << errLines << endl;
                            }
                        }
                    }

                    closeTestEngine(handle);
                }

        """
        self.TestEngineDLL.tePioSet.restype = CT.c_int32
        self.TestEngineDLL.tePioSet.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint32, CT.c_uint32, CT.c_uint32, CT.c_void_p]
        errLines = CT.c_uint32(_errLines)
        retval = self.TestEngineDLL.tePioSet(handle, bank, mask, direction, value, CT.byref(errLines))
        return retval, errLines.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) tePioGet(uint32 handle, uint16 bank, uint32* direction, uint32* value);
    #########################################################
    # TestEngine::tePioGet
    @fntrace
    def tePioGet(self, handle, bank, _direction=0, _value=0):
        r"""
        Function wrapper for TestEngine::tePioGet().

        Python API:
            tePioGet(handle, bank, _direction=0, _value=0)
            returns retval, direction.value, value.value

        Generated from C API:

            Function :      int32 tePioGet(uint32 handle, uint16 bank, uint32* direction,
                                           uint32* value)

            Parameters :    handle -
                                Handle to the device

                            bank -
                                PIO bank.

                            direction -
                                Pointer to uint32 value which stores the direction of the
                                PIO port lines as a bit field, where a 1 indicates that the
                                PIO is an output, and a 0 means that the PIO is an input.

                            value -
                                Pointer to uint32 value which stores the values of the PIO lines
                                as a bit field.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function gets the direction and state of the PIO lines.
                            If the PIOs to be read include lines which default to alternative
                            uses, it is necessary to call tePioMap first to map the lines
                            as PIOs.

            Example :

                uint16 pioBank = 2; // The PIO bank we want to use

                cout << "Trying to connect..." << endl;
                uint32 handle = openTestEngine(TRB, "1", 0, 5000, 0);

                if (handle != 0)
                {
                    cout << "Connected!" << endl;

                    // Get the direction and value of the PIO lines.
                    uint32 direction;
                    uint32 value;
                    int32 success = tePioGet(handle, pioBank, &direction, &value);
                    if (success == TE_OK)
                    {
                        cout << "Direction = " << hex << "0x" << direction << endl;
                        cout << "Value = " << hex << "0x" << value << endl;
                    }

                    closeTestEngine(handle);
                }

        """
        self.TestEngineDLL.tePioGet.restype = CT.c_int32
        self.TestEngineDLL.tePioGet.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_void_p, CT.c_void_p]
        direction = CT.c_uint32(_direction)
        value = CT.c_uint32(_value)
        retval = self.TestEngineDLL.tePioGet(handle, bank, CT.byref(direction), CT.byref(value))
        return retval, direction.value, value.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teAudioGetSource(uint32 handle, uint16 device, uint16 iface, uint16 channel, uint16* sid);
    #########################################################
    # TestEngine::teAudioGetSource
    @fntrace
    def teAudioGetSource(self, handle, device, iface, channel, _sid=0):
        r"""
        Function wrapper for TestEngine::teAudioGetSource().

        Python API:
            teAudioGetSource(handle, device, iface, channel, _sid=0)
            returns retval, sid.value

        Generated from C API:

            Function :      int32 teAudioGetSource(uint32 handle, uint16 device,
                                                   uint16 iface, uint16 channel,
                                                   uint16* sid)

            Parameters :    handle -
                                Handle to the device.

                            device -
                                The stream device identifier.

                            iface -
                                The hardware interface instance, e.g. 0, 1, etc.

                            channel -
                                Channel selection where:
                                <table>
                                    <tr><td>0 = Channel A
                                    <tr><td>1 = Channel B
                                    <tr><td>2 = Channels A and B
                                </table>
                                <p>If a single channel is selected, the other channel will be
                                muted.

                            sid -
                                Pointer to where the source ID will be stored.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to get the ID for an audio stream endpoint
                            source device. This sid can be used when configuring audio
                            configuration parameters using teAudioConfigure.
                            <p>
                            For further details of supported parameter values, please see the
                            Audio API reference documentation relevant to the ADK used to
                            build the IC application firmware (see the StreamAudioSource
                            function).

            Example:        See example code for teAudioConnect.

        """
        self.TestEngineDLL.teAudioGetSource.restype = CT.c_int32
        self.TestEngineDLL.teAudioGetSource.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16, CT.c_uint16, CT.c_void_p]
        sid = CT.c_uint16(_sid)
        retval = self.TestEngineDLL.teAudioGetSource(handle, device, iface, channel, CT.byref(sid))
        return retval, sid.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teAudioCloseSource(uint32 handle, uint16 sid);
    #########################################################
    # TestEngine::teAudioCloseSource
    @fntrace
    def teAudioCloseSource(self, handle, sid):
        r"""
        Function wrapper for TestEngine::teAudioCloseSource().

        Python API:
            teAudioCloseSource(handle, sid)
            returns retval

        Generated from C API:

            Function :      int32 teAudioCloseSource(uint32 handle, uint16 sid)

            Parameters :    handle -
                                Handle to the device.

                            sid -
                                Source ID obtained using teAudioGetSource.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to close the connection with an audio
                            stream source obtained using teAudioGetSource.

            Example:        See example code for teAudioConnect.

        """
        self.TestEngineDLL.teAudioCloseSource.restype = CT.c_int32
        self.TestEngineDLL.teAudioCloseSource.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.teAudioCloseSource(handle, sid)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teAudioGetSink(uint32 handle, uint16 device, uint16 iface, uint16 channel, uint16* sid);
    #########################################################
    # TestEngine::teAudioGetSink
    @fntrace
    def teAudioGetSink(self, handle, device, iface, channel, _sid=0):
        r"""
        Function wrapper for TestEngine::teAudioGetSink().

        Python API:
            teAudioGetSink(handle, device, iface, channel, _sid=0)
            returns retval, sid.value

        Generated from C API:

            Function :      int32 teAudioGetSink(uint32 handle, uint16 device, uint16 iface,
                                                 uint16 channel, uint16* sid)

            Parameters :    handle -
                                Handle to the device.

                            device -
                                The stream device identifier.

                            iface -
                                The hardware interface instance, e.g. 0, 1, etc.

                            channel -
                                Channel selection where:
                                <table>
                                    <tr><td>0 = Channel A
                                    <tr><td>1 = Channel B
                                    <tr><td>2 = Channels A and B
                                </table>
                                <p>If a single channel is selected, the other channel will be
                                muted.

                            sid -
                                Pointer to where the sink ID will be stored.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to get the ID for an audio stream endpoint
                            sink device. This sid can be used when configuring audio
                            configuration parameters using teAudioConfigure.
                            <p>
                            For further details of supported parameter values, please see the
                            Audio API reference documentation relevant to the ADK used to
                            build the IC application firmware (see the StreamAudioSink
                            function).

            Example:        See example code for teAudioConnect.

        """
        self.TestEngineDLL.teAudioGetSink.restype = CT.c_int32
        self.TestEngineDLL.teAudioGetSink.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16, CT.c_uint16, CT.c_void_p]
        sid = CT.c_uint16(_sid)
        retval = self.TestEngineDLL.teAudioGetSink(handle, device, iface, channel, CT.byref(sid))
        return retval, sid.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teAudioCloseSink(uint32 handle, uint16 sid);
    #########################################################
    # TestEngine::teAudioCloseSink
    @fntrace
    def teAudioCloseSink(self, handle, sid):
        r"""
        Function wrapper for TestEngine::teAudioCloseSink().

        Python API:
            teAudioCloseSink(handle, sid)
            returns retval

        Generated from C API:

            Function :      int32 teAudioCloseSink(uint32 handle, uint16 sid)

            Parameters :    handle -
                                Handle to the device.

                            sid -
                                Sink ID obtained using teAudioGetSink.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to close the connection with an audio
                            stream sink obtained using teAudioGetSink.

            Example:        See example code for teAudioConnect.

        """
        self.TestEngineDLL.teAudioCloseSink.restype = CT.c_int32
        self.TestEngineDLL.teAudioCloseSink.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.teAudioCloseSink(handle, sid)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teAudioConnect(uint32 handle, uint16 sourceId, uint16 sinkId, uint16* tid);
    #########################################################
    # TestEngine::teAudioConnect
    @fntrace
    def teAudioConnect(self, handle, sourceId, sinkId, _tid=0):
        r"""
        Function wrapper for TestEngine::teAudioConnect().

        Python API:
            teAudioConnect(handle, sourceId, sinkId, _tid=0)
            returns retval, tid.value

        Generated from C API:

            Function :      int32 teAudioConnect(uint32 handle, uint16 sourceId,
                                                 uint16 sinkId, uint16* tid)

            Parameters :    handle -
                                Handle to the device.

                            sourceId -
                                The source stream device identifier obtained using
                                teAudioGetSource.

                            sinkId -
                                The sink stream device identifier obtained using
                                teAudioGetSink.

                            tid -
                                Pointer to where the transform ID will be stored.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to connect a source audio stream device to
                            a sink device. This tid can be used to disconnect the streams
                            using teAudioDisconnect.

            Example:

                // Example: Stereo loopback

                static const uint16 CODEC_DEVICE = 3;
                static const uint16 AUDIO_INTERFACE = 1;

                cout << "Trying to connect..." << endl;
                uint32 handle = openTestEngine(TRB, "1", 0, 5000, 0);

                if (handle != 0)
                {
                    cout << "Connected!" << endl;

                    // Get sources and sinks for stereo loopback operation
                    uint16 sourceId0;
                    uint16 sourceId1;
                    uint16 sinkId0;
                    uint16 sinkId1;
                    int32 success = teAudioGetSource(handle, CODEC_DEVICE, AUDIO_INTERFACE, 0, &sourceId0);
                    if (success == TE_OK)
                    {
                        success = teAudioGetSource(handle, CODEC_DEVICE, AUDIO_INTERFACE, 1, &sourceId1);
                    }
                    if (success == TE_OK)
                    {
                        success = teAudioGetSink(handle, CODEC_DEVICE, AUDIO_INTERFACE, 0, &sinkId0);
                    }
                    if (success == TE_OK)
                    {
                        success = teAudioGetSink(handle, CODEC_DEVICE, AUDIO_INTERFACE, 1, &sinkId1);
                    }

                    // Connect the source and sink for each channel
                    uint16 transformId0;
                    uint16 transformId1;
                    if (success == TE_OK)
                    {
                        success = teAudioConnect(handle, sourceId0, sinkId0, &transformId0);
                    }
                    if (success == TE_OK)
                    {
                        success = teAudioConnect(handle, sourceId1, sinkId1, &transformId1);
                    }

                    // Audio loopback established - perform checks / measurements here

                    // Disconnect the audio streams
                    if (success == TE_OK)
                    {
                        success = teAudioDisconnect(handle, transformId0);
                    }
                    if (success == TE_OK)
                    {
                        success = teAudioDisconnect(handle, transformId1);
                    }

                    // Close the sources and sinks
                    if (success == TE_OK)
                    {
                        success = teAudioCloseSource(handle, sourceId0);
                    }
                    if (success == TE_OK)
                    {
                        success = teAudioCloseSource(handle, sourceId1);
                    }
                    if (success == TE_OK)
                    {
                        success = teAudioCloseSink(handle, sinkId0);
                    }
                    if (success == TE_OK)
                    {
                        success = teAudioCloseSink(handle, sinkId1);
                    }

                    closeTestEngine(handle);
                }

        """
        self.TestEngineDLL.teAudioConnect.restype = CT.c_int32
        self.TestEngineDLL.teAudioConnect.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16, CT.c_void_p]
        tid = CT.c_uint16(_tid)
        retval = self.TestEngineDLL.teAudioConnect(handle, sourceId, sinkId, CT.byref(tid))
        return retval, tid.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teAudioDisconnect(uint32 handle, uint16 tid);
    #########################################################
    # TestEngine::teAudioDisconnect
    @fntrace
    def teAudioDisconnect(self, handle, tid):
        r"""
        Function wrapper for TestEngine::teAudioDisconnect().

        Python API:
            teAudioDisconnect(handle, tid)
            returns retval

        Generated from C API:

            Function :      int32 teAudioDisconnect(uint32 handle, uint16 tid)

            Parameters :    handle -
                                Handle to the device.

                            tid -
                                Transform ID obtained using teAudioConnect.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to disconnect audio streams connected
                            using teAudioConnect.

            Example:        See example code for teAudioConnect.

        """
        self.TestEngineDLL.teAudioDisconnect.restype = CT.c_int32
        self.TestEngineDLL.teAudioDisconnect.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.teAudioDisconnect(handle, tid)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teAudioConfigure(uint32 handle, uint16 sid, uint16 key, uint32 value);
    #########################################################
    # TestEngine::teAudioConfigure
    @fntrace
    def teAudioConfigure(self, handle, sid, key, value):
        r"""
        Function wrapper for TestEngine::teAudioConfigure().

        Python API:
            teAudioConfigure(handle, sid, key, value)
            returns retval

        Generated from C API:

            Function :      int32 teAudioConfigure(uint32 handle, uint16 sid, uint16 key,
                                                   uint32 value)

            Parameters :    handle -
                                Handle to the device.

                            sid -
                                The source or sink stream device identifier obtained using
                                teAudioGetSource or teAudioGetSink.

                            key -
                                Audio configuration key to set.

                            value -
                                The value to set for the audio configuration key.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to set an audio configuration key value
                            for a given audio source / sink.
                            <p>
                            For further details of supported configuration keys, please see
                            the Audio API reference documentation relevant to the ADK used to
                            build the IC application firmware (see the Source/SinkConfigure
                            functions).

            Example:        See example code for teAudioStartOperators.

        """
        self.TestEngineDLL.teAudioConfigure.restype = CT.c_int32
        self.TestEngineDLL.teAudioConfigure.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16, CT.c_uint32]
        retval = self.TestEngineDLL.teAudioConfigure(handle, sid, key, value)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teAudioCreateOperator(uint32 handle, uint16 capabilityId, uint16* operatorId);
    #########################################################
    # TestEngine::teAudioCreateOperator
    @fntrace
    def teAudioCreateOperator(self, handle, capabilityId, _operatorId=0):
        r"""
        Function wrapper for TestEngine::teAudioCreateOperator().

        Python API:
            teAudioCreateOperator(handle, capabilityId, _operatorId=0)
            returns retval, operatorId.value

        Generated from C API:

            Function :      int32 teAudioCreateOperator(uint32 handle, uint16 capabilityId,
                                                        uint16* operatorId)

            Parameters :    handle -
                                Handle to the device.

                            capabilityId -
                                The capability identifier for which to create an operator.

                            operatorId -
                                Pointer to where the operator ID will be stored.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to create an audio operator for a given
                            audio capability.
                            <p>
                            For further details of supported capabilities, please see the
                            Kymera Capability Library User Guide documentation relevant to the
                            ADK used to build the IC application firmware (see Capability
                            Identifiers).

            Example:        See example code for teAudioStartOperators.

        """
        self.TestEngineDLL.teAudioCreateOperator.restype = CT.c_int32
        self.TestEngineDLL.teAudioCreateOperator.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_void_p]
        operatorId = CT.c_uint16(_operatorId)
        retval = self.TestEngineDLL.teAudioCreateOperator(handle, capabilityId, CT.byref(operatorId))
        return retval, operatorId.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teAudioDestroyOperators(uint32 handle, const uint16* operators, uint16 count);
    #########################################################
    # TestEngine::teAudioDestroyOperators
    @fntrace
    def teAudioDestroyOperators(self, handle, operators, count):
        r"""
        Function wrapper for TestEngine::teAudioDestroyOperators().

        Python API:
            teAudioDestroyOperators(handle, operators, count)
            returns retval

        Generated from C API:

            Function :      int32 teAudioDestroyOperators(uint32 handle,
                                                          const uint16* operators,
                                                          uint16 count)

            Parameters :    handle -
                                Handle to the device.

                            operators -
                                Array of operator IDs.

                            count -
                                Number of operator IDs, i.e. the size of the operators array.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to destroy operators created using
                            teAudioCreateOperator.

            Example:        See example code for teAudioStartOperators.

        """
        self.TestEngineDLL.teAudioDestroyOperators.restype = CT.c_int32
        self.TestEngineDLL.teAudioDestroyOperators.argtypes = [CT.c_uint32, CT.c_void_p, CT.c_uint16]
        __operators = (CT.c_uint16 * len(operators))(*operators)
        retval = self.TestEngineDLL.teAudioDestroyOperators(handle, CT.byref(__operators), count)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teAudioOperatorMessage(uint32 handle, uint16 operatorId, const uint16* message, uint16 messageLength);
    #########################################################
    # TestEngine::teAudioOperatorMessage
    @fntrace
    def teAudioOperatorMessage(self, handle, operatorId, message, messageLength):
        r"""
        Function wrapper for TestEngine::teAudioOperatorMessage().

        Python API:
            teAudioOperatorMessage(handle, operatorId, message, messageLength)
            returns retval

        Generated from C API:

            Function :      int32 teAudioOperatorMessage(uint32 handle, uint16 operatorId,
                                                         const uint16* message,
                                                         uint16 messageLength)

            Parameters :    handle -
                                Handle to the device.

                            operatorId -
                                Operator ID.

                            message -
                                Array of uint16 values forming the operator message.

                            messageLength -
                                Length of the message array (number of uint16s).

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to send a message to an operator created
                            using teAudioCreateOperator.
                            <p>
                            For further details of supported operator messages, please see the
                            Kymera Capability Library User Guide documentation relevant to the
                            ADK used to build the IC application firmware (see Operator
                            Messages).

            Example:        See example code for teAudioStartOperators.

        """
        self.TestEngineDLL.teAudioOperatorMessage.restype = CT.c_int32
        self.TestEngineDLL.teAudioOperatorMessage.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_void_p, CT.c_uint16]
        __message = (CT.c_uint16 * len(message))(*message)
        retval = self.TestEngineDLL.teAudioOperatorMessage(handle, operatorId, CT.byref(__message), messageLength)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teAudioStartOperators(uint32 handle, const uint16* operators, uint16 count);
    #########################################################
    # TestEngine::teAudioStartOperators
    @fntrace
    def teAudioStartOperators(self, handle, operators, count):
        r"""
        Function wrapper for TestEngine::teAudioStartOperators().

        Python API:
            teAudioStartOperators(handle, operators, count)
            returns retval

        Generated from C API:

            Function :      int32 teAudioStartOperators(uint32 handle,
                                                        const uint16* operators,
                                                        uint16 count)

            Parameters :    handle -
                                Handle to the device.

                            operators -
                                Array of operator IDs.

                            count -
                                Number of operator IDs, i.e. the size of the operators array.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to start operators created using
                            teAudioCreateOperator.

            Example:

                // Example: Play tone through codec

                static const uint16 CODEC_DEVICE = 3;
                static const uint16 AUDIO_INTERFACE = 1;
                static const uint16 AUDIO_CHANNEL = 2; // L & R
                static const uint16 RINGTONE_GENERATOR_CAPABILITY = 0x37;
                static const uint16 STREAM_CODEC_OUTPUT_RATE_KEYID = 0x301;
                static const uint16 STREAM_CODEC_OUTPUT_RATE = 8000;
                static const uint16 TONE_DURATION_MS = 1000;

                // Message corresponds to:
                // RINGTONE_DECAY(0),
                // RINGTONE_TIMBRE(sine),
                // RINGTONE_TEMPO(0xFF),
                // RINGTONE_NOTE(C5, SEMIBREVE),
                // RINGTONE_NOTE_TIE(C5, SEMIBREVE),
                // RINGTONE_NOTE_TIE(C5, SEMIBREVE),
                // RINGTONE_NOTE_TIE(C5, SEMIBREVE),
                // RINGTONE_NOTE_TIE(C5, SEMIBREVE),
                // RINGTONE_NOTE_TIE(C5, SEMIBREVE),
                // RINGTONE_NOTE_TIE(C5, SEMIBREVE),
                // RINGTONE_END
                //
                // It gives a tone approx. 6 seconds in duration (i.e. max duration would be approx. 6 seconds)
                static const uint16 RINGTONE_MESSAGE[] = { 0x1, 0xC000, 0xB000, 0x90FF, 0x1E01, 0x5E01, 0x5E01, 0x5E01, 0x5E01, 0x5E01, 0x5E01, 0x8000 };

                cout << "Trying to connect..." << endl;
                uint32 handle = openTestEngine(TRB, "1", 0, 5000, 0);

                if (handle != 0)
                {
                    cout << "Connected!" << endl;

                    // Get sink (output endpoint)
                    uint16 sinkId;
                    int32 success = teAudioGetSink(handle, CODEC_DEVICE, AUDIO_INTERFACE, AUDIO_CHANNEL, &sinkId);

                    // Configure the output sample rate
                    if (success == TE_OK)
                    {
                        success = teAudioConfigure(handle, sinkId, STREAM_CODEC_OUTPUT_RATE_KEYID, STREAM_CODEC_OUTPUT_RATE);
                    }

                    // Create an operator
                    uint16 operatorId;
                    if (success == TE_OK)
                    {
                        success = teAudioCreateOperator(handle, RINGTONE_GENERATOR_CAPABILITY, &operatorId);
                    }

                    // Messsage the operator with ringtone elements
                    if (success == TE_OK)
                    {
                        success = teAudioOperatorMessage(handle, operatorId, RINGTONE_MESSAGE, (sizeof(RINGTONE_MESSAGE) / sizeof(uint16)));
                    }

                    // Connect the source to the sink
                    uint16 transformId;
                    if (success == TE_OK)
                    {
                        // Need to get the source ID from the operator ID:
                        //     sourceId = opId + operator_connection_number + 0x2000
                        // The ringtone generator capability only has one output connection, so operator_connection_number = 0.
                        uint16 sourceId = operatorId + 0 + 0x2000;
                        success = teAudioConnect(handle, sourceId, sinkId, &transformId);
                    }

                    // Start the operator (starts tone)
                    if (success == TE_OK)
                    {
                        success = teAudioStartOperators(handle, &operatorId, 1);
                    }

                    // Tone now playing

                    // Play for intended duration
                    Sleep(TONE_DURATION_MS);

                    // Stop the operator (stops tone)
                    if (success == TE_OK)
                    {
                        success = teAudioStopOperators(handle, &operatorId, 1);
                    }

                    // Disconnect the audio streams
                    if (success == TE_OK)
                    {
                        success = teAudioDisconnect(handle, transformId);
                    }

                    // Destroy the operator
                    if (success == TE_OK)
                    {
                        success = teAudioDestroyOperators(handle, &operatorId, 1);
                    }

                    // Close the sink
                    if (success == TE_OK)
                    {
                        success = teAudioCloseSink(handle, sinkId);
                    }

                    closeTestEngine(handle);
                }

        """
        self.TestEngineDLL.teAudioStartOperators.restype = CT.c_int32
        self.TestEngineDLL.teAudioStartOperators.argtypes = [CT.c_uint32, CT.c_void_p, CT.c_uint16]
        __operators = (CT.c_uint16 * len(operators))(*operators)
        retval = self.TestEngineDLL.teAudioStartOperators(handle, CT.byref(__operators), count)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teAudioStopOperators(uint32 handle, const uint16* operators, uint16 count);
    #########################################################
    # TestEngine::teAudioStopOperators
    @fntrace
    def teAudioStopOperators(self, handle, operators, count):
        r"""
        Function wrapper for TestEngine::teAudioStopOperators().

        Python API:
            teAudioStopOperators(handle, operators, count)
            returns retval

        Generated from C API:

            Function :      int32 teAudioStopOperators(uint32 handle, const uint16* operators,
                                                       uint16 count)

            Parameters :    handle -
                                Handle to the device.

                            operators -
                                Array of operator IDs.

                            count -
                                Number of operator IDs, i.e. the size of the operators array.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to stop operators started using
                            teAudioStartOperators.

            Example:        See example code for teAudioStartOperators.

        """
        self.TestEngineDLL.teAudioStopOperators.restype = CT.c_int32
        self.TestEngineDLL.teAudioStopOperators.argtypes = [CT.c_uint32, CT.c_void_p, CT.c_uint16]
        __operators = (CT.c_uint16 * len(operators))(*operators)
        retval = self.TestEngineDLL.teAudioStopOperators(handle, CT.byref(__operators), count)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teAudioSetAncIirFilter(uint32 handle, uint16 ancInstance, uint16 pathId, const uint16* coefficients, uint16 numCoeffs);
    #########################################################
    # TestEngine::teAudioSetAncIirFilter
    @fntrace
    def teAudioSetAncIirFilter(self, handle, ancInstance, pathId, coefficients, numCoeffs):
        r"""
        Function wrapper for TestEngine::teAudioSetAncIirFilter().

        Python API:
            teAudioSetAncIirFilter(handle, ancInstance, pathId, coefficients, numCoeffs)
            returns retval

        Generated from C API:

            Function :      int32 teAudioSetAncIirFilter(uint32 handle, uint16 ancInstance,
                                                         uint16 pathId,
                                                         const uint16* coefficients,
                                                         uint16 numCoeffs)

            Parameters :    handle -
                                Handle to the device.

                            ancInstance -
                                ANC channel instance

                            pathId -
                                ID of the ANC path (and therefore the IIR filter instance to
                                configure).

                            coefficients -
                                Array of filter coefficients.

                            numCoeffs -
                                Number of coefficients supplied.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to configure an ANC IIR filter.
                            <p>
                            For further details of supported parameter values, please see the
                            Audio API reference documentation relevant to the ADK used to
                            build the IC application firmware (see the AudioAncFilterIirSet
                            function).

            Example:        See example code for teAudioStreamAncEnable.

        """
        self.TestEngineDLL.teAudioSetAncIirFilter.restype = CT.c_int32
        self.TestEngineDLL.teAudioSetAncIirFilter.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16, CT.c_void_p, CT.c_uint16]
        __coefficients = (CT.c_uint16 * len(coefficients))(*coefficients)
        retval = self.TestEngineDLL.teAudioSetAncIirFilter(handle, ancInstance, pathId, CT.byref(__coefficients), numCoeffs)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teAudioSetAncLpfFilter(uint32 handle, uint16 ancInstance, uint16 pathId, uint16 shift1, uint16 shift2);
    #########################################################
    # TestEngine::teAudioSetAncLpfFilter
    @fntrace
    def teAudioSetAncLpfFilter(self, handle, ancInstance, pathId, shift1, shift2):
        r"""
        Function wrapper for TestEngine::teAudioSetAncLpfFilter().

        Python API:
            teAudioSetAncLpfFilter(handle, ancInstance, pathId, shift1, shift2)
            returns retval

        Generated from C API:

            Function :      int32 teAudioSetAncLpfFilter(uint32 handle, uint16 ancInstance,
                                                         uint16 pathId, uint16 shift1,
                                                         uint16 shift2)

            Parameters :    handle -
                                Handle to the device.

                            ancInstance -
                                ANC channel instance

                            pathId -
                                ID of the ANC path (and therefore the LPF filter instance to
                                configure).

                            shift1 -
                                Used to derive the first LPF coefficient.

                            shift2 -
                                Used to derive the second LPF coefficient.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to configure an ANC LPF filter
                            <p>
                            For further details of supported parameter values, please see the
                            Audio API reference documentation relevant to the ADK used to
                            build the IC application firmware (see the AudioAncFilterLpfSet
                            function).

            Example:        See example code for teAudioStreamAncEnable.

        """
        self.TestEngineDLL.teAudioSetAncLpfFilter.restype = CT.c_int32
        self.TestEngineDLL.teAudioSetAncLpfFilter.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16, CT.c_uint16, CT.c_uint16]
        retval = self.TestEngineDLL.teAudioSetAncLpfFilter(handle, ancInstance, pathId, shift1, shift2)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teAudioStreamAncEnable(uint32 handle, uint16 anc0, uint16 anc1);
    #########################################################
    # TestEngine::teAudioStreamAncEnable
    @fntrace
    def teAudioStreamAncEnable(self, handle, anc0, anc1):
        r"""
        Function wrapper for TestEngine::teAudioStreamAncEnable().

        Python API:
            teAudioStreamAncEnable(handle, anc0, anc1)
            returns retval

        Generated from C API:

            Function :      int32 teAudioStreamAncEnable(uint32 handle, uint16 anc0,
                                                         uint16 anc1)

            Parameters :    handle -
                                Handle to the device.

                            anc0 -
                                Bit field that enables the ANC input and output paths of the
                                ANC0 instance.

                            anc1 -
                                Bit field that enables the ANC input and output paths of the
                                ANC1 instance.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to enable or disable ANC.
                            <p>
                            For further details of supported parameter values, please see the
                            Audio API reference documentation relevant to the ADK used to
                            build the IC application firmware (see the AudioAncStreamEnable
                            function).

            Example:

                // Example: Use of ANC audio functions

                static const uint16 CODEC_DEVICE = 3;
                static const uint16 AUDIO_INTERFACE = 0;
                static const uint16 STREAM_CODEC_INPUT_RATE_KEYID = 0x300;
                static const uint16 STREAM_CODEC_OUTPUT_RATE_KEYID = 0x301;
                static const uint16 STREAM_ANC_INSTANCE_KEYID = 0x1100;
                static const uint16 STREAM_ANC_INPUT_KEYID = 0x1101;
                static const uint16 STREAM_CODEC_SAMPLE_RATE = 48000;

                cout << "Trying to connect..." << endl;
                uint32 handle = openTestEngine(TRB, "1", 0, 5000, 0);

                if (handle != 0)
                {
                    cout << "Connected!" << endl;

                    // Get sources and sinks
                    uint16 sourceId0;
                    uint16 sourceId1;
                    uint16 sinkId0;
                    uint16 sinkId1;
                    int32 success = teAudioGetSource(handle, CODEC_DEVICE, AUDIO_INTERFACE, 0, &sourceId0);
                    if (success == TE_OK)
                    {
                        success = teAudioGetSource(handle, CODEC_DEVICE, AUDIO_INTERFACE, 1, &sourceId1);
                    }
                    if (success == TE_OK)
                    {
                        success = teAudioGetSink(handle, CODEC_DEVICE, AUDIO_INTERFACE, 0, &sinkId0);
                    }
                    if (success == TE_OK)
                    {
                        success = teAudioGetSink(handle, CODEC_DEVICE, AUDIO_INTERFACE, 1, &sinkId1);
                    }

                    // Configure the sample rates
                    if (success == TE_OK)
                    {
                        success = teAudioConfigure(handle, sourceId0, STREAM_CODEC_INPUT_RATE_KEYID, STREAM_CODEC_SAMPLE_RATE);
                    }
                    if (success == TE_OK)
                    {
                        success = teAudioConfigure(handle, sourceId1, STREAM_CODEC_INPUT_RATE_KEYID, STREAM_CODEC_SAMPLE_RATE);
                    }
                    if (success == TE_OK)
                    {
                        success = teAudioConfigure(handle, sinkId0, STREAM_CODEC_OUTPUT_RATE_KEYID, STREAM_CODEC_SAMPLE_RATE);
                    }
                    if (success == TE_OK)
                    {
                        success = teAudioConfigure(handle, sinkId1, STREAM_CODEC_OUTPUT_RATE_KEYID, STREAM_CODEC_SAMPLE_RATE);
                    }

                    // Set ANC instances / inputs
                    if (success == TE_OK)
                    {
                        success = teAudioConfigure(handle, sourceId0, STREAM_ANC_INSTANCE_KEYID, 1);
                    }
                    if (success == TE_OK)
                    {
                        success = teAudioConfigure(handle, sourceId1, STREAM_ANC_INSTANCE_KEYID, 2);
                    }
                    if (success == TE_OK)
                    {
                        success = teAudioConfigure(handle, sourceId0, STREAM_ANC_INPUT_KEYID, 1);
                    }
                    if (success == TE_OK)
                    {
                        success = teAudioConfigure(handle, sourceId1, STREAM_ANC_INPUT_KEYID, 1);
                    }
                    if (success == TE_OK)
                    {
                        success = teAudioConfigure(handle, sinkId0, STREAM_ANC_INSTANCE_KEYID, 1);
                    }
                    if (success == TE_OK)
                    {
                        success = teAudioConfigure(handle, sinkId1, STREAM_ANC_INSTANCE_KEYID, 2);
                    }

                    // Set IIR filters
                    static const uint16 NUM_IIR_COEFFS = 15;
                    uint16 iirCoefficients[NUM_IIR_COEFFS] = { 0xFE4A, 0x71, 0xFF5E, 0x29, 0xFFFF, 0x1, 0xFFFF, 0x167, 0x1B2, 0xFDDB, 0xFFC0, 0x21, 0xFFA2, 0xFFFF, 0x1 };
                    if (success == TE_OK)
                    {
                        success = teAudioSetAncIirFilter(handle, 1, 1, iirCoefficients, NUM_IIR_COEFFS);
                    }
                    if (success == TE_OK)
                    {
                        success = teAudioSetAncIirFilter(handle, 2, 1, iirCoefficients, NUM_IIR_COEFFS);
                    }

                    // Set LPF filters
                    if (success == TE_OK)
                    {
                        success = teAudioSetAncLpfFilter(handle, 1, 1, 5, 5);
                    }
                    if (success == TE_OK)
                    {
                        success = teAudioSetAncLpfFilter(handle, 2, 1, 5, 5);
                    }

                    // Enable ANC
                    if (success == TE_OK)
                    {
                        success = teAudioStreamAncEnable(handle, 9, 9);
                    }

                    // Close the sources and sinks
                    if (success == TE_OK)
                    {
                        success = teAudioCloseSource(handle, sourceId0);
                    }
                    if (success == TE_OK)
                    {
                        success = teAudioCloseSource(handle, sourceId1);
                    }
                    if (success == TE_OK)
                    {
                        success = teAudioCloseSink(handle, sinkId0);
                    }
                    if (success == TE_OK)
                    {
                        success = teAudioCloseSink(handle, sinkId1);
                    }

                    closeTestEngine(handle);
                }

        """
        self.TestEngineDLL.teAudioStreamAncEnable.restype = CT.c_int32
        self.TestEngineDLL.teAudioStreamAncEnable.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16]
        retval = self.TestEngineDLL.teAudioStreamAncEnable(handle, anc0, anc1)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teCapacitiveSensorRead(uint32 handle, uint16 pad, uint32* value);
    #########################################################
    # TestEngine::teCapacitiveSensorRead
    @fntrace
    def teCapacitiveSensorRead(self, handle, pad, _value=0):
        r"""
        Function wrapper for TestEngine::teCapacitiveSensorRead().

        Python API:
            teCapacitiveSensorRead(handle, pad, _value=0)
            returns retval, value.value

        Generated from C API:

            Function :      int32 teCapacitiveSensorRead(uint32 handle, uint16 pad,
                                                         uint16* value)

            Parameters :    handle -
                                Handle to the device

                            pad -
                                The pad to read.

                            value -
                                Absolute capacitance read from the pad in units of 1fF.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function reads the capacitance value of the specified
                            capacitative sense pad.

        """
        self.TestEngineDLL.teCapacitiveSensorRead.restype = CT.c_int32
        self.TestEngineDLL.teCapacitiveSensorRead.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_void_p]
        value = CT.c_uint32(_value)
        retval = self.TestEngineDLL.teCapacitiveSensorRead(handle, pad, CT.byref(value))
        return retval, value.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teCheckLicense(uint32 handle, uint8 featureId, uint8* result);
    #########################################################
    # TestEngine::teCheckLicense
    @fntrace
    def teCheckLicense(self, handle, featureId, _result=0):
        r"""
        Function wrapper for TestEngine::teCheckLicense().

        Python API:
            teCheckLicense(handle, featureId, _result=0)
            returns retval, result.value

        Generated from C API:

            Function :      int32 teCheckLicense(uint32 handle, uint8 featureId, uint8* result)

            Parameters :    handle -
                                Handle to the device

                            featureId -
                                The ID number of the feature to check.

                            result -
                                Boolean value (0 = license check failed, 1 = license check
                                passed).

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function gets the license status for a given feature.

        """
        self.TestEngineDLL.teCheckLicense.restype = CT.c_int32
        self.TestEngineDLL.teCheckLicense.argtypes = [CT.c_uint32, CT.c_uint8, CT.c_void_p]
        result = CT.c_uint8(_result)
        retval = self.TestEngineDLL.teCheckLicense(handle, featureId, CT.byref(result))
        return retval, result.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teAppDisable(uint32 handle, uint16 reserved);
    #########################################################
    # TestEngine::teAppDisable
    @fntrace
    def teAppDisable(self, handle, reserved):
        r"""
        Function wrapper for TestEngine::teAppDisable().

        Python API:
            teAppDisable(handle, reserved)
            returns retval

        Generated from C API:

            Function :      int32 teAppDisable(uint32 handle, uint16 reserved)

            Parameters :    handle -
                                Handle to the device

                            reserved -
                                Currently unused.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   For applicable ICs, this disables any application running in apps
                            processor 1. It is not supported for BlueCore or CSRC9xxx ICs.

        """
        self.TestEngineDLL.teAppDisable.restype = CT.c_int32
        self.TestEngineDLL.teAppDisable.argtypes = [CT.c_uint32, CT.c_uint16]
        retval = self.TestEngineDLL.teAppDisable(handle, reserved)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teAppWrite(uint32 handle, uint8 channel, const uint16 data[], uint16 length);
    #########################################################
    # TestEngine::teAppWrite
    @fntrace
    def teAppWrite(self, handle, channel, data, length):
        r"""
        Function wrapper for TestEngine::teAppWrite().

        Python API:
            teAppWrite(handle, channel, data, length)
            returns retval

        Generated from C API:

            Function :      int32 teAppWrite(uint32 handle, uint8 channel, const uint16* data,
                                             uint16 length)

            Parameters :    handle -
                                Handle to the device.

                            channel -
                                The channel number for the message (0...127).

                            data -
                                A pointer to an array of 16-bit unsigned integers containing
                                the message payload.

                            length -
                                The number of words to send. Must be between 1 and 80, and
                                no larger than the size of the data array.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to write a message to an application
                            running on CSRA681xx and later ICs.
                            <p>This function is not supported for BlueCore ICs - use vmWrite
                            instead.

            Example :

                cout << "Trying to connect..." << endl;
                uint32 handle = openTestEngine(TRB, "1", 0, 5000, 0);

                if (handle != 0)
                {
                    cout << "Connected!" << endl;

                    // Write a message
                    static const uint16 MSG_LEN = 3;
                    uint16 txData[MSG_LEN] = { 1, 2, 3 };
                    int32 success = teAppWrite(handle, 0, txData, MSG_LEN);
                    if (success == TE_OK)
                    {
                        cout << "Message sent to application" << endl;

                        // Read a message (e.g. response from application to above message)
                        static const uint16 MAX_MSG_LEN = 80;
                        uint16 rxData[MAX_MSG_LEN];
                        uint8 channel;
                        uint16 readLength;
                        success = teAppRead(handle, &channel, rxData, MAX_MSG_LEN, &readLength, 1000);
                        if (success == TE_OK)
                        {
                            cout << "Message received from application on channel "
                                 << static_cast<uint16>(channel) << ":" << endl;
                            for (uint16 i = 0; i < readLength; ++i)
                            {
                                cout << rxData[i] << endl;
                            }
                        }
                    }

                    closeTestEngine(handle);
                }

        """
        self.TestEngineDLL.teAppWrite.restype = CT.c_int32
        self.TestEngineDLL.teAppWrite.argtypes = [CT.c_uint32, CT.c_uint8, CT.c_void_p, CT.c_uint16]
        __data = (CT.c_uint16 * len(data))(*data)
        retval = self.TestEngineDLL.teAppWrite(handle, channel, CT.byref(__data), length)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teAppRead(uint32 handle, uint8* channel, uint16 data[], uint16 dataLength, uint16* readLength, uint16 timeoutMs);
    #########################################################
    # TestEngine::teAppRead
    @fntrace
    def teAppRead(self, handle, data, dataLength, timeoutMs, _channel=0, _readLength=0):
        r"""
        Function wrapper for TestEngine::teAppRead().

        Python API:
            teAppRead(handle, data, dataLength, timeoutMs, _channel=0, _readLength=0)
            returns retval, channel.value, readLength.value

        Generated from C API:

            Function :      int32 teAppRead(uint32 handle, uint8* channel, uint16* data,
                                            uint16 dataLength, uint16* readLength,
                                            uint16 timeoutMs)

            Parameters :    handle -
                                Handle to the device.

                            channel -
                                Location where the channel number of the message will be
                                stored.

                            data -
                                A pointer to an array of 16-bit unsigned integers where the
                                message data will be stored.

                            dataLength -
                                The length of the data array. Must be >= 1.

                            readLength -
                                Location where the length of any message read will be stored.
                                If the dataLength indicates that the data array is too small to
                                hold the message data, TE_ERROR will be retured and the value
                                will be set to the length of the message. As much data as
                                possible will be stored in the data array in this case.

                            timeoutMs -
                                A timeout, in milliseconds, of the time to wait for a message.
                                A value of 0 means the function will just poll.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to read any message returned from an
                            application running on CSRA681xx and later ICs.
                            <p>If a message has already been recieved, it will be returned
                            immediately, otherwise the function will wait for a message until
                            the timeout has expired. If no data has been read within the
                            timeout period, TE_ERROR will be returned and readLength will be
                            set to zero.
                            <p>This function is not supported for BlueCore ICs - use vmRead
                            instead.

            Example:        See example code for teAppWrite.

        """
        self.TestEngineDLL.teAppRead.restype = CT.c_int32
        self.TestEngineDLL.teAppRead.argtypes = [CT.c_uint32, CT.c_void_p, CT.c_void_p, CT.c_uint16, CT.c_void_p, CT.c_uint16]
        channel = CT.c_uint8(_channel)
        __data = (CT.c_uint16 * len(data))(*data)
        readLength = CT.c_uint16(_readLength)
        retval = self.TestEngineDLL.teAppRead(handle, CT.byref(channel), CT.byref(__data), dataLength, CT.byref(readLength), timeoutMs)
        return retval, channel.value, readLength.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teAdcGet(uint32 handle, uint16 adc, uint16 delay, uint16 reserved, uint16* result);
    #########################################################
    # TestEngine::teAdcGet
    @fntrace
    def teAdcGet(self, handle, adc, delay, reserved, _result=0):
        r"""
        Function wrapper for TestEngine::teAdcGet().

        Python API:
            teAdcGet(handle, adc, delay, reserved, _result=0)
            returns retval, result.value

        Generated from C API:

            Function :      int32 teAdcGet(uint32 handle, uint16 adc, uint16 delay,
                                           uint16 reserved, uint16* result)

            Parameters :    handle -
                                Handle to the device

                            adc -
                                ADC which will be read. The ADCs which can be read are IC
                                dependent.

                            delay -
                                Delay in milliseconds used before the read is taken.
                                <p>This parameter is ignored for BlueCore ICs.

                            reserved -
                                Currently unused.

                            result -
                                Holds the result of the ADC reading.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function will perform an ADC reading on the specified ADC.

        """
        self.TestEngineDLL.teAdcGet.restype = CT.c_int32
        self.TestEngineDLL.teAdcGet.argtypes = [CT.c_uint32, CT.c_uint16, CT.c_uint16, CT.c_uint16, CT.c_void_p]
        result = CT.c_uint16(_result)
        retval = self.TestEngineDLL.teAdcGet(handle, adc, delay, reserved, CT.byref(result))
        return retval, result.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teChargerSetConfig(uint32 handle, const uint16 config[]);
    #########################################################
    # TestEngine::teChargerSetConfig
    @fntrace
    def teChargerSetConfig(self, handle, config):
        r"""
        Function wrapper for TestEngine::teChargerSetConfig().

        Python API:
            teChargerSetConfig(handle, config)
            returns retval

        Generated from C API:

            Function :      int32 teChargerSetConfig(uint32 handle, const uint16* config)

            Parameters :    handle -
                                Handle to the device.

                            config -
                                Pointer to an array of exactly 13 16-bit unsigned integers
                                which define the configuration parameters. Each indexed value
                                maps to a configuration setting as follows:
                                <tr><td>0 = pre_external_milliohms
                                <tr><td>1 = fast_external_milliohms
                                <tr><td>2 = trickle_milliamps
                                <tr><td>3 = pre_milliamps
                                <tr><td>4 = fast_milliamps
                                <tr><td>5 = pre_fast_threshold
                                <tr><td>6 = float_constant_voltage
                                <tr><td>7 = termination_current
                                <tr><td>8 = termination_debounce
                                <tr><td>9 = standby_fast_hysteris
                                <tr><td>10 = charger_configure_type (unused for firmware before ADK6.1)
                                <tr><td>11 = unused (reserved for future use)
                                <tr><td>12 = enable

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to configure the battery charger.

            Example:        See example code for teChargerGetStatus.

        """
        self.TestEngineDLL.teChargerSetConfig.restype = CT.c_int32
        self.TestEngineDLL.teChargerSetConfig.argtypes = [CT.c_uint32, CT.c_void_p]
        __config = (CT.c_uint16 * len(config))(*config)
        retval = self.TestEngineDLL.teChargerSetConfig(handle, CT.byref(__config))
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teChargerGetStatus(uint32 handle, uint16* status);
    #########################################################
    # TestEngine::teChargerGetStatus
    @fntrace
    def teChargerGetStatus(self, handle, _status=0):
        r"""
        Function wrapper for TestEngine::teChargerGetStatus().

        Python API:
            teChargerGetStatus(handle, _status=0)
            returns retval, status.value

        Generated from C API:

            Function :      int32 teChargerGetStatus(uint32 handle, uint16* status)

            Parameters :    handle -
                                Handle to the device.

                            status -
                                Pointer to a 16-bit unsigned integer to hold the returned
                                status of the charger.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to read the state of the battery charger.

            Example:

                cout << "Trying to connect..." << endl;
                uint32 handle = openTestEngine(TRB, "1", 0, 5000, 0);

                if (handle != 0)
                {
                    cout << "Connected!" << endl;

                    // Configure the charger parameters
                    uint16 configParams[] = {
                        0,      // pre_external_milliohms
                        0,      // fast_external_milliohms
                        50,     // trickle_milliamps
                        100,    // pre_milliamps
                        200,    // fast_milliamps
                        2,      // pre_fast_threshold
                        11,     // float_constant_voltage
                        0,      // termination_current
                        3,      // termination_debounce
                        1,      // standby_fast_hysteris
                        0,      // charger_configure_type
                        0,      // unused_expansion2
                        1       // enable
                    };

                    int32 success = teChargerSetConfig(handle, configParams);

                    // Get the charger status
                    if (success == TE_OK)
                    {
                        uint16 status;
                        success = teChargerGetStatus(handle, &status);
                        if (success == TE_OK)
                        {
                            cout << "Charger status = " << status << endl;
                        }
                    }

                    closeTestEngine(handle);
                }

        """
        self.TestEngineDLL.teChargerGetStatus.restype = CT.c_int32
        self.TestEngineDLL.teChargerGetStatus.argtypes = [CT.c_uint32, CT.c_void_p]
        status = CT.c_uint16(_status)
        retval = self.TestEngineDLL.teChargerGetStatus(handle, CT.byref(status))
        return retval, status.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teNfcConfigure(uint32 handle, uint8 enable);
    #########################################################
    # TestEngine::teNfcConfigure
    @fntrace
    def teNfcConfigure(self, handle, enable):
        r"""
        Function wrapper for TestEngine::teNfcConfigure().

        Python API:
            teNfcConfigure(handle, enable)
            returns retval

        Generated from C API:

            Function :      int32 teNfcConfigure(uint32 handle, uint8 enable)

            Parameters :    handle -
                                Handle to the device.

                            enable -
                                Boolean value (0 = Disable NFC, 1 = Enable NFC).

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to enable / disable NFC for testing.

        """
        self.TestEngineDLL.teNfcConfigure.restype = CT.c_int32
        self.TestEngineDLL.teNfcConfigure.argtypes = [CT.c_uint32, CT.c_uint8]
        retval = self.TestEngineDLL.teNfcConfigure(handle, enable)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teConfigCacheInit(uint32 handle, const char * configDb);
    #########################################################
    # TestEngine::teConfigCacheInit
    @fntrace
    def teConfigCacheInit(self, handle, configDb):
        r"""
        Function wrapper for TestEngine::teConfigCacheInit().

        Python API:
            teConfigCacheInit(handle, configDb)
            returns retval

        Generated from C API:

            Function :      int32 teConfigCacheInit(uint32 handle, const char * configDb)

            Parameters :    handle -
                                Handle to the device.

                            configDb -
                                Configuration database specifier in the following format:
                                "DbFilePath:SysVerLabel", e.g. "hyd.sdb:CSRA68100_CONFIG".
                                <p>The MIB database file should be a *.sdb file. The system
                                version label determines the dataset within the database.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to initialise the configuration cache. It
                            must be called before any other teConfigCache* functions are used.

            Example :       See example code for teConfigCacheWrite.

        """
        self.TestEngineDLL.teConfigCacheInit.restype = CT.c_int32
        self.TestEngineDLL.teConfigCacheInit.argtypes = [CT.c_uint32, CT.c_char_p]
        retval = self.TestEngineDLL.teConfigCacheInit(handle, configDb)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teConfigCacheRead(uint32 handle, const char* file, uint16 reserved);
    #########################################################
    # TestEngine::teConfigCacheRead
    @fntrace
    def teConfigCacheRead(self, handle, file, reserved):
        r"""
        Function wrapper for TestEngine::teConfigCacheRead().

        Python API:
            teConfigCacheRead(handle, file, reserved)
            returns retval

        Generated from C API:

            Function :      int32 teConfigCacheRead(uint32 handle, const char* file,
                                                    uint16 reserved)

            Parameters :    handle -
                                Handle to the device.

                            file -
                                Path of a *.htf file containing configuration data to load
                                into the cache, or NULL to specify that configuration data
                                should be read from the connected device.

                            reserved -
                                Currently unused.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to read configuration data into the cache.

            Example :       See example code for teConfigCacheWrite.

        """
        self.TestEngineDLL.teConfigCacheRead.restype = CT.c_int32
        self.TestEngineDLL.teConfigCacheRead.argtypes = [CT.c_uint32, CT.c_char_p, CT.c_uint16]
        retval = self.TestEngineDLL.teConfigCacheRead(handle, file, reserved)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teConfigCacheReadItem(uint32 handle, const char* key, char* value, uint32* maxLen);
    #########################################################
    # TestEngine::teConfigCacheReadItem
    @fntrace
    def teConfigCacheReadItem(self, handle, key, _value='', _maxLen=0):
        r"""
        Function wrapper for TestEngine::teConfigCacheReadItem().

        Python API:
            teConfigCacheReadItem(handle, key, _value='', _maxLen=0)
            returns retval, value.value, maxLen.value

        Generated from C API:

            Function :      int32 teConfigCacheReadItem(uint32 handle, const char* key,
                                                        char* value, uint32* maxLen)

            Parameters :    handle -
                                Handle to the device.

                            key -
                                Key specifier in the following format:
                                "Subsys[Layer]:Name", e.g. "curator3:XtalFreqTrim", or
                                "curator:XtalFreqTrim".

                            value -
                                Location where the key value will be stored in the same
                                format as is used when written to a file (but without
                                descriptive text).

                            maxLen -
                                Input / output parameter, where the input value is the length
                                of the value array (in bytes). If the given length is
                                insufficient to store the key's value, an error is returned
                                (and the value will be set to the required length).

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to read a configuration key value from the
                            cache. If the layer part of the key specifier is omitted, the key
                            value read will be that set in the highest layer.

            Example :       See example code for teConfigCacheWrite.

        """
        self.TestEngineDLL.teConfigCacheReadItem.restype = CT.c_int32
        self.TestEngineDLL.teConfigCacheReadItem.argtypes = [CT.c_uint32, CT.c_char_p, CT.c_char_p, CT.c_void_p]
        value = CT.create_string_buffer(_value, 255)
        maxLen = CT.c_uint32(_maxLen)
        retval = self.TestEngineDLL.teConfigCacheReadItem(handle, key, value, CT.byref(maxLen))
        return retval, value.value, maxLen.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teConfigCacheWriteItem(uint32 handle, const char* key, const char* value);
    #########################################################
    # TestEngine::teConfigCacheWriteItem
    @fntrace
    def teConfigCacheWriteItem(self, handle, key, value):
        r"""
        Function wrapper for TestEngine::teConfigCacheWriteItem().

        Python API:
            teConfigCacheWriteItem(handle, key, value)
            returns retval

        Generated from C API:

            Function :      int32 teConfigCacheWriteItem(uint32 handle, const char* key,
                                                         const char* value)

            Parameters :    handle -
                                Handle to the device.

                            key -
                                Key specifier in the following format:
                                "SubsysLayer:Name", e.g. "curator3:XtalFreqTrim".

                            value -
                                The value to write. The format is the same as is used for
                                text (*.htf) files (but without any descriptive text).
                                <p>To delete a value, use the value "NULL" or an empty
                                string.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to write a configuration key value to the
                            cache.

            Example :       See example code for teConfigCacheWrite.

        """
        self.TestEngineDLL.teConfigCacheWriteItem.restype = CT.c_int32
        self.TestEngineDLL.teConfigCacheWriteItem.argtypes = [CT.c_uint32, CT.c_char_p, CT.c_char_p]
        retval = self.TestEngineDLL.teConfigCacheWriteItem(handle, key, value)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teConfigCacheWrite(uint32 handle, const char* file, uint16 reserved);
    #########################################################
    # TestEngine::teConfigCacheWrite
    @fntrace
    def teConfigCacheWrite(self, handle, file, reserved):
        r"""
        Function wrapper for TestEngine::teConfigCacheWrite().

        Python API:
            teConfigCacheWrite(handle, file, reserved)
            returns retval

        Generated from C API:

            Function :      int32 teConfigCacheWrite(uint32 handle, const char* file,
                                                     uint16 reserved)

            Parameters :    handle -
                                Handle to the device.

                            file -
                                Path of a *.htf file to write the configuration cache data to.
                                If NULL, the configuration cache data is written to the
                                connected device.

                            reserved -
                                Currently unused.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function is used to write configuration data from the cache.

            Example :

                static const char* const CFG_DB_PARAM = "hyd.sdb:CSRA68100_CONFIG";
                static const uint32 KEY_READ_BUFFER_LEN = 128;

                cout << "Trying to connect..." << endl;
                uint32 handle = openTestEngine(TRB, "1", 0, 5000, 0);

                if (handle != 0)
                {
                    cout << "Connected!" << endl;

                    // Initialise the configuration cache
                    int32 success = teConfigCacheInit(handle, CFG_DB_PARAM);

                    // Read the configuration into the cache from the device
                    uint16 unused = 0;
                    if (success == TE_OK)
                    {
                        success = teConfigCacheRead(handle, NULL, unused);
                    }

                    // Read current XTAL trim value
                    char valueString[KEY_READ_BUFFER_LEN];
                    uint32 maxLen = KEY_READ_BUFFER_LEN;
                    if (success == TE_OK)
                    {
                        success = teConfigCacheReadItem(handle, "curator:XtalFreqTrim", valueString, &maxLen);
                        if (success == TE_OK)
                        {
                            cout << "curator:XtalFreqTrim = " << valueString << endl;
                        }
                    }

                    // Write updated XTAL trim value
                    if (success == TE_OK)
                    {
                        success = teConfigCacheWriteItem(handle, "curator15:XtalFreqTrim", "3");
                    }

                    // Write Bluetooth address
                    if (success == TE_OK)
                    {
                        success = teConfigCacheWriteItem(handle, "bt15:PSKEY_BDADDR", "{0x3456,0x5B,0x02}");
                    }

                    // Write the configuration cache to the device
                    if (success == TE_OK)
                    {
                        success = teConfigCacheWrite(handle, NULL, unused);
                    }

                    closeTestEngine(handle);
                }

        """
        self.TestEngineDLL.teConfigCacheWrite.restype = CT.c_int32
        self.TestEngineDLL.teConfigCacheWrite.argtypes = [CT.c_uint32, CT.c_char_p, CT.c_uint16]
        retval = self.TestEngineDLL.teConfigCacheWrite(handle, file, reserved)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) teChipReset(uint32 handle, uint32 reserved);
    #########################################################
    # TestEngine::teChipReset
    @fntrace
    def teChipReset(self, handle, reserved):
        r"""
        Function wrapper for TestEngine::teChipReset().

        Python API:
            teChipReset(handle, reserved)
            returns retval

        Generated from C API:

            Function :      int32 teChipReset(uint32 handle, uint32 reserved)

            Parameters :    handle -
                                Handle to the device.

                            reserved -
                                Currently unused.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   This function causes a reset of the connected device. It will
                            attempt to reinitialise communication using the same transport
                            settings as used when connecting.

        """
        self.TestEngineDLL.teChipReset.restype = CT.c_int32
        self.TestEngineDLL.teChipReset.argtypes = [CT.c_uint32, CT.c_uint32]
        retval = self.TestEngineDLL.teChipReset(handle, reserved)
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) tePsRead(uint32 handle, uint32 keyId, uint16 valueLen, uint16 value[], uint16* readLen);
    #########################################################
    # TestEngine::tePsRead
    @fntrace
    def tePsRead(self, handle, keyId, valueLen, value, _readLen=0):
        r"""
        Function wrapper for TestEngine::tePsRead().

        Python API:
            tePsRead(handle, keyId, valueLen, value, _readLen=0)
            returns retval, readLen.value

        Generated from C API:

            Function :      int32 tePsRead(uint32 handle, uint32 keyId, uint16 valueLen,
                                           uint16* value, uint16* readLen)

            Parameters :    handle -
                                Handle to the device.

                            keyId -
                                PS key ID to read. Maximum value 0xFFFF.

                            valueLen -
                                Size of the data array allocated to hold the key value.
                                Must be between 0 and 64, where 0 indicates that only the
                                length of the key's value is to be read.

                            value -
                                Array of 16-bit words of size "valueLen" to hold the key
                                value. Can be NULL if valueLen is 0.

                            readLen -
                                Pointer to a 16-bit word to hold to length of the key's value.
                                This is the length of the valid value data read for the key
                                (may be less the "valueLen" given if that exceeds the actual
                                length of the key's value).
                                <p>If the key has not been written this value will be set to
                                zero.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Used to read the persistent store key specified for USR (user),
                            DSP, CONNLIB (connectivity library) and CUSTOMER PS keys (for
                            audio PS keys use tePsAudioRead).
                            <p>
                            Refer to the PS key documentation in the relevant ADK for the
                            details of supported PS keys (the ps_if.h file included in ADK
                            releases defines the PS key ID ranges for each of the supported
                            key groups).
                            <p>
                            This function is unsupported for BlueCore and CSRC9xxx ICs (for
                            BlueCore ICs use the psRead* functions).

        """
        self.TestEngineDLL.tePsRead.restype = CT.c_int32
        self.TestEngineDLL.tePsRead.argtypes = [CT.c_uint32, CT.c_uint32, CT.c_uint16, CT.c_void_p, CT.c_void_p]
        __value = (CT.c_uint16 * len(value))(*value)
        readLen = CT.c_uint16(_readLen)
        retval = self.TestEngineDLL.tePsRead(handle, keyId, valueLen, CT.byref(__value), CT.byref(readLen))
        return retval, readLen.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) tePsWrite(uint32 handle, uint32 keyId, uint16 valueLen, const uint16 value[]);
    #########################################################
    # TestEngine::tePsWrite
    @fntrace
    def tePsWrite(self, handle, keyId, valueLen, value):
        r"""
        Function wrapper for TestEngine::tePsWrite().

        Python API:
            tePsWrite(handle, keyId, valueLen, value)
            returns retval

        Generated from C API:

            Function :      int32 tePsWrite(uint32 handle, uint32 keyId, uint16 valueLen,
                                            const uint16* value)

            Parameters :    handle -
                                Handle to the device.

                            keyId -
                                PS key ID to write. Maximum value 0xFFFF.

                            valueLen -
                                Size of the data array holding the key value.
                                Must be between 0 and 64, where 0 effectively means that the
                                key will be deleted.

                            value -
                                Array of 16-bit words of size "valueLen" holding the key
                                value.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Used to write the persistent store key specified for USR (user),
                            DSP, CONNLIB (connectivity library) and CUSTOMER PS keys (for
                            audio PS keys use tePsAudioWrite).
                            <p>
                            Refer to the PS key documentation in the relevant ADK for the
                            details of supported PS keys (the ps_if.h file included in ADK
                            releases defines the PS key ID ranges for each of the supported
                            key groups).
                            <p>This function is unsupported for BlueCore and CSRC9xxx ICs
                            (for BlueCore ICs use the psWrite* functions).

        """
        self.TestEngineDLL.tePsWrite.restype = CT.c_int32
        self.TestEngineDLL.tePsWrite.argtypes = [CT.c_uint32, CT.c_uint32, CT.c_uint16, CT.c_void_p]
        __value = (CT.c_uint16 * len(value))(*value)
        retval = self.TestEngineDLL.tePsWrite(handle, keyId, valueLen, CT.byref(__value))
        return retval
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) tePsAudioRead(uint32 handle, uint32 keyId, uint16 valueLen, uint16 value[], uint16* readLen);
    #########################################################
    # TestEngine::tePsAudioRead
    @fntrace
    def tePsAudioRead(self, handle, keyId, valueLen, value, _readLen=0):
        r"""
        Function wrapper for TestEngine::tePsAudioRead().

        Python API:
            tePsAudioRead(handle, keyId, valueLen, value, _readLen=0)
            returns retval, readLen.value

        Generated from C API:

            Function :      int32 tePsAudioRead(uint32 handle, uint32 keyId, uint16 valueLen,
                                                uint16* value, uint16* readLen)

            Parameters :    handle -
                                Handle to the device.

                            keyId -
                                PS key ID to read. Maximum value 0xFFFFFF.

                            valueLen -
                                Size of the data array allocated to hold the key value. A
                                valueLen of 0 indicates that only the length of the key's
                                value is to be read.

                            value -
                                Array of 16-bit words of size "valueLen" to hold the key
                                value. Can be NULL if valueLen is 0.

                            readLen -
                                Pointer to a 16-bit word to hold to length of the key's value.
                                This is the length of the valid value data read for the key
                                (may be less the "valueLen" given if that exceeds the actual
                                length of the key's value).
                                <p>If the key has not been written or has a zero length value,
                                this value will be set to zero.


            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Used to read the audio persistent store key specified (for
                            other PS keys use tePsRead).
                            <p>In the event of failure, if the readLen value has been set to
                            a value greater than valueLen, it means that insufficient storage
                            was provided to read the key value which has a length of readLen.
                            <p>This function is unsupported for BlueCore and CSRC9xxx ICs
                            (for BlueCore ICs use the psRead* functions).
                            <p>
                            Refer to the document CS-00304075-SP for further details regarding
                            audio PS keys.

        """
        self.TestEngineDLL.tePsAudioRead.restype = CT.c_int32
        self.TestEngineDLL.tePsAudioRead.argtypes = [CT.c_uint32, CT.c_uint32, CT.c_uint16, CT.c_void_p, CT.c_void_p]
        #__value = (CT.c_uint16 * len(value))(*value)
        readLen = CT.c_uint16(_readLen)
        retval = self.TestEngineDLL.tePsAudioRead(handle, keyId, valueLen, CT.byref(value), CT.byref(readLen))
        return retval, readLen.value
    #########################################################

    #########################################################
    # TESTENGINE_API(int32) tePsAudioWrite(uint32 handle, uint32 keyId, uint16 valueLen, const uint16 value[]);
    #########################################################
    # TestEngine::tePsAudioWrite
    @fntrace
    def tePsAudioWrite(self, handle, keyId, valueLen, value):
        r"""
        Function wrapper for TestEngine::tePsAudioWrite().

        Python API:
            tePsAudioWrite(handle, keyId, valueLen, value)
            returns retval

        Generated from C API:

            Function :      int32 tePsAudioWrite(uint32 handle, uint32 keyId, uint16 valueLen,
                                                 const uint16* value)

            Parameters :    handle -
                                Handle to the device.

                            keyId -
                                PS key ID to write. Maximum value 0xFFFFFF.

                            valueLen -
                                Size of the data array holding the key value. Setting this to
                                zero causes any existing value data for the key to be deleted,
                                with the key remaining present in (or being written to) the
                                store as a flag-like entry.

                            value -
                                Array of 16-bit words of size "valueLen" holding the key
                                value.

            Returns :       <table>
                                <tr><td>-1 = Invalid handle
                                <tr><td>0 = Error
                                <tr><td>1 = Success
                                <tr><td>2 = Unsupported function
                            </table>

            Description :   Used to write the audio persistent store key specified (for
                            other PS keys use tePsWrite).
                            <p>This function is unsupported for BlueCore and CSRC9xxx ICs
                            (for BlueCore ICs use the psWrite* functions).
                            <p>
                            Refer to the document CS-00304075-SP for further details regarding
                            audio PS keys.

        """
        self.TestEngineDLL.tePsAudioWrite.restype = CT.c_int32
        self.TestEngineDLL.tePsAudioWrite.argtypes = [CT.c_uint32, CT.c_uint32, CT.c_uint16, CT.c_void_p]
        #__value = (CT.c_uint16 * len(value))(*value)
        retval = self.TestEngineDLL.tePsAudioWrite(handle, keyId, valueLen, CT.byref(value))
        return retval
    #########################################################
    bccmdPsuBuckReg = bccmdPsuSmpsEnable
    bccmdPsuMicEn = bccmdPsuHvLinearEnable
    bccmdLed0 = bccmdLed0Enable
    bccmdLed1 = bccmdLed1Enable
    bccmdChargerSupressLed0 = bccmdChargerSuppressLed0
