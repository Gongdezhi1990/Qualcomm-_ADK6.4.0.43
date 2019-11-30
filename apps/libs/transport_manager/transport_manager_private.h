/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    transport_manager_private.h
 
DESCRIPTION
    This module provides functionality to access all the private data of transport manager.
*/

#ifndef TRANSPORT_MANAGER_PRIVATE_H_
#define TRANSPORT_MANAGER_PRIVATE_H_

#include <transport_adaptation.h>
#include <panic.h>
#include <message.h>
#include "transport_manager.h"

#define MAKE_TRANS_MGR_MESSAGE(type, pointer_name) type##_T *pointer_name = PanicUnlessNew(type##_T)

/*Used for synchronization while registering transport*/
#define BLOCKED      1
#define UNBLOCKED    0

/* Invalid trans link id*/
#define INVALID_TRANS_LINK_ID     0
#define INVALID_TRANS_TYPE        0
#define NO_DATA_AVAILABLE         0

/* No of TRANSPORT_GATT_MESSAGE_MORE_DATA to be buffered*/
#define MAX_MESSAGE_MORE_DATA (5)

/* Default size of GATT data that could be sent to the remote device */
#define GATT_DEFAULT_SPACE_AVAILABLE (23)

#define isTransportTypeValid(type) ((type < transport_mgr_type_max) && \
                                        (type > transport_mgr_type_none) )
#define TRANSPORT_MGR_ASSERT_INIT {if((transportMgrGetPrivateData()) == NULL)Panic();}

#define TRANSPORT_MGR_ASSERT_NULL(PTR, MSG) {if(NULL == PTR) PRINT(MSG); PanicNull((void*)PTR);}

typedef struct __transport_mgr_link_data_t transport_mgr_link_data_t;

typedef struct __transport_mgr_data_t transport_mgr_data_t;

/*
    brief Internal message for transport manager
*/
typedef enum
{
    TRANSPORT_MGR_INTERNAL_REGISTER_REQ ,
    TRANSPORT_MGR_INTERNAL_DEREGISTER_REQ,
    TRANSPORT_MGR_INTERNAL_CONNECT_REQ,
    TRANSPORT_MGR_INTERNAL_DISCONNECT_REQ
}transport_manager_internal_t;


/*
    brief Identifier to find the block from transport manager
    database
*/
typedef enum
{
    transport_mgr_link_data,
    transport_mgr_data
}transport_mgr_data_id_t;


/*
    brief private block of transport manager dynamic database
 
    param trans_list : This is considered to be the head of the database list.
    param trans_reg_state: This is used to maintain the synchronization 
                                           while registering for transports.
    param transport_mgr_task: transport manager handler task data
*/
typedef struct 
{
    transport_mgr_link_data_t *trans_list;
    transport_mgr_data_t *trans_data_list;
    uint16 trans_reg_state;
    TaskData transport_mgr_task;
}transport_mgr_private_t;


/*
    brief link information stored of transport manager
 
    param trans_link_id : The Link id used for communication with
                                        underlying transport.
    param application_task: The task handler to which the transport manager 
                              would direct all the messages to.
    param sink: the respective transport sink
    param next: the pointer to the next instance in the list
*/
struct __transport_mgr_link_data_t
{
    transport_mgr_link_cfg_t *link_cfg;
    Task application_task;
    Sink sink;
    transport_mgr_link_data_t *next;
};


/*
    brief Data block of transport manager
 
    param trans_link_id : The Link id used for communication with
                                        underlying transport.
    param application_task: The task handler to which the transport manager 
                              would direct all the messages to.
    param sink: the respective transport sink
    param next: the pointer to the next instance in the list
*/
struct __transport_mgr_data_t
{
    /* Pointer to the next data block*/
    transport_mgr_data_t    *next;
    /* Handle being accessed. */
    uint16                  data_id;
    /* Length of the value. */
    uint16                  data_size;
    /*Value data. */
    uint8                   data[1];
};



/*
    brief Structure for the internal registration message

    param applicationTask: The task handler to which the transport manager 
                              would direct all the messages to.
    param pointer to transport_mgr_link_cfg_t which represents the link 
             configuration
*/

typedef struct
{
    Task application_task;
    transport_mgr_link_cfg_t link_cfg;
}TRANSPORT_MGR_INTERNAL_REGISTER_REQ_T;


/*
    brief Structure for the internal Deregister message

    param transport_type
    param trans_link_id : The Link id used for communication with
                                        underlying transport.
*/

typedef struct
{
    transport_mgr_type_t type;
    uint16 trans_link_id;
    Task application_task;
}TRANSPORT_MGR_INTERNAL_DEREGISTER_REQ_T;




/*
    brief Structure for the internal connect req

    param transport_type
    param trans_link_id : The Link id of local device used for communication 
    param remote_trans_link_id : The Link id of the remote device used for communication 
    param remote_addr: Bluetooth device address
*/

typedef struct
{
transport_mgr_type_t trans_type;
uint16 link_id;
uint8 remote_link_id;
tp_bdaddr *bd_addr;
}TRANSPORT_MGR_INTERNAL_CONNECT_REQ_T;



/*
    brief Structure for the internal dis connect req

    param transport_type
    param Sink
*/

typedef struct
{
transport_mgr_type_t trans_type;
Sink sink;
}TRANSPORT_MGR_INTERNAL_DISCONNECT_REQ_T;


/***************************************************************************
DESCRIPTION
    Retrieve the state of the lock which is maintained for synchronization
 
PARAMS
 
RETURNS
    pointer to the lock variable
*/
uint16 *isTransportMgrBusy(void);

