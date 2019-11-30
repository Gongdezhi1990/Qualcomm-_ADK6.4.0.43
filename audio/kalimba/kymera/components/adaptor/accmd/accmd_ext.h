/****************************************************************************
 * Copyright (c) 2016 - 2019 Qualcomm Technologies International, Ltd 
****************************************************************************/
/**
 * \ingroup adaptor
 *
 * \file accmd_ext.h
 * \ingroup accmd
 * 	Non-Hydra ACCMD dependencies and public interface
 *
 */

 /****************************************************************************
 Include Files
 */

#ifndef _ACCMD_EXT_H_
#define _ACCMD_EXT_H_

#include "types.h"
#include "accmd.h"




/****************************************************************************
Public Macro Definitions
*/




/****************************************************************************
Public Type Declarations
*/


/** Tx function for a external connection */
typedef bool (*ACCMD_ExtConTxHandler)(ACCMD_PDU pdu, unsigned int length);


/**
 * \brief  Function to create a connection record for architectures that do not support services (like Napier)
 *
 * \param  tag           tag to identify the created connection
 * \param  tx_fn         funtcion pointer to external transmit function
 * \param  result		 pointer to an optional result to transport reason codes
 *
 * \return  TRUE for success. Returns FALSE if there is no memory left in the system.
 */
extern bool accmd_create_con_record_ext(uint16 tag, ACCMD_ExtConTxHandler tx_fn,  uint16 *result );



/**
 * \brief  Function to recive an accmd message for an external transport protocoll
 *
 * \param  tag          	tag to identify the connection
 * \param  pdu        	    pointer to payload
 * \param  pdu_len_words   	payload length in words
 *
 * \return  TRUE for success. Returns FALSE if there is no memory left in the system.
 */
bool accmd_ext_rx_handler( uint16 tag, const uint16 * pdu, uint16 pdu_len_words);


#endif /* _ACCMD_EXT_H_ */
