/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Intrusive Set interface.
 */

#ifndef UTILS_SET_H
#define UTILS_SET_H

/*****************************************************************************
 * Interface Dependencies
 ****************************************************************************/

#include "utils/utils_sll.h" /* singly linked list */

/*****************************************************************************
 * Public Type Declarations
 ****************************************************************************/

/**
 * Intrusive Set
 *
 * Features:-
 * - Supports dynamic, heterogeneous, membership (M).
 * - Storage = (1 + M) * dataPtr.
 * - Avoids under, over & dynamic storage allocation.
 * - Avoids preprocessor and preprocessing tricks.
 * - Not thread safe (wrap or specialise if required).
 * - No member ordering guarantees whatever (that would be a List not a Set).
 * - It is an error to attempt to add a SetMember to an Set more than once.
 * - It is an error to attempt to add a SetMember to more than one Set at
 * a time.
 * - It is an error to attempt to remove a SetMember from an Set it is not
 * a member of.
 *
 * Known Uses:-
 *
 * - Implementation of utils_Event
 *
 * Example use (module_ prefix omitted):-
 *\verbatim

    ** A potential set member *
    struct Apple
    {
       SetMember basketMembership; **< N.B. Intrusive Set membership *
       ...
    }

    ** Helpers *
    #define Apple_basketMembership(t)       (&(t)->basketMembership)
    #define Apple_fromBasketMembership(m)   \
                      ((Apple*)((char *)m - offsetof(Apple, basketMembership)))

    **
    * Operation to perform over the Apples in a Set
    * (Complies with SetOperator)
    *
    void Apple_bite(SetMember *m)
    {
       ** NB Recover Apple from reference to its set-membership member *
       Apple *a = Apple_fromBasketMembership(m);
       ...
    }

    ** Declare some apples (potential set members) and a basket (a set) *
    Apple a1, a2;
    Set basket;

    ** Put apples in the basket *
    ** NB. Added by reference to the set membership member *
    Set_add(&basket, Apple_basketMembership(&a1));
    Set_add(&basket, Apple_basketMembership(&a2));

    ** Bite each apple in the basket *
    Set_forEach(&basket, &Apple_bite);
\endverbatim
*/
typedef utils_SLL utils_Set;

/**
 * Intrusive Set Member (Base)
 *
 * Objects should contain one of these for each set they may wish to join.
 *
 * \see Example use in utils_Set.
 */
typedef utils_SLLMember utils_SetMember;

/**
 * Intrusive Set Function
 *
 * Function signature used as argument to utils_Set_forEach()
 * allowing a simple function to be invoked on each member of a Set.
 *
 * \see utils_Set for example application.
 */
typedef utils_SLLFunction utils_SetFunction;

/**
 * Intrusive Set Functor (Abstract Base)
 *
 * Used as argument to utils_Set_visitEach() allowing a
 * functor (context + function) to be invoked on each member.
 *
 * This is useful in any situation where a specific context is needed when
 * processing each member. E.g. for implementing aggregate functions (such
 * as sum, max, min).
 *
 * \see utils_Set_visitEach()
 *
 * \class utils_SetFunctor
 */
typedef utils_SLLFunctor utils_SetFunctor;

/**
 * SetFunctor Callback Function.
 */
typedef utils_SLLFunctorFunction utils_SetFunctorFunction;

/*****************************************************************************
 * Public Function Declarations
 ****************************************************************************/

/**
 * Initialise this Set.
 *
 * \public \memberof utils_Set
 *
 * \post utils_Set_isEmpty(s)
 *
 * \note
 * Implemented as a macro.
 */
#ifdef DOXYGEN_ONLY
extern void utils_Set_init(utils_Set *s);
#else /* DOXYGEN_ONLY */
#define utils_Set_init(s) utils_SLL_init(s)
#endif /* DOXYGEN_ONLY */

