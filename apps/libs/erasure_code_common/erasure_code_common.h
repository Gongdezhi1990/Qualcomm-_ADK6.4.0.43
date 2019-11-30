/****************************************************************************
//  Copyright (c) 2017 Qualcomm Technologies International, Ltd.
 
****************************************************************************/

/*!
    @file erasure_code_common.h
    @brief Erasure code common definitions.

*/

#ifndef EC_COMMON_H
#define EC_COMMON_H

#ifdef KCC
#define EC_ASSERT(x) do { if (!(x)) kalimba_error(); } while (0)
#endif

/* Encode K packets into N packets.
   Receive K packets from N to reconstuct original K packets. */
#define EC_K 2
#define EC_N 5

/* There are N! / (K! * (N - K)!) combinations selecting K from N */
#define EC_2_5_PAIR_COMBINATIONS 10

#define EC_STREAM_ID_COUNT_TABLE_SIZE 3

/* (2,5) erasure codes */
#define EC_A              0
#define EC_B              1
#define EC_APB            2
#define EC_AP2B           3
#define EC_2APB           4
#define EC_NOT_ENCODED    15

/* Any stream IDs may be used by the application except this invalid stream id */
#define EC_STREAM_ID_INVALID 0

/* Tx buffer states */
#define EC_STATE_A              (EC_A)
#define EC_STATE_B              (EC_B)
#define EC_STATE_APB            (EC_APB)
#define EC_STATE_AP2B           (EC_AP2B)
#define EC_STATE_2APB           (EC_2APB)
#define EC_STATE_COMPLETE       5
#define EC_STATE_NOT_ENCODED    (EC_NOT_ENCODED)
#define EC_STATE_ALLOCED        16
#define EC_STATE_FREE           17
#define EC_STATE_WAIT           18

#define WIDTH_TO_MASK(b) ((1 << (b)) - 1)

/* Erasure code coding info  header */
#define EC_CI_CODING_ID_WIDTH 4
#define EC_CI_CODING_ID_MASK WIDTH_TO_MASK(EC_CI_CODING_ID_WIDTH)
#define EC_CI_CODING_ID_SHIFT 0

#define EC_CI_SEQUENCE_NUMBER_WIDTH 1
#define EC_CI_SEQUENCE_NUMBER_MASK WIDTH_TO_MASK(EC_CI_SEQUENCE_NUMBER_WIDTH)
#define EC_CI_SEQUENCE_NUMBER_SHIFT 4

#define EC_CI_PADDING_OCTETS_WIDTH 2
#define EC_CI_PADDING_OCTETS_MASK WIDTH_TO_MASK(EC_CI_PADDING_OCTETS_WIDTH)
#define EC_CI_PADDING_OCTETS_SHIFT 6

/*  The AFH channel map change pending bit is toggled each time there is a change 
    pending to the broadcaster's AFH channel map. When the receiver detects that
    this bit has changed, it will scan for a CSB sync train, which will define the
    new AFH channel map and the instant at which the channel map will be changed. */
#define EC_CI_AFH_CHANNEL_MAP_CHANGE_PENDING_WIDTH 1
#define EC_CI_AFH_CHANNEL_MAP_CHANGE_PENDING_MASK WIDTH_TO_MASK(EC_CI_AFH_CHANNEL_MAP_CHANGE_PENDING_WIDTH)
#define EC_CI_AFH_CHANNEL_MAP_CHANGE_PENDING_SHIFT 5

/* Macros for reading fields from received header */
#define EC_CI_CODING_ID(h)       (((h) >> EC_CI_CODING_ID_SHIFT) & EC_CI_CODING_ID_MASK)
#define EC_CI_SEQUENCE_NUMBER(h) (((h) >> EC_SEQUENCE_NUMBER_SHIFT) & EC_SEQUENCE_NUMBER_MASK)
#define EC_CI_PADDING_OCTETS(h)  (((h) >> EC_CI_PADDING_OCTETS_SHIFT) & EC_CI_PADDING_OCTETS_MASK)
#define EC_CI_AFH_CHANNEL_MAP_CHANGE_PENDING(h)  (((h) >> EC_CI_AFH_CHANNEL_MAP_CHANGE_PENDING_SHIFT) & EC_CI_AFH_CHANNEL_MAP_CHANGE_PENDING_MASK)

/* Size of erasure code header */
#define EC_HEADER_SIZE_OCTETS   (3)
#define EC_HEADER_SIZE_BITS     (BITS_PER_OCTET * EC_HEADER_SIZE_OCTETS)

#define EC_PACKET_MIN_PAYLOAD_OCTETS   (0)
#define EC_PACKET_MIN_PAYLOAD_BITS     (BITS_PER_OCTET * EC_MIN_PAYLOAD_BITS)

#define EC_PACKET_MAX_PAYLOAD_OCTETS   (BT_PACKET_2DH5_MAX_DATA_OCTETS - EC_HEADER_SIZE_OCTETS)
#define EC_PACKET_MAX_PAYLOAD_BITS     (BITS_PER_OCTET * EC_MAX_PAYLOAD_OCTETS)


#define EC_BUFFER_PAYLOAD_SIZE_BITS         ((EC_K * BT_PACKET_2DH5_MAX_DATA_OCTETS - EC_HEADER_SIZE_OCTETS) * BITS_PER_OCTET)
#define EC_BUFFER_PAYLOAD_SHORT_SIZE_BITS   ((BT_PACKET_2DH5_MAX_DATA_OCTETS - EC_HEADER_SIZE_OCTETS) * BITS_PER_OCTET)

#endif
