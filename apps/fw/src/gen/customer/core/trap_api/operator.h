#ifndef __OPERATOR_H__
#define __OPERATOR_H__
#include <source_.h>
#include <sink_.h>
#include <operator_.h>
#include <app/file/file_if.h>
#include <app/stream/stream_if.h>
#include <app/operator/operator_if.h>

/*! file  @brief Traps that provide access to the VM-DSP Manager interface
**
**
Operator traps provide access to the DSP Manager interface.
**
DSP Manager is a feature of BlueCore that allows the VM, to
control how data sources and sinks are connected to each other.
It introduces a new kind of functional block called
operator, which is used to process data.
**
An operator is an active entity within DSPManager. 
An operator has one or more inputs and one or more outputs.
It need not have the same number of input and output.
It processes the incoming data in some way to generate the
outgoing data - this can be as simple as copying
with a volume change, through mixing two inputs or splitting
an input into two outputs, all the way to things like active
noise cancellation or MP3 encoding or decoding.
**
The behaviour of an operator is called its capability 
and an operator instantiates that capability. Each capability 
has a unique identifier (the capability identifier or capid).
The general meaning of a given capability will never change 
(thus capability 1 is defined as being "mono pass through" 
and will always be this). 
However, new features may be added to a capability from time to
time in an upwards-compatible manner (that is, host software
that is unaware of the new feature will not be affected by it).
When an operator is created it has a unique operator identifier 
or opid that is valid until it is destroyed. Several operators
can instantiate the same capability and all are independent  
from each other. When an operator is destroyed its identifier
becomes invalid. The identifier will eventually be re-used, 
but normally this re-use will be delayed as long as possible.
Operators can be running or stopped. Starting an operator changes
it from stopped to running; stopping an operator
changes it back to stopped again.
An operator cannot be destroyed while running.
**
The VM may be able to configure an operator by sending it 
messages. The operator can also send unsolicited messages to 
the VM to inform it of significant events. The messages that 
are available (in both directions) and their meaning depend 
on the specific capability.
**
**
*/
#define OperatorDestroy(opid) OperatorDestroyMultiple(1, &(opid), NULL)
#define OperatorStart(opid) OperatorStartMultiple(1, &(opid), NULL)
#define OperatorStop(opid) OperatorStopMultiple(1, &(opid), NULL)
#define OperatorReset(opid) OperatorResetMultiple(1, &(opid), NULL)

#if TRAPSET_OPERATOR

/**
 *  \brief Creates a new operator that instantiates the requested capability 
 *   Note that the DSP software/operator framework must be loaded before creating
 *  an operator. 
 *   Check OperatorFrameworkEnable documentation for more details.
 *   If the loaded DSP software supports the requested capability then DSP creates
 *   an operator in stopped state. If it does not support the requested capability,
 *   then this trap returns zero(0).
 *   @note
 *   If DSP is already loaded with DSP software that does not support operators
 *   then BlueCore firmware returns zero(0) on a call to this trap, till the DSP
 *   gets powered off.
 *  \param cap_id Type of the operator (or Capability ID). 
 *  \param num_keys Number of key-value pairs specified. 
 *  \param info Points to list of key-value pairs for setting certain parameters for the
 *  operator at the time of its creation. 
 *  \return Operator ID if the operator was created successfully, zero(0) otherwise.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_operator
 */
Operator OperatorCreate(uint16 cap_id, uint16 num_keys, OperatorCreateKeys * info);

