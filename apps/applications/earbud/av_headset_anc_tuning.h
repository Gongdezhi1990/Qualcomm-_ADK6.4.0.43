/*!
\copyright  Copyright (c) 2018 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file
*/
#ifndef AV_HEADSET_ANC_TUNING_H
#define AV_HEADSET_ANC_TUNING_H


/*! Peer Sync module data. */
typedef struct
{
    TaskData task;                      /*!< Peer sync module Task. */
    unsigned enabled:1;
    unsigned active:1;
} ancTuningTaskData;

extern void appAncTuningEarlyInit(void);
extern void appAncTuningEnable(bool enable);
extern void appAncTuningEnumerated(void);
extern void appAncTuningSuspended(void);


#endif // AV_HEADSET_ANC_TUNING_H
