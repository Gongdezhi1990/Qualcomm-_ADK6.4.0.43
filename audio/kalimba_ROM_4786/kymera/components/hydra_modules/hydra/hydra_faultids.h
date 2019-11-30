/*****************************************************************************
*
* Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
*
*****************************************************************************/
/* This header fragment is to be included by faultids.h
 * It cannot stand alone
 */

    /*
     * This fault indicates that the chip has rebooted due to a panic.
     * The argument is the panic code that caused the reboot.
     */
    FAULT_HYDRA_PANIC = 0x1001,

    /*
     * The software attempted to divide a number by zero. This should never
     * happen.
     */
    FAULT_HYDRA_DIVIDE_BY_ZERO = 0x1002,

    /*
     * The firmware has failed to read a value from the MIB, but the
     * firmware's coding indicates that it expects and requires the MIB
     * to provide the value.  The argument is the mibkey.
     */
    FAULT_HYDRA_MIB_REQ_VAL_ABSENT = 0x1003,

    /*
     * The firmware has failed to read a value from the MIB; the failure
     * has occurred because the firmware and the MIB hold the MIB
     * variable's value in different types.  The argument is the mibkey.
     */
    FAULT_HYDRA_MIB_TYPE_CLASH = 0x1004,

    /*
     * The firmware has found that the data held in the MIB RAM store
     * is internally corrupt.  The argument is the mibkey.
     */
    FAULT_HYDRA_MIB_RAM_CORRUPT = 0x1005,

    /*
     * The firmware has found that the data held in the MIB ROM store
     * is internally corrupt.  The argument is the mibkey.
     */
    FAULT_HYDRA_MIB_ROM_CORRUPT = 0x1006,

    /*
     * A request has been made to access a data item in a table using an
     * invalid table index - i.e., with value zero.  The request probably
     * came from the core firmware rather than from the host.  The argument
     * is the mibkey.
     */
    FAULT_HYDRA_MIB_INVALID_INDEX = 0x1007,

    /*
     * The core firmware is attempting either (a) to write an integer value
     * to the MIB that is outside the MIB variable's allowed limits, or
     * (b) to write an octet string to the MIB, the length of which is
     * outside the MIB variable's allowed length limits.  Because the core
     * firmware is writing, the action will normally succeed, so an
     * unacceptable value should end up in the MIB.  (If the host attempts
     * to breach these limits it is refused, and the fault_diatribe() call is not
     * made.)  The fault_diatribe() argument is the mibkey.
     */
    FAULT_HYDRA_MIB_LIMIT = 0x1008,

    /*
     * One of the VLINT (variable length integer) conversion functions
     * has been asked to write a value to a store that is too small for
     * the value.  For example, it may have been asked to write 1000000
     * to a uint16.  The fault_diatribe() argument is zero.
     */
    FAULT_HYDRA_VLINT_OVERFLOW = 0x1009,

    /*
     * The MIB places an upper limit on the size of record it can store
     * in RAM.  At the time of writing this comment this is sufficient to
     * store a mibkey identifier, two table indices and an int64 value.
     * This fault code indicates that an attempt has been made to store
     * a block of data greater than this limit, and that the value has
     * not been recorded.  The fault_diatribe() argument is the size of record
     * that was rejected as being too large.
     */
    FAULT_HYDRA_MIB_RAM_OVERFLOW = 0x100a,

    /*
     * The mibps subsystem has encountered an unknown failure.  The MIB's
     * default values may be only partially written.  The argument is the
     * identifier (psrecid) of the persistent store block being read when
     * the failure occurred.
     */
    FAULT_HYDRA_MIBPS_FAIL = 0x100b,

    /*
     * A record of the MIB persistent store (default configuration)
     * database has been found to be corrupt.  The argument is the
     * corresponding MIBID (MIB value identifier in the database).
     */
    FAULT_HYDRA_MIBPS_DATA_CORRUPT = 0x100c,

    /*
     * The MIB has rejected a value written to it by the mibps subsystem.
     * The mibps subsystem takes MIB (default configuration) values from
     * the chip's persistent store - normally EEPROM or flash memory.
     * This rejection may indicate that a record in the MIB persistent
     * store is corrupt, that the value is outside the limits for the
     * MIB variable, that the MIB variable was supplied with unacceptable
     * table indices.  The failure can also arise from one of many, highly
     * unlikely, system failures, notably memory exhaustion.  The argument
     * is the corresponding MIBID (MIB value identifier in the database).
     */
    FAULT_HYDRA_MIBPS_WRITE_FAIL = 0x100d,

    /*
     * The persistent store code does not support the installed persistent
     * store data version.  All data in the persistent store will be
     * ignored by the firmware.  The argument is the data version.
     */
    FAULT_HYDRA_PS_UNSUPPORTED_VERSION = 0x100e,

    /*
     * The contents of the ps (persistent store), the system's
     * per-design and per-device configuration database, has been
     * found to be corrupt.  The contents of the ps are normally used
     * during system initialisation; the occurrence of this fault
     * message normally indicates that some part of the system has
     * not been full initialised.  For example, it might mean that the
     * device's MAC address has not been set.  The ps normally holds
     * its configuration data in either EEPROM or flash memory, and its
     * contents are normally configured when the device is manufacturered.
     * The fault argument is the identifier of the part of the ps found
     * to be corrupt.
     */
    FAULT_HYDRA_PS_CORRUPT = 0x100f,

    /*
     * A VLINT (variable length integer) with a negative value has been
     * written to an unsigned variable.  The fault argument is zero.
     */
    FAULT_HYDRA_VLINT_NEGATIVE_UNSIGNED_VALUE = 0x1010,

    /*
     * In this system a VLINT (variable length integer) has a maximum
     * length of 10 octets (#defined as MAXVLINTLEN) - big enough to
     * hold a uint64 value.  This fault signals that a VLINT has been
     * found of length greater than MAXVLINTLEN.  The fault argument is
     * the VLINT length.
     */
    FAULT_HYDRA_VLINT_TOO_BIG = 0x1011,

    /*
     * There was a problem reading the persistent store in EEPROM.
     * The argument is the (uint16) offset which failed.
     */
    FAULT_HYDRA_PS_EEPROM_READ_FAIL = 0x1012,

    /*
     * The MIB has detected an internal coding error.  The argument is
     * the mibkey.
     */
    FAULT_HYDRA_MIB_ASSERT_FAIL = 0x1013,

    /*
     * This indicates that a MIB ram record has been found to be corrupt.
     * The argument is unused.
     */
    FAULT_HYDRA_MIB_RAM_REC_CORRUPT = 0x1014,

    /*
     * An attempt has been made to claim a PIO pin via the usrpio subsystem,
     * but the PIO has already been claimed.  This normally arises because
     * two parts of the system are trying to claim a given PIO, usually
     * chosen by clashing MIB configuration values.  It can also arise if
     * the host code attempts to claim a PIO that is already in use.  The
     * argument indicates the clashing bit(s).
     */
    FAULT_HYDRA_USRPIO_ALREADY_PROTECTED = 0x1017,

    /*
     * An attempt has been made to release the claim on a PIO where the
     * PIO was not already claimed.  This normally arises where some part
     * of the system, or sometimes the host, releases a claim multiple
     * times.  The argument gives the disputed bits.
     */
    FAULT_HYDRA_USRPIO_NOT_PROTECTED = 0x1018,

    /*
     * Some MIB entries service their get/set operations via function
     * calls to other parts of the firmware.  For example, this mechanism
     * is used to access the system's random number generator via the MIB.
     * This fault code indicates that the mapping between a MIB variable
     * and its function has been misconfigured.  The argument identifies
     * the MIB variable.
     */
    FAULT_HYDRA_MIB_MISCONFIGURED_FN_MAPPING = 0x1019,

    /*
     * Reported length of received PDU is too short or too long.
     * The argument is the length.
     */
    FAULT_SUBMSG_INVALID_RX_LENGTH = 0x1022,

    /*
     * Received a PDU on an unknown submsg channel.
     * The argument is the channel number.
     */
    FAULT_SUBMSG_INVALID_RX_CHANNEL = 0x1023,

    /**
     * A CCP "recv" function has been passed a PDU whose length
     * length is incorrect for the PDU.  This is presumably an
     * error on the sending subsystem.  The argument is the
     * corresponding PDU type.
     */
    FAULT_CCP_INVALID_RX_LENGTH = 0x1024,

    /**
     * \c subserv received a command PDU it couldn't decipher.
     * The argument is the PDU identifier.
     */
    FAULT_SUBSERV_BAD_PDU = 0x1028,

    /**
     * \c subserv received an auxiliary command service message
     * that wasn't associated with a currently running service
     * instance.
     * (No longer in use -- now we just quietly drop any such
     * message on the floor.)
     */
    FAULT_SUBSERV_BAD_AUX_CMD_MESSAGE = 0x1029,

    /**
     * The \c submsg protocol in a received frame didn't make sense.
     */
    FAULT_SUBMSG_BAD_PROTOCOL = 0x102a,

    /* fileserv runs on subsystems as well as curator */
    /*
     * fileserv received a badly formed request
     * The argument shows the issmsg PDU type
     *
     * This is done because the FTP protocol doesn't include
     * responses for these kinds of cases
     */
    FAULT_FILESERV_BAD_ARGS = 0x102b,

    /**
     * fileserv received a request when one was already in progress
     * The argument is irrelevant
     *
     * This is done because the FTP protocol doesn't include
     * responses for these kinds of cases
     */
    FAULT_FILESERV_IN_PROGRESS = 0x102c,

    /**
     * A pdu arrived from fileserv with no data
     */
    FAULT_FILESERV_EMPTY_PDU = 0x102d,

    /**
     * A file operation was attempted without a session in progress
     */
    FAULT_FILESERV_NO_SESSION = 0x102e,

    /**
     * A file operation was attempted with the wrong session
     */
    FAULT_FILESERV_BAD_SESSION = 0x102f,

    /**
     * A file read operation was attempted with no current handle
     */
    FAULT_FILESERV_NO_HANDLE = 0x1030,

    /**
     * A file operation was attempted with an incorrect handle
     */
    FAULT_FILESERV_BAD_HANDLE = 0x1031,

    /**
     * session create failed
     */
    FAULT_CONMAN_SESSION_FAIL = 0x1032,

    /* conman runs on subsystems as well as curator */
    /**
     * conman received a patch set which was not for it
     * The argument is the faulty id
     */
    FAULT_CONMAN_BAD_ID = 0x1033,

    /**
     * conman received an incorrect msg_id from fileserv
     * The argument is the faulty id
     */
    FAULT_CONMAN_BAD_MSG = 0x1034,

    /**
     * conman received an open fail from fileserv
     */
    FAULT_CONMAN_OPEN_FAIL = 0x1035,

    /**
     * conman received a read fail on an open file from fileserv
     */
    FAULT_CONMAN_READ_FAIL = 0x1036,

    /**
     * conman received a read fail on an open file from fileserv
     */
    FAULT_CONMAN_CLOSE_FAIL = 0x1037,

    /**
     * conman received a read fail on an open file from fileserv
     */
    FAULT_CONMAN_SESSION_DESTROY_FAIL = 0x1038,

    /**
     * isp router received a pdu not addressed to it.
     * Parameter is the sub-system that is the source of the pdu
     */
    FAULT_ISP_BAD_ROUTING = 0x1039,

    /**
     * isp router received a pdu from the host destined for a sub-system 
     * but there is no link to pass it over.
     * Parameter is ((destination_address << 8) | protocol).
     */
    FAULT_ISP_NO_LINK_FOR_ROUTING = 0x103a,

    /**
     * isp router received a pdu from the host destined for a sub-system 
     * on an in-out-band link but the pdu is too large to be copied into
     * memory for sending.
     * Parameter is the message length in words.
     */
    FAULT_ISP_PDU_TOO_LARGE_FOR_COPY = 0x103b,

    /**
     * isp_router received a pdu from the host but there is no handler
     * for that protocol
     * Parameter is the protocol.
     */
    FAULT_ISP_NO_ROUTE_FOR_PDU_FROM_HOST = 0x103c,

    /**
     * isp_router received a pdu from the subsystem but there is no handler
     * for that protocol.
     * Parameter is the (protocol << 8) | source_subsystem.
     */
    FAULT_ISP_NO_ROUTE_FOR_PDU_FROM_SUBSYSTEM = 0x103d,

    /**
     * hostio received a ccp message with an invalid length
     */
    FAULT_HOSTIO_INVALID_CCP_MESSAGE_LEN = 0x103e,

    /**
     * Transaction bus read exceeded current limit (initialised to design
     * limit, raised each time current limit is exceeded).
     *
     * Parameter is the nr uSecs exceeded [-0, +1).
     */
    FAULT_HYDRA_TXBUS_RD_LIMIT_EXCEEDED = 0x103f,

    /**
     * subsleep received a bad message about deep sleep.
     * If the argument is 0, the message did not come from the
     * Curator so was ignored.
     * If the argument is anything else, a message of that signal ID
     * was badly formatted.  This probably indicates a version mismatch.
     */
    FAULT_SUBSLEEP_BAD_PDU = 0x1040,

    /**
     * Conman determined that a patch was internally malformed
     * Possible causes include
     * code_len > bundle_len
     * The instructions part has a length different from
     * that derived from n_hw, n_sw and n_dp
     */
    FAULT_CONMAN_BAD_PATCH = 0x1041,

    /**
     * A subsystem watchdog ping request was received when one was
     * already pending.  The argument is the ISP address from
     * which the first request was received.
     */
    FAULT_SUBWD_PING_ALREADY_PENDING = 0x1042,
    /**
     * The subwd background interrupt handler was invoked when
     * no subsystem watchdog ping was pending.
     */
    FAULT_SUBWD_PING_NOT_PENDING = 0x1043,

    /**
     * The subsystem needed to wake from deep sleep before the message
     * that it could sleep was confirmed.  As the subsystem needed to
     * poll for confirmation for the message, it continued to do so
     * instead of waking.  The argument indicates the number of 1024 us
     * units beyond the wake up time before the message was sent.
     */
    FAULT_HYDRA_DELAYED_SLEEP_MSG = 0x1044,

    /**
     * isp_router cannot allocate memory for a link to the host requested
     * by a subsystem.
     * Parameter is the (protocol << 8) | source_subsystem.
     */
    FAULT_ISP_NO_RESOURCES_FOR_LINK_WITH_HOST = 0x1045,

    /**
     * Conman has been delivered more data than it requested
     * This fault specifically refers to the length provided
     * via FTP_DATA, and indicates a serious malfunction, at least
     * in the view of the fw, of the ftp system
     */
    FAULT_CONMAN_TOO_MUCH_DATA = 0x1046,

    /**
     * Conman didn't receive the amount of data it was promised.
     * This occurs when a read confirm arrives but conman thinks there is
     * some segment remaining data.
     */
    FAULT_CONMAN_TOO_LITTLE_DATA = 0x1047,

    /**
     * Conman found during config a key with insufficient data
     */
    FAULT_CONMAN_CONFIG_TOO_LITTLE = 0x1048,

    /**
     * Conman (or related code) failed to set a mibkey during config
     */
    FAULT_CONFIG_SET_FAIL = 0x1049,

    /**
     * MibCmd couldn't complete the request because it couldn't allocate
     * the pmalloc blocks needed.
     */
    FAULT_MIBCMD_PMALLOC_EXHAUSTION = 0x104A,
    
    /**
     * MibCmd has been passed a vlint for an integer key that is longer than
     * the maximum needed for a uint64 (MAXUINT64VLINTLEN defined in vlint.h).
     * The parameter is the length of the vlint that was passed.
     */
    FAULT_MIBCMD_VLINT_TOO_LONG = 0x104B,

    /**
     * A vlint function has been passed the wrong type of data. I.e. passed an octet sting VLDATA
     * when this is not supported.
     */
    FAULT_VLINT_WRONG_TYPE = 0x104C,

    /**
     * The subres module received a message it was unexpecting.
     * If the argument is 0xFFFFu, the sender was not the Curator.
     * Otherwise, the argument is the CCP signal ID of the message.
     */
    FAULT_SUBRES_BAD_MESSAGE = 0x104D,

    /**
     * Warn that the insomniac stress testing task is linked and running.
     *
     * This is an extra safety measure in case it slips through all the other
     * nets into code it should not be in.
     */
    FAULT_INSOMNIAC_TASK_IS_RUNNING = 0x104E,

    /**
     * Warn that the file delivered to conman isn't what was asked for
     *
     * Expected behaviour is for the file to be ignored
     */
    FAULT_CONMAN_FILENAME_MISMATCH = 0x104F,

    /*
     * fileserv received some sort of protocol mismatch
     * The argument shows the failed version
     */
    FAULT_FILESERV_BAD_VERSION = 0x1050,

    /*
     * The ssccp module was passed a CCP signal for which it
     * had no handler.
     */
    FAULT_SSCCP_UNKNOWN_CCP_SIGNAL_ID = 0x1051,

    /*
     * A CME Message Interface send operation has failed
     * as reported by SubServ.
     */
    FAULT_CME_MESSAGE_FAILURE = 0x1052,

    /*
     * The CME has received an incoherent event notification.
     * It is suspected that the firmware has lost coherency.
     */
    FAULT_CME_GENERIC_ERROR = 0x1053,

    /*
     * Some very dangerous test commands have been compiled into
     * the firmware. Ensure that the macro
     * INSTALL_SS_TEST_CMDS
     * is NOT defined to remove this fault.
     */
    FAULT_SS_TEST_CMDS_COMPILED_IN = 0x1054,

    /*
     * A fault has been found in a ccp clock info message
     */
    FAULT_CCP_CLOCK_INFO = 0x1055,

    /**
     * The USB hardware has detected a missing or corrupt packet.
     * The argument is the firmware endpoint index.
     */
    FAULT_HOSTIO_USB_PACKET_ERROR = 0x1056,

    /**
     * The hydraCore log buffer offset got corrupted during a reset.
     * The argument is the corrupted buffer offset.
     */
    FAULT_HYDRA_LOG_BUFFER_OFFSET_CORRUPTED = 0x1057,

    /*
     * These next two really ought to be kalimba specific
     * But, the fault mapper code in coal doesn't deal with
     * conditional compilation. So, they're left here
     * They should disappear in time as mentioned below
     */
    /**
     * An unsupported operation has been attempted.
     * Note that this should not get into release code
     * It's there to support porting of things from elsewhere
     * until we produce a better solution
     */
    FAULT_HYDRA_UNSUPPORTED = 0x1058,

    /**
     * An out of range MMU buffer offset has been found
     * This can only occur on systems using the audio style mmu (BAC)
     */
    FAULT_HYDRA_MMU_BUFFER_OFFSET = 0x1059,

    /**
     * A bad timeout time (in the past) or malformed interval
     * has been requested for timed_event
     */
    FAULT_HYDRA_TIMED_EVENT_BAD_PARMS = 0x105a,

    /**
     * NOTE: 0x17ff is the highest fault code in the CSR range
     * CSR add codes from the lowest free number towards this high one
     */
    /**
     * NOTE: 0x1800 is the lowest fault code in the SCSC range
     * SCSC add codes from the highest free number towards this low one
     */
    /**
     * A PDU supplied to the \c subpmu CCP handler was invalid.
     */
    FAULT_SUBPMU_BAD_PDU = 0x1fff,
    /**
     * NOTE: 0x1fff is the highest fault code in the SCSC range
     */
