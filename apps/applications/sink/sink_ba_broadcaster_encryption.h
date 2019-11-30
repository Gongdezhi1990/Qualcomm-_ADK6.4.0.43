/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

 
FILE NAME
    sink_ba_broadcaster_encryption.h
 
DESCRIPTION
    Handles encryption keys.
*/

#ifndef SINK_BA_BROADCASTER_ENCRYPTION_H_
#define SINK_BA_BROADCASTER_ENCRYPTION_H_

/***************************************************************************
DESCRIPTION
    Reads from configuration or generates encryption keys.
    Then passes them to the GATT server and audio plugins.
*/
#ifdef ENABLE_BROADCAST_AUDIO
void setupEncryption(void);
#else
#define setupEncryption() ((void)0)
#endif

#endif /* SINK_BA_BROADCASTER_ENCRYPTION_H_ */
