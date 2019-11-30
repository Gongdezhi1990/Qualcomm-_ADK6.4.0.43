/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup adaptor Definition of the Connection ID and macros to access its
 * internal information.
 *
 * \file connection_id.h
 * \ingroup adaptor
 *
 */

#ifndef _CONNECTION_ID_H_
#define _CONNECTION_ID_H_

#include "types.h"

/* TODO: with multiple transports, later condition this & similar per platform. Ideally, though,
 * the client ID definitions for OBPM communication will be divorced from transport-related
 * information and this "legacy" can be changed.
 */

#include "spi/spi_for_adaptors.h"

#if defined(INSTALL_DUAL_CORE_SUPPORT) || defined(AUDIO_SECOND_CORE)
#include "adaptor/kip/kip_msg_adaptor.h"
#endif

#ifdef CHIP_BASE_BC7
#include "bc_comms_for_adaptors.h"
#endif


/****************************************************************************
Public Constant Declarations
*/

/* TODO: routing for any OBPM is for now as per existing (transport-related) client ID.
 * Once multiple transports come into being, rationalise this and divorce client ID from transport.
 */
#define RESPOND_TO_OBPM                     RESPOND_TO_SPI

/****************************************************************************
Public Macro Definitions
*/

/* Connection id contains sender and receiver 8-bit ids as follows:
 *
 *  | 15 - 8 |  7 - 0  |
 *  --------------------
 *  | RCV ID | SEND ID |
 *
 *  These values are swapped in response messages.
 *
 *  When a command is first received from the client, only the SEND ID carries
 *  a value. The RCV ID was kept unused until the advent of Multicore Kymera.
 *  Currently the RCV ID field is used different purposes: in particular for
 *  the aggregate id, in commands sent to multiple operators, and for the
 *  processor id, in commands sent to secondary cores.
 *  We should be careful not to use the connection id as synonymous of
 *  the SEND ID, but extract this information from it (maybe stronger typing
 *  would help).
 */

/* Constants for accessing sender and receiver fields from connection ID.
 * The following are meant for private use to define public macros.
 */

/** The number of bits to shift the sender/receiver id in the connection id field */
#define RECEIVER_SENDER_ID_SHIFT    8
/** Mask for extracting the receiver id */
#define RECEIVER_ID_MASK            0xFF00
/** Mask for extracting the sender id */
#define SENDER_ID_MASK              0x00FF

/** Mask off upper 8 bits of routing information */
#define ROUTING_VALUE_MASK          SENDER_ID_MASK

/* Public MACROs to access and use the information available in the
 * connection id.
 */

/**
 * \brief Used for constructing the connection id from routing info
 *
 * \param send_id Carries information about the sender.
 * \param recv_id Carries information about the receiver.
 *
 * \return formatted connection id.
 */
#define PACK_CON_ID(send_id, recv_id) \
    ((((recv_id) & ROUTING_VALUE_MASK) << RECEIVER_SENDER_ID_SHIFT) | \
    ((send_id) & ROUTING_VALUE_MASK))

/**
 * \brief Used for extracting the sender id from a connection id.
 *
 * \param con_id Connection id.
 *
 * \return Sender id: information about the sender.
 */
#define GET_CON_ID_SEND_ID(con_id) \
    ((con_id) & SENDER_ID_MASK)

/**
 * \brief Used for extracting the receiver id from a connection id.
 *
 * \param con_id Connection id.
 *
 * \return Receiver id: information about the receiver.
 */
#define GET_CON_ID_RECV_ID(con_id) \
    (((con_id) & RECEIVER_ID_MASK) >> RECEIVER_SENDER_ID_SHIFT)

/**
 * \brief Used for extracting the client information (sender id) from a
 *        NOT reversed connection id.
 *
 * \param con_id Not-Reversed Connection id.
 *
 * \return Client Info: information about the client (sender).
 */
#define GET_CON_ID_CLIENT_INFO(con_id) \
    ((con_id) & ROUTING_VALUE_MASK)

