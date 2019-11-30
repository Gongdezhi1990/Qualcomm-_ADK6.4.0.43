/****************************************************************************
Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    hid_common.c
DESCRIPTION
    Various functions called from throughout the HID library, also includes
    state transition functions.
*/

#include <bdaddr.h>
#include <assert.h>
#include <source.h>
#include <stream.h>
#include <sink.h>

#include "hid.h"
#include "hid_common.h"
#include "hid_private.h"
#include "hid_profile_handler.h"
#include "hid_control_handler.h"

#include <app/bluestack/types.h>
#include <app/bluestack/bluetooth.h>
#include <app/bluestack/l2cap_prim.h>
#include <app/vm/vm_if.h>

/****************************************************************************
NAME 
    hidCreate
DESCRIPTION
    Creates a new HID instance.
RETURNS
    HID - Pointer to HID structure, or NULL if creation failed.
*/
HID *hidCreate(HID_LIB *hid_lib, const bdaddr *addr)
{
    HID *hid = (HID *) malloc(sizeof(HID));
    if (hid)
    {
        /* Initialise HID structure */
        hid->task.handler = hidProfileHandler;
        hid->app_task = hid_lib->app_task;
        hid->state = hidIdle;
        hid->disconnect_status = hid_disconnect_success;
        hid->hid_lib = hid_lib;
        hid->packet_pending = 0;
        hid->packet_data = NULL;

        /* Initialise control and interrupt channel states */
        hid->connection[HID_CON_CONTROL].state = hidConIdle;
        hid->connection[HID_CON_INTERRUPT].state = hidConIdle;
        
        /* Store address */
        hid->remote_addr = *addr;
        
        /* Add HID instance to head of list */
        hid->prev = NULL;
        hid->next = hid_lib->list;
        if (hid_lib->list)
            hid_lib->list->prev = hid;
        hid_lib->list = hid;
    }
    return hid;
}

/****************************************************************************
NAME 
    hidFindFromBddr
DESCRIPTION
    Searches for a HID instance with matching address.
RETURNS
    HID - Pointer to HID instance, or NULL if no match.
*/
HID *hidFindFromBddr(HID_LIB *hid_lib, const bdaddr *addr)
{
    HID *hid = hid_lib->list;
    while (hid)
    {
        if (BdaddrIsSame(&hid->remote_addr, addr))
            break;
        hid = hid->next;        
    }
    
    return hid;
}

/****************************************************************************
NAME 
    hidDestroy
DESCRIPTION
    Destroys specified HID instance, deletes any associated memory.
RETURNS
    void
*/
void hidDestroy(HID *hid)
{
    HID_PRINT(("hidDestroy\n"));

    if (hid->prev)
    {
        hid->prev->next = hid->next;
        if (hid->next)
            hid->next->prev = hid->prev;
    }
    else
    {
        hid->hid_lib->list = hid->next;
        if (hid->next)
            hid->next->prev = NULL;
    }
    MessageFlushTask(&hid->task);
    free(hid);
}

/****************************************************************************
NAME 
    hidConnIsConnected
DESCRIPTION
    Checks if specified connection is connected.
RETURNS
    bool - TRUE if connection is connected, FALSE otherwise.
*/
bool hidConnIsConnected(HID *hid, int psm)
{
    if (psm == HID_PSM_CONTROL)
        return hid->connection[HID_CON_CONTROL].state == hidConConnected;
    else if (psm == HID_PSM_INTERRUPT)
        return hid->connection[HID_CON_INTERRUPT].state == hidConConnected;
    else
        Panic();

    return FALSE;
}

/****************************************************************************
NAME 
    hidConnIsDisconnected
DESCRIPTION
    Check if specified connection is disconnected.
RETURNS
    bool - TRUE if connection is disconnected or idle, FALSE otherwise.
*/
bool hidConnIsDisconnected(HID *hid, int psm)
{
    if (psm == HID_PSM_CONTROL)
        return (hid->connection[HID_CON_CONTROL].state == hidConDisconnected) || (hid->connection[HID_CON_CONTROL].state == hidConIdle);
    else if (psm == HID_PSM_INTERRUPT)
        return (hid->connection[HID_CON_INTERRUPT].state == hidConDisconnected) || (hid->connection[HID_CON_INTERRUPT].state == hidConIdle);
    else
        Panic();

    return FALSE;
}

