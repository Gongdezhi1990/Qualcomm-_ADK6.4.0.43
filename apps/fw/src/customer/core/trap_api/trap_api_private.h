/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
#ifndef TRAP_API_PRIVATE_H_
#define TRAP_API_PRIVATE_H_

#include "trap_api/trap_api.h"
#include "trap_api/api.h"
#include "sched_oxygen/sched_oxygen.h"
#include "ipc/ipc_msg_types.h"
#include "ipc/ipc.h"
#include "pio/pio.h"
#include "piodebounce/piodebounce.h"
#include "pmalloc/pmalloc.h"
#include "utils/utils_bit.h"
#include <message.h>
#include <sink_.h>
#include <source_.h>
#include "hydra_log/hydra_log.h"
#define IO_DEFS_MODULE_K32_CORE
#include "hal/hal_macros.h"

/**
 * Structure for storing a customer task handler
 */
typedef struct
{
    Task atask;
    taskid stask;
} TRAP_API_TASK;

/**
 * Enumerated type definition for the types of items that are logged.
 * These correspond to the log types in xIDE VM message logging.
 */
typedef enum TRAP_API_LOG_ACTION
{
    TRAP_API_LOG_SEND,
    TRAP_API_LOG_DELIVER,
    TRAP_API_LOG_FREE,
    TRAP_API_LOG_CANCEL
} TRAP_API_LOG_ACTION;

struct SINK_SOURCE_HDLR_ENTRY;

/**
 * Array of App-Task/sched-taskid pairings for registered message handlers
 *
 * \note The initialiser for this array must correspond to the enumeration above
 */
extern Task registered_hdlrs[];


typedef void (*Handler)(Task t, MessageId id, Message m);

/** Enumeration of possible bit-widths of the variable on which a message send
* is conditionalised.
*/
typedef enum
{
    CONDITION_WIDTH_UNUSED = 0,/*!< The condition is not used */
    CONDITION_WIDTH_16BIT = 16,/*!< 16 bits */
    CONDITION_WIDTH_32BIT = 32 /*!< 32 bits */
    /* Don't add wider types: 32 bits is the most the conditional send logic
     * is expecting */
} CONDITION_WIDTH;

/** Structure used for queue entries that can be either simple messages,
 * conditional or timed.
 */
typedef struct AppMessage
{
    struct AppMessage *next;
    Task task;
    uint16 id;
    void *message;
    const void *condition_addr;     /**< Pointer to condition value */
    CONDITION_WIDTH c_width;        /**< Width of condition value */
    uint32 due;                 /**< Millisecond time to deliver this message */
} AppMessage;


/**
 * Macro to determine if a buffer is a stream. Stubbed out for now.
 */
#define IS_STREAM(x) FALSE

/**
 * Look for a scheduler task corresponding to the supplied App-space task,
 * optionally creating a new one if none is found
 *
 * @param task The App-space task to get the taskid of
 * @param create Whether to create a new task if a matching one wasn't found
 * @return ID of the matching scheduler task if one was found or created, else
 * NO_PRIORITY_TASK_ID
 */
taskid trap_api_sched_get_taskid(Task task, bool create);

/**
 * Helper function to register a trap API Task with the Oxygen scheduler,
 * returning the previously-registered Task (NULL if none)
 * @param task Task to be registered
 * @param msg_type_id Index of the handler in the msg handler array
 * @return Task previously registered in the task record (NULL if none)
 */
Task trap_api_register_message_task(Task task, IPC_MSG_TYPE msg_type_id);


struct SINK_SOURCE_HDLR_ENTRY *trap_api_get_sink_source_hdlr_entry(Sink sink,
                                                                   Source source,
                                                                   bool create);

/**
 * Handler function that can be used to create a staging area for messages that
 * come up from the core.  They can be retrieved individually by calling
 * \c trap_api_test_get_next()
 */
void trap_api_test_task_handler(Task t, MessageId id, Message m);

struct TEST_MESSAGE_LIST;
/**
 *  Returns the head of the list of delivered messages
 * @return The first entry in the list.  Ownership is passed to the caller
 */
struct TEST_MESSAGE_LIST *trap_api_test_get_next(void);

/**
 * Resets the amount of space the test code reserves to make copies of message
 * bodies, since the handler can't know the size of message.  The default is 32.
 */
uint32 trap_api_test_reset_max_message_body_bytes(uint32 new_size);

/**
 * Do an arbitrary write to the given offset inside VM BUFFER window
 * to force the hardware to map in a VM page
 * @param offset Offset inside VM BUFFER window to write to.
 * Probably the start of an MMU page.
 */
void trap_api_test_map_page_at(uint32 offset);

/**
 * Helper function for catching a PIO event (registered to piodebounce). This
 * sends a message to the customer task. We require this to be run in the
 * background.
 */
void pio_handler(uint16 group, uint16 bank);

/**
 * Helper function for constructing and sending a MessagePioChanged message to
 * the customer task.
 * \param task Customer task.
 * \param bank PIO bank index.
 * \param state PIO levels.
 * \param time Timestamp in milliseconds.
 */
void trap_pio_send_msg(Task task, uint16 bank, pio_size_bits state, uint32 time);

/**
 * Log the state of a message now.
 * \param action: what is happening to the message.
 * \param msg: the AppMessage structure for the message.
 * \param now: the current time in milliseconds
 */
void trap_api_message_log_now(
    TRAP_API_LOG_ACTION action,
    AppMessage *        msg,
    uint32 nowtime);

/**
 * Log the state of a message
 * \param action: what is happening to the message.
 * \param msg: the AppMessage structure for the message.
 */
#define trap_api_message_log(action, msg) trap_api_message_log_now(action, msg, get_milli_time())


/*@}*/

#endif /* TRAP_API_PRIVATE_H_ */
/*@}*/
