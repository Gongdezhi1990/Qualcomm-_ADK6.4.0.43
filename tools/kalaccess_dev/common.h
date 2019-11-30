// ***********************************************************************
// * Copyright 2014-2018 Qualcomm Technologies International, Ltd.
// ***********************************************************************

/// \file 
/// This module contains
/// - the majority of the core types used in the API.
/// - generic library-level functionality (e.g. error handling)
/// - device enumeration functions
/// - functions connection and disconnection from a target
/// - query APIs for target properties.


// *************************************************************************************************
// NOTE ON MEMORY MANAGEMENT
//
// When a function returns a non-NULL ::ka_err pointer, it is the caller's responsibility to call
// ka_free_error() to release the memory associated with it. Calling a function and
// ignoring its error is a potential memory leak.
//
// *************************************************************************************************

#ifndef INCLUDED_KALACCESS_COMMON_H
#define INCLUDED_KALACCESS_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/// KALACCESS_API controls function export on Windows:
/// - DLL builds of this library define this macro to export the API functions
/// - Consumers of the DLL do not need to do anything other than link in the normal way.
/// - Consumers of the static library, should define KALACCESS_STATIC to avoid linker errors.
/// - On non-Windows platforms, this macro has no effect.
#ifdef WIN32
    #ifdef KALACCESS_STATIC
        #define KALACCESS_API
    #elif defined(KALACCESS_EXPORTS)
        #define KALACCESS_API __declspec(dllexport)
    #else
        #define KALACCESS_API __declspec(dllimport)
    #endif
#else
    #define KALACCESS_API
#endif


// Ensure a bool type is defined if consuming the library from C99.
#include <stdbool.h>
// The kalaccess interface uses stdint fixed-width types, e.g. uint32_t.
#include <stdint.h>


/// \c _kalaccess is an internal type whose implementation is hidden. Library users only encounter
/// pointers to a ::kalaccess structure; this should be used as a connection handle (and will not
/// dereference, as there is no available implementation).
typedef struct _kalaccess kalaccess;


/// Supported instruction sets; not all may apply to the connected chip.
typedef enum
{
    KA_INSN_SET_MAXIMODE,
    KA_INSN_SET_MINIMODE
} ka_instruction_set;


/// Error codes that may be returned by this library. Wrapped in a ::ka_err.
typedef enum
{
    KA_COULD_NOT_CONNECT = 1,
    KA_TRANSPORT_FAILURE,
    KA_NOT_CONNECTED,
    KA_UNRECOGNISED_CHIP,
    KA_BREAKPOINT_ID_INVALID,
    KA_PM_ACCESS_OUT_OF_BOUNDS,
    KA_PM_REGIONS_OVERLAP,
    KA_ADDRESS_ALREADY_CONTAINS_BREAK,
    KA_NO_BREAKPOINTS_FREE,
    KA_NO_BREAKPOINT_AT_SPECIFIED_ADDRESS,
    KA_NO_PM_REGIONS_REGISTERED,
    KA_INVALID_REGISTER_ID,
    KA_INTERLOCK_NOT_FOUND,
    KA_COULD_NOT_INIT_INTERLOCK,
    KA_COULD_NOT_ACQUIRE_INTERLOCK,
    KA_COULD_NOT_RELEASE_INTERLOCK,
    KA_COULD_NOT_UPDATE_STORED_PM_DATA,
    KA_DSP_MEM_READ_TOO_SLOW,
    KA_DSP_MEM_WRITE_TOO_SLOW,
    KA_UNALIGNED_ACCESS,
    KA_DATA_BREAKPOINT_RANGE_UNSUPPORTED,
    KA_NO_CURATOR,
    KA_WRONG_PTTRANS_DLL_VERSION,
    KA_SINGSTEP_TIMEOUT,
    KA_WORD_OVERFLOW,
    KA_FEATURE_REQUIRES_TRB,
    KA_INVALID_SUBSYS_OR_PROCESSOR_ID,
    KA_INVALID_ARGUMENT,
    KA_BREAKPOINT_SITE_DOES_NOT_CONTAIN_VALID_CODE,
    KA_COULD_NOT_ACQUIRE_TRANSPORT_LOCK,
    KA_CORE_URI_INVALID,
    KA_CORE_URI_WRONG_CHIP,
    KA_ACCESS_SIZE_UNSUPPORTED,
} ka_err_code;