/****************************************************************************
NAME 
    hidConnConnecting
DESCRIPTION
    Called to put the specified connection into the connecting state.
RETURNS
    void
*/
void hidConnConnecting(HID *hid, int psm, uint16 con_id, uint8 identifier)
{
    hidConnection *conn = NULL;

    /* Find connection structure */
    if (psm == HID_PSM_CONTROL)
        conn = &hid->connection[HID_CON_CONTROL];
    else if (psm == HID_PSM_INTERRUPT)
        conn = &hid->connection[HID_CON_INTERRUPT];
    else
        Panic();

    /* Update fields */
    conn->state = hidConConnecting;
    conn->con.id = con_id;
    conn->identifer = identifier;
}

/****************************************************************************
NAME 
    hidConnConnected
DESCRIPTION
    Called to put the specified connection into the connected state.
RETURNS
    void
*/
void hidConnConnected(HID *hid, int psm, Sink sink, uint16 mtu)
{
    hidConnection *conn = NULL;

    /* Configure sink not to send MESSAGE_MORE_SPACE */
    SinkConfigure(sink, VM_SINK_MESSAGES, VM_MESSAGES_NONE);

    /* Configure source to only send one outstanding MESSAGE_MORE_DATA */
    SourceConfigure(StreamSourceFromSink(sink), VM_SOURCE_MESSAGES, VM_MESSAGES_SOME);

    /* Find connection structure */
    if (psm == HID_PSM_CONTROL)
        conn = &hid->connection[HID_CON_CONTROL];
    else if (psm == HID_PSM_INTERRUPT)
        conn = &hid->connection[HID_CON_INTERRUPT];
    else
        Panic();

    /* Update fields */
    conn->state = hidConConnected;
    conn->con.sink = sink;
    conn->remote_mtu = mtu;
}

/****************************************************************************
NAME 
    hidConnDisconnected
DESCRIPTION
    Called to put the specified connection into the disconnected state.
RETURNS
    void
*/
void hidConnDisconnected(HID *hid, Sink sink)
{
    hidConnection *conn = NULL;

    /* Find connection structure */
    if (((hid->connection[HID_CON_CONTROL].state == hidConConnected) || (hid->connection[HID_CON_CONTROL].state == hidConDisconnecting)) &&
        (sink == hid->connection[HID_CON_CONTROL].con.sink))
            conn = &hid->connection[HID_CON_CONTROL];
    else if (((hid->connection[HID_CON_INTERRUPT].state == hidConConnected) || (hid->connection[HID_CON_INTERRUPT].state == hidConDisconnecting)) &&
        (sink == hid->connection[HID_CON_INTERRUPT].con.sink))
            conn = &hid->connection[HID_CON_INTERRUPT];
    else
        Panic();
        
    /* Dispose of any messages left in the l2cap sources to ensure the buffers are destroyed */
    StreamDisconnect(StreamSourceFromSink(sink), 0);
    StreamConnectDispose(StreamSourceFromSink(sink));		

    /* Move connection into disconnected state */
    conn->state = hidConDisconnected;
}

/****************************************************************************
NAME 
    hidConnDisconnectedPsm
DESCRIPTION
    Called to put the specified connection into the disconnected state.
RETURNS
    void
*/
void hidConnDisconnectedPsm(HID *hid, int psm)
{
    hidConnection *conn = NULL;

    /* Find connection structure */
    if ((psm == HID_PSM_CONTROL) && (hid->connection[HID_CON_CONTROL].state == hidConConnecting))
        conn = &hid->connection[HID_CON_CONTROL];
    else if ((psm == HID_PSM_INTERRUPT) && (hid->connection[HID_CON_INTERRUPT].state == hidConConnecting))
        conn = &hid->connection[HID_CON_INTERRUPT];
    else
        Panic();

    /* Move connection into disconnected state */
    conn->state = hidConDisconnected;
}

/****************************************************************************
NAME 
    hidConnDisconnect
DESCRIPTION
    Called to disconnect the specified connection.
RETURNS
    void
*/
void hidConnDisconnect(HID *hid, int psm)
{
    hidConnection *conn = NULL;

    /* Find connection structure */
    if ((psm == HID_PSM_CONTROL) && (hid->connection[HID_CON_CONTROL].state == hidConConnected))
        conn = &hid->connection[HID_CON_CONTROL];
    else if ((psm == HID_PSM_INTERRUPT) && (hid->connection[HID_CON_INTERRUPT].state == hidConConnected))
        conn = &hid->connection[HID_CON_INTERRUPT];
    else
        Panic();

    /* Move connection into disconnecting state */
    conn->state = hidConDisconnecting;

    /* Disconnect connection */
    ConnectionL2capDisconnectRequest(&hid->task, conn->con.sink);
}

