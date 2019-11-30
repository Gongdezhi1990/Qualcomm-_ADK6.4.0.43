/*******************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_dfu_ps.h

DESCRIPTION
    Interface File to DFU PS Configuration.
    
NOTES

*/

#ifndef SINK_DFU_PS_H
#define SINK_DFU_PS_H

#include <message.h>
#include <csrtypes.h>


/*******************************************************************************
NAME
    SinkDfuPsUpdate
    
DESCRIPTION
    Updates the PS keys (Audio PS key and Non config PS key) after Upgrade.
    This is an example code for customer reference.

NOTE
    SinkDfuPsUpdate() contains example implementation to modify audio PS keys and non-config PS keys.
    Customers are expected to change the implementation in SinkDfuPsUpdate() suiting their specific needs 
    and NOT use the function as is.
    Refer DFU Developer User Guide for details on this feature.

PARAMETERS
    None
    
RETURNS
    None
*/
#ifdef ENABLE_DFU_PS
void SinkDfuPsUpdate(void);
#else
#define SinkDfuPsUpdate() ((void)(0))
#endif


#endif /* SINK_DFU_PS_H */
