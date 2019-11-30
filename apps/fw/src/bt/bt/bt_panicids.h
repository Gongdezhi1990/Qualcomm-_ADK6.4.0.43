/* Copyright (c) 2016 - 2019 Qualcomm Technologies International, Ltd. */
/*    */

    /**
     * Bluestack ACL Manager client attempts to lock ACL > 15 times.
     */
    PANIC_DM_ACL_LOCKS_EXHAUSED = 0x2000,

    /**
     * Invalid ECC result
     */
    PANIC_ECC_RESULT_INVALID = 0x2001,

    /** */
    PANIC_FRAGMENTED_DEBUG_REQUEST = 0x2002,

    /** */
    PANIC_FSM_BAD_POINTER = 0x2003,

    /**
     * BlueStack has received an invalid primitive
     */
    PANIC_INVALID_BLUESTACK_PRIMITIVE = 0x2004,

    /**
     * Invalid tx interval received by sync manager from hci
     */
    PANIC_INVALID_ESCO_TX_INTERVAL = 0x2005,

    /**
     * An invalid ULP read buffer size response has been seen.
     */
    PANIC_INVALID_ULP_BUFFER_SIZE_RESPONSE = 0x2006,
    /**
     * L2CAP has lost track of HCI data credits.
     */
    PANIC_L2CAP_HCI_DATA_CREDITS_INCONSISTENT = 0x2007,
    /**
     * L2CAP has unexpectedly exhausted MBLK reference count.
     */
    PANIC_L2CAP_MBLK_REFCOUNT_EXHAUSTED = 0x2008,
    /**
     */
    PANIC_MBLK_CREATE_FAILURE = 0x2009,
    /**
     */
    PANIC_MBLK_DISCARD_TAIL_ERROR = 0x200a,
    /**
     * Attempt to set a destructor for a duplicate MBLK.
     */
    PANIC_MBLK_DUPLICATE_DESTRUCTOR = 0x200b,
    /**
     * The MBLK map/unmap can't handle chains - fatal error
     */
    PANIC_MBLK_MAP_ERROR = 0x200c,
    /**
     */
    PANIC_MBLK_MSGFRAG_COALESCE_FAILURE = 0x200d,
    /**
     * The useless catch-all.  Used to indicate an error where no other
     * appropriate panic code exists.
     */
    PANIC_MYSTERY = 0x200e,
    /**
     */
    PANIC_RFCOMM_INVALID_TIMER_CONTEXT = 0x200f,
    /**
     */
    PANIC_RFCOMM_INVALID_TIMER_TYPE = 0x2010,
    /**
     */
    PANIC_RFCOMM_L2CAP_REGISTER_FAILED = 0x2011,
    /**
     */
    PANIC_RFCOMM_STREAM_MISMATCH = 0x2012,
    /**
     */
    PANIC_RFCOMM_TIMER_ALREADY_STARTED = 0x2013,
    /**
     * Inconsistent L2CAP messages received by Security Manager.
     */
    PANIC_SM_L2CAP_HANDLER = 0x2014,

    PANIC_NONHCI_CONVERT_READ_BUFFER_ODD_BYTE = 0x2015,

    /** Couldn't create some part of the interface structure because of resource
     * exhaustion */
    PANIC_BLUESTACK_IF_RESOURCE_EXHAUSTION = 0x2016,

    /** bluestack_if had a problem with the message it was processing */
    PANIC_BLUESTACK_IF_BAD_MESSAGE = 0x2017,

    /**
     * The bluestack test interface to-host buffer was too full to write to
     */
    PANIC_BLUESTACK_IF_BUFFER_OVERFLOW = 0x2018,

    /**
     * hcishim couldn't write to a buffer because of a lack of space.  The
     * identity of the buffer is indicated by the diatribe
     */
    PANIC_HCISHIM_BUFFER_OVERFLOW = 0x2019,

    /**
     * hcishim apparently got bad information. The diatribe indicates what.
     */
    PANIC_HCISHIM_PARAMETER_ERROR = 0x201a,

    /**
     * hcishim ran out of resources for creating primitives or data structures
     * to send upstream
     */
    PANIC_HCISHIM_RESOURCE_EXHAUSTION = 0x201b,

    /**
     * Bluestack_if received a bg_int while the bt transport was either not
     * started or still starting.  The argument is the state it was reporting
     * itself as being in.
     */
    PANIC_BLUESTACK_IF_BAD_TRANSPORT_STATE = 0x201c,
    
    /**
     * hciconvert received an invalid data type.
     */
    PANIC_HCI_CONVERT_INVALID_DATA_TYPE = 0x201d,
    
    /**
     * Generic ATT panic code.
     */
    PANIC_ATT_INVALID_STATE = 0x201e,

    /**
     * An ATT structure has an invalid type 
     */
    PANIC_ATT_INVALID_TYPE = 0x201f,

    /**
     * BlueStack has received an invalid handle
     */
    PANIC_DM_INVALID_HANDLE = 0x2020,
