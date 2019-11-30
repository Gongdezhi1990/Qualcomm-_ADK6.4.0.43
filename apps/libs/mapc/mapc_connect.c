/****************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.
 

FILE NAME
    mapc_connect.c

DESCRIPTION
    This file defines all APIs for Message Access Service Initialization
    and Connection management. The MAPC library allows multiple device 
    and profile sessions at a time, but it is the applications responsibility
    to maintain those connections and control the optimal number 
    of maximum connections. The recommended is not more than 7 MAS sessions
    at a time (Assuming that there are no other active profile connections). 
*/

#include "mapc_extern.h"

/**************************************************************************
 *NAME
 *  MapcMasSdpAttrSearchRequest
 *
 *DESCRIPTION 
 * The API to initiate an SDP Service Search Attribute request to a peer 
 * MSE device to fetch the service attributes of all
 * available MAS Service records at the MSE device. The application will 
 * receive CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM with the attributes. 
 * The application can use sdp_parse library to extract the values. 
 * Refer definition of CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T in connection.h
 * for more details. 
 *
 *PARAMETERS
 *  Refer mapc.h for details. 
 ************************************************************************/ 
void MapcMasSdpAttrSearchRequest( Task theAppTask, const bdaddr* addr )
{
    /* The application must provide valid values, 
       Otherwise Panic on debug */
    MAPC_ASSERT(theAppTask);
    MAPC_ASSERT(addr);

    mapcSdpSearch( theAppTask, addr );
}
  

/**************************************************************************
 *NAME
 *  MapcMasConnectRequest
 *
 *DESCRIPTION
 *  The API to establish one or more MAS Sessions. 
 *
 *PARAMETERS
 *  Refer mapc.h for details 
 **************************************************************************/
void MapcMasConnectRequest( Task theAppTask, const bdaddr* addr, uint8 masChannel )
{
    /* The application must provide valid values, Otherwise Panic on debug */
    MAPC_ASSERT(theAppTask);
    MAPC_ASSERT(addr);
    MAPC_ASSERT_RANGE(masChannel, 1, 0x20); /* valid channels are 1 - 31 */ 

    mapcPostConnectReq( theAppTask, addr, masChannel);
}

/**************************************************************************
 *NAME
 *  MapcMasDisconnectRequest
 *
 *DESCRIPTION
 *  The API to disconnect the MAS Session. 
 *
 *PARAMETERS
 *  Refer mapc.h for details 
 **************************************************************************/
void MapcMasDisconnectRequest( Mas session )
{
    MAPC_ASSERT( session );
    mapcPostTaskDelete( session );
}

/**************************************************************************
 *NAME
 *  MapcMasDisconnectResponse
 *
 *DESCRIPTION
 *  The API to Delete the Session. The application must call on receiving 
 *  MAPC_MAS_DISCONNECT_IND message
 *
 *PARAMETERS
 *  Refer mapc.h for details 
 **************************************************************************/
void MapcMasDisconnectResponse( Mas session )
{
    MAPC_ASSERT( session );
    mapcPostTaskDelete( session );

}