/**
 *  \brief Destroys one or more operators 
 *   This trap destroys all the operator(s) passed to it as a list. If DSP fails
 *   to destroy one of the operators then no attempt is made to destroy the
 *   subsequent operator(s) in the list. The number of successfully destroyed 
 *   operator(s) is placed in @a success_ops parameter, which is passed by
 *   reference.
 *   
 *   @note
 *   VM application must handle the allocation and de-allocation of memory space
 *   for API parameters. If an operator is running, any attempt to destroy the
 *   operator will fail. An operator can only be destroyed when it has been
 *   stopped. To destroy a single operator, refer \# OperatorDestroy.
 *  \param n_ops Number of operators to destroy 
 *  \param oplist List of operators to destroy 
 *  \param success_ops Number of successfully destroyed operators, which is passed by reference. If
 *  set to NULL then this parameter is ignored and application will only know that
 *  some (or all) operators could not be destroyed from the return status. 
 *  \return TRUE if all operator(s) were successfully destroyed, FALSE otherwise.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_operator
 */
bool OperatorDestroyMultiple(uint16 n_ops, Operator * oplist, uint16 * success_ops);

/**
 *  \brief Starts one or more operators 
 *   This starts all the operator(s) passed to it as a list. If DSP fails to
 *   start one of the operators then no attempt is made to start the subsequent
 *   operator(s). The number of successfully started operator(s) is placed in
 *   @a success_ops parameter, which is passed by reference.
 *   @note
 *   VM application must handle the allocation and de-allocation of memory space
 *   for API parameters. It is permitted to start an operator that has already been
 *   started. Starting an operator that has nothing connected to it, or has
 *   insufficient connections, may fail (depending on the capability).
 *  \param n_ops Number of operators to start 
 *  \param oplist List of operators to start 
 *  \param success_ops Number of successfully started operators, which is passed by reference. If set
 *  to NULL then this parameter is ignored and application will only know that
 *  some (or all) operators could not be started from the return status. 
 *  \return TRUE if all operator(s) were successfully started, FALSE otherwise.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_operator
 */
bool OperatorStartMultiple(uint16 n_ops, Operator * oplist, uint16 * success_ops);

/**
 *  \brief Stops one or more operators 
 *   This API stops all the operator(s) passed to it as a list. If DSP fails to 
 *   stop one of the operators then no attempt is made to stop the subsequent
 *   operator(s). The number of successfully stopped operator(s) is placed
 *   in @a success_ops parameter only if it is not NULL.
 *   @note
 *   VM application must handle the allocation and de-allocation of memory space
 *   for API parameters. It is permitted to stop an operator that has never been
 *   started or has already been stopped.
 *  \param n_ops Number of operators to stop 
 *  \param oplist List of operators to stop 
 *  \param success_ops Number of successfully stopped operators, which is passed by reference. If set
 *  to NULL then this parameter is ignored and application will only know that
 *  some (or all) operators could not be stopped from the return status. 
 *  \return TRUE if all operator(s) were successfully stopped, FALSE otherwise.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_operator
 */
bool OperatorStopMultiple(uint16 n_ops, Operator * oplist, uint16 * success_ops);

/**
 *  \brief Resets one or more operators 
 *   This API resets all the operator(s) passed to it as a list. If DSP fails
 *   to reset one of the operators then no attempt is made to reset the subsequent
 *   operator(s). The number of operators have successfully been reset is placed
 *   in @a success_ops parameter only if it is not NULL.
 *   @note
 *   VM application must handle the allocation and de-allocation of memory space
 *   for API parameters. It is permitted to reset an operator that has never been
 *   started or has already been stopped.
 *  \param n_ops Number of operators to reset 
 *  \param oplist List of operators to reset 
 *  \param success_ops Number of operators have successfully been reset, which is passed by reference.
 *  If set to NULL, then this parameter is ignored and application will only know
 *  that some (or all) operators could not be reset from the return status. 
 *  \return TRUE if all the operator(s) have successfully been reset, FALSE otherwise. 
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_operator
 */
bool OperatorResetMultiple(uint16 n_ops, Operator * oplist, uint16 * success_ops);