/***************************************************************************
DESCRIPTION
    Set the lock which is maintained for synchronization
 
PARAMS
    
 
RETURNS
    
*/
void transportMgrSetBusy(uint16 state);


/***************************************************************************
DESCRIPTION
    Message Handler for Transport Manager task
 
PARAMS
    Task -  Task associated with the message
    MessageId -  Identifier
    Message	- Message itself
 
RETURNS
    
*/

void transportMgrMessageHandler(Task task, MessageId id, Message message);


/****************************************************************************
DESCRIPTION
    Add a datablock to the list

PARAMS
Void pointer to the instance that has to be added.
Identifier for the type of the instance that needs addition in the data base.

RETURNS

*/
void transportMgrAddRef(void *trans_data,transport_mgr_data_id_t data_block_id);

/****************************************************************************
DESCRIPTION
    Remove a link info datablock from the list

PARAMS
pointer to transport_mgr_link_data_t.

RETURNS
TRUE/FALSE
*/
bool transportMgrRemoveLinkRef(transport_mgr_link_data_t *trans_data);

/****************************************************************************
DESCRIPTION
    Remove a data info datablock from the list

PARAMS
pointer to transport_mgr_data_t.

RETURNS
TRUE/FALSE
*/
bool transportMgrRemoveDataRef(transport_mgr_data_t *trans_data);

/****************************************************************************
DESCRIPTION
    Update datablock with Sink

PARAMS
Link identifier
Sink
trans_type

RETURNS
TRUE/FALSE
*/
void transportMgrUpdateSink(uint16 trans_link_id, Sink sink_param,transport_mgr_type_t trans_type);

/****************************************************************************
DESCRIPTION
    Find a transport link identifier from the list with sink as ref. This utility function works only 
    when the transport_mgr_type_t is not  transport_mgr_type_gatt. 

PARAMS
Sink

RETURNS
transport identifier
and INVALID_TRANS_LINK_ID if the transport type is GATT.

*/
uint16 transportMgrFindTranslinkIdFromSink(Sink ref_sink);

/****************************************************************************
DESCRIPTION
    Find a data block from the list and send message to the respective task

PARAMS
void pointer to message
Message identifier
Link Identifier
transType

RETURNS
TRUE/FALSE

*/
void transportMgrListFindAndSendMessage(uint16 trans_link_id,MessageId id,void *message,transport_mgr_type_t trans_type);

/****************************************************************************
DESCRIPTION
    Find a datablock from the trans_list with link_id as ref

PARAMS
transType
transLinkId.

RETURNS
pointer to transport_manager_data_t
*/
transport_mgr_link_data_t *transportMgrFindDataWithTransInfo(uint16 link_id,transport_mgr_type_t trans_type);


/****************************************************************************
DESCRIPTION
    Find a datablock from the data_list with handle as ref

PARAMS
Data identifier

RETURNS
pointer to transport_manager_data_t
*/
transport_mgr_data_t *transportMgrFindDataFromDataList(uint16 data_id);



/****************************************************************************
DESCRIPTION
    Find the size of the list that is maintained in transport manager

PARAMS
Identifier for the type of the instance that needs addition in the data base.

RETURNS
size of the list
*/
uint16 transportMgrFindSizeOfDataList(void);

/****************************************************************************
DESCRIPTION
    Initialization of private block

PARAMS

RETURNS
void
*/
void transportMgrInitPrivateData(void);

/****************************************************************************
DESCRIPTION
    Retrieve the instance of private data block
PARAMS

RETURNS
pointer to transport_mgr_private_t 
*/
transport_mgr_private_t * transportMgrGetPrivateData(void);

/****************************************************************************
DESCRIPTION
    Retrieve the transport handler of private data block
PARAMS

RETURNS
Task for transport manager handler
*/
Task transportMgrGetTransportTask(void);

/****************************************************************************
DESCRIPTION
    Flushes the remaining data available at the source corresponding to
    the transport id.
PARAMS
type type of underlying transport
transid unique id corresponding to transport
RETURNS
void
*/
void transportMgrFlushInput(transport_mgr_type_t type, uint16 trans_id);

/****************************************************************************
DESCRIPTION
    Find a datablock from the list with Sink as ref

PARAMS
sink.

RETURNS
pointer to transport_manager_data_t
*/
transport_mgr_link_data_t *transportMgrFindTransDataFromSink(Sink ref_sink);
/****************************************************************************
DESCRIPTION
    Send message to all the task registered with transport type as GATT
PARAMS
id
message

RETURNS

*/

void transportMgrListBroadcastGattMessage(MessageId id,void *message);


/****************************************************************************
DESCRIPTION
    Writes data to the sink associated with a transport
PARAMS
type type of underlying transport
transid unique id corresponding to transport
data pointer to the data to be written
len No of bytes to write
RETURNS
TRUE,if successful/FALSE,if failure
*/
bool transportMgrWriteSinkData(transport_mgr_type_t type, uint16 trans_id,uint8 * data, uint16 len);


/****************************************************************************
DESCRIPTION
    Sends the data written to the sink to the transport
PARAMS
type type of underlying transport
transid unique id corresponding to transport
len No of bytes to send
RETURNS
TRUE,if successful/FALSE,if failure
*/
bool transportMgrSendSinkData(transport_mgr_type_t type, uint16 trans_id, uint16 len);

#ifdef HOSTED_TEST_ENVIRONMENT
/****************************************************************************
DESCRIPTION
    Deinitialise the transport manager
PARAMS

RETURNS
 
*/
void transportMgrDeinit();
#endif /*VA_TEST_BUILD*/

#endif