/**
 * \brief Reverse connection ID - Swap values of sender and reeiver.
 *
 * \param con_id Connection id.
 *
 * \return Reversed connection id.
 */
#define REVERSE_CONNECTION_ID(con_id) \
        ( (((con_id) & SENDER_ID_MASK) << RECEIVER_SENDER_ID_SHIFT | \
          (((con_id) & RECEIVER_ID_MASK) >> RECEIVER_SENDER_ID_SHIFT)) )


/* Value representing and invalid connection. N.B. This is a theoretically
 * valid value although impossible. */
#define INVALID_CON_ID  (SENDER_ID_MASK | RECEIVER_ID_MASK)

/*
 * Sender and receiver id contain processor and client id as follows:
 *
 *  |  7 - 5       |  4    |  3 - 0       |
 *  ---------------------------------------
 *  |  PROC ID     |Special| CLIENT INDEX |
 *  ---------------------------------------
 *                 |    CLIENT ID         |
 *  ---------------------------------------
 * Processor ID is top 3 bits of sneder or receiver id.
 * Client id is last 5 bits, where:
 * bit 4 distinguishes between an application client (i.e. ACCMD service instance,
 * value 0), and a special client (i.e. OBPM or internal operator client, value 1)
 * bottom 3 bits identify the client by a unique index.
 */

/* Constants for extracting processor and client IDs from sender or receiver IDs.
 * The following are meant for private use to define public macros.
 */
#define CONID_PROCESSOR_ID_SHIFT          5
#define CONID_PROCESSOR_ID_MASK           0x0007
#define CONID_CLIENT_ID_MASK              0x001F
#define CONID_PACKED_CLIENT_ID_MASK       0x1F1F
#define CONID_PACKED_RECV_PROC_ID_MASK    0xE000

/* Constants for extracting client index and the special client flag from
 * sender or receiver IDs.
 * The following are meant for private use to define public macros.
 */
#define CONID_SPECIAL_CLIENT_ID_MASK      0x0010
#define CONID_CLIENT_INDEX_MASK           0x000F


/* Public MACROs to access and use the information available in the
 * sender and receiver id.
 */

/**
 * \brief Used for building a sender OR receiver ID from client ID and processor ID.
 *
 * \param proc_id Processor id [IPC_PROCESSOR_ID_NUM]
 * \param client_id Client id, with the special flag information.
 *
 * \return Sender or Receiver id.
 */
#define PACK_SEND_RECV_ID(proc_id, client_id) \
        ((((proc_id) & CONID_PROCESSOR_ID_MASK) << CONID_PROCESSOR_ID_SHIFT) | \
          ((client_id) & CONID_CLIENT_ID_MASK))

/**
 * \brief Get client ID from sender OR receiver ID.
 *
 * \param id Sender or Receiver id.
 *
 * \return Client ID.
 */
#define GET_SEND_RECV_ID_CLIENT_ID(id) \
    ((id) & CONID_CLIENT_ID_MASK)

/**
 * \brief Get owner client ID from NOT reversed connection id.
 *
 * \param con_id Connection ID.
 *
 * \return Client ID.
 */
#define GET_CON_ID_OWNER_CLIENT_ID(id) \
    (GET_CON_ID_SEND_ID(con_id) & CONID_CLIENT_ID_MASK)

/**
 * \brief Extract the receiver client id from a connection id,
 *        without the information on the processor id.
 *
 * \param con_id Connection id.
 *
 * \return Receiver client ID.
 */
#define GET_EXT_CON_ID_RECV_ID(con_id) \
    (((con_id) >> RECEIVER_SENDER_ID_SHIFT) & CONID_CLIENT_ID_MASK)

/**
 * \brief Extract the packed information about the receiver and sender client id
 *        from a connection id, without the information on the processor id.
 *
 * \param conid Connection id.
 *
 * \return Connection ID without processor information.
 */
#define GET_UNPACKED_CONID(conid) \
    (((uint16)(conid)) & CONID_PACKED_CLIENT_ID_MASK)

