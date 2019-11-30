/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file cvc_processing_c.h
 * \ingroup cvc_processing
 *
 */

#ifndef _CVC_PROCESSING_C_H_
#define _CVC_PROCESSING_C_H_


/**
 *  Register CVC component with security library. If ommitted, component will not
 *  work properly. CVC-specific unregister function is not (yet) necessary, hence 
 *  need to use the unregister "base" function from security library.
 *
 * \param instance  Pointer to CVC instance data of some sort.
 * \param payload Message payload from the field that comes after opmsg ID onwards.
 * \return TRUE/FALSE for execution success/failure. In case of failed registration,
 *         the component will not be running properly.
 */
bool cvc_send_register_component(void* instance);


#endif      /* _CVC_PROCESSING_C_H_ */

