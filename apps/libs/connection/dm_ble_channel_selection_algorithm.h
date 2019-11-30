/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_ble_channel_selection_algorithm.h      

DESCRIPTION
   
    This file is a stub for Channel Selection Algorithm.
    Builds requiring this should include CONFIG_CHANNEL_SELECTION in the
    makefile. e.g.
        CONFIG_FEATURES:=CONFIG_CHANNEL_SELECTION

NOTES

*/

/****************************************************************************
NAME    
    connectionBluestackHandlerDmChannelSelectionAlgorithm

DESCRIPTION
    Handler for Channel Selection Algorithm messages

RETURNS
    TRUE if message handled, otherwise false
*/
bool connectionBluestackHandlerDmChannelSelectionAlgorithm(const DM_UPRIM_T *message);


