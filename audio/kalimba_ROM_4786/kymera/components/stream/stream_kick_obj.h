/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file stream_kick_obj.h
 * \ingroup stream
 *
 */

#ifndef KICK_OBJ_H_
#define KICK_OBJ_H_

/******************************************************************************
Include Files
*/
#include "stream/stream.h"

/******************************************************************************
Public Type Definitions
*/

/**
 * Kick type enum is used to specify the event type that will generate the kick.
 */
typedef enum
{
    KICK_EVENT_TYPE_0 = 0,
    KICK_EVENT_TYPE_1,
    KICK_EVENT_TYPE_2,
} KICK_EVENT_TYPE;

/**
 * The object that receives a kick when an interrupt is generated.
 */
struct KICK_OBJECT
{
    /**
     * The endpoint responsible for scheduling this kick.
     */
    ENDPOINT *sched_ep;
    /**
     * The endpoint at the head of this chain. This is the endpoint where data
     * processing will be initiated when the kick interrupt occurs.
     */
    ENDPOINT *kick_ep;
    /**
     * Pointer to the next kick object in the list.
     */
    struct KICK_OBJECT *next;
};

typedef struct KICK_OBJECT KICK_OBJECT;

/******************************************************************************
Public Function Definitions
*/

KICK_OBJECT *kick_obj_create(ENDPOINT *sched_ep, ENDPOINT *head_ep);

/**
 * \brief Destructor for a kick object.
 *
 * \param ko pointer to the KICK_OBJECT to destroy.
 */
void kick_obj_destroy(KICK_OBJECT *ko);

/**
 * \brief Interrupt handler for the chain which causes the appropriate
 * scheduling to be performed and the chain's audio processing to begin.
 *
 * \param kick_object pointer to the kick object the interrupt is for.
 */
void kick_obj_kick(void *kick_object);

/**
 * \brief Finds the kick object that is responsible to kick the given endpoint.
 *
 * \param ep pointer to the dnpoint in question.
 *
 * \return the KICK_OBJECT that was found, or NULL if nothing found.
 */
KICK_OBJECT *kick_obj_from_sched_endpoint(ENDPOINT *ep);

/**
 * \brief Set the endpoint that this kick object is scheduling
 *
 * \param ko pointer to the kick object
 * \param ep pointer to the endpoint in question.
 */

void kick_set_sched_ep(KICK_OBJECT *ko, ENDPOINT *ep);

/**
 * \brief Set the endpoint that this kick object is kicking
 *
 * \param ko pointer to the kick object
 * \param ep pointer to the endpoint in question.
 */

void kick_set_kick_ep(KICK_OBJECT *ko, ENDPOINT *ep);

/**
 * \brief Get the endpoint that this kick object is scheduling
 *
 * \param ko pointer to the kick object
 *
 * \return pointer to the endpoint in question.
 */

ENDPOINT *kick_get_sched_ep(KICK_OBJECT *ko);

/**
 * \brief Get the endpoint that this kick object is kicking
 *
 * \param ko pointer to the kick object
 *
 * \return pointer to the endpoint in question.
 */

ENDPOINT *kick_get_kick_ep(KICK_OBJECT *ko);


#endif /* KICK_OBJ_H_ */
