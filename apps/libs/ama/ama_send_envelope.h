/*****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    ama_send_envelope.h

DESCRIPTION
    Sends control envelope to transport
*/
#ifndef _AMA_SEND_ENVELOPE_H
#define _AMA_SEND_ENVELOPE_H

#include "accessories.pb-c.h"

/***************************************************************************
DESCRIPTION
    Send control envelope to transport
 
PARAMS
    control_envelope_out Control envelope to be sent
*/
void amaSendEnvelope(ControlEnvelope *control_envelope_out);

#endif /* _AMA_SEND_ENVELOPE_H */
