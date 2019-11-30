#ifndef __MESSAGE_H__
#define __MESSAGE_H__
#include <sink_.h>
#include <message_.h>
#include <app/message/system_message.h>
#include <app/status/status_if.h>
#include <operator_.h>

/*!
No delay, perform immediately.
*/
#define D_IMMEDIATE ((Delay) -1)
/*!
Number of seconds to delay for.
*/
#define D_SEC(s)    ((Delay) ((s) * (Delay) 1000))
/*!
Number of minutes to delay for.
*/
#define D_MIN(m)    ((Delay) ((m) * (Delay) 1000 * (Delay) 60))
/*!
Number of hours to delay for.
*/
#define D_HOUR(h)   ((Delay) ((h) * (Delay) 1000 * (Delay) 60) * (Delay) 60)
/*! file  @brief Control message passing
** 
** 
@par Tasks and Message Queues
** 
The messaging functions provide a mechanism for asynchronously posting 
messages between tasks. Messages are posted to MessageQueues which are owned by Tasks.
A Task which owns a non-empty MessageQueue will be run by the scheduler.
** 
@par Creating and Destroying Messages
** 
Messages are dynamically allocated which means that they come out of a very limited
dynamic-block budget. It is therefore important to ensure that messages are consumed as
soon as possible after being produced. Put another way, messages are intended to be a 
signalling mechanism rather than a data-buffering mechanism.
** 
All messages have an identifier property, and some may also contain a payload.
*/
/*!
@brief Allocate a message, suitable for sending.
This macro allocates space for a message and initialises a variable
with a pointer to the space.
The pointer should be passed to one of the MessageSend functions, or
passed to free.
@param NAME the name of the variable to be declared and initialised with the pointer
@param TYPE the type to use to determine how much space to allocate
*/
#define MESSAGE_MAKE(NAME,TYPE) \
TYPE * const NAME = PanicUnlessNew(TYPE)

#if TRAPSET_STATUS

/**
 *  \brief Register a task to receive a message when status values change.
 *   
 *   The registered task
 *   will receive just one message and must register itself again if it
 *   wishes to receive another.
 *   A message will be sent when any of the listed status fields may have
 *   changed.
 *  \param task The task to receive the \#MESSAGE_STATUS_CHANGED message. 
 *  \param count How many fields to monitor 
 *  \param fields The fields to monitor
 *  \return The previously registered task, if any.
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_status
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
Task MessageStatusTask(Task task, uint16 count, const status_field * fields);
#endif /* TRAPSET_STATUS */
#if TRAPSET_CORE

/**
 *  \brief Send a message to the corresponding task after the given delay in ms. The
 *  message will be passed to free after delivery.
 *  \param task The task to deliver the message to. 
 *  \param id The message type identifier. 
 *  \param message The message data (if any). 
 *  \param delay The delay in ms before the message will be sent.
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
void MessageSendLater(Task task, MessageId id, void * message, uint32 delay);

/**
 *  \brief Cancel the first queued message with the given task and message id.
 *  \param task The task whose messages will be searched. 
 *  \param id The message identifier to search for. 
 *  \return TRUE if such a message was found and cancelled.
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
bool MessageCancelFirst(Task task, MessageId id);

/**
 *  \brief Block waiting for the next message. 
 *   This function will either:
 *    - Fill out 'm' if a message is ready for delivery.
 *    - Send the VM to sleep until message delivery time if a message 
 *      exists but is not ready for delivery.
 *    - Send the VM to sleep for the range of a uint32 if no message exists.
 *  \param m This will be filled out if a message is ready to be delivered.
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
void MessageWait(void * m);

/**
 *  \brief Send a message to be be delivered when the corresponding uint16 is zero.
 *  \param t The task to deliver the message to. 
 *  \param id The message identifier. 
 *  \param m The message data. 
 *  \param c The condition that must be zero for the message to be delivered.
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
void MessageSendConditionally(Task t, MessageId id, Message m, const uint16 * c);

/**
 *  \brief Frees the memory pointer to by data. 
 *  \param id  The message identifier. 
 *  \param data A pointer to the memory to free.
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
void MessageFree(MessageId id, Message data);

/**
 *  \brief Cancel all queued messages (independent of id) for the given task. Normally
 *  used as part of the process of freeing a task.
 *   This function will also stop the task being registered with the firmware
 *   as the recipient of any system messages.
 *  \param task The task to flush all message for. 
 *  \return The number of messages removed from the queue.
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
uint16 MessageFlushTask(Task task);

/**
 *  \brief Register a task to handle system-wide messages.
 *   
 *   Currently the system-wide messages are:
 *   - \#MESSAGE_USB_ENUMERATED 
 *   - \#MESSAGE_USB_SUSPENDED
 *   - \#MESSAGE_USB_DECONFIGURED
 *   - \#MESSAGE_USB_ALT_INTERFACE
 *   - \#MESSAGE_USB_ATTACHED
 *   - \#MESSAGE_USB_DETACHED
 *   - \#MESSAGE_PSFL_FAULT
 *   - \#MESSAGE_TX_POWER_CHANGE_EVENT
 *   - \#MESSAGE_SD_MMC_INSERTED
 *   - \#MESSAGE_SD_MMC_REMOVED
 *   Other such messages may be added.
 *  \param task The task which will receive the messages.
 *  \return The old task (or zero).
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
Task MessageSystemTask(Task task);

/**
 *  \brief Register a task to handle PIO changes.
 *  \param task This task will receive \#MESSAGE_PIO_CHANGED messages when the pins configured
 *  by PioDebounce32(), PioDebounce32Bank() and PioDebounceGroup32Bank() change. 
 *  \return The old task (or zero).
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
Task MessagePioTask(Task task);
#endif /* TRAPSET_CORE */
#if TRAPSET_NFC

