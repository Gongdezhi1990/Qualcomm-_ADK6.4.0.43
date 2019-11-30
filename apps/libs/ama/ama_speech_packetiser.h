/*****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    ama_speech_packetiser.h

DESCRIPTION
    Copies data from encoder output source into buffer and sends
*/

#ifndef _AMA_SPEECH_PACKETISER_H
#define _AMA_SPEECH_PACKETISER_H

#include <source.h>

/***************************************************************************
DESCRIPTION
     Copies data from source once a suitable threshold of available
           data is available and sends it to the transport.
PARAMS
    source The source to the the data from.
*/
bool amaSendMsbcSourceSpeechData(Source source);

#endif /* _AMA_SPEECH_PACKETISER_H */

