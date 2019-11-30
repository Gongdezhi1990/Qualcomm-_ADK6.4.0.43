/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    resource_manager.h

DESCRIPTION
    A library to help manage the ownership of limited firmware or hardware
    resources. For example a digital microphone input that multiple audio
    plugins want to use but can only be used by one at a time.

    This library does not interact with the resources directly. 

    It is the responsibility of the client(s) using a resource to call the
    appropriate APIs in this library before using the resource, to make sure
    it is not in use by another client.
*/

#ifndef RESOURCE_MANAGER_H_
#define RESOURCE_MANAGER_H_

#include <csrtypes.h>


/*!
    @brief Status codes returned by Resource Manager.
*/
typedef enum {
    /*! Operation completed successfully. */
    rm_status_success = 0,

    /*! One of the parameters is out of range or otherwise 'bad'. */
    rm_status_bad_parameter,

    /*! Requested resource is in use but request has been queued. */
    rm_status_in_use_queued,

    /*! Requested resource is in use and queue is full. */
    rm_status_in_use_rejected,

    /*! Operation failed due to a lack of memory. */
    rm_status_no_memory
} rm_status_t;

/*!
    @brief Ids of resources tracked by the Resource Manager.
*/
typedef enum
{
    /*! The microphone used for voice input. */
    rm_resource_voice_mic = 0,
    
    /*! Dummy resource used for unit testing */
    rm_resource_dummy,
    
    rm_resource_last
} rm_resource_id_t;

/*!
    @brief Message Ids sent to the callback.
*/
typedef enum
{
    /*! Request to release an acquired resource. */
    rm_msg_release_req = 0,

    /*! Notification that a requested resource is now free. */
    rm_msg_available_ind,
    
    rm_msg_last
} rm_msg_id_t;

/*!
    @brief Callback used to inform a client of a change in status of a resource.

    A resource may be requested by another client, or it may have been released
    and this client was queued up waiting for it. In either case, the callback
    should return TRUE if the client wants to acquire (or keep) the resource
    or FALSE if it wants to release (or not acquire) it.

    When a client decides to release a resource it must take the appropriate
    steps to leave the resource in a state ready for another client to use it
    before returning out of the callback.

    Note: The client must not call any other Resource Manager API from inside
          the callback.
    
    @param msg Id of the message to send to the client.
    @param res Resource that the message applies to.
*/
typedef bool (*rm_callback)(rm_msg_id_t msg, rm_resource_id_t res);

/*!
    @brief Acquire ownership of a resource.
    
    If the resource is currently owned by another module then that module will
    be notified, via its callback, and it will either release the resource or
    keep it.
    
    If the other module keeps the resource this module will be queued and
    notified when the resource becomes free again.
    
    A maximum of one module can be queued for a resource. Any more acquire
    requests for that will be rejected immediately.

    @param res Resource to request access to.
    @param callback Callback used to notify caller when resource status changes.

    @return rm_status_t rm_status_success if the resource was allocated to the 
                        caller, an error code otherwise.
*/
rm_status_t ResourceManagerAcquireResource(rm_resource_id_t res, rm_callback callback);

/*!
    @brief Release ownerdhip of a resource.
    
    Release ownership of a resource. If another module is waiting for the
    resource it will be notified via its callback.
    
    Releasing a resource must only be done once you have released all system
    resources it was using (e.g. disconnecting and closing a Source for a 
    microphone).

    @param res Resource to request access to.
    @param callback Callback used when reserving the resource.
*/
void ResourceManagerReleaseResource(rm_resource_id_t res, rm_callback callback);

    
#endif /* RESOURCE_MANAGER_H_ */