/**
 *  \brief  Register a task to receive NFC primitives on APP P1 sent 
 *             from APP P0. The nfc primitives to be received are defined in the 
 *             header file "nfc_prim.h".
 *               
 *  \param task  The task which will receive the 
 *                 messages.
 *  \return  The old task (or zero). 
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_nfc
 */
Task MessageNfcTask(Task task);
#endif /* TRAPSET_NFC */
#if TRAPSET_BLUESTACK

/**
 *  \brief Register a task to handle BlueStack primitives.
 *  \param task This task will receive MESSAGE_BLUESTACK_*_PRIM, except
 *  \#MESSAGE_BLUESTACK_ATT_PRIM that are handled by the MessageAttTask(). 
 *  \return The old task (or zero).
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_bluestack
 */
Task MessageBlueStackTask(Task task);
#endif /* TRAPSET_BLUESTACK */
#if TRAPSET_STREAM

/**
 *  \brief Associate a Task with a Sink.
 *   
 *   @note
 *   \# Task cannot be registered for operator sink which is not
 *      connected to a source.
 *   \# Task cannot be registered for operator sink which is connected
 *      to an operator source.
 *   \# Task can be registered on all streams irrespective of whether
 *      they are connected or not.
 *  \param sink The sink to use. 
 *  \param task The task to associate.
 *  \return The previous task for this sink, if any, or zero(0) otherwise.
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
Task MessageStreamTaskFromSink(Sink sink, Task task);

/**
 *  \brief Get the Task currently associated with a sink. 
 *  \param sink The sink to use.
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
Task MessageStreamGetTaskFromSink(Sink sink);

/**
 *  \brief Associate a task with a source.
 *   
 *   @note
 *   \# Task cannot be registered for operator source which is not
 *     connected to a sink.
 *   \# Task cannot be registered for operator source which is connected
 *     to an operator sink.
 *   \# Task can be registered on all streams irrespective of whether
 *     they are connected or not.
 *  \param source The source to use 
 *  \param task The task to associate with
 *  \return The previous task for this source, if any, or zero(0) otherwise.
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
Task MessageStreamTaskFromSource(Source source, Task task);

/**
 *  \brief Get the task currently associated with a source. 
 *  \param source The source to use.
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
Task MessageStreamGetTaskFromSource(Source source);
#endif /* TRAPSET_STREAM */

/**
 *  \brief Send a message to the corresponding task immediately. The message will be
 *  passed to free after delivery. 
 *  \param task The task to deliver the message to. 
 *  \param id The message type identifier. 
 *  \param message The message data (if any).
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset___special_inline
 */
void MessageSend(Task task, MessageId id, void * message);

/**
 *  \brief Cancel all queued messages with the given task and message id.
 *  \param task The task to cancel message for. 
 *  \param id The message identifier to search for. 
 *  \return A count of how many such messages were cancelled.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset___special_inline
 */
