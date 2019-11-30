/*
Copyright (c) 2018 Qualcomm Technologies International, Ltd.
*/

/*!
\file    
\ingroup sink_app
\brief   
   Interface to the PTS qualification for SPP Qualification tests. 
   These functions are written only for the qualification tests associated with SPP.
*/


#include <source.h>
#include <sink.h>
#include <string.h>
#include <stdlib.h>
#include <vm.h>
#include <bdaddr.h>
#include "sink_main_task.h"
#include "sink_spp_qualification.h"
#include "sink_devicemanager.h"
#include "connection_no_ble.h"
#include "sppc.h"
#include "spp_common.h"

typedef struct
{
    Sink    spp_sink;
}spp_data;

static spp_data gsppData = {NULL};

#define SPP_DATA  gsppData

/****************************************************************************
NAME    
    sppUsrConnectRequest
    
DESCRIPTION
    Issue an SPP connect request for the 1st device assuming that PTS is the 
    only device connected with the sink.
    
PARAMETERS
    void     
RETURNS
    void
*/
void sppUsrConnectRequest(void)
{
    typed_bdaddr  ag_addr;
    sink_attributes attributes;
    /*Setting the first param of deviceManagerGetIndexedAttributes() to 0 assuming that 
    PTS is the only device connected to the IUT.*/
    if(deviceManagerGetIndexedAttributes(0, &attributes, &ag_addr))
    {
        SppConnectRequest(sinkGetMainTask(), &ag_addr.addr, 0, 0);
    }

}
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
void sppUsrDisconnectRequest(void)
{
     if(SPP_DATA.spp_sink != NULL)
     {
        /*RFCOMM disconnect request initiated here.*/
        ConnectionRfcommDisconnectRequest(sinkGetMainTask(),SPP_DATA.spp_sink);
        /*Reset the sink data*/
        SPP_DATA.spp_sink = NULL;
     }
}
/****************************************************************************
NAME    
    sppSetSinkData
    
DESCRIPTION
    Sets the sink data recieved as a part of SPP connect confirmation .
    
PARAMETERS
    void     
RETURNS
    void
*/
void sppSetSinkData(Sink sink)
{
    if(SPP_DATA.spp_sink == NULL)
    {
        /*sets the sink value received as a part of SPP cnfm into the global run data */
        SPP_DATA.spp_sink = sink;
    }
}
/****************************************************************************
NAME    
    sppGetSinkData
    
DESCRIPTION
    This function checks if if there is a valid sink associated with the PTS.
    
PARAMETERS
    void     
RETURNS
    the sink value of the PTS stored in the global data 
    else , returns NULL if there is no valid sink data.
*/
Sink sppGetSinkData(void)
{
	/*PTS connected, returns the sink value.*/
	return SPP_DATA.spp_sink;

}

