/*
Copyright (c) 2004 - 2016 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
*/
#ifndef SINK_BUTTONS_H
#define SINK_BUTTONS_H

#include "sink_buttonmanager.h"

/* The reserved logical inputs for the charger events. */
#define VREG_LOGICAL_INPUT_ID    (24)
#define CHG_LOGICAL_INPUT_ID     (25)

/* The mask values for the charger logical inputs. */
#define VREG_LOGICAL_INPUT_ID_MASK ((uint32)1 << VREG_LOGICAL_INPUT_ID)
#define CHG_LOGICAL_INPUT_ID_MASK ((uint32)1 << CHG_LOGICAL_INPUT_ID)

/* The current charger logical input mask values based on charger state. */
#define CHARGER_VREG_VALUE ( (uint32)((PsuGetVregEn()) ? VREG_LOGICAL_INPUT_ID_MASK:0 ) )
#define CHARGER_CONNECT_VALUE ( (uint32)( (powerManagerIsChargerConnected())  ? CHG_LOGICAL_INPUT_ID_MASK:0 ) )

#define DOUBLE_PRESS 2
#define TRIPLE_PRESS 3

/****************************************************************************
DESCRIPTION
 Initialises the button event 
*/
void ButtonsInit (  void ) ;

/****************************************************************************
DESCRIPTION
 Initialise the PIO hardware.

 Initialises the button hardware.mapping physical input numbers to produce
 masks and initialising all the PIOs.
*/
void ButtonsInitHardware ( void ) ;

/****************************************************************************
DESCRIPTION
 	Called after the configuration has been read and will trigger buttons events
    if a pio has been pressed or held whilst the configuration was still being loaded
    , i.e. the power on button press    
*/
void ButtonsCheckForChangeAfterInit(bool useButtonDebounceConfiguration);

/****************************************************************************
DESCRIPTION
 	this function remaps the capacitive sensor and pio bitmask into an input assignment
    pattern specified by pskey user 10, this allows buttons to be triggered from 
    pios of anywhere from 0 to 31 and capacitive sensor 0 to 5

*/ 
uint32 ButtonsTranslate(uint16 CapacitiveSensorState, pio_common_allbits *PioState);


/*!
    this function remaps an input assignment into a capacitive sensor or pio bitmask
    pattern specified by the configuration.

    A pointer to the mask variable to be returned is supplied as a parameter.
    
    @param[in,out] mask pointer to the pio_common_allbits to initialise and return
    @param[in] event_config The event configuration being translated
    @param include_capacitive_sensor Whether capacitive sensor inputs are included in the translation
*/
pio_common_allbits *ButtonsTranslateInput(pio_common_allbits *mask, 
                                          const event_config_type_t *event_config, bool include_capacitive_sensor);



#endif