uint16 MessageCancelAll(Task task, MessageId id);

/**
 *  \brief The main scheduler loop; it waits until the next message is due and then sends
 *  it to the corresponding task. Never returns.
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset___special_inline
 */
void MessageLoop(void );

/**
 *  \brief Send a message to be be delivered when the corresponding Task is zero.
 *  \param t The task to deliver the message to. 
 *  \param id The message identifier. 
 *  \param m The message data. 
 *  \param c The task that must be zero for the message to be delivered.
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset___special_inline
 */
void MessageSendConditionallyOnTask(Task t, MessageId id, Message m, const Task * c);
#if TRAPSET_BITSERIAL

/**
 *  \brief Register Bitserial events task.
 *           Register a task to receive MESSAGE_BITSERIAL_EVENT message
 *           on transfer completion. 
 *           
 *  \param task The task which will receive the messages.
 *  \return The old task (or zero).
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_bitserial
 */
Task MessageBitserialTask(Task task);
#endif /* TRAPSET_BITSERIAL */
#if TRAPSET_KALIMBA

/**
 *  \brief Register a task to handle messages from Kalimba.
 *   
 *  \param task This task will receive \#MESSAGE_FROM_KALIMBA and \#MESSAGE_FROM_KALIMBA_LONG
 *  messages.
 *  \return The previous task, if any, or zero otherwise.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_kalimba
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
Task MessageKalimbaTask(Task task);
#endif /* TRAPSET_KALIMBA */
#if TRAPSET_INFRARED

/**
 *  \brief Register a task to handle infrared messages.
 *   
 *  \param task The task which will receive the messages.
 *  \return The old task (or zero).
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_infrared
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
Task MessageInfraredTask(Task task);
#endif /* TRAPSET_INFRARED */
#if TRAPSET_HOST

/**
 *  \brief Register a task to handle HostComms primitives.
 *  \param task This task will receive \#MESSAGE_FROM_HOST 
 *  \return The old task (or zero).
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_host
 */
Task MessageHostCommsTask(Task task);
#endif /* TRAPSET_HOST */
#if TRAPSET_CHARGERMESSAGE

/**
 *  \brief Register a task to handle messages from the onchip battery charger and power
 *  system hardware. 
 *  \param task This task will receive \#MESSAGE_CHARGER_CHANGED messages when parts of the
 *  charger or power system hardware changes. 
 *  \return The old task (or zero).
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_chargermessage
 */
Task MessageChargerTask(Task task);
#endif /* TRAPSET_CHARGERMESSAGE */
#if TRAPSET_OPERATOR

/**
 *  \brief Registers a task to get the unsolicited messages from operator 
 *   
 *   VM application should register a task for each operator to get the
 *   unsolicited messages from the operator. The operator posts unsolicited
 *   messages to the registered task, if they have any.
 *  \param opid operator which registers the task 
 *  \param task Task which will receive the unsolicited messages from the operator
 *  \return The previous task, if any, or zero otherwise.
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_operator
 */
Task MessageOperatorTask(Operator opid, Task task);

/**
 *  \brief Registers a task to get the unsolicited messages from DSP framework 
 * Note that the DSP operator framework must be loaded before registering the task
 *  to receive
 * unsolicited messages. Check OperatorFrameworkEnable documentation for more
 *  details.
 *         
 *  \param task Task which will receive the unsolicited messages from DSP framework 
 *  \return The previous task, if any, or zero otherwise.
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_operator
 */
Task MessageOperatorFrameworkTask(Task task);
#endif /* TRAPSET_OPERATOR */
#if TRAPSET_ATT

/**
 *  \brief Register a task to handle BlueStack ATT primitives.
 *  \param task This task will receive \#MESSAGE_BLUESTACK_ATT_PRIM. 
 *  \return The old task (or zero).
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_att
 */
Task MessageAttTask(Task task);
#endif /* TRAPSET_ATT */
#if TRAPSET_CAPACITIVE_SENSOR

/**
 *  \brief Register a task to handle touch sensor messages. 
 *     
 *  \param task The task which will receive the messages.
 *  \return The old task (or zero).
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_capacitive_sensor
 */
Task MessageCapacitiveSensorTask(Task task);
#endif /* TRAPSET_CAPACITIVE_SENSOR */
#endif
