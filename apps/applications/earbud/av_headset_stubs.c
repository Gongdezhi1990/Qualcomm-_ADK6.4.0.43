/*!
\copyright  Copyright (c) 2017 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_stubs.c
\brief      Interim file adding stubs for audio and power functions called within
            the gaia library which is used for DFU.

Without this a cascade of libraries is needed, some of which require
initialisation.
*/

#include "av_headset_battery.h"

#include <panic.h>
#include <power.h>
#include <audio.h>


static Task fake_audio_task = NULL;

/*! \brief Stub for the AudioLibraryInit() fucntion

    This is a stub function so the application doesn't need to include
    the audio library to use the GAIA library
*/
void AudioLibraryInit (  void )
{
}

/*! \brief Stub for the AudioBusyTask() fucntion

    This is a stub function so the application doesn't need to include
    the audio library to use the GAIA library

    \returns NULL. Normally this would be a pointer to a task that is
                   keeping audio busy.
*/
Task AudioBusyTask(void)
{
    return fake_audio_task;
}


/*! \brief Stub for the SetAudioBusy() fucntion

    This is a stub function so the application doesn't need to include
    the audio library to use the GAIA library

    \param[in] newtask  Not used. Normally, this would be a pointer to a task.
                        The value pointed to would become NULL when 
                        no longer busy.
*/
void SetAudioBusy(TaskData* newtask)
{
    UNUSED(newtask);
}

void SetAudioInUse(bool status)
{
    UNUSED(status);
}


/*! \brief Stub for the PowerBatteryGetVoltage() fucntion

    This is a stub function so the application doesn't need to include
    the power library to use the GAIA library

    \param[out] vbat    Pointer to location to take the battery reading

    \returns TRUE always. This is a stub function.
*/
bool PowerBatteryGetVoltage(voltage_reading* vbat)
{
    /* The GAIA library uses the voltage only so we don't need
       to set the level within the vbat structure to a correct level */
    vbat->voltage = appBatteryGetVoltage();
    vbat->level = 0; /* May need to update this in the future */

    return TRUE;
}

