/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
#ifndef OP_MSG_HELPERS_H
#define OP_MSG_HELPERS_H


#include "op_msg_utilities.h"
#include "opmgr/opmgr_for_ops.h"

/****************************************************************************
Public Macro Definitions
*/

#define CPS_NUM_BLOCKS_MASK       (0x0FFF)

typedef enum
{
   CPS_SOURCE_HOST=0,
   CPS_SOURCE_OBPM_ENABLE=2,
   CPS_SOURCE_OBPM_DISABLE=3
}CPS_CONTROL_SOURCE;

/***************************** Public functions *************************** */

extern bool cpsInitParameters(CPS_PARAM_DEF *param_def_ptr,unsigned *default_ptr,unsigned *param_ptr,unsigned param_size);
extern void cpsSetDefaults(unsigned *in_ptr, unsigned *out_ptr,unsigned length);
extern unsigned cpsComputeGetParamsResponseSize(OPMSG_PARAM_BLOCK *in_ptr,unsigned length, unsigned num_params);
extern void cpsGetDefaults(OPMSG_PARAM_BLOCK *in_ptr,unsigned length, unsigned *params,unsigned *out_ptr);
extern void cpsGetParameters(OPMSG_PARAM_BLOCK *in_ptr,unsigned length, unsigned *params,unsigned *out_ptr);
extern unsigned cpsSetParameters(OPMSG_SET_PARAM_BLOCK *in_ptr,unsigned length, unsigned *params,unsigned num_params);

/**
 * \brief Helper function for building response message to CPS parameter requests.
 *
 * \param  param_def_ptr  Pointer to parameter definition
 * \param  obpm_msg       Pointer to the generic operator message structure (OP_MSG_REQ).
 * \param  resp_length    Pointer to response message length in words
 * \param  resp_data      Pointer to a pointer to response message structure, allocated in this function.
 *
 * \return TRUE if completed successfully, FALSE on fail.
 */
extern bool cpsGetParameterMsgHandler(CPS_PARAM_DEF *param_def_ptr,void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);


/**
 * \brief Helper function for building response message to CPS defaults requests.
 *
 * \param  param_def_ptr  Pointer to parameter definition
 * \param  obpm_msg       Pointer to the generic operator message structure (OP_MSG_REQ).
 * \param  resp_length    Pointer to response message length in words
 * \param  resp_data      Pointer to a pointer to response message structure, allocated in this function.
 *
 * \return TRUE if completed successfully, FALSE on fail.
 */
extern bool cpsGetDefaultsMsgHandler(CPS_PARAM_DEF *param_def_ptr,void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);


/**
 * \brief Helper function for setting parameters from CPS set param messages.
 *
 * \param  param_def_ptr  Pointer to parameter definition
 * \param  obpm_msg       Pointer to the generic operator message structure (OP_MSG_REQ).
 * \param  resp_length    Pointer to response message length in words
 * \param  resp_data      Pointer to a pointer to response message structure, allocated in this function.
 *
 * \return TRUE if completed successfully, FALSE on fail.
 */
extern bool cpsSetParameterMsgHandler(CPS_PARAM_DEF *param_def_ptr,void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);



/**
 * \brief Helper function for setting UCID from CPS set UCID messages.
 *
 * \param  param_def_ptr  Pointer to parameter definition
 * \param  obpm_msg       Pointer to the generic operator message structure (OP_MSG_REQ).
 * \param  resp_length    Pointer to response message length in words
 * \param  resp_data      Pointer to a pointer to response message structure, allocated in this function.
 *
 * \return TRUE if completed successfully, FALSE on fail.
 */
extern bool cpsSetUcidMsgHandler(CPS_PARAM_DEF *param_def_ptr,void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);



/**
 * \brief Helper function for setting UCID from CPS Get UCID messages.
 *
 * \param  param_def_ptr  Pointer to parameter definition
 * \param  cap_id         Capability ID
 * \param  obpm_msg       Pointer to the generic operator message structure (OP_MSG_REQ).
 * \param  resp_length    Pointer to response message length in words
 * \param  resp_data      Pointer to a pointer to response message structure, allocated in this function.
 *
 * \return TRUE if completed successfully, FALSE on fail.
 */
extern bool cpsGetUcidMsgHandler(CPS_PARAM_DEF *param_def_ptr,unsigned cap_id,void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);



/**
 * \brief Helper function for setting parameters from PsStore
 *
 * \param  param_def_ptr  Pointer to parameter definition
 * \param  length         Length of Data Block from Ps Store
 * \param  data           Pointer to Data Block from Ps Store
 * \param  status         Result of request 
 *
 * \return TRUE if completed successfully, FALSE on fail.
 */
extern bool cpsSetParameterFromPsStore(CPS_PARAM_DEF *param_def_ptr,uint16 length, unsigned* data, STATUS_KYMERA status);


/**
 * \brief Helper function for building response message to OBPM parameter or defaults requests.
 *
 * \param  obpm_msg       Pointer to the generic operator message structure (OP_MSG_REQ).
 * \param  resp_length    Pointer to response message length in words
 * \param  resp_data      Pointer to a pointer to response message structure, allocated in this function.
 * \param  size           size (in words) of the parameter pr defaults structure that is to be sent to OBPM
 *
 * \return TRUE if completed successfully, FALSE on fail.
 */
extern bool common_obpm_response_helper(OP_MSG_REQ *obpm_msg, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data, unsigned size);



/**
 * \brief Helper function for simple OBPM opmsg. It builds simple response,
 *        it does not perform any operation on the message data.
 *
 * \param  message_data   Pointer to the OBPM operator message.
 * \param  resp_length    Pointer to response message length in words
 * \param  resp_data      Pointer to a pointer to response message structure, allocated in this function.
 * \param  num_controls   Pointer to return number of controls in message
 *
 * \return TRUE if completed successfully, FALSE on fail.
 */
extern bool cps_control_setup(void* message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data,unsigned *num_controls);

/**
 * \brief Helper function for simple OBPM opmsg. It builds simple response,
 *        it does not perform any operation on the message data.
 *
 * \param  message_data   Pointer to the OBPM operator message.
 * \param  index          Index of control in message
 * \param  value          Pointer to return value
 * \param  source         Pointer to return source
 *
 * \return Control ID
 */
extern unsigned cps_control_get(void* message_data, unsigned index,unsigned *value,CPS_CONTROL_SOURCE *source);

/**
 * \brief Helper function for setcontrol OBPM opmsg. It builds simple response,
 *        it does not perform any operation on the message data.
 *
 * \param  resp_data    Pointer to a pointer to response message structure
 * \param  result       result of operation
 *
 * \return none
 */
extern void cps_response_set_result(OP_OPMSG_RSP_PAYLOAD **resp_data,unsigned result);


/**
 * \brief Helper function for statistics OBPM opmsg. It builds response,
 *        it does not perform any operation on the message data.
 *
 * \param  message_data   Pointer to the OBPM operator message.
 * \param  resp_length    Pointer to response message length in words
 * \param  resp_data      Pointer to a pointer to response message structure, allocated in this function.
 * \param  size           Size of statistics block.
 * \param  resptr         Pointer to a pointer to statistics block in response.
 *
 * \return TRUE if completed successfully, FALSE on fail.
 */

extern bool common_obpm_status_helper(void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data, unsigned size,unsigned **resptr);

#endif // OP_MSG_HELPERS_H
