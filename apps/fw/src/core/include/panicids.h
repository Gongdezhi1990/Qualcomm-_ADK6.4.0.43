/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Reason Y codes for dying noisily
*/

#ifndef PANICIDS_H
#define PANICIDS_H

/* Reasons to be cheerless */
typedef enum panicid
{
#include "hydra/hydra_panicids.h"
#include "bt/bt/bt_panicids.h"
#include "nfc/nfc/nfc_panicids.h"

    /**
     * An interrupt has fired that has no handler installed.
     */
    PANIC_UNREGISTERED_INTERRUPT = 0x7e,

    /**
     * An attempt has been made to install an interrupt handler using an
     * invalid priority (level).
     */
    PANIC_INVALID_INTERRUPT_PRIORITY = 0x7f,

    /**
     * An attempt was made to create a transport channel (e.g. transport_bt)
     * with a handle that is already being used. The parameter is the handle
     * number.
     */
    PANIC_ATTEMPT_TO_CREATE_DUPLICATE_CHANNEL = 0x80,

    /**
     * When a client has requested the result of a DMA the result didn't
     * match the entry in the buffer. This is a programming error.
     * The parameter is (expected tag <<8) | (tag from the result buffer).
     */
    PANIC_DMA_UNMATCHED_RESULT_ENTRY = 0x82,

   /**
     * Buffer creation failed during an uart raw init call
     */
    PANIC_UART_RAW_BUFFER_CREATION_FAILED = 0x83,

   /**
     * Received an unknown event. Most likely the status descriptor was 
     * corrupted.
     */
    PANIC_UART_RAW_UNKNOWN_EVENT = 0x84,

   /**
     * Received an unexpected event. Something has gone wrong in HW.
     */
    PANIC_UART_RAW_UNEXPECTED_EVENT = 0x85,

   /**
     * Transmission tag mismatch.
     */ 
    PANIC_UART_RAW_TAG_MISMATCH = 0x86,

    /**
     * Kalimba has raised a memory exception.  The type of exception is given
     * in the diatribe as per the io_defs enum \c exception_type. (The value is
     * simply obtained from the register \c EXCEPTION_TYPE and just appears in
     * the diatribe for convenience).
     */
    PANIC_KALIMBA_MEMORY_EXCEPTION = 0x87,

    /**
     * A queued DMA transfer operation has failed.
     * The parameter is the (queue number << 24) ORed with the value of
     * the DMA_QUEUE_x_STATUS register (bits 17:0).
     */
    PANIC_DMA_QUEUE_TRANSFER_FAILURE = 0x88,

    /**
     *  The Bluetooth service start request ended in failure.
     */
    PANIC_TRANSPORT_BT_SERVICE_START_FAILED = 0x89,

    /**
     * Received an unknown event. 
     */
    PANIC_BITSERIAL_UNKNOWN_EVENT = 0x8a,
    
    /**
     * Bitserial operational error. 
     */
    PANIC_BITSERIAL_OP_ERROR = 0x8b,

    /**
     * Bitserial HAL error. 
     */
    PANIC_BITSERIAL_HAL_ERROR = 0x8c,

    /**
     * IPC couldn't create its reserved P0-P1 buffer.
     */
    PANIC_IPC_RESERVED_BUFFER_CREATION_FAILURE = 0x8d,

    /**
     * IPC received a message with no handler.  The message ID is in the
     * diatribe.
     */
    PANIC_IPC_UNHANDLED_MESSAGE_ID = 0x8e,

    /**
     * A blocking message was received while we weren't blocking.
     */
    PANIC_IPC_UNEXPECTED_BLOCKING_MSG = 0x8f,

    /** USB Host Core did not recover from FIFO reset */
    PANIC_USB_HOST_FIFO_RESET_TIMEOUT = 0x90,

    /** USB Host Core FIFO buffers configuration error */
    PANIC_USB_HOST_SRAM_IS_NOT_BIG_ENOUGH = 0x91,

    /** USB Host Core hardware problem */
    PANIC_USB_HOST_HARDWARE_ERROR = 0x92,

    /** USB Host Core enumeration state machine error */
    PANIC_USB_HOST_ENUMERATION_FSM_ERRROR = 0x93,

    /** USB Host Core error while populating DMA descriptors */
    PANIC_USB_HOST_DMA_DESC_POPULATION_ERRROR = 0x94,

    /** USB Host Core error while processing isochronous DMA descriptors */
    PANIC_USB_HOST_ISOC_DMA_DESC_SYNC_ERRROR = 0x95,

    /** USB Host Core did not recover from reset */
    PANIC_USB_HOST_CORE_RESET_TIMEOUT = 0x96,

    /**
     * Scheduler task creation failed when attempting to create a mapped task
     * for a new customer app task handler.  The diatribe is the address of the
     * customer app task handler function.
     */
    PANIC_TRAP_API_TASK_CREATION_FAILED = 0x97,

    /**
     * A generic error in the streams module
     */
    PANIC_STREAMS_ERROR  = 0x98,

    /**
     * Two many streams have been created
     */
    PANIC_STREAMS_TO_MANY_STREAMS  = 0x99,

    /**
     * No more space in the underlying streams buffer
     */
    PANIC_STREAMS_BUFFER_OVERFLOW  = 0x9a,

    /**
     * Internal error, unsupported stream type
     */
    PANIC_STREAM_NO_SUCH_STREAM = 0x9b,

    /**
     * An exception outside the kalimba core occurred (exceptions within
     * the core cause PANIC_KALIMBA_MEMORY_EXCEPTION). The diatribe is the
     * value from APPS_BANKED_EXCEPTIONS_P1_STATUS.
     */
    PANIC_KALIMBA_EXCEPTION = 0x9c,

    /** USB Host resource request/release failed */
    PANIC_USB_HOST_SUBRES_FAILED = 0x9d,

    /**
     * A resource needed by a trap has failed.
     */
    PANIC_TRAP_RESOURCE_FAILED = 0x9e,

    /**
     * bluestack_if has either received a primitive from the App but the
     * interface has already been taken over by the host (presumably for test
     * purposes) - diatribe 0, or has received a second primitive before it has
     * managed to dispatch the first one - diatribe 1
     */
    PANIC_BLUESTACK_IF_CLIENT_ERROR = 0x9f,

    /**
     * We have received a bccmd with a sequence number that doesn't appear in
     * our pending list.  The sequence number is passed as the diatribe.
     */
    PANIC_BCCMD_CLIENT_UNRECOGNISED_SEQUENCE_NUMBER = 0xa0,

    /** 
     * The firmware failed to get the requested the PIOs for the SD slots 
     * when it requested the resource from the Curator. The diatribe value
     * gives the slot number for the request which failed. Each slot is 
     * enabled by a MIBKEY. For example SD Slot 0 is enabled via SDHostSlot0
     */
    PANIC_SD_HOST_SUBRES_FAILED = 0xa1,

    /** A fatal problem occurred with the Bluestack interface.  The
     * diatribe is a member of the enum BLUESTACK_IF_PANIC_REASON */
    PANIC_BLUESTACK_IF = 0xa2,

    /** The string descriptor passed via UsbAddStringDescriptor Trap API
     * is suspected to be not a correct Unicode string */
    PANIC_NON_UNICODE_STRING_DESCRIPTOR = 0xa3,

    /** An unhadled IPC operation was found. The diatribe is the op code. */
    PANIC_PIO_UNHANDLED_IPC_OP = 0xa4,

    /** The subres request failed. This is a messaging problem, not a request
        denial from Curator. */
    PANIC_PIO_SUBRES_FAILED = 0xa5,

    /** Some inconsistency was detected in the stream subsystem (try to use an invalid
     *  handle to create (duplicate) or destroy (unknown) a SCO stream).
     */
    PANIC_STREAM_GONE_MISSING = 0xa6,

    /** P1 has panicked. We panic P0 as well. */
    PANIC_P1_PANICKED = 0xa7,

    /** P1 application panic. */
    PANIC_APP_FAILED = 0xa8,

    /** Inconsistency in the use of siflash_claim() and siflash_release()
     * has been detected: expected siflash_release().
     */
    PANIC_SQIF_ALREADY_CLAIMED = 0xa9,

    /** Inconsistency in the use of siflash_claim() and siflash_release()
     * has been detected: expected siflash_claim().
     */
    PANIC_SQIF_NOT_CLAIMED = 0xaa,

    /** SQIF chip identification failed */
    PANIC_FAILED_TO_FIND_EXPECTED_SQIF = 0xab,

    /**
     * A resource needed by licence manager has failed.
     */
    PANIC_LMGR_RESOURCE_FAILED = 0xac,

    /**
     * CPU1 has tried to access a restricted address. 
     */
    PANIC_CPU1_RESTRICTED_ACCESS = 0xad,

    /**
     * Psflash encountered failure of a serial flash command.
     */
    PANIC_PSFLASH_SERIAL_FLASH_COMMAND_FAILED = 0xae,

    /**
     * Psflash can't map storage for reading
     */
    PANIC_PSFLASH_STORE_MAP_FAILED = 0xaf,

    /**
     * P1 application watchdog timed-out - when
     * VmSoftwareWdKick trap is not kicked in the
     * expected timeout window.
     */
    PANIC_VM_SW_WD_EXPIRED = 0xb0,
    
    /**
     * The boot image at the start of flash is invalid. The curator
     * should have checked this before starting the apps.
     */
    PANIC_INVALID_FLASH_BOOT_IMAGE = 0xb1,

    /**
     * The image header read from flash is invalid.
     * The diatribe has 0 for the primary image or 1 for the secondary.
     * The curator should have checked this before starting the apps.
     */
    PANIC_INVALID_FLASH_IMAGE = 0xb2,

    /**
     * There was a problem with requesting the PIO controller
     */
    PANIC_PIO_8051_SUBRES_FAIL = 0xb3,
    
    /**
     * PIO 8051 firmware included a bad data address. The argument is the
     * offset into the 8051 memory map in octets. Note this offset is not the
     * same as the offset into the Janitor memory map seen by the Apps, still
     * less an offset into the Appses own memory map.
     */
    PANIC_PIO_8051_BAD_ADDRESS = 0xb4,
    
    /**
    * EFUSE stop-dead bit prevents Apps Firmware from running
    */
    PANIC_EFUSE_STOP_DEAD = 0xb5,
    
    /** P0 has panicked. We panic P1 as well. */
    PANIC_P0_PANICKED = 0xb6,
    
    /**
    * There is a discrepancy between the Curator and the Apps version of
    * CCP_PIO_CONFIG_KEY.
    */
    PANIC_PIO_KEY_VALUE_ERROR = 0xb7,

    /** Invalid transform or transform chain. */
    PANIC_INVALID_TRANSFORM = 0xb8,

    /** Unable to stop CSB processing service whilst stopping CSB transmission. */
    PANIC_STOP_CSB_PROC_SERVICE_FAILED = 0xb9,

    /* There was a timeout in reading BT wallclock information from shared mmu
     * handle and offset. BT wallclock counter indicates that it is being
     * updated, but apps has timed out waiting for the update to complete.
     * This probably indicates a BT not behaving properly or apps reading the
     * buffer wrongly.
     * The diatribe contains the client_id of wallclock service which caused
     * the panic.
     */
    PANIC_BT_WALLCLOCK_UPDATE_TIMEOUT = 0x00ba,

    /**
     * SD/MMC stream card transfer failed.
     */
    PANIC_SD_MMC_STREAM_CARD_XFER_FAILED = 0xbb,

    /**
     * Siflash configuration failed.
     */
    PANIC_SIFLASH_CFG_FAILED = 0xbc,


    /**
     * Apps P1 application inactive after restart of the chip.
     *
     * It is expected that the application running in Apps P1
     * shall invoke VmSoftwareWdKick() trap within the time
     * specified in APPSP1INACTIVETIMEOUT MIB key after the chip reset.
     */
    PANIC_APPS_P1_INACTIVE_AFTER_RESET = 0xbd,

    /**
     * The UART raw module failed to request Curator UART
     * via subres, either due to the request send failed or because
     * Curator UART resource was not available.
     */
    PANIC_UART_SUBRES_FAILED = 0xbe,

    /**
     * The Trap API version of P1 code is incompatible with P0.
     * When this occurs check apps0.fw.slt.trap_verion against
     * apps1.fw.slt.trap_verion or use apps1.fw.check_version_info()
     * CS-328404-DD section 7.2.1 describes the compatibility criteria -
     * namely that major versions must match and the minor version of P1
     * code must be less than or equal to the minor version of P0 code.
     */
    PANIC_TRAP_API_VERSION_INCOMPATIBILITY = 0xbf,

    /**
     * The trapsets supported by P0 are not sufficient for the P1 code
     * When this occurs check apps0.fw.slt.trapset_bitmap against
     * apps1.fw.slt.trapset_bitmap or use apps1.fw.check_version_info().
     * Bits set in the app1 bitmap that
     * are not set in the apps0 bitmap indicate traps that are required
     * by P1 but not supported by P0.
     */
    PANIC_INCOMPATIBLE_TRAPSETS = 0xc0,

    /**
     * VM USB device configuration failed.
     */
    PANIC_VM_USB_DEVICE_CFG_FAILED = 0xc1,

    /**
     * PSU module encountered an internal error.
     */
    PANIC_PSU_INTERNAL_ERROR = 0xc2,

    /**
     * RWFS task has been resumed before requested operation completed. This
     * indicates there is a problem with scheduler priorities.
     */
    PANIC_RWFS_INVALID_TASK_SCHEDULING = 0xc3,

    /**
     * data_conv panic code.
     */
    PANIC_DATA_CONV = 0xc4,

    /**
     * data_conv_seq panic code.
     */
    PANIC_DATA_CONV_SEQ = 0xc5,

    /**
     * temperature panic code.
     */
    PANIC_TEMPERATURE = 0xc6,

    /**
     * vm_adc panic code.
     */
    PANIC_VM_ADC = 0xc7,

    /**
     * Read-write config and filesystem image sections are not contiguous in
     * the QSPI device.
     */
    PANIC_FLASH_HEADER_RW_SECTIONS_NOT_CONTIGUOUS = 0xc8,

    /**
     * The configuration manager failed to process a configuration file.
     */
    PANIC_CONMAN_CONFIG_FAILED = 0xc9,

    /**
     * The AES-CBC authentication failed for the flash section passed in the
     * diatribe (0 = config RO FS, 1 = device RO FS, 2 = RO FS, 5 = Apps P1).
     */
    PANIC_FLASH_SECTION_AUTHENTICATION_FAILED = 0xca,

    /** Writable filesystem - available flash space not enough */
    PANIC_NOT_ENOUGH_FLASH_SPACE = 0xcb,

    /** Remote ADC panic code */
    ADC_REMOTE_PANIC = 0xcc,

    /**
     * Unexpected toolcommand data response from Curator. 
     * The panic diatribe contains the received response.
     */
    PANIC_UNEXPECTED_TOOLCMD_DATA_RSP = 0xcd,

    /**
     * A mandatory flash section is not found in the Apps QSPI image header.
     * The panic diatribe contains the missing flash section.
     */
    PANIC_FLASH_SECTION_NOT_FOUND = 0xce,

    /**
     * PIO configuration panic code.
     */
    PANIC_PIO_CFG = 0xcf,

    /**
     * PIO trap parameter error. This is used for traps that don't have any
     * capability of reporting errors.
     */
    PANIC_PIO_TRAP_BAD_PARAMETERS = 0xd0,

    /**
     * Unsupported IPC message parameters.
     */
    PANIC_IPC_MESSAGE_PARAMETERS = 0xd1,

    /**
     * UART PIO setup failure.
     */
    PANIC_UART_RAW_PIO_SETUP_FAILED = 0xd2,

    /**
     * Licence manager has failed during licence check operation.
     */
    PANIC_LICENCE_MANAGER_FAILED = 0xd3,

    /**
     * The 8051 PIO controller received a message it wasn't expecting.
     *
     * The 8051 PIO controller message handler doesn't expect any messages so
     * this panic implies a message has been sent to the wrong queue ID.
     */
    PANIC_PIO_8051_UNEXPECTED_MESSAGE = 0xd4,

    /**
     * USB Type-C configuration is incorrect. This includes UsbTypeCCCSource
     * MIB that defines PIO for CC-line monitoring and CcVoltageThresholdAt*
     * MIBs for voltage levels.
     *
     * Diatribe gives the value that caused the panic.
     */
    PANIC_CHARGER_TYPE_C_CONFIG_INCORRECT = 0xd5,

    /**
     * Critical error while doing USB Type-C related charger support.
     */
    PANIC_CHARGER_TYPE_C_INTERNAL_ERROR = 0xd6,

    /**
     * Error encountered while configuring the PIO for CC-line polling.
     * Diatribe has command in the high word and status in the low word.
     */
    PANIC_CHARGER_TYPE_C_PIO_CFG_ERROR = 0xd7,

    /**
    * Security features prevent Apps Firmware from running
    * Arg 0 - Could not run efuse runtime security check
    * Arg 1 - Does not meet minimum fw build requirement
    * Arg 2 - Does not meet part number requirement
    * Arg 3 - Part number has been tampered
    */
    PANIC_EFUSE_SECURITY_CHECK_FAILED = 0xd8,

    /**
    * Audio capability download operation has failed. The diatribe
    * indicates failure reason:
    * 0 - Hardware can't support download operation.
    * 1 - Key_index doesn't match hardware configuration.
    * 2 - Authentication check has failed.
    * 3 - Audio capability has not been signed
    */
    PANIC_AUDIO_CAPABILITY_DOWNLOAD_FAILED = 0xd9,

     /**
     * Not enough space was available in the production test TX command buffer.
     * The diatribe contains the number of octets that was required by the
     * production test operation.
     */
    PANIC_PRODUCTION_TEST_NOT_ENOUGH_BUFFER_SPACE = 0xda,

    /**
    * Psflash failed. The diatribe indicates failure reason.
    */
    PANIC_PSFLASH_RANDCACHE_FAILED = 0xdb,

    /**
     * A production test buffer contains a command that starts at an
     * address that isn't aligned to 32 bits.
     * The diatribe is the unaligned command's address.
     */
    PANIC_PRODUCTION_TEST_BAD_ALIGNMENT = 0xdc,

    /**
     * The UART RX buffer has overflowed.
     */
    PANIC_STREAMS_UART_BUFFER_OVERFLOW = 0xdd,

    /**
     * Curator SLT table check has failed. The diatribe
     * indicates failure reason.
     * 0 - Magic fingerprint mismatch found
     * 1 - Efuse cache entry not found
     */
    PANIC_CURATOR_SLT_CHECK_FAILED = 0xde,

    /**
     * Writable filesystem can't be mounted owing to inappropriate flash
     * configuration.
     * Supported flash configuration is 4KB block sized. Any RWFS operations
     * on a non-4KB block sized configured flash, results in a hard fault.
     */
    PANIC_RWFS_UNSUPPORTED_FLASH_CFG = 0xdf,

    /**
     * A function pointer points to a non-push instruction, and is therefore
     * assumed not to point at a function.
     */
    PANIC_P1_INVALID_FUNCTION_POINTER = 0xe0,

    /**
     * An unrecognised entry has been read from the transport control buffer
     */
    PANIC_TRANSPORT_BT_UNRECOGNISED_CMD = 0xe5,

    /**
     * Bitserial transfer has not finished when the timeout expired.
     */
    PANIC_BITSERIAL_TIMEOUT = 0xe7,

    /**
     * The DMA hardware is being polled for it to return to IDLE with the
     * clock request released but the clock request is still high after
     * more than a millisecond.
     */
    PANIC_DMA_FAILED_TO_RETURN_TO_IDLE = 0xe8,

    /**
     * NOTE: 0x07ff is the highest panic code in the Qualcomm range
     * Qualcomm add codes from the lowest free number towards this high one
     */
    
    /**
     * Do not use this special value - it indicates "not a panic"
     * (placed at the end of the list to prevent some compilers complaining
     * about a superfluous final comma within the enum declaration) */
    PANIC_NONE = 0x00
} panicid;

/**
  * The pool memory manager (pmalloc) has run out of memory. 
  *  
  * PANIC_HEAP_EXHAUSTION is used in the BT stack cdoe.  It has 
  * the same value as the HYDRA panic code. 
  *  
  * Adding the following in bt_panicid.h 
  * PANIC_HEAP_EXHAUSTION = 
  * PANIC_HYDRA_PRIVATE_MEMORY_EXHAUSTION) upsets the pydbg 
  * script (on 11/11/2014).  The translation from PANIC code to 
  * panic string does not work for duplicated values. 
  *  
  * It was judged not worthwhile to fix the python script so the
  * following solution was adopted. 
  */
#define PANIC_HEAP_EXHAUSTION (PANIC_HYDRA_PRIVATE_MEMORY_EXHAUSTION)

#endif /* PANICIDS_H */
