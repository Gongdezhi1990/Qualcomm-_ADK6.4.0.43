/* Copyright (c) 2016 - 2019 Qualcomm Technologies International, Ltd. */
/*    */

FAULT_BLUESTACK_UNSUPPORTED_PROTOCOL = 0x2000,
FAULT_BLUESTACK_NONHCI_CONVERT_FAILURE = 0x2001,

/**
 * The in-flight DM primitive counter has underflowed.
 * This should never happen in a correctly implemented application.
 */
FAULT_BLUESTACK_IF_DM_COUNT_UNDERFLOW = 0x2002,

/**
 * The in-flight ATT primitive counter has underflowed.
 * This should never happen in a correctly implemented application.
 */
FAULT_BLUESTACK_IF_ATT_COUNT_UNDERFLOW = 0x2003,

/**
 * Bluestack received an unexpected command complete event.
 * This should never happen in a correctly implemented controller.
 */
FAULT_BLUESTACK_DM_UNEXPECTED_CMD_CMPLT_EVENT = 0x2004,