/// Holds error information. When a function fails, a pointer to a ka_err is returned. This should
/// be freed via ka_free_error().
typedef struct
{
    ka_err_code err_code;
    char *err_string;
} ka_err;


/// List of all possible registers. Some may not apply to the connected chip.
typedef enum
{
    RegFirstReg                  = 0,
    RegPC                        = 0,
    RegRMAC2                     = 1,
    RegRMAC1                     = 2,
    RegRMAC0                     = 3,
    RegRMAC24                    = 4,
    RegR0                        = 5,
    RegR1                        = 6,
    RegR2                        = 7,
    RegR3                        = 8,
    RegR4                        = 9,
    RegR5                        = 10,
    RegR6                        = 11,
    RegR7                        = 12,
    RegR8                        = 13,
    RegR9                        = 14,
    RegR10                       = 15,
    RegRLINK                     = 16,
    RegRFLAGS                    = 17,
    RegRINTLINK                  = 18,
    RegI0                        = 19,
    RegI1                        = 20,
    RegI2                        = 21,
    RegI3                        = 22,
    RegI4                        = 23,
    RegI5                        = 24,
    RegI6                        = 25,
    RegI7                        = 26,
    RegM0                        = 27,
    RegM1                        = 28,
    RegM2                        = 29,
    RegM3                        = 30,
    RegL0                        = 31,
    RegL1                        = 32,
    RegL4                        = 33,
    RegL5                        = 34,
    RegRUNCLKS                   = 35,
    RegRUNINSTRS                 = 36,
    RegNUMSTALLS                 = 37,
    RegRMACB2                    = 38,
    RegRMACB1                    = 39,
    RegRMACB0                    = 40,
    RegB0                        = 41,
    RegB1                        = 42,
    RegB4                        = 43,
    RegB5                        = 44,
    RegFP                        = 45,
    RegSP                        = 46,
    RegRMACB24                   = 47,      // In kal arch 3 and above, RMACB24 replaces RINTLINK
    RegArithmeticMode            = 48,
    RegDoloopStart               = 49,
    RegDoloopEnd                 = 50,
    RegDivResult                 = 51,
    RegDivRemainder              = 52,
    RegDbgCountersEn             = 53,
    RegNumRegs,
    RegNoSuchRegister            = 0xffff,
} ka_register_id;


/// "Fixed" subsystem identifiers for CDA chips. Other subsystems are discovered by enumeration;
/// see ::ka_connection_details and ka_trans_build_device_table().
typedef enum
{
    /// Used if the connected device is not a CDA chip; in this case there is a
    /// single Kalimba debug target (Bluecore family, A7DA). 
    KaSubsysIdNone = -1,
    /// The Curator subsystem on CDA targets has ID 0.
    KaSubsysIdCurator = 0
} ka_subsystem_id;


/// Contains information which may be used to connect to a device.
typedef struct _ka_connection_details
{
    /// For SPI: a pttransport-style transport string
    /// For TRB: either
    /// a) the older form: "SPITRANS=trb". This attempts connection to a Murphy.
    /// b) new form: "trb/usb2trb/123456" or "trb/qs", where "123456" represents the usb2trb dongle 
    /// serial number. 
    char const *transport_string;
    /// CDA subsystem ID, or ka_subsystem_id::KaSubsysIdNone if the target chip is not a CDA chip.
    int subsys_id;
    /// Zero-based processor ID; 0 if only one processor exists.
    unsigned processor_id;
    /// A string identifying the debug dongle; typically will contain a name and a unique identifier
    /// such as a serial number.
    char const *dongle_id_string;
} ka_connection_details;


// *************************************************************************************************
// General library operations
// *************************************************************************************************


/// Retrieve the code revision this library was built from.
KALACCESS_API char const *ka_get_ka_version();


/// Free an error object returned by a function on this API.
KALACCESS_API void ka_free_error(ka_err *err);


// *************************************************************************************************
// Enumeration
// *************************************************************************************************


/// Retrieve a list of available, debuggable, Kalimba cores.
/// Each debug transport is queried, and the identity of the chip at the other end of the connection
/// is determined. Matching Kalimba cores are returned; for CDA chips, one entry is returned per
/// core, so a chip with two Kalimba subsystems with two cores in each contributes four entries.
/// Free the resultant list with ka_trans_free_device_table(). 
KALACCESS_API ka_err *ka_trans_build_device_table(ka_connection_details **devices, int *count);