/**
 * \brief Extract the packed information about the receiver and sender client id
 *        from a connection id and reverse it.
 *
 * \param conid Connection id.
 *
 * \return Reversed connection ID without processor information.
 */
#define UNPACK_REVERSE_CONID(conid) \
    REVERSE_CONNECTION_ID(GET_UNPACKED_CONID(conid))

#if defined(INSTALL_DUAL_CORE_SUPPORT) || defined(AUDIO_SECOND_CORE)

/**
 * \brief Extract the processor ID from the receiver information of a
 *        connection id.
 *
 * \param conid Connection id.
 *
 * \return Receiver processor ID [IPC_PROCESSOR_ID_NUM].
 */
#define GET_RECV_PROC_ID(conid) \
            (IPC_PROCESSOR_ID_NUM)((GET_CON_ID_RECV_ID(conid) >> \
                                    CONID_PROCESSOR_ID_SHIFT) & CONID_PROCESSOR_ID_MASK)
/**
 * \brief Extract the processor ID from the sender information of a
 *        connection id.
 *
 * \param conid Connection id.
 *
 * \return Sender processor ID [IPC_PROCESSOR_ID_NUM].
 */
#define GET_SEND_PROC_ID(conid) \
            (IPC_PROCESSOR_ID_NUM)((GET_CON_ID_SEND_ID(conid) >> \
                                    CONID_PROCESSOR_ID_SHIFT) & CONID_PROCESSOR_ID_MASK)

/**
 * \brief Add processor information to the connection id.
 *
 * \param conid Packed connection id.
 * \param procid Processor id.
 *
 * \return Packed connection ID with processor information.
 */
#define PACK_CONID_PROCID(conid, procid) \
            (uint16)(GET_UNPACKED_CONID(conid)| \
                    ((uint16)((procid) & CONID_PROCESSOR_ID_MASK) \
                    << (RECEIVER_SENDER_ID_SHIFT + CONID_PROCESSOR_ID_SHIFT)) | \
                    ((uint16)(hal_get_reg_processor_id()  & CONID_PROCESSOR_ID_MASK) \
                     <<  CONID_PROCESSOR_ID_SHIFT))

#else

#define GET_RECV_PROC_ID(conid) 0
#define GET_SEND_PROC_ID(conid) 0

#define PACK_CONID_PROCID(conid, procid) \
            (uint16)(GET_UNPACKED_CONID(conid)| \
                    ((uint16)((procid) & CONID_PROCESSOR_ID_MASK) \
                    << (RECEIVER_SENDER_ID_SHIFT + CONID_PROCESSOR_ID_SHIFT)))

#endif

/**
 * \brief Tells if the client is a special client (as opposed to an application
 *        client).
 *
 * \param id Sender or Receiver (client) id.
 *
 * \return TRUE if the client is a special client.
 */
#define GET_SEND_RECV_ID_IS_SPECIAL_CLIENT(id) \
            ( ((id) & CONID_SPECIAL_CLIENT_ID_MASK) \
            == CONID_SPECIAL_CLIENT_ID_MASK )

/**
 * \brief Get the client index from a Sender or Receiver (client) id.
 *
 * \param id Sender or Receiver (client) id.
 *
 * \return Client unique index.
 */
#define GET_SEND_RECV_ID_CLIENT_INDEX(id) \
            ( (id) & CONID_CLIENT_INDEX_MASK )


/**
 * \brief Makes a special client's id from a client index.
 *
 * \param index Client index.
 *
 * \return Client id.
 */
#define MAKE_SPECIAL_CLIENT_ID(index) \
            ( (index) | CONID_SPECIAL_CLIENT_ID_MASK )

/**
 * \brief Makes an application client's id from a client index.
 *
 * \param index Client index.
 *
 * \return Client id.
 */
#define MAKE_APPLICATION_CLIENT_ID(index) \
            ( (index) )

/****************************************************************************
Public Type Definitions
*/


/****************************************************************************
Public Function Definitions
*/

#endif /* _CONNECTION_ID_H_ */
