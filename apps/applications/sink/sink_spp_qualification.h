/*
Copyright (c) 2018 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief       Interface to the PTS qualification for SPP Qualification tests. 
   These functions are written only for the qualification tests associated with SPP.
*/
#include "spp_common.h"
/****************************************************************************
NAME    
    sppUsrConnectRequest
    
DESCRIPTION
    Issue an SPP connect request for a particular device.
    
PARAMETERS
    void     
RETURNS
    void
*/
void sppUsrConnectRequest(void);
/****************************************************************************
NAME    
    sppUsrDisconnectRequest
    
DESCRIPTION
    Issue an RFcomm disconnecte request to the PTS.
    
PARAMETERS
    void     
RETURNS
    void
*/
void sppUsrDisconnectRequest(void);
/****************************************************************************
NAME    
    sppSetSinkData
    
DESCRIPTION
    Sets the sink data of the PTS recieved as a part of SPP connect confirmation .
    
PARAMETERS
    void     
RETURNS
    void
*/
void sppSetSinkData(Sink snk);
/****************************************************************************
NAME    
    sppGetSinkData
    
DESCRIPTION
    This function returns the sink value associated with the PTS.
    
PARAMETERS
    void     
RETURNS
    The sink value stored in the global run time data .
*/
Sink sppGetSinkData(void);