/// As ka_trans_build_device_table(), but additionally checks whether the given transport string 
/// corresponds to any of the devices returned. If the transport string does correspond to a device,
/// *index_of_transport_string is set to the index of that device.
KALACCESS_API ka_err *ka_trans_build_device_table_and_check_trans_string(ka_connection_details **devices, int *count, 
                                                                         char const *transport_string_to_check, 
                                                                         int *index_of_transport_string);


/// Free a list of devices returned by ka_trans_build_device_table() or 
/// ka_trans_build_device_table_and_check_trans_string().
KALACCESS_API void ka_trans_free_device_table(ka_connection_details *table, int count);


// *************************************************************************************************
// Connection
// *************************************************************************************************


/// Returns whether the supplied kalaccess session is valid.
KALACCESS_API bool ka_is_connected(kalaccess *ka);


/// \brief Connect to a device. This function returns a kalaccess session for use with most of the
/// other functions in this API.
/// \param[in] conn_details contains the parameter to be used for the connection. The
/// transport_string member should either:
/// - be a valid pttransport-style "SPITRANS" setting (for a SPI connection)
/// - begin with "trb" (for a transaction bridge connection)
/// - begin with "tc/usb2tc" (for a USBDebug connection)
/// For SPI, valid SPITRANS options include 'usb', 'lpt' 'kalsim', e.g. "SPITRANS=kalsim".
/// Other SPI transport variables include SPICLOCK, SPIPORT and SPIMUL; see the pttransport 
/// documentation for more details. When making a SPI connection, transport_string is forwarded to
/// pttransport.
/// Valid examples of transport strings for TRB and USBDebug, respectively:-
/// - "trb/scar/<n>", where <n> is the serial number of the debug dongle, e.g. 123456.
/// - "tc/usb2tc/<n>", where <n> is the USBDebug device identifier, e.g. 100.
/// Transport strings can be obtained by enumeration: see ka_trans_build_device_table() and 
/// ka_connection_details.
/// \param[in] ignore_fw whether to skip initialisation of the firmware interlock, on relevant
/// architectures (Kalimba architecture 3 or below). Ignored on architectures 4 and 5.
/// \param[out] kalaccess_result receives the kalaccess handle
KALACCESS_API ka_err *ka_connect(ka_connection_details const *conn_details, bool ignore_fw, kalaccess **kalaccess_result);


/// \brief Connect by supplying only a transport string. This form is provided for backwards
/// compatibility, and ease of use, where the ability to select a transaction bus
/// subsystem, or a specific processor, is not needed or not relevant.
KALACCESS_API ka_err *ka_connect_simple(char const *transport_string, bool ignore_fw, kalaccess **kalaccess_result);


/// \brief Connect to a device using a Heracles URI.
/// \param[in] uri a valid Heracles core URI. Examples include:
/// - device://trb/scarlet/138026/crescendo/app/p0
/// - device://tc/usb2tc/100/crescendo/app/p0
/// - device://spi/babel/241148/gordon/audio
/// - device://tcp/localhost:31400/spi/sim/gordon/audio
/// \param[out] kalaccess_result receives the kalaccess handle
KALACCESS_API ka_err *ka_connect_uri(char const *uri, kalaccess **kalaccess_result);


/// \brief Disconnects the given session and frees associated resources.
/// This function removes breakpoints set by the given session.
/// \param[in] ka a session previously supplied by one of the connection functions. This object is
/// freed by this function, so the pointer should not be used after it returns.
KALACCESS_API void ka_disconnect(kalaccess *ka);


// *************************************************************************************************
// Transport properties
// *************************************************************************************************


/// Retrieve the value of a transport variable such as SPICLOCK, SPIPORT, SPITRANS and SPIMUL.
/// Valid variables depend on the transport being used for the supplied session.
KALACCESS_API char const *ka_trans_get_var(kalaccess *ka, char const *var);


/// Set the value of a transport variable. The format of the variable is transport-dependent.
/// See ka_trans_get_var().
KALACCESS_API void ka_trans_set_var(kalaccess *ka, char const *var, char const *val);


// *************************************************************************************************
// Chip identity and properties
// *************************************************************************************************