/**
 * Is this set empty?
 *
 * \public \memberof utils_Set
 *
 * \note
 * Implemented as a macro.
 */
#ifdef DOXYGEN_ONLY
extern bool utils_Set_isEmpty(utils_Set *s);
#else /* DOXYGEN_ONLY */
#define utils_Set_isEmpty(s) utils_SLL_isEmpty(s)
#endif /* DOXYGEN_ONLY */

/**
 * Does this Set contain the specified Member?
 *
 * \public \memberof utils_Set
 *
 * \note
 * Implemented as a macro.
 */
#ifdef DOXYGEN_ONLY
extern bool utils_Set_contains(utils_Set *s, utils_SetMember *m);
#else /* DOXYGEN_ONLY */
#define utils_Set_contains(s, m) utils_SLL_contains(s, m)
#endif /* DOXYGEN_ONLY */

/**
 * Add a new member to this Set.
 *
 * \public \memberof utils_Set
 *
 * \pre !utils_Set_contains(s,m)
 * \post utils_Set_contains(s,m)
 *
 * \note
 * Implemented as a macro.
 */
#ifdef DOXYGEN_ONLY
extern void utils_Set_add(utils_Set *s, utils_SetMember *m);
#else /* DOXYGEN_ONLY */
#define utils_Set_add(s, m) utils_SLL_prepend(s, m)
#endif /* DOXYGEN_ONLY */

/**
 * Remove an existing member from this Set.
 *
 * \public \memberof utils_Set
 *
 * \pre utils_Set_contains(s,m)
 * \post !utils_Set_contains(s,m)
 *
 * \note
 * Implemented as a macro.
 */
#ifdef DOXYGEN_ONLY
extern void utils_Set_remove(utils_Set *s, utils_SetMember *m);
#else /* DOXYGEN_ONLY */
#define utils_Set_remove(s, m) utils_SLL_remove(s, m)
#endif /* DOXYGEN_ONLY */

/**
 * Apply the specified function to each member of this Set.
 *
 * The specified function will be called for each member of this set.
 *
 * \public \memberof utils_Set
 *
 * \note
 * Implemented as a macro.
 */
#ifdef DOXYGEN_ONLY
extern void utils_Set_forEach(const utils_Set *s, utils_SetFunction op);
#else /* DOXYGEN_ONLY */
#define utils_Set_forEach(s, fn) utils_SLL_forEach(s, fn)
#endif /* DOXYGEN_ONLY */

/**
 * Apply the specified functor to each member of this Set.
 *
 * The functor's "visit" function will be called for each member of this set.
 * The functor's own address is passed back as context, along with the current
 * member's address.
 *
 * \see                 utils_SetFunctor for more info.
 *
 * \public \memberof    utils_Set
 *
 * \note
 * Implemented as a macro.
 */
#ifdef DOXYGEN_ONLY
extern void utils_Set_visitEach(const utils_Set *s, utils_SetFunctor *v);
#else /* DOXYGEN_ONLY */
#define utils_Set_visitEach(s, v) utils_SLL_visitEach(s, v)
#endif /* DOXYGEN_ONLY */

/**
 * Initialise this SetFunctor.
 *
 * \protected \memberof utils_SetFunctor
 *
 * \note
 * Implemented as a macro.
 */
#ifdef DOXYGEN_ONLY
extern void utils_SetFunctor_init(
    utils_SetFunctor *f,
    utils_SetFunctorFunction visitCallback
);
#else /* DOXYGEN_ONLY */
#define utils_SetFunctor_init(f, cb) utils_SLLFunctor_init(f, cb)
#endif /* DOXYGEN_ONLY */

/*****************************************************************************
 * Private Type Definitions
 ****************************************************************************/

/*****************************************************************************
 * Private Function Declarations
 ****************************************************************************/

/*****************************************************************************
 * Public Function Definitions
 ****************************************************************************/

#endif /* UTILS_SET_H */
