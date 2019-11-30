/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup cvc_receive
 * \file  cvc_receive.h
 * \ingroup capabilities
 *
 * CVC receive operator public header file. <br>
 *
 */


#ifndef __CVC_RECEIVE_H__
#define __CVC_RECEIVE_H__

#include "opmgr/opmgr_for_ops.h"

/* Capability structure references */
#ifdef INSTALL_OPERATOR_CVC_RECEIVE
extern const CAPABILITY_DATA cvc_receive_nb_cap_data;
extern const CAPABILITY_DATA cvc_receive_wb_cap_data;
extern const CAPABILITY_DATA cvc_receive_fe_cap_data;
extern const CAPABILITY_DATA cvc_receive_uwb_cap_data;
extern const CAPABILITY_DATA cvc_receive_swb_cap_data;
extern const CAPABILITY_DATA cvc_receive_fb_cap_data;
#endif


#endif /* __CVC_RECEIVE_H__ */