/**
 *  \brief Sends a message to the operator 
 *   @note
 *   VM application must handle the allocation and de-allocation of memory space
 *   for API parameters. Determining the size of the message to be sent and that
 *   of the corresponding response-message to be received depends on the
 *   respective structures being used for the message to be sent and received.
 *   @note
 *   If DSP is already loaded with DSP software that does not support operators
 *   then BlueCore firmware returns FALSE for any operator message request
 *   till DSP gets powered off.
 *  \param opid Operator to which VM application sends a message 
 *  \param send_msg Message to the operator 
 *  \param send_len_words Length of the message to be sent in units of uint16s 
 *  \param recv_msg Message response, which will be received from the DSP in response to the sent
 *  message 
 *  \param recv_len_words Length of the response message in units of uint16s 
 *  \return TRUE if it gets a response from the specific operator, otherwise FALSE. The
 *  messages that are available and their meaning depend on the specific
 *  capability that the operator instantiates.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_operator
 */
bool OperatorMessage(Operator opid, const uint16 * send_msg, uint16 send_len_words, uint16 * recv_msg, uint16 recv_len_words);

/**
 *  \brief Load a "bundle" file containing one or more audio processing capabilities into
 *  the audio subsystem.
 *   This causes code to be loaded from a file in the application subsystem's
 *  filesystem
 *   into the audio subsystem, ready for OperatorCreate() to be invoked to
 *  instantiate any of the newly available
 *   capabilities (a .dkcs file may implement more than one capability). The
 *  downloaded bundle will occupy resources
 *   until unloaded with OperatorBundleUnload(), or the audio subsystem is shut
 *  down with OperatorFrameworkEnable(MAIN_PROCESSOR_OFF).
 *   It is not possible to load or unload capabilities within a single .dkcs
 *  selectively.
 *  \param index Specifies a .dkcs file containing capability code.
 *  \param processor Indicates which audio processor the code is intended to run on.
 *  FIXME: we don't know yet if this is a hard restriction or just a hint.
 *  \return A handle for the resources occupied by the loaded code which can be passed to
 *  OperatorBundleUnload().
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_operator
 */
BundleID OperatorBundleLoad(FILE_INDEX index, uint16 processor);

/**
 *  \brief Unload previously loaded capability/ies from the audio subsystem.
 *  \param bundle A BundleID previously returned from OperatorBundleLoad().
 *  \return TRUE if the resources were freed, FALSE if not.
 *  This call can fail if any operators are currently instantiated from this
 *  bundle.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_operator
 */
bool OperatorBundleUnload(BundleID bundle);

/**
 *  \brief Sets DSP operator framework configuration parameters 
 * Note that the DSP operator framework must be loaded before setting the
 *  configuration parameters. 
 * Check OperatorFrameworkEnable documentation for more details.
 *         
 *  \param key Configuration parameter identifier 
 *  \param send_msg Message containing configuration parameters to be set 
 *  \param send_len_words Length of message to be sent in units of uint16s 
 *  \return TRUE if the DSP operator framework parameters have successfully been set, FALSE
 *  otherwise.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_operator
 */
bool OperatorFrameworkConfigurationSet(uint16 key, const uint16 * send_msg, uint16 send_len_words);

/**
 *  \brief Gets DSP operator framework configuration parameters 
 * Note that the DSP operator framework must be loaded before getting the
 *  configuration parameters. 
 * Check OperatorFrameworkEnable documentation for more details.
 *   @note
 *   send_msg specifies more about what exactly is to be fetched.
 *  \param key Configuration parameter identifier 
 *  \param send_msg Message to DSP framework for getting the configuration parameters 
 *  \param send_len_words Length of message to be sent in units of uint16s 
 *  \param recv_msg The response message containing the requested configuration parameters 
 *  \param recv_len_words Length of the response message in units of uint16s 
 *  \return TRUE if it gets a response from the DSP framework, otherwise FALSE.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_operator
 */
bool OperatorFrameworkConfigurationGet(uint16 key, const uint16 * send_msg, uint16 send_len_words, uint16 * recv_msg, uint16 recv_len_words);

