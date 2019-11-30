/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Intrusive Singly Linked List (interface).
 */

#ifndef UTILS_SLL_H
#define UTILS_SLL_H

/*****************************************************************************
 * Interface Dependencies
 ****************************************************************************/

#include "hydra/hydra_types.h"

/*****************************************************************************
 * Public Type Declarations
 ****************************************************************************/

/**
 * Intrusive Singly Linked List
 *
 * Features:-
 * - Supports dynamic, heterogeneous, membership (M).
 * - Storage = (1 + M) * dataPtr.
 * - Not thread safe (wrap or specialise if required).
 * - It is an error to attempt to add a ListMember to any List more than once.
 * - It is an error to attempt to add a ListMember to more than one List
 * at a time.
 * - It is an error to attempt to remove a ListMember from a List it is not a
 * member of.
 *
 * Known Uses:-
 * - Implementation of utils_set
 *
 * Potential Uses:-
 * - Implementation of utils_fifo
 * - Implementation of utils_lifo.
 */
typedef struct utils_SLL utils_SLL;

/**
 * Intrusive SLL Member (Base)
 *
 * Objects should contain one of these for each set they may wish to join.
 *
 * \see Example use in utils_SLL.
 */
typedef struct utils_SLLMember utils_SLLMember;

/**
 * Intrusive SLL Function
 *
 * Function signature used as argument to utils_SLL_forEach()
 * allowing a simple function to be invoked on each member of a SLL.
 *
 * \see utils_SLL for example application.
 */
typedef void (*utils_SLLFunction)(utils_SLLMember *m);

/**
 * Intrusive SLL Functor (Abstract Base)
 *
 * Used as argument to utils_SLL_visitEach() allowing a
 * functor (context + function) to be invoked on each member.
 *
 * This is useful in any situation where a specific context is needed when
 * processing each member. E.g. for implementing aggregate functions (such
 * as sum, max, min).
 *
 * \see utils_SLL_visitEach()
 */
typedef struct utils_SLLFunctor utils_SLLFunctor;

/**
 * SLLFunctor Callback Function.
 */
typedef void (*utils_SLLFunctorFunction)(utils_SLLFunctor *f,
                                                          utils_SLLMember *m);

/*****************************************************************************
 * Public Function Declarations
 ****************************************************************************/

/**
 * Initialise this SLL.
 *
 * \public \memberof utils_SLL
 *
 * \post utils_SLL_isEmpty(s)
 *
 * \note
 * Implemented as a macro.
 */
#ifdef DOXYGEN_ONLY
extern void utils_SLL_init(utils_SLL *s);
#else /* DOXYGEN_ONLY */
#define utils_SLL_init(s) \
    ((void)((s)->first = NULL))
#endif /* DOXYGEN_ONLY */

/**
 * Is this set empty?
 *
 * \public \memberof utils_SLL
 *
 * \note
 * Implemented as a macro.
 */
#ifdef DOXYGEN_ONLY
extern bool utils_SLL_isEmpty(utils_SLL *s);
#else /* DOXYGEN_ONLY */
#define utils_SLL_isEmpty(s) \
    (NULL == (s)->first)
#endif /* DOXYGEN_ONLY */

/**
 * Return the member at the head of this SLL.
 *
 * \public \memberof utils_SLL
 *
 * \note
 * Implemented as a macro.
 */
#ifdef DOXYGEN_ONLY
extern void utils_SLL_head(utils_SLL *s);
#else /* DOXYGEN_ONLY */
#define utils_SLL_head(s) \
    ((s)->first)
#endif /* DOXYGEN_ONLY */

/**
 * Does this SLL contain the specified Member?
 *
 * \public \memberof utils_SLL
 *
 * \note
 * Implemented as a macro.
 */
#ifdef DOXYGEN_ONLY
extern bool utils_SLL_contains(utils_SLL *s, utils_SLLMember *m);
#else /* DOXYGEN_ONLY */
#define utils_SLL_contains(s,m) \
    (NULL != utils_SLL_findLink(s,m))
#endif /* DOXYGEN_ONLY */

