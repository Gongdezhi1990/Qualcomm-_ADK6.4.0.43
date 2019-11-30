/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Intrusive Event interface.
 */

#ifndef UTILS_EVENT_H
#define UTILS_EVENT_H

/*****************************************************************************
 * Interface Dependencies
 ****************************************************************************/

#include "utils/utils_set.h"

/*****************************************************************************
 * Public Types
 ****************************************************************************/

/**
 * Intrusive Event.
 *
 * Implements classic Event-Listener pattern.
 *
 * "listener-ship" state is intrusive on the listener. This implementation
 * side-steps dynamic and global storage allocation issues. Every potential
 * listener embeds the storage (2 ptrs) needed for them to be a listener.
 *
 * Features:-
 *
 * - Supports dynamic Set of heterogeneous Listeners (L).
 * - Storage = 1.dataPtr + L.(dataPtr + fnPtr)
 * - Avoids under, over and dynamic storage allocation.
 * - Avoids preprocessor and preprocessing tricks.
 * - Not thread safe (wrap or specialise if required).
 * - No guarantee on the order that multiple Listeners will be notified of
 * an event.
 * - It is an error to attempt to add a Listener to an event more than once.
 * - It is an error to attempt to add a Listener to more than one event at
 * a time.
 * - It is an error to attempt to remove a Listener from an event it is not
 * listening to.
 *
 * Known Uses:-
 *
 * - Implementation of curator/subman state change notification.
 *
 * Example application:-
 *\verbatim

    *
    * Event notification handlers.
    * The listener context is normally v. useful but is not used here.
    *
    void jump(utils_EventListener *o) { printf("Jump\n"); }
    void shout(utils_EventListener *o) { printf("WAH!\n"); }

    * An event and 2 listeners *
    utils_Event          loudbang;
    utils_EventListener  jumper;
    utils_EventListener  shouter;

    * Init event *

    utils_Event_init(&loudbang);

    * Init listeners *

    utils_EventListener_init(&jumper, jump);
    utils_EventListener_init(&shouter, shout);

    * Register listeners with event *

    utils_Event_add(&loudbang, &jumper);
    utils_Event_add(&loudbang, &shouter);

    * Fire the event *

    utils_Event_signal(&loudbang);

    * Output (ordering of Listener notification is undefined) *
    Jump
    WAH!

\endverbatim
*/
typedef struct utils_Event utils_Event;

/**
 * Intrusive Event Listener (Abstract Base)
 *
 * Classes that wish to listen to an Event should include a member of this
 * type (one per event they might be interested in) and override/initialise
 * the event call-back method.
 *
 * \see utils_Event for example application.
 */
typedef struct utils_EventListener utils_EventListener;

/**
 * Intrusive Event Notification CallBack signature.
 *
 * The address of the EventListener is passed to the callback to
 * provide a context.
 *
 * \see utils_Event for example application.
 */
typedef void (*utils_EventCallBack)(utils_EventListener *l);

/*****************************************************************************
 * Public Function Declarations
 ****************************************************************************/

/**
 * Initialise this Event.
 *
 * \public \memberof utils_Event
 *
 * \note
 * Implemented as a macro.
 */
#ifdef DOXYGEN_ONLY
extern void utils_Event_init(utils_Event *e);
#else /* DOXYGEN_ONLY */
#define utils_Event_init(e) \
    utils_Set_init(utils_Event_listeners(e))
#endif /* DOXYGEN_ONLY */

/**
 * Does this Event have any listeners?
 *
 * \public \memberof utils_Event
 *
 * \note
 * Implemented as a macro.
 */
#ifdef DOXYGEN_ONLY
extern bool utils_Event_isListenedTo(const utils_Event *e);
#else /* DOXYGEN_ONLY */
#define utils_Event_isListenedTo(e) \
    (!utils_Set_isEmpty(utils_Event_listeners(e)))
#endif /* DOXYGEN_ONLY */

/**
 * Is the specified listener listening to this Event?
 *
 * \public \memberof utils_Event
 *
 * \note
 * Implemented as a macro.
 */
#ifdef DOXYGEN_ONLY
extern bool utils_Event_hasListener(
    const utils_Event *e,
    const utils_EventListener *l
);
#else /* DOXYGEN_ONLY */
#define utils_Event_hasListener(e,l) \
    utils_Set_contains(utils_Event_listeners(e), \
                       utils_EventListener_membershipOfListeners(l))