/**
 *  \brief Loads DSP operator framework / powers-off DSP 
 *   @note
 *   Multiple VM libraries or the application can load and power-off DSP
 *   independently. All the VM libraries and application MUST follow the
 *   below sequences to reduce system power consumption by powering-off DSP.
 *   Step-1: OperatorFrameworkEnable (MAIN_PROCESSOR_ON);
 *   Step-2: Create operators / Download a new capability
 *   Step-3: Perform operator actions / Send message to operators
 *   Step-4: Destroy operators / Remove downloaded capability
 *   Step-5: OperatorFrameworkEnable (MAIN_PROCESSOR_OFF);
 *   The VM library / application cannot assume DSP being powered off even
 *   after receiving a TRUE return from
 *  OperatorFrameworkEnable(MAIN_PROCESSOR_OFF) because
 *   some other part of the application might keep it on.
 *   Reason for DSP could not be powered off can be any of the following:
 *   1: The main processor can only be powered off when the second processor
 *      is already off
 *   2: The firmware keeps a reference count of how many times each processor
 *      was enabled/disabled. Counter is incremented for enable and decremented
 *  for disable.
 *      Based on the count value, DSP will be powered off.
 *   3. If the OperatorFrameworkTriggerNotificationStart was started by the
 *  application, 
 *      then calling OperatorFrameworkEnable(MAIN_PROCESSOR_OFF) may transition to
 *  a low power mode 
 *      when the reference count is zero (count including current call) and the
 *  second
 *      processor is already powered-off.
 *   The VM library / application should not rely on getting a FALSE return for
 *   OperatorFrameworkEnable(MAIN_PROCESSOR_OFF) as the FALSE return could be
 *  because of an
 *   error made a long time ago. It is at most an opportunity to detect errors.
 *  \param state DSP operator framework power state for specific DSP core 
 *  \return TRUE if DSP operator framework is loaded / unloaded successfully, FALSE
 *  otherwise.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_operator
 */
bool OperatorFrameworkEnable(OperatorFrameworkPowerState state);

/**
 *  \brief Promote one operator to be in control of others, or revoke this control 
 *   @note
 *  This function allows a specially designed 'graph manager' Operator to 
 *  be promoted to own a subgraph of other Operators, and to have limited
 *  supervisory control of them; principally, reacting to and sending
 *  messages to them without the application's involvement. (Any
 *  task registered with MessageOperatorTask() will not receive messages
 *  from an operator while control is delegated; they will go to the
 *  graph manager instead.)
 *  
 *  \param op_client The 'graph manager' or 'operator client' operator that
 *                          controls other operators. Set to 0 to revert control
 *  to the
 *                          application. 
 *  \param n_ops The number of Operators in \p oplist. May be 0. 
 *  \param oplist The set of operators to be placed under \p op_client's control. 
 *             May be NULL if \p n_ops is 0. 
 *  \return TRUE if all went well. Can return FALSE if arguments are invalid, or in case of
 *  resource shortage.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_operator
 */
bool OperatorDelegateMultiple(Operator op_client, uint16 n_ops, Operator * oplist);

/**
 *  \brief Load configuration data file for an operator.
 *   This causes operator configuration data from a file in 
 * the application subsystem's filesystem to be loaded into the audio-DSP 
 * operator framework. This should be done before calling OperatorStart() 
 * so that the operator capability can use the configuration data during 
 * it's operation. Application should call OperatorMessage() to setup the 
 * operator to use the downloaded operator data. The downloaded data will 
 * occupy resources until unloaded with OperatorDataUnloadEx(), or the 
 * audio-DSP is powered off by using OperatorFrameworkEnable(MAIN_PROCESSOR_OFF).
 * However, the application can request operator framework to unload the operator 
 * data on calling OperatorDestroy() for the operator configured to use it.
 * @note
 * The SVA capability expects the detection phrase as downloaded operator
 * data. This api can be used to download the SVA trigger phrase to operator 
 * framework before starting SVA capability. 
 *         
 *  \param index Index of the file containing operator data
 *  \param type Type of operator data file
 *  \param storage Type of storage used by audio-DSP framework for storing operator data 
 *  \param auto_remove TRUE if operator data needs to be removed on operator destroy, FALSE othewise
 *  \return A handle for the resources occupied by the loaded data file which can be passed
 *  to OperatorDataUnloadEx() 
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_operator
 */