/// Retrieve the major part of the global chip version of the chip associated with the given
/// kalaccess connection.
KALACCESS_API unsigned   ka_get_global_chip_version(kalaccess *ka);


/// Retrieve the minor part of the global chip version of the chip associated with the given
/// kalaccess connection.
KALACCESS_API unsigned   ka_get_minor_chip_version(kalaccess *ka);


/// Retrieve the name of the chip associated with the given kalaccess connection.
KALACCESS_API const char *ka_get_chip_name(kalaccess *ka);


/// Check if the chip associated with the given kalaccess connection is a match for the given name.
/// This function checks against the name returned by ka_get_chip_name() as well as any known
/// aliases.
/// The match is case-insensitive.
KALACCESS_API bool ka_check_chip_name(kalaccess *ka, char const *name);


/// Retrieve the Kalimba architecture number of the chip associated with the given kalaccess 
/// connection.
KALACCESS_API unsigned ka_get_arch(kalaccess *ka);


/// Retrieve the Kalimba architecture number of a chip, given its name.
/// \returns zero if no match was found.
KALACCESS_API unsigned ka_get_arch_from_name(char const *name);


/// Retrieve the CDA subsystem ID associated with the given kalaccess connection.
/// \returns ka_subsystem_id::KaSubsysIdNone if the target is not a CDA chip.
KALACCESS_API int ka_get_subsystem_id(kalaccess *ka);


/// Retrieve the ID of the connected processor.
/// \returns ::KA_NOT_CONNECTED if not connected.
KALACCESS_API ka_err* ka_get_processor_id(kalaccess *ka, unsigned *result);


/// Get the address width, i.e. the number of bits in an address (for PM or DM, assumed to be
/// equal), in units of bits.
KALACCESS_API unsigned ka_get_address_width(kalaccess *ka);


/// Get the (DM) data width, i.e. the number of bits in a word of DM, in units of bits.
KALACCESS_API unsigned ka_get_data_width(kalaccess *ka);


/// Query whether the target supports sub-word (i.e., octet-based) accesses to DM.
KALACCESS_API bool ka_dm_subword_addressing(kalaccess *ka);


/// Query whether the target supports sub-word (i.e., octet-based) accesses to PM.
KALACCESS_API bool ka_pm_subword_addressing(kalaccess *ka);


/// Query whether the target supports the Minim instruction set.
KALACCESS_API bool ka_supports_minim(kalaccess *ka);


// *************************************************************************************************
// Register access
// *************************************************************************************************


/// Get a list of the names of all the possible core registers across all architectures.
/// The list is terminated with the entry "NumRegs".
KALACCESS_API char const **ka_get_register_names();


/// Get the id associated with the named register.
/// \return ka_register_id::RegNoSuchRegister if either
/// - the register name is not known 
/// - the register is not applicable to the connected chip.
KALACCESS_API ka_register_id ka_get_register_id(kalaccess *ka, char const *name);


/// Get the bit width of the specified register. 
/// \returns zero if either
/// - the register name is not known
/// - the register is not applicable to the connected chip.
KALACCESS_API unsigned ka_get_register_width(kalaccess *ka, ka_register_id id);


/// Get properties associated with the specified register.
/// \returns an error if either
/// - the register name is not known
/// - the register is not applicable to the connected chip.
KALACCESS_API ka_err *ka_get_register_properties(kalaccess *ka, ka_register_id id, 
                                                 bool *read_only, bool *read_sensitive);


/// Get grouping information for a register, e.g. "General registers", "Stack registers".
/// \returns an empty string if there is no associated group.
KALACCESS_API char const *ka_get_register_grouping(ka_register_id id);


/// Bitfield properties returned by ka_get_register_bitfield_info().
typedef struct
{
    uint16_t    start_bit;
    uint16_t    bit_count;
    char const *name;
} ka_register_bitfield;


/// Get information about the bitfields in the specified register.
/// If the register does not contain bitfields, *count is set to zero and *bitfield_results is set 
/// to null.
/// If the register does contain bitfields, the memory associated with the results is valid until 
/// the library is unloaded. 
/// \returns an error if either
/// - the register name is not known
/// - the register is not applicable to the connected chip.
KALACCESS_API ka_err *ka_get_register_bitfield_info(kalaccess *ka, ka_register_id id, 
                                                    uint16_t *count, 
                                                    ka_register_bitfield const **bitfield_results);

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_KALACCESS_COMMON_H
