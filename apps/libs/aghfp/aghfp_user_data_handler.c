/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

*/

#include "aghfp.h"
#include "aghfp_private.h"
#include "aghfp_common.h"
#include "aghfp_send_data.h"
#include "aghfp_user_data_handler.h"


/*******************************************************************************/
void aghfpHandleUserDataReq(AGHFP *aghfp, const AGHFP_INTERNAL_USER_DATA_REQ_T *req)
{
    /* Attempt to send the data */
    aghfpSendRawData(aghfp, req->size_data, req->data);
}


/*******************************************************************************/
void aghfpHandleUserDataReqFail(AGHFP *aghfp, const AGHFP_INTERNAL_USER_DATA_REQ_T *req)
{
    /* Send an immediate fail we're in the wrong state to send the data */
    aghfpSendCommonCfmMessageToApp(AGHFP_USER_DATA_CFM, aghfp, aghfp_fail);
}