DataFileID OperatorDataLoadEx(FILE_INDEX index, OperatorDataFileType type, OperatorDataStorageType storage, bool auto_remove);

/**
 *  \brief Unload operator configuration data file which has been previously loaded by
 *  using OperatorDataLoadEx() 
 *  \param file_id A handle for the operator data file to be unloaded.
 *  \return TRUE if operator data file has been unloaded, FALSE otherwise.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_operator
 */
bool OperatorDataUnloadEx(DataFileID file_id);
#endif /* TRAPSET_OPERATOR */
#if TRAPSET_WAKE_ON_AUDIO

/**
 *  \brief Starts trigger notification from audio-DSP in low power mode operation
 * Before calling this function the application should register a task to handle
 * messages from the operator using the MessageOperatorTask() call.
 * Messages will be sent to the task when the trigger conditions occur.
 * The audio graph of the operator passed to this trigger function will
 * remain running after a call to OperatorFrameworkEnable(MAIN_PROCESSOR_OFF). In
 *  this low 
 * power mode the operator will continue to produce messages in response 
 * to the trigger. The low power mode can be exited by calling 
 * OperatorFrameworkTriggerNotificationStop().
 *         
 *  \param type Type of trigger notification 
 *  \param opid Operator used to trigger the notification 
 *  \return TRUE if trigger notification start request has succeeded, FALSE otherwise.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_wake_on_audio
 */
bool OperatorFrameworkTriggerNotificationStart(OperatorFrameworkTriggerType type, Operator opid);

/**
 *  \brief Stops trigger notification from audio-DSP in low power mode operation
 *  \return TRUE if trigger notification stop request has succeeded, FALSE otherwise.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_wake_on_audio
 */
bool OperatorFrameworkTriggerNotificationStop(void );

/**
 *  \brief Load configuration data file for an operator.
 *   This causes operator configuration data from a file in 
 * the application subsystem's filesystem to be loaded into the audio-DSP 
 * operator framework. This should be done before calling OperatorStart() 
 * so that the operator capability can use the configuration data during 
 * it's operation. Application should call OperatorMessage() to setup the 
 * operator to use the downloaded operator data. The downloaded data will 
 * occupy resources until unloaded with OperatorDataUnload(), or the 
 * audio-DSP is powered off by using OperatorFrameworkEnable(MAIN_PROCESSOR_OFF)
 * without starting trigger notification. However, the application can 
 * request operator framework to unload the operator data on calling 
 * OperatorDestroy() for the operator configured to use it.
 * @note
 * The SVA capability expects the detection phrase as downloaded operator
 * data. This api can be used to download the SVA trigger phrase to operator
 * framework before starting SVA capability.
 * @note This function has been deprecated. Please use OperatorDataLoadEx instead.
 *         
 *  \param index Index of the file containing operator data
 *  \param type Type of operator data file
 *  \param auto_remove TRUE if operator data needs to be removed on operator destroy, FALSE othewise
 *  \return A handle for the resources occupied by the loaded data file which can be passed
 *  to OperatorDataUnload().
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_wake_on_audio
 */
DataFileID OperatorDataLoad(FILE_INDEX index, OperatorDataFileType type, bool auto_remove);

/**
 *  \brief Unload operator configuration data file which has been previously loaded by
 *  using OperatorDataLoad()
 * @note This function has been deprecated. Please use OperatorDataUnloadEx
 *  instead.
 *       
 *  \param file_id A handle for the operator data file to be unloaded.
 *  \return TRUE if operator data file has been unloaded, FALSE otherwise.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_wake_on_audio
 */
bool OperatorDataUnload(DataFileID file_id);

