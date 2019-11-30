#ifndef SINK_GATT_SERVER_LOGGING_H
#define SINK_GATT_SERVER_LOGGING_H

#ifdef ACTIVITY_MONITORING
#ifndef GATT_ENABLED
#undef GATT_LOGGING_SERVER
#endif


#include <gatt_logging_server.h>

#include <csrtypes.h>
#include <message.h>

/* Logging notification update time as per spec the Logging report
    shall be notified for every one second.
*/
#define GATT_SERVER_LOGGING_UPDATE_TIME    1000

/* Max logging data that can be send in one ATT notification */
#define MAX_ATT_LOGGING_NOTI_LEN                              (23)

/* Maximum size of algorithm context that we report (otherwise it is truncated) */
#define MAX_ALGO_BYTES (50)

#define MAX_ARGS (3)
#define GATT_LOGGING_MAX_DEBUG_WORDS (40)
typedef struct __debug_logging_db_t
{
    int16 debug_log_data_size;
    uint32 debug_log_data_buffer[GATT_LOGGING_MAX_DEBUG_WORDS];
}debug_logging_db_t;

#ifdef GATT_LOGGING_SERVER
#define sinkGattLoggingServerGetSize() sizeof(GLOG_T)
#else
#define sinkGattLoggingServerGetSize() 0
#endif

/*******************************************************************************
NAME
    sinkGattLoggingServerInitialise

DESCRIPTION
    Initialize Logging server task.

PARAMETERS
    ptr - pointer to allocated memory to store server task's loggging report data.

RETURNS
    TRUE if the Logging server task was initialized, FALSE otherwise.
*/
#ifdef GATT_LOGGING_SERVER
bool sinkGattLoggingServerInitialise(uint16 **ptr);
#else
/* This is set to TRUE if the server is not defined, so as not to block the flow
 * of execution for initialiseOptionalServerTasks in sink_gatt_init.c. If it was
 * false, that function would return prematurely denoting an error in
 * initialization instead of merely skipping an undefined service. */
#define sinkGattLoggingServerInitialise(ptr) (TRUE)
#endif

/*******************************************************************************
NAME
    sinkBleNotifyLoggingReport

DESCRIPTION
    Handles notifying the logging report when
    BLE_INTERNAL_MESSAGE_LOGGING_READ_TIMER internal message is received.

PARAMETERS
    None

RETURNS
    None.
*/
#ifdef GATT_LOGGING_SERVER
void sinkBleNotifyLoggingReport(void);
#else
#define sinkBleNotifyLoggingReport() ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkBleNotifyLoggingDebugReport

DESCRIPTION
    Handles notifying the logging debug report when
    BLE_INTERNAL_MESSAGE_EVENT_DEBUG_DELAY internal message is received.

PARAMETERS
    None

RETURNS
    None.
*/
#ifdef GATT_LOGGING_SERVER
void sinkBleNotifyLoggingDebugReport(void);
#else
#define sinkBleNotifyLoggingDebugReport() ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkGattLoggingServerMsgHandler

DESCRIPTION
    Handle messages from the GATT Logging Service library

PARAMETERS
    task    The task the message is delivered
    id      The ID for the GATT message
    message The message payload

RETURNS
    void
*/
#ifdef GATT_LOGGING_SERVER
void sinkGattLoggingServerMsgHandler(Task task, MessageId id, Message message);
#else
#define sinkGattLoggingServerMsgHandler(task, id, message) ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkGattLoggingServiceEnabled

DESCRIPTION
    Returns whether logging service is enabled or not.
*/
#ifdef GATT_LOGGING_SERVER
#define sinkGattLoggingServiceEnabled(void) (TRUE)
#else
#define sinkGattLoggingServiceEnabled(void) (FALSE)
#endif

/*******************************************************************************
NAME
    get_debug_logging_db

DESCRIPTION
    Collects debug log data.
*/
void get_debug_logging_db(debug_logging_db_t *p_db);

/*******************************************************************************
NAME
    debug_logging_db_init

DESCRIPTION
    Initialises debug log.
*/

void debug_logging_db_init(void);

#endif  /* ACTIVITY_MONITORING */
#endif  /* SINK_GATT_SERVER_LOGGING_H */