/****************************************************************************
NAME 
    hidConnectingLocalEnter
DESCRIPTION
    Called whenever the HID instance enters the local connecting state.
    Initiates the control channel L2CAP connection.
RETURNS
    void
*/
static void hidConnectingLocalEnter(HID *hid)
{
    HID_PRINT(("hidConnectingLocalEnter\n"));

    /* Start outgoing control connection */
    hidConnConnecting(hid, HID_PSM_CONTROL, 0, 0);

    /* Connect using default L2CAP configuration */
    ConnectionL2capConnectRequest(&hid->task, &hid->remote_addr, HID_PSM_CONTROL, HID_PSM_CONTROL, 0, NULL);

    /* Set default connect status */
    hid->connect_status = hid_connect_success;
}

/****************************************************************************
NAME 
    hidConnectingRemoteEnter
DESCRIPTION
    Called whenever the HID instance enters the remote connecting state.
    Sends a HID_CONNECT_IND to the application.
RETURNS
    void
*/
static void hidConnectingRemoteEnter(HID *hid)
{
    MAKE_HID_MESSAGE(HID_CONNECT_IND);

    HID_PRINT(("hidConnectingRemoteEnter\n"));

    /* Send HID_CONNECT_IND to application */
    message->bd_addr = hid->remote_addr;
    message->hid = hid;
    MessageSend(hid->app_task, HID_CONNECT_IND, message);

    /* Set default connect status */
    hid->connect_status = hid_connect_success;
}

/****************************************************************************
NAME 
    hidConnectingExit
DESCRIPTION
    Called whenever the HID instance exits either of the connecting states.
    Sends a HID_CONNECT_CFM to the application.
RETURNS
    void
*/
static void hidConnectingExit(HID *hid)
{            
    HID_PRINT(("hidConnectingExit\n"));

    /* Cancel pending connection timer */
    MessageCancelAll(&hid->task, HID_INTERNAL_CONNECT_TIMEOUT_IND);

    /* Send HID_CONNECT_CFM to application */
    if (hid->app_task)
    {
        MAKE_HID_MESSAGE(HID_CONNECT_CFM);
        message->hid = hid;
        message->interrupt_sink = hid->connection[HID_CON_INTERRUPT].con.sink;
        message->status = hid->connect_status;
        MessageSend(hid->app_task, HID_CONNECT_CFM, message);
    }
}

/****************************************************************************
NAME 
    hidConnectedEnter
DESCRIPTION
    Called whenever the HID instance enters the connected state.
RETURNS
    void
*/
static void hidConnectedEnter(HID *hid)
{
    Source source = StreamSourceFromSink(hid->connection[HID_CON_CONTROL].con.sink);

    HID_PRINT(("hidConnectedEnter\n"));

    /* Send a message to ourselves to handle any waiting control packets*/
    if (SourceSize(source) > 0)
        MessageSend(&hid->task, HID_INTERNAL_CHECK_CONTROL_REQ, 0);
}

/****************************************************************************
NAME 
    hidDisconnectingLocalEnter
DESCRIPTION
    Called whenever the HID instance enters the local disconnecting state.
    Initiates disconnecting both control and interrupt L2CAP connections.
RETURNS
    void
*/
static void hidDisconnectingLocalEnter(HID *hid)
{
    HID_PRINT(("hidDisconnectingLocalEnter\n"));

    /* Disconnect both channels */
    if (hidConnIsConnected(hid, HID_PSM_INTERRUPT))
        hidConnDisconnect(hid, HID_PSM_INTERRUPT);
    if (hidConnIsConnected(hid, HID_PSM_CONTROL))
        hidConnDisconnect(hid, HID_PSM_CONTROL);
}

/****************************************************************************
NAME 
    hidDisconnectingLocalExit
DESCRIPTION
    Called whenever the HID instance exits the local disconnecting state.
    Sends a HID_DISCONNECT_CFM message to the application.
RETURNS
    void
*/
static void hidDisconnectingLocalExit(HID *hid)
{
    HID_PRINT(("hidDisconnectingLocalExit\n"));

    /* Send HID_DISCONNECT_IND if disconnecting due to VC_UNPLUG */
    if (hid->disconnect_status == hid_disconnect_virtual_unplug)
    {
        MAKE_HID_MESSAGE(HID_DISCONNECT_IND);

        /* Send HID_DISCONNECT_IND to application */
        message->hid = hid;
        message->status = hid->disconnect_status;
        MessageSend(hid->app_task, HID_DISCONNECT_IND, message);
    }
    else
    {
        MAKE_HID_MESSAGE(HID_DISCONNECT_CFM);

        /* Send HID_DISCONNECT_IND to application */
        message->hid = hid;
        message->status = hid->disconnect_status;
        MessageSend(hid->app_task, HID_DISCONNECT_CFM, message);
    }
}

