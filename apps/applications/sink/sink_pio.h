/*
Copyright (c) 2004 - 2016 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief
    Part of the ledmanager Module responsible for managing the PIO outputs excluding LEDs

*/

#ifndef SINK_PIO_H
#define SINK_PIO_H

#include <csrtypes.h>
#include <pio_common.h>

#include "sink_events.h"

/* We need a fixed value to check if PIOs are mappped to the signals above or not before driving */
#define NO_PIO 0xFF

#define PIO_POWER_ON        sinkAudioGetPowerOnPio()
#define PIO_AMP_MUTE        sinkAudioGetAudioMutePio()
#define PIO_AUDIO_ACTIVE    sinkAudioGetAudioActivePio()

/****************************************************************************
NAME	
	PioInit

DESCRIPTION
       Function to Initialize PIO module
    
RETURNS
	TRUE: Success, FALSE: Failure
*/
bool PioInit(void);

/****************************************************************************
NAME	
	PioSetPowerPin

DESCRIPTION
    Set / Clear a power pin for the device
    
RETURNS
	void
*/
void PioSetPowerPin( bool enable ) ;


/****************************************************************************
NAME	
	PioSetPio

DESCRIPTION
    Fn to change a PIO
    Set drive TRUE to drive PIO, FALSE to pull PIO
    Set dir TRUE to set high, FALSE to set low
    
RETURNS
	void
*/
void PioSetPio(uint8 pio , pio_common_dir drive, bool dir);

/****************************************************************************
NAME	
	PioGetPio

DESCRIPTION
    Fn to read a PIO
    
RETURNS
	TRUE if set, FALSE if not
*/
bool PioGetPio(uint8 pio);

/****************************************************************************
NAME	
	PioIsInverted

DESCRIPTION
    Determine if a PIO has inverted logic, i.e. it is active low
    
RETURNS
	TRUE if PIO logic is inverted, FALSE otherwise
*/
bool PioIsInverted(uint8 pio);

/****************************************************************************
NAME	
	PioDrivePio

DESCRIPTION
	Drive a PIO to the passed state if configured. PIOs may be active high or
    active low depending on the pio_invert bitmap.
    
RETURNS
	void
    
*/
void PioDrivePio(uint8 pio, bool state);

/****************************************************************************
NAME	
	PioIsPioConfigured

DESCRIPTION
	Returns whether the passed pio is a valid pio.
    
RETURNS
	TRUE if valid, else FALSE
    
*/
#define PioIsPioConfigured(pio) ((pio == NO_PIO) ? FALSE : TRUE)

/****************************************************************************
NAME
        PioGetPioMap

DESCRIPTION
    Interface to get the PIO map, which will be placed into the map parameter
    a pointer to which is returned;

PARAMETERS
    map Pointer to a pio_common_allbits structures which is initialised and populated with the PioMap if set

RETURNS
    The map parameter passed. The map will be cleared if no map was found.
*/
pio_common_allbits *PioGetPioMap(pio_common_allbits *map);

/****************************************************************************
NAME
        PioGetPioMask

DESCRIPTION
    Interface to get the PIO mask based on the bank number

PARAMETERS
	bank_number which specify the pio pack range

RETURNS
    uint32 value based on the bank number
*/
uint32 PioGetPioMask(uint8 bank_number);

/****************************************************************************
NAME
    PioDriveUserPio

DESCRIPTION
    Sets the state of the user-configurable PIO

RETURNS
    void
*/
void PioDriveUserPio(bool state);

/****************************************************************************
NAME
    PioGetUserPioState

DESCRIPTION
    Gets the stored state of the PIO configured for user event control

RETURNS
    TRUE if PIO is active, FALSE otherwise
*/
bool PioGetUserPioState(void);

/****************************************************************************
NAME
    PioSetUserPioState

DESCRIPTION
    Sets the stored state of the PIO configured for user event control

RETURNS
    void
*/
void PioSetUserPioState(bool state);

/****************************************************************************
NAME
    PioHandleUserEvent

DESCRIPTION
    Handles a user event which changes the state of the configured PIO

RETURNS
    void
*/
void PioHandleUserEvent(sinkEvents_t event);

/****************************************************************************
NAME
    PioWriteSessionData

DESCRIPTION
    Writes module session data to configuration storage

RETURNS
    void
*/
void PioWriteSessionData(void);

#endif
