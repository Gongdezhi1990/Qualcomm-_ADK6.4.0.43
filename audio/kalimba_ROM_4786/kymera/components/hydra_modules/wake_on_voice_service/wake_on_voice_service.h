/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup wake_on_service Audio data service
 * \file wake_on_service.h
 *
 * This file contains public audio data
 * service types.
 */
#ifdef INSTALL_AOV
#ifndef WAKE_ON_SERVICE_H
#define WAKE_ON_SERVICE_H

#include "subserv/subserv.h"
#include "wake_on_voice_service_aux_prim.h"

typedef enum
{
    LPMODE_OFF,
    LPMODE_ON,
    LPMODE_UNSPECIFIED = 0x7fff
} WAKE_ON_VOICE_STATE;

/*
 * \brief Get the SSTAG id
 *
 * \return wake on service tag
 */
SUBSERV_TAG wake_on_voice_service_get_sstag(void);

/*
 * \brief sends an low power activate confirmation message to the apps
 *
 * \param status: The status of audio after the reception of
 * low power activate request
 *
 */
void wake_on_voice_service_aux_msg_send_lpmode_activate_cfm(WAKE_ON_VOICE_SERVICE_AUX_LPMODE_ACTIVATE_CFM_STATE status);

/*
 * \brief sends a message from an operator over the wake on service aux channel
 *
 */
void wake_on_voice_service_aux_msg_send_message_from_operator(const OP_UNSOLICITED_MSG * msg, unsigned msg_len);

#endif /* WAKE_ON_SERVICE_H */
#endif /*  INSTALL_AOV */