/****************************************************************************
NAME 
    hidDisconnectingRemoteEnter
DESCRIPTION
    Called whenever the HID instance enters the remote disconnecting state.
    Starts the disconnect timer.
RETURNS
    void
*/
static void hidDisconnectingRemoteEnter(HID *hid)
{
    HID_PRINT(("hidDisconnectingRemoteEnter\n"));

    /* Start disconnect timer */
    MessageSendLater(&hid->task, HID_INTERNAL_DISCONNECT_TIMEOUT_IND, 0, D_SEC(HID_DISCONNECT_TIMEOUT));
}

/****************************************************************************
NAME 
    hidDisconnectingRemoteExit
DESCRIPTION
    Called whenever the HID instance exits the remote disconnecting state.
    Sends a HID_DISCONNECT_IND message to the applications and stops the
    disconnect timer.
RETURNS
    void
*/
static void hidDisconnectingRemoteExit(HID *hid)
{
    MAKE_HID_MESSAGE(HID_DISCONNECT_IND);
    HID_PRINT(("hidDisconnectingRemoteExit\n"));

    /* Send HID_DISCONNECT_IND to application */
    message->hid = hid;
    message->status = hid->disconnect_status;
    MessageSend(hid->app_task, HID_DISCONNECT_IND, message);

    /* Stop disconnect timer */
    MessageCancelAll(&hid->task, HID_INTERNAL_DISCONNECT_TIMEOUT_IND);
}

/****************************************************************************
NAME 
    hidDisconnectingFailedEnter
DESCRIPTION
    Called whenever the HID instance enters the disconnecting (connecting failed) state.
    Initiates disconnecting both control and interrupt L2CAP connections.
RETURNS
    void
*/
static void hidDisconnectingFailedEnter(HID *hid)
{
    HID_PRINT(("hidDisconnectingFailedEnter\n"));

    /* Disconnect both channels */
    if (hidConnIsConnected(hid, HID_PSM_INTERRUPT))
    {
        HID_PRINT(("Disconnecting interrupt channel\n"));
        hidConnDisconnect(hid, HID_PSM_INTERRUPT);
    }
    if (hidConnIsConnected(hid, HID_PSM_CONTROL))
    {
        HID_PRINT(("Disconnecting control channel\n"));
        hidConnDisconnect(hid, HID_PSM_CONTROL);
    }
}

/****************************************************************************
NAME 
    hidDisconnectedEnter
DESCRIPTION
    Called whenever the HID instance enters the disconnected state.
    Destroys the HID instance.
RETURNS
    void
*/
static void hidDisconnectedEnter(HID *hid)
{
    HID_PRINT(("hidDisconnected\n"));

    /* Our connections must be disconnected */
    HID_ASSERT(hidConnIsDisconnected(hid, HID_PSM_CONTROL));
    HID_ASSERT(hidConnIsDisconnected(hid, HID_PSM_INTERRUPT));

    /* We can now delete ourselves */
    hidDestroy(hid);
}

/****************************************************************************
NAME 
    hidSetState
DESCRIPTION
    The function is called to change the state of the HID instance.
    It automatically handles calling the appropriate state entry and exit
    function.
RETURNS
    void
*/
void hidSetState(HID *hid, hidState state)
{
    if (state != hid->state)
    {
        /* Handle exiting old state */
        switch (hid->state)
        {
            case hidConnectingLocal:
            case hidConnectingRemote:
                hidConnectingExit(hid);
                break;				
            case hidDisconnectingLocal:
                hidDisconnectingLocalExit(hid);
                break;
            case hidDisconnectingRemote:
                hidDisconnectingRemoteExit(hid);
                break;
            default:
                break;
        }

        /* Store new state */
        hid->state = state;

        /* Handle entering new state */
        switch (hid->state)
        {
            case hidConnectingLocal:
                hidConnectingLocalEnter(hid);
                break;
            case hidConnectingRemote:
                hidConnectingRemoteEnter(hid);
                break;
            case hidConnected:
                hidConnectedEnter(hid);
                break;
            case hidDisconnectingLocal:
                hidDisconnectingLocalEnter(hid);
                break;
            case hidDisconnectingRemote:
                hidDisconnectingRemoteEnter(hid);
                break;
            case hidDisconnectingFailed:
                hidDisconnectingFailedEnter(hid);
                break;
            case hidDisconnected:
                hidDisconnectedEnter(hid);
                break;
            default:
                break;
        }
    }
}

/****************************************************************************
NAME 
    hidGetState
DESCRIPTION
    The function returns the current state of the HID instance.
RETURNS
    hidState - Current state of HID instance.
*/
hidState hidGetState(HID *hid)
{
    return hid->state;
}
