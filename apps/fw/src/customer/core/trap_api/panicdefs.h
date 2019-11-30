/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Reason codes for application failure.
 */

#ifndef PANICDEFS_H
#define PANICDEFS_H 

/**
 * Type definition for application panics.
 */
typedef enum panicdefs
{
    PANIC_APP_NONE = 0,
    PANIC_APP_GENERIC = 1,
    PANIC_APP_POINTER_IS_NULL = 2,
    PANIC_APP_POINTER_IS_NOT_NULL = 3
} panicdefs;

#endif /* PANICDEFS_H */ 
