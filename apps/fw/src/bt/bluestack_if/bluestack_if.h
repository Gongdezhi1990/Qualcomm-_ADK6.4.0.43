/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
#ifndef BLUESTACK_IF_H_
#define BLUESTACK_IF_H_

#include "buffer/buffer.h"

/**
 * Enumeration of possible host (test) interfaces
 */
typedef enum
{
    BLUESTACK_HOST_IF_NONE = 0,  /**< No test interface: data comes from App */
    BLUESTACK_HOST_IF_APPCMD = 1,/**< Test data exchanged via AppCmd */
    BLUESTACK_HOST_IF_ISP = 2    /**< Test data exchanged via ISP */
} BLUESTACK_HOST_IF;


/**
 * Initialise the bluestack interface to talk either to appcmd or submsg,
 * depending on the specified HOST_IF or, if FROM_MIB specified, the MIB key
 * BLUESTACK_INTERFACE
 *
 * \param host_if Host interface type to bring up
 * \return TRUE if successful; FALSE if something failed - this indicates that
 * Bluetooth can't be started.  Panics if there is an unexpected
 * situation, such as the host interface already having been set up.
 */
extern bool bluestack_if_create_host(BLUESTACK_HOST_IF host_if);

/**
 * Tear down the interface and release memory resources
 */
extern void bluestack_if_destroy_host(void);

/**
 * Notification that new data is available in the rx buffer.
 * \param hostio_channel_id The hostio protcol id to read the primtive.
 */
extern void bluestack_if_data_ind(uint16 hostio_channel_id);

/**
 * Push the given primitive onto the appropriate interface queue
 * \param sched_channel_id Protocol the primitive is for, in the bluetooth.h
 * numbering, used in scheduler messages
 * \param prim The primitive itself
 * \param from_app Has the primitive come from the customer App or from the test
 * interface?
 */
extern void bluestack_if_forward_prim(uint16 sched_channel_id,
                                      void *prim, bool from_app);

/**
 * Push the given primitive onto the appropriate interface queue if bluestack
 * has started, or else cache it and trigger bluestack start
 * @param channel_id Protocol the primitive is for, in the bluetooth.h
 * numbering, used in scheduler messages
 * @param prim The primitive itself
 */
extern void bluestack_if_forward_prim_from_app(uint16 channel_id, void *prim);

/**
 * Enable/Disable HQ PDU handling in bluestack interface.
 * \param enable - TRUE if hq pdu handling is required, FALSE otherwise
 * \return TRUE if enable/disable request has been successful, FALSE otherwise.
 */
extern bool bluestack_if_enable_hq(bool enable);

/**
 * @brief Send a prim to the application via IPC.
 *
 * The prim filter may decide to not send this primitive if it is an unreliable
 * prim and a number of other prims are currently in flight. If the prim isn't
 * sent it will be freed by this function and the function will return FALSE.
 *
 * @param  protocol  Protocol tag, as defined in bluetooth.h.
 * @param  prim      A pointer to the primitive.
 * @retval TRUE      If the primitive was sent to the application.
 * @retval FALSE     If the primitive was not sent. In this case the primitive
 *                   will have been deleted before the function returns.
 */
extern bool bluestack_if_ipc_send(uint16 protocol, void *prim);

#endif /* BLUESTACK_IF_H_ */
