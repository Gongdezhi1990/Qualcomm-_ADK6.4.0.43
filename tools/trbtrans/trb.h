// ***********************************************************************
// * Copyright 2014-2016 Qualcomm Technologies International, Ltd.
// ***********************************************************************

/// \file
/// Public header for the trbtrans library.
///
/// NOTES ON THREAD SAFETY:
/// This API is only safe if any particular stream handle is used from a single thread at any one
/// time. If you wish to share a stream handle between two or more threads, you must arrange for the
/// proper mutual exclusion, to prevent threads from simultaneously calling a function from this 
/// API.
/// Different streams can be used freely from multiple threads.

#ifndef INCLUDED_TRBTRANS_TRB_H
#define INCLUDED_TRBTRANS_TRB_H

// This header uses exact width-integer types as usually provided by stdint.h. Some toolchains do
// not supply stdint.h, so this header does not include it -- it is left up to the user to supply
// the appropriate include for their environment.

#ifdef __cplusplus
extern "C" {
#endif

/// TRBTRANS_API controls function export on Windows:
/// - DLL builds of this library define this macro to export the API functions
/// - Consumers of the DLL do not need to do anything other than link in the normal way.
/// - Consumers of the static library build should define TRBTRANS_STATIC to avoid linker errors.
/// - On non-Windows platforms, this macro has no effect.
#ifdef _WIN32
    #ifdef TRBTRANS_STATIC
        #define TRBTRANS_API
    #elif defined(TRBTRANS_EXPORTS)
        #define TRBTRANS_API __declspec(dllexport)
    #else
        #define TRBTRANS_API __declspec(dllimport)
    #endif
#else
    #define TRBTRANS_API
#endif

//****************************************************************************
// Types
//****************************************************************************

/// Error codes returned by functions on this API.
typedef enum
{
    TRB_ERR_NO_ERROR,
    TRB_ERR_COULD_NOT_ENUMERATE_DEVICE,
    TRB_ERR_DRIVER_IO_FAILED,
    TRB_ERR_DRIVER_IO_TIMEOUT,
    TRB_ERR_DEVICE_VERSION_TOO_OLD,
    TRB_ERR_BAD_DATA_RECEIVED,
    TRB_ERR_TBUS_ACCESS_FAILED,
    TRB_ERR_ACTION_NEEDS_TOO_MANY_TRANSACTIONS,
    TRB_ERR_LOGGING_ALREADY_ENABLED,
    TRB_ERR_COULD_NOT_OPEN_FILE,
    TRB_ERR_INVALID_STREAM,
    TRB_ERR_BRIDGE_LINK_IS_DOWN,
    TRB_ERR_OS_UNSUPPORTED,
    TRB_ERR_OS_ERROR,
    TRB_ERR_UNKNOWN_DRIVER,
    TRB_ERR_DRIVER_BUFFER_WRAPPED,
    TRB_ERR_BAD_PARAMETER,
    TRB_ERR_COULD_NOT_QUERY_DRIVER_INFO,
    TRB_ERR_BUFFER_TOO_SHORT,
    TRB_ERR_OPERATION_NOT_SUPPORTED,
    TRB_ERR_DEVICE_INCOMPATIBLE,
    TRB_ERR_LINK_RESET,
    // When updating this enum, remember to update 
    // a) private_common.cpp's error_code_to_string()
    // b) the Python bindings.
} trb_err;


/// Transaction Bus status codes given in responses in data accesses.
typedef enum
{
    TBUS_STATUS_NO_ERROR          = 0,
    TBUS_SUBSYSTEM_NO_POWER       = 1,
    TBUS_SUBSYSTEM_ASLEEP         = 2,
    TBUS_ROUTING_ERROR            = 3,
    TBUS_LOCK_ERROR               = 4,
    // 5-10 unused
    TBUS_ACCESS_PROTECTION_ERROR  = 11,
    TBUS_NO_MEMORY_HERE           = 12,
    TBUS_TRANSACTION_WRONG_LENGTH = 13,
    TBUS_MEMORY_NOT_WRITABLE      = 14,
    TBUS_ACCESS_BAD_ALIGNMENT     = 15,
} tbus_debug_status_code;


/// Available debug dongles / drivers. Pass to trb_stream_open().
typedef enum
{
    TRB_DRIVER_UNKNOWN     = -1,
    TRB_DRIVER_QUICKSILVER = 0,
    TRB_DRIVER_SCARLET     = 1,
    TRB_DRIVER_NUM_DRIVERS = 2,
} trb_driver;


/// Each action in the array passed to trb_sequence() has a type; choose from these values.
typedef enum
{
    TRB_ACTION_DEBUG_READ,
    TRB_ACTION_DEBUG_WRITE,
    TRB_ACTION_DATA_READ,
    TRB_ACTION_DATA_WRITE,
} trb_action_type;


/// This struct provides a higher level way to request a hardware access than
/// raw transactions can. For example, a single trb_action can represent
/// a large number of debug read request transactions. When the action is
/// executed (using trb_sequence()) the trbtrans library code will wait for
/// all the read response transactions and unpack the data they contain
/// into the trb_action.
typedef struct
{
    trb_action_type type;
    int             dest_subsys_id;
    int             dest_block_id;
    uint32_t        addr;
    uint32_t        num_bytes;
    int             num_bytes_per_transaction; ///< Typically 2 for XAP subsystems and 4 for Kalimba subsystems
    uint8_t         *data;
} trb_action;


/// A 96-bit transaction and 32-bit timestamp suitable to be sent to or received from the driver.
typedef struct 
{
    uint32_t    timestamp;              // Ignored when sent to the debug dongle
    uint8_t     opcode_and_subsys_src_id;
    uint8_t     block_src_id_and_subsys_dest_id;
    uint8_t     block_dst_id_and_tag;
    uint8_t     payload[9];
} transaction_with_timestamp;


/// Contains enough information about a debug dongle to establish a connection using 
/// trb_stream_open(). An array of these is returned from trb_build_dongle_list().
typedef struct
{
    trb_driver  driver;
    int         id;          ///< For example, the serial number of the Scarlet.
    char const* description; ///< A string suitable for human presentation.
} trb_dongle_details;


/// Declare an opaque type to hide the implementation details of the stream structure.
/// trb_stream* is not dereferencable by clients of the library.
typedef struct _trb_stream trb_stream;


//****************************************************************************
// General library functions
//****************************************************************************


/// Returns version information for the trbtrans library itself.
TRBTRANS_API char const *trb_get_version();


/// Returns the maximum number of transactions allowed in an action.
TRBTRANS_API int trb_get_max_transactions();


/// Returns the details of the last error encountered. This uses "thread local storage" behind the
/// scenes to ensure that the correct details are returned to the calling thread.
/// The buffer should not be freed by the caller; if the error message is required to persist, it should be copied.
TRBTRANS_API trb_err trb_get_last_error(int *actions_succeeded, const char **buf);


//****************************************************************************
// Enumeration, transport strings, etc -- these do not require an open stream
//****************************************************************************

/// Dongle enumeration. Retrieves details of all attached and installed debug dongles of the given type.
/// The memory for the dongle list is allocated by trbtrans. Free this memory via trb_free_dongle_list().
/// These functions are only supported for Scarlet.
TRBTRANS_API trb_err trb_build_dongle_list(trb_driver driver, trb_dongle_details **dongles, unsigned *dongle_count);


/// Free the list returned by trb_build_dongle_list().
TRBTRANS_API trb_err trb_free_dongle_list(trb_dongle_details *dongles, unsigned dongle_count);


/// Map from a string identifier for a dongle type to a member of trb_driver. This is useful for
/// producing connection parameters for trb_stream_open() from Heracles device URIs.
/// \param[in] driver_name a string representing a supported driver or an alias thereof. Supported
/// values are:
/// - Scarlet: "usb2trb", "scarlet", "scar"
/// - Murphy: "quicksilver", "qs", "murphy"
/// \param[out] result receives the enumeration result
/// \returns
/// - ::TRB_ERR_NO_ERROR if the name was translated successfully
/// - ::TRB_ERR_UNKNOWN_DRIVER if the driver name was not recognised.
TRBTRANS_API trb_err trb_get_driver_from_name(char const *driver_name, trb_driver *result);


/// Get the canonical string name for the given debug dongle, as used in transport strings formed by
/// trb_form_transport_string(). This is the inverse of trb_get_driver_from_name().
/// \param[out] result receives the name. \c *result will point to a valid string constant upon 
/// success. This string should not be freed, and is valid for the lifetime of the trbtrans library.
/// \c *result is set to null upon error.
TRBTRANS_API trb_err trb_get_name_from_driver(trb_driver driver, char const **result);


/// Given some dongle details, probably obtained from trb_build_dongle_list(), construct a transport
/// string, which can be later decoded back into dongle a driver type and dongle ID using 
/// trb_decode_transport_string(). The format of the returned string is compatible with the 
/// "transport/dongle_name/dongle_id" fragment in a Heracles URI.
/// \param[in] dongle details from which to form a transport string
/// \param[in,out] buffer caller-allocated buffer of length buf_len.
/// \param[in,out] buf_len length of \p buffer
/// \returns
/// - ::TRB_ERR_NO_ERROR if the string was formed successfully.
/// - ::TRB_ERR_BAD_PARAMETER If the provided buffer is too short. The caller is required to retry with a larger buffer.
TRBTRANS_API trb_err trb_form_transport_string(trb_dongle_details const *const dongle, char* buffer, unsigned buf_len);


/// Decode a transport string into a driver and dongle ID.
TRBTRANS_API trb_err trb_decode_transport_string(char const *transport_string, trb_driver *driver, int *dongle_id);


/// Given details of attached dongles (probably obtained from trb_build_dongle_list()), determine if
/// the given transport string is valid for any of them.
/// If no error occurs, and the transport string is valid, *dongle_index_result is set to the index of the corresponding device.
/// If no error occurs, and the transport string is not valid, *dongle_index_result is set to -1.
TRBTRANS_API trb_err trb_is_valid_transport_string(char const* transport_string, 
                                                   trb_dongle_details *attached_dongles, unsigned dongle_count, 
                                                   int *dongle_index_result);


//****************************************************************************
// Functions for opening and closing streams
//****************************************************************************


/// Open a stream to the given debug dongle.
/// When connecting to a Scarlet, the serial number of the Scarlet should be specified in 'dongle_id'.
/// When connecting to a Murphy, the 'dongle_id' parameter is ignored.
/// Alternatively, obtain these details via trb_build_dongle_list().
/// On Scarlet, approximately 240 standard streams are available, shared across all applications.
/// On Murphy, 16 streams are available.
TRBTRANS_API trb_err trb_stream_open(trb_driver driver, int dongle_id, trb_stream **stream);


/// Open the sniffer stream. Only one stream may be opened for sniffing, system-wide, at any one
/// time; this function will return an error if the sniffer is already in use.
TRBTRANS_API trb_err trb_sniffer_stream_open(trb_driver driver, int dongle_id, trb_stream **stream);


/// Open a stream intended as a destination for samples generated by the DUT.
/// This stream has a fixed routing; transactions addressed to:
/// Block destination ID = 0xF
/// Tag = 0xE
/// will be routed to this stream.
/// The Scarlet driver implementation assigns a larger transaction receive buffer to this stream,
/// so that buffer wrap conditions will occur less frequently when the incoming data rate is high
/// (e.g. when tracing processor execution).
/// There is only one same stream available; attempting to open it twice will result in an error.
TRBTRANS_API trb_err trb_sample_stream_open(trb_driver driver, int dongle_id, trb_stream **stream);


/// Close a stream. Call only once for a given stream.
TRBTRANS_API void trb_stream_close(trb_stream *stream);


//****************************************************************************
// Per-stream informational functions
//****************************************************************************


/// Returns human-readable info about the debug dongle we're using for this connection.
/// This is the same information as returned in trb_dongle_details::descriptive_device_name.
TRBTRANS_API char const *trb_get_dongle_name(trb_stream *stream);


/// Retrieve the dongle details from an open stream.
/// This information can be passed back to trb_stream_open() in order to open another stream to the
/// same dongle.
/// 'dongle' should not be passed to trb_free_dongle_list(). The 'description' field is valid for as
/// long as the stream is open.
TRBTRANS_API trb_err trb_get_dongle_details(trb_stream *stream, trb_dongle_details *dongle);


/// Returns the version of the Scarlet firmware running on the dongle.
/// Only applicable to Scarlet.
TRBTRANS_API trb_err trb_get_firmware_version(trb_stream *stream, int *firmware_version);


/// Returns the active version of the dongle driver.
TRBTRANS_API char const *trb_get_driver_version(trb_stream *stream);


/// Returns the FPGA version of the debug dongle.
TRBTRANS_API trb_err trb_get_fpga_version(trb_stream *stream, unsigned *fpga_version);


/// Returns the Hydra subsystem ID of the transaction bridge block.
TRBTRANS_API unsigned trb_get_bridge_subsys_id(trb_stream *stream);


/// Returns whether the transaction bridge link is up.
/// Note that this is simply a snapshot of the hardware link status. To query whether a given stream
/// is usable for I/O, use trb_is_stream_valid().
TRBTRANS_API trb_err trb_get_link_status(trb_stream *stream, bool *result);


/// Query whether the given stream is usable for I/O. This requires the TRB link to be currently up,
/// and the stream to not be "stale". A stream becomes stale when the TRB link goes down.
/// Does not apply to Murphy, and will return TRB_ERR_OPERATION_NOT_SUPPORTED.
TRBTRANS_API trb_err trb_is_stream_valid(trb_stream *stream, bool *result);


/// Returns some statistics on the transaction bridge link:
/// - the number of transactions received
/// - the number of transactions sent
/// - the number of CRC failures recorded for received transactions.
/// Be aware that these numbers are 8-bit counters, and will regularly wrap.
TRBTRANS_API trb_err trb_get_link_statistics(trb_stream *stream,
                                             uint8_t *transactions_rx,
                                             uint8_t *transactions_tx,
                                             uint8_t *num_crc_failures);


/// Returns the global chip version of the connected chip.
TRBTRANS_API uint16_t trb_get_chip_version(trb_stream *stream);


typedef enum
{
    USB_CONNECTION_UNKNOWN_SPEED = 0,
    USB_CONNECTION_FULL_SPEED,
    USB_CONNECTION_HIGH_SPEED,
    USB_CONNECTION_SUPER_SPEED
} usb_speed;


/// Retrieve the USB connection speed for the Scarlet associated with the given stream.
/// Does not apply to Murphy/QS, and will return TRB_ERR_OPERATION_NOT_SUPPORTED.
TRBTRANS_API trb_err trb_get_usb_connection_speed(trb_stream *stream, usb_speed *speed);


/// Reset the DUT attached to the other end of the TBridge, e.g. via a special FPGA register poke
/// on the debug dongle which generates a "reset transaction" for the DUT. This only works if
/// both the TBridge on the debug dongle and the DUT actually support this operation.
///
/// If reset operation (I/O) to the driver succeeds, this function will return ::TRB_ERR_NO_ERROR.
/// 
/// After issuing the reset:-
///
/// - For usb2trb:-
///    - first wait for stream to become invalid as determined by the driver. This ensures that the
///    TRB link actually did cycle.
///    - if this succeeds, wait for the TRB link to be up and recycle the stream to be usable for 
///    I/O again.
///
/// - For Murphy, perform a simple wait according to link_reset_timeout_millis. The TRB link status 
/// is then checked; the wait is assumed to be long enough for the link to have cycled.
///
/// These waits uses a timeout; see \p link_reset_timeout_millis. If the timeout expires, 
/// \p *saw_link_reset will come back as \c false. In this case, either the reset didn't work (but
/// we can't be sure), or it took longer than the upper bound allowed for.
///
/// Note that even if \p *saw_link_reset comes back \c true, the DUT may not be fully booted --
/// we only know that the TBridge link is up again. The caller may want to apply chip-specific logic
/// to wait for boot.
///
/// \param link_reset_timeout_millis if zero, use a default timeout value. Otherwise the link reset 
/// wait uses this timeout.
/// \param saw_link_reset see notes above.
TRBTRANS_API trb_err trb_reset_dut(trb_stream *stream, unsigned link_reset_timeout_millis, bool *saw_link_reset);


/// Perform the "wait" part of trb_reset_dut() without first issuing a reset over TRB.
/// Some action that will trigger a link reset is assumed to have been peformed prior to calling
/// this function.
/// Not supported on Murphy: will return TRB_ERR_OPERATION_NOT_SUPPORTED.
TRBTRANS_API trb_err trb_wait_for_link_cycle(
    trb_stream *stream,
    unsigned link_reset_timeout_millis,
    bool *saw_link_reset);


//****************************************************************************
// I/O functions
//****************************************************************************


/// Perform a sequence of actions.
TRBTRANS_API trb_err trb_sequence(trb_stream *stream, trb_action const *actions, int num_actions);


/// Low level access. Write transactions directly to the driver. Obtain responses, if applicable,
/// using trb_read_raw_transactions().
TRBTRANS_API trb_err trb_write_raw_transactions(trb_stream *stream, transaction_with_timestamp *txns, int num_txns);


/// trb_read_raw_transactions() tries to read up to num_txns_requested raw transactions from the device,
/// waiting for a maximum of timeout_millis milliseconds. If timeout_millis is negative, this function
/// waits forever.
/// NOTE: On versions of the Scarlet driver package <= 1.0.3.6, calling this function will have the
/// side effect of endian flipping the inputted array of txns.
/// The number of transactions read is returned in *num_actually_got.
/// This function may return:
/// - TRB_ERR_NO_ERROR if n <= num_txns_requested transactions were successfully
/// read (whether or not the timeout was hit), and no unexpected I/O errors occurred.
/// - TRB_ERR_DRIVER_BUFFER_WRAPPED if the driver's internal data buffer overflowed, causing data to be
/// lost. In this case, there may still be transaction data, but the transactions will be
/// discontinuous in time with respect to the last read. Inspect num_actually_got and txns as usual.
/// - Other trb_err codes in other error scenarios.
TRBTRANS_API trb_err trb_read_raw_transactions(trb_stream *stream, transaction_with_timestamp *txns, 
                                               int num_txns_requested, int *num_actually_got, int timeout_millis);


/// Returns the last Transaction Bus error code recorded for the given stream.
TRBTRANS_API tbus_debug_status_code trb_get_last_tbus_error(trb_stream *stream);


/// Configure I/O retries when a ::TBUS_SUBSYSTEM_ASLEEP error is encountered in a trb_sequence()
/// call on this stream.
///
/// Defaults to ON, with the library's default retry number and interval.
/// The library never retries when doing raw I/O via trb_read_raw_transactions() or 
/// trb_write_raw_transactions().
///
/// ::TBUS_SUBSYSTEM_ASLEEP may occur when the chip is waking from deep sleep -- in this case the 
/// I/O should succeed when the subsystem has been clocked and woken up. It may also simply occur 
/// when the chip is awake, but the subsystem is not clocked, in which case retrying will not help.
/// Therefore, when core dumping a crashed chip, for example, it may be appropriate to turn this 
/// feature off and arrange for clocks to be enabled separately.
/// 
/// Retries are never applied when doing raw I/O.
///
/// \param[in] num_retries set to a negative number to use the library's default. Set to zero to
/// disable the feature. Set to a positive number to use that many retries.
/// \param[in] retry_interval_millis how long to wait between retries. Set to zero to use the 
/// library's default.
TRBTRANS_API void trb_configure_sleep_retries(trb_stream *stream, int num_retries, unsigned retry_interval_millis);

//****************************************************************************
// Peek and poke the debug dongle itself
//****************************************************************************


/// Read a register from the debug dongle (i.e., the Murphy / Scarlet)
TRBTRANS_API trb_err trb_read_dongle_reg(trb_stream *stream, int bar, int offset, uint32_t *val);


/// Write a register on the debug dongle.
TRBTRANS_API trb_err trb_write_dongle_reg(trb_stream *stream, int bar, int offset, uint32_t val);

/// Reset the Scarlet (not the DUT).
/// Does not apply to Murphy, and will return TRB_ERR_OPERATION_NOT_SUPPORTED.
TRBTRANS_API trb_err trb_dongle_reset(trb_stream *stream);


//****************************************************************************
// Logging
//****************************************************************************


/// Turn on logging for this stream. If the specified path is NULL, the environment variable
/// TRBDEBUG_LOGPATH will be used as a directory in which to create log files.
/// If that is also missing, then a system default temporary path will be used.
/// On Windows, this can be accessed via %TEMP%.
/// On Linux, $TMPDIR is used if set, otherwise "/tmp/".
///
/// The filename created will be "trblog_<processname>_<PID>.log". If the environment variable 
/// TRBDEBUG is set to "ON", then logging will begin as soon as the stream is created. In that case, 
/// calling this function will return the error TRB_ERR_LOGGING_ALREADY_ENABLED.
/// 
/// In addition to the above, the environment variable TRBDEBUG_FULL may be set to "ON"
/// to enable more verbose logging. This is typically useful for debugging possible problems with 
/// this library.
TRBTRANS_API trb_err trb_start_log(trb_stream *stream, char const *path);


/// Stop logging for this stream.
TRBTRANS_API trb_err trb_stop_log(trb_stream *stream);


/// Get the path of the log file in the specified buffer. Buffer will be set to an empty
/// string if logging is not currently enabled.
TRBTRANS_API trb_err trb_get_log_path(trb_stream *stream, char *buf, unsigned buf_len);


//****************************************************************************
// Specialist functions for low-level information about the debug dongle.
//****************************************************************************


typedef struct
{
	uint32_t channel_state;
	uint32_t total_buffer_size_in_bytes;
	uint32_t producer_transfer_byte_count;
	uint32_t consumer_transfer_byte_count;
} scar_dma_channel_stats;


#define NUMBER_OF_SCARLET_APPLICATION_DMA_CHANNELS (7)
typedef struct
{
	scar_dma_channel_stats channel_stats[NUMBER_OF_SCARLET_APPLICATION_DMA_CHANNELS];
} scar_dma_device_stats;


typedef struct
{
    uint32_t phy_error_count;
    uint32_t link_error_count;
} scar_usb_device_stats;


typedef struct
{
	scar_usb_device_stats usb_stats;
	scar_dma_device_stats dma_stats;
} scar_device_stats;


/// The functions below are not intended for normal use cases. They provide access to the
/// Cypress FX3 USB link quality indicators on Scarlet. They are used in EMC testing, but
/// might possibly provide useful diagnostic info when someone encounters flaky USB
/// performance.
TRBTRANS_API trb_err trb_get_device_statistics(trb_stream *stream, scar_device_stats *device_stats);


TRBTRANS_API trb_err trb_zero_device_statistics(trb_stream *stream);


/// Retrieve the uptime of the Scarlet Cypress firmware.
/// Does not apply to Murphy, and will return TRB_ERR_OPERATION_NOT_SUPPORTED.
TRBTRANS_API trb_err trb_get_firmware_uptime(trb_stream *stream, uint32_t *uptime);


#ifdef __cplusplus
} //extern "C"
#endif

#endif