/**
 * Add a new member at head of this SLL.
 *
 * Order(1)
 *
 * \public \memberof utils_SLL
 *
 * \pre !utils_SLL_contains(s,m)
 * \post utils_SLL_contains(s,m)
 */
extern void utils_SLL_prepend(utils_SLL *s, utils_SLLMember *m);

/**
 * Add a new member at tail of this SLL.
 *
 * Order(N)
 *
 * \public \memberof utils_SLL
 *
 * \pre !utils_SLL_contains(s,m)
 * \post utils_SLL_contains(s,m)
 */
extern void utils_SLL_append(utils_SLL *s, utils_SLLMember *m);

/**
 * Remove head of this SLL (or NULL if empty)
 *
 * Order(1)
 *
 * \public \memberof utils_SLL
 */
extern utils_SLLMember *utils_SLL_removeHead(utils_SLL *s);

/**
 * Remove tail of this SLL (or NULL if empty)
 *
 * Order(N)
 *
 * \public \memberof utils_SLL
 */
extern utils_SLLMember *utils_SLL_removeTail(utils_SLL *s);

/**
 * Remove an existing member from this SLL (wherever it is in the list)
 *
 * Order(n)
 *
 * \note For the avoidance of doubt removing a member from an SLL does not free
 * any storage associated with the member. SLL membership is totally
 * independent of storage management. The SLL has no idea if a member's storage
 * is dynamically or statically allocated. This is a key feature
 * of this intrusive SLL - it allows dynamic lists of statically allocated
 * members and has no dependency on a dynamic allocator at all.
 *
 * \public \memberof utils_SLL
 * \pre utils_SLL_contains(s,m)
 * \post !utils_SLL_contains(s,m)
*/
extern void utils_SLL_remove(utils_SLL *s, utils_SLLMember *m);

/**
 * Apply the specified function to each member of this SLL.
 *
 * The specified function will be called for each member of this set.
 *
 * \public \memberof utils_SLL
 */
extern void utils_SLL_forEach(const utils_SLL *s, utils_SLLFunction op);

/**
 * Apply the specified functor to each member of this SLL.
 *
 * The functor's "visit" function will be called for each member of this set.
 * The functor's own address is passed back as context, along with the current
 * member's address.
 *
 * \see                 utils_SLLFunctor for more info.
 *
 * \public \memberof    utils_SLL
 */
extern void utils_SLL_visitEach(const utils_SLL *s, utils_SLLFunctor *v);

/**
 * Initialise this SLLFunctor.
 *
 * \protected \memberof utils_SLLFunctor
 *
 * \note
 * Implemented as a macro.
 */
#ifdef DOXYGEN_ONLY
extern void utils_SLLFunctor_init(
    utils_SLLFunctor *f,
    utils_SLLFunctorFunction visitCallback
);
#else /* DOXYGEN_ONLY */
#define utils_SLLFunctor_init(f, visitCallback) \
    ((void)((f)->visitMember = (visitCallback)))
#endif /* DOXYGEN_ONLY */

/*****************************************************************************
 * Private Type Definitions
 ****************************************************************************/

struct utils_SLL
{
    /**
     * \private    First member of SLL.
     */
    utils_SLLMember *first;
};

struct utils_SLLMember
{
    /**
     * \private    Next member of SLL.
     */
    utils_SLLMember *next;
};

struct utils_SLLFunctor
{
    /**
     * \private   SLLMember-visiting callback function.
     *
     * This function is invoked by utils_SLL_visitEach for each member of
     * the set.
     */
    utils_SLLFunctorFunction visitMember;
};

/*****************************************************************************
 * Private Function Declarations
 ****************************************************************************/

/**
 * Returns the address of the _link_ pointing to SLL member or NULL if no
 * such member.
 *
 * Used internally for list traversal by list management functions.
 *
 * \private \memberof     utils_SLL
 */
extern utils_SLLMember **utils_SLL_findLink(
    utils_SLL *s,
    const utils_SLLMember *m
);

/*****************************************************************************
 * Public Function Definitions
 ****************************************************************************/

#endif /* UTILS_SLL_H */