/**
 *  \brief Mark operators and endpoints for retention in low-power mode. 
 *  @note
 *  Normally, if OperatorFrameworkEnable(MAIN_PROCESSOR_OFF) is called
 *  as many times as OperatorFrameworkEnable(MAIN_PROCESSOR_ON), any
 *  Operators, audio Sources and Sinks, and Transforms involving at least
 *  one of these that are still in existence at that point are automatically
 *  destroyed and/or reset to their default configuration. This happens even
 *  if the audio subsystem continues to run in a low-power mode due to use of
 *  OperatorFrameworkTriggerNotificationStart().
 *  This function can be used to ensure that these objects are retained when
 *  the audio subsystem is in low-power mode. Any Transform between two
 *  objects on these lists will also be retained. Any handles to these objects
 *  will remain valid.
 *  While in low-power mode, the operators should not emit any messages,
 *  as they will not be routed. (If message handling is required, control of
 *  the Operators should instead be delegated with OperatorDelegateMultiple()
 *  to a graph manager operator which is prepared to handle that Operator's
 *  messages.) Operators preserved in this way should not be connected to
 *  any non-audio Sources or Sinks (such as L2CAP, SCO, or File endpoints).
 *  Use of this trap does not prevent destruction of these objects if the
 *  audio subsystem is genuinely powered off (that is, if neither
 *  OperatorFrameworkEnable() nor OperatorFrameworkTriggerNotificationStart()
 *  is keeping it powered).
 *  If control of an Operator has been delegated with
 *  OperatorDelegateMultiple(), or an Operator has been designated as a
 *  trigger source with OperatorFrameworkTriggerNotificationStart(), that
 *  Operator will naturally be preserved; it is not necessary to mark it
 *  for preservation with this function.
 *         
 *  \param n_ops The number of Operators in \p oplist. May be 0. 
 *  \param oplist The set of Operators to be preserved in low-power modes.
 * May be NULL if \p n_ops is 0. 
 *             
 *  \param n_srcs The number of Sources in \p srclist. May be 0. 
 *  \param srclist The set of Sources to be preserved in low-power modes. 
 * These can only be sources obtained from StreamAudioSource(). May be NULL if \p
 *  n_srcs is 0.
 *             
 *  \param n_sinks The number of Sinks in \p sinklist. May be 0. 
 *  \param sinklist The set of Sinks to be preserved in low-power modes. 
 * These can only be sinks obtained from StreamAudioSink(). May be NULL if \p
 *  n_sinks is 0.
 *             
 *  \return TRUE if all went well. Can return FALSE if arguments are invalid, or in case of
 *  resource shortage.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_wake_on_audio
 */
bool OperatorFrameworkPreserve(uint16 n_ops, Operator * oplist, uint16 n_srcs, Source * srclist, uint16 n_sinks, Sink * sinklist);

/**
 *  \brief Mark operators and endpoints for automatic destruction.
 *  This is the reverse of OperatorFrameworkPreserve().
 *  \param n_ops The number of Operators in \p oplist. May be 0. 
 *  \param oplist The set of Operators to be released. May be NULL if \p n_ops is 0.
 *  \param n_srcs The number of Sources in \p srclist. May be 0. 
 *  \param srclist  The set of Sources to be released. These can only be sources  
 * obtained from StreamAudioSource(). May be NULL if \p n_srcs is 0.
 *             
 *  \param n_sinks The number of Sinks in \p sinklist. May be 0. 
 *  \param sinklist The set of Sinks to be released. These can only be sinks 
 * obtained from StreamAudioSink(). May be NULL if \p n_sinks is 0.
 *             
 *  \return TRUE if all went well. Can return FALSE if arguments are invalid, or in case of
 *  resource shortage.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_wake_on_audio
 */
bool OperatorFrameworkRelease(uint16 n_ops, Operator * oplist, uint16 n_srcs, Source * srclist, uint16 n_sinks, Sink * sinklist);
#endif /* TRAPSET_WAKE_ON_AUDIO */
#endif
