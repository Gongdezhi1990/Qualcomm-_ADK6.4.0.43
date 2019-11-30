/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */

#ifndef FAULTIDS_H
#define FAULTIDS_H

/* Reasons to be cheerless. */
typedef enum faultid
{
#include "hydra/hydra_faultids.h"
#include "bt/bt/bt_faultids.h"
#include "nfc/nfc/nfc_faultids.h"
    /**
     * Insert Application subsystem specific fault IDs here
     */
    /**
     * There's no room to put another response message into the Appcmd response
     * buffer, so it is dropped
     */
    FAULT_APPCMD_RSP_BUF_PACKET_DROPPED = 0x1,

    /**
     * TODO If and when the file module becomes hydra generic code these fault
     * codes need to be moved to hydra_faultids
     */

    /**
     * An invalid file offset has been requested, or the filesystem claims that
     * it is bigger than its partition
     */
    FAULT_FILE_SYSTEM_ADDR_RANGE_CHK_FAIL = 0x2,

    /**
     * The file module failed to mount a filesystem.  When there are multiple
     * filesystems supported the argument is the filesystem index.
     */
    FAULT_FILE_BAD_FILESYSTEM = 0x3,

    /**
     * The file module failed to read from the eeprom device. The argument
     * is the LSW of the address being accessed.
     */
    FAULT_FILE_FAILED_READ = 0x4,

    /**
     * The BCCMD response for InquiryGetPriority had a not-OK response status.
     * We have a fault because the trap API has no way of indicating this
     * outcome to the App.
     */
     FAULT_INQUIRY_GET_PRIORITY_BAD_RSP = 0x5,

    /**
     * A fault occured in licence manager while starting a service to verify
     * the licence.
     */
    FAULT_LICENCE_MANAGER_STARTING_SERVICE = 0x6,

    /**
     * There wasn't enough room in the Sink buffer whilst trying to perform
     * a copy and the data was dropped.
     */
    FAULT_TRANSFORM_COPY_FAIL_DATA_DROPPED = 0x7,

    /**
      * The IPC needs to be able to translate const pointers from p1
      * program memory to p0 code memory. Either P1 is running from
      * SQIF1 or the image table doesn't have the right entries. This
      * fault means that P1 may not be running at all.
      */
    FAULT_IPC_UNSUPPORTED_CONFIG = 0x8,
    
    /**
     * IPC PIO operation has no effect.
     */
    FAULT_IPC_PIO_NO_EFFECT = 0x9,

    /**
     * The store of reset data supplied by the Curator was accessed before it
     * was available or the Curator is out of date and therefore didn't supply
     * any.
     */
    FAULT_RESET_DATA_NOT_AVAILABLE = 0xa,

    /**
     * Isochronous USB RX data was dropped due to no space being in the
     * destination buffer. It's likely no module is consuming the data
     * or it's doing so too slowly.
     */
    FAULT_USB_ISOCHRONOUS_RX_DATA_DROPPED = 0xc,
     
    /**
     * Isochronous USB TX data was dropped due to no space being in the
     * destination buffer. It's likely the host is not consuming the data
     * or it's doing so too slowly.
     */
    FAULT_USB_ISOCHRONOUS_TX_DATA_DROPPED = 0xd,

    /**
     * A NULL pointer was passed to HostSendMessage.
     */
    FAULT_HOST_COMMS_NULL_MESSAGE = 0xe,

    /**
     * The message passed to HostSendMessage was formatted to target a stream
     * or was malformed. i.e. The top 8 bits of the sub-type were something
     * other than 0.
     */
    FAULT_HOST_COMMS_BAD_MODE = 0xf,

    /**
     * A message received from the host had a sub-type other than 0, 1 or 2.
     */
    FAULT_HOST_COMMS_BAD_SUBTYPE = 0x10,

    /**
     * The PDU was too short to contain the message header.
     */
    FAULT_HOST_COMMS_BAD_PDU_LENGTH = 0x11,

    /**
     * The claimed length of a message received from the host was longer than
     * the PDU that it arrived in, or the claimed length of the message was
     * less than the length of the header.
     */
    FAULT_HOST_COMMS_BAD_LENGTH = 0x12,

    /**
     * A message received from the host had a channel value greater than the
     * max of 0x7f.
     */
    FAULT_HOST_COMMS_BAD_CHANNEL = 0x13,

    /**
     * A message received from the host was destined for a stream, which is
     * unsupported in this firmware build.
     */
    FAULT_HOST_COMMS_STREAMS_UNSUPPORTED = 0x14,

    /**
     * USB Type-C CC-line polling got reading that is outside configured
     * ranges.
     */
    FAULT_CHARGER_USB_TYPE_C_CC_READING_OUTSIDE_RANGE = 0x15,

    /**
     * P1 has requested a pointer to a file in the read only filesystem using
     * FileMap(), but the pointer is not within P1's Direct SQIF Flash Window.
     * The diatribe contains the P1 pointer that was detected as outside the
     * SQIF window.
     */
    FAULT_FILE_MAP_BAD_FLASH_LAYOUT = 0x16,

#ifdef FAULTIDS_EXTRA
    FAULTIDS_EXTRA
#endif



    /*
     * Do not use this special value - it indicates "not a fault".  (It
     * has been placed at the end of the list to prevent some compilers
     * complaining about a superfluous final comma within the enum
     * declaration.)
     */
    FAULT_NONE = 0x0000

} faultid;

#endif /* FAULTIDS_H */
