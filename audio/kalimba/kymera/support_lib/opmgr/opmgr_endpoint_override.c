/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  opmgr_endpoint_override.c
 * \ingroup override
 *
 * Override module from Operator Manager.
 * This file contains the override specific operations for endpoints.
 *
 */
#include "stream/stream.h"
#include "stream/stream_for_override.h"
#include "opmgr/opmgr_endpoint_override.h"
#define EP_RATE_ADJUST_OP ((uint32)0x10018)
#define EP_HW_WARP_APPLY_MODE ((uint32)0x10019)
#define EP_CURRENT_HW_WARP ((uint32)0x1001A)

#ifdef INSTALL_DELEGATE_RATE_ADJUST_SUPPORT
/* get_override_ep_rate_adjust_op */
bool get_override_ep_rate_adjust_op(OVERRIDE_EP_HANDLE ep_hdl, uint32* value)
{
    ENDPOINT_GET_CONFIG_RESULT result;
    result.u.value = 0;
    bool success = stream_get_connected_to_endpoint_config((ENDPOINT*)ep_hdl, EP_RATE_ADJUST_OP, &result);
    *value = result.u.value;
    return success;
}
#endif /* INSTALL_DELEGATE_RATE_ADJUST_SUPPORT */
/* set_override_ep_set_hw_warp_apply_mode */
bool set_override_ep_set_hw_warp_apply_mode(OVERRIDE_EP_HANDLE ep_hdl, uint32 value)
{
    return stream_configure_connected_to_endpoint((ENDPOINT*)ep_hdl, EP_HW_WARP_APPLY_MODE, value);
}


/* get_override_ep_current_hw_warp */
bool get_override_ep_current_hw_warp(OVERRIDE_EP_HANDLE ep_hdl, uint32* value)
{
    ENDPOINT_GET_CONFIG_RESULT result;
    result.u.value = 0;
    bool success = stream_get_connected_to_endpoint_config((ENDPOINT*)ep_hdl, EP_CURRENT_HW_WARP, &result);
    *value = result.u.value;
    return success;
}
