/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_ble_phy_preferences.h      

DESCRIPTION
   
    Handler functions for PHY Preferences, this allows the configuration
    of 2Mbps LE.
    Builds requiring this should include CONFIG_PHY_REFERENCES in the
    makefile. e.g.
        CONFIG_FEATURES:=CONFIG_PHY_REFERENCES

NOTES

*/

/****************************************************************************
NAME    
    connectionBluestackHandlerDmPhyPreferences

DESCRIPTION
    Handler for PHY Preferences messages

RETURNS
    TRUE if message handled, otherwise FALSE
*/
bool connectionBluestackHandlerDmPhyPreferences(const DM_UPRIM_T *message);