#endif /* DOXYGEN_ONLY */

/**
 * Add a listener to this Event.
 *
 * \public \memberof utils_Event
 * \pre             !utils_Event_hasListener(e,l)
 * \post            utils_Event_hasListener(e,l)
 *
 * \note
 * Implemented as a macro.
 */
#ifdef DOXYGEN_ONLY
extern void utils_Event_addListener(utils_Event *e, utils_EventListener *l);
#else /* DOXYGEN_ONLY */
#define utils_Event_addListener(e,l) \
    utils_Set_add(utils_Event_listeners(e), \
                  utils_EventListener_membershipOfListeners(l))
#endif /* DOXYGEN_ONLY */

/**
 * Remove a listener from this Event.
 *
 * \public \memberof utils_Event
 * \pre             utils_Event_hasListener(e,l)
 * \post            !utils_Event_hasListener(e,l)
 *
 * \note
 * Implemented as a macro.
 */
#ifdef DOXYGEN_ONLY
extern void utils_Event_removeListener(utils_Event *e, utils_EventListener *l);
#else /* DOXYGEN_ONLY */
#define utils_Event_removeListener(e,l) \
    utils_Set_remove(utils_Event_listeners(e), \
                     utils_EventListener_membershipOfListeners(l))
#endif /* DOXYGEN_ONLY */

/**
 * Signal occurance of this Event.
 *
 * All registered Listeners will be notified. The order that they are notified
 * in is undefined.
 *
 * \public \memberof utils_Event
 *
 * \note
 * Implemented as a macro.
 */
#ifdef DOXYGEN_ONLY
extern void utils_Event_signal(utils_Event *e);
#else /* DOXYGEN_ONLY */
#define utils_Event_signal(e) \
    (utils_Event_isListenedTo(e) ? \
    utils_Set_forEach(utils_Event_listeners(e), utils_Event_notifyListener): \
    ((void)0))
#endif /* DOXYGEN_ONLY */

/**
 * Initialise this Listener.
 *
 * The event call-back function will be invoked when a listened-to Event is
 * signalled.
 *
 * \protected \memberof utils_EventListener
 *
 * \note
 * Implemented as a macro.
 */
#ifdef DOXYGEN_ONLY
extern void utils_EventListener_init(
    utils_EventListener *l,
    utils_EventCallBack cb
);
#else /* DOXYGEN_ONLY */
#define utils_EventListener_init(l, notify_cb) \
    ((void)((l)->notify = (notify_cb)))
#endif /* DOXYGEN_ONLY */

/*****************************************************************************
 * Private Types
 ****************************************************************************/

struct utils_Event {
    /** \private    The set of Listeners. */
   utils_Set listeners;
};

struct utils_EventListener
{
    /** \private    Listener set membership. */
    utils_SetMember membershipOfListeners;
    /** \private    Event notification callback. */
    utils_EventCallBack notify;
};

/*****************************************************************************
 * Private Function Declarations
 ****************************************************************************/

/**
 * Get address of this Event's Listener set.
 *
 * \private \memberof    utils_Event
 *
 * \note
 * Implemented as a macro.
 */
#ifdef DOXYGEN_ONLY
extern utils_Set *utils_Event_listeners(utils_Event *e);
#else /* DOXYGEN_ONLY */
#define utils_Event_listeners(e) \
    (&(e)->listeners)
#endif /* DOXYGEN_ONLY */

/**
 * Notifies EventListener of this Event's occurrence via its set membership.
 *
 * Known uses:-
 * - Passed as parameter to utils_Set_forEach by utils_Event_signal.
 *
 * \private \memberof    utils_Event
 */
extern void utils_Event_notifyListener(utils_SetMember *m);

/**
 * Get this Listener's listener set membership member.
 *
 * \private \memberof    utils_EventListener
 *
 * \note
 * Implemented as a macro.
 */
#ifdef DOXYGEN_ONLY
extern utils_SetMember *utils_EventListener_membershipOfListeners(
    utils_EventListener *l
);
#else /* DOXYGEN_ONLY */
#define utils_EventListener_membershipOfListeners(l) \
    (&(l)->membershipOfListeners)
#endif /* DOXYGEN_ONLY */

/*****************************************************************************
 * Private Function (Macro) Definitions
 ****************************************************************************/

/*****************************************************************************
 * Public Function (Macro) Definitions
 ****************************************************************************/

#endif /* UTILS_EVENT_H */
