/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 */
#ifndef _UTILS_STRDUP_H_
#define _UTILS_STRDUP_H_

/**
 *  Returns a pointer to a new string which is a duplicate of the string
 *  \c in_str. Memory for the new string is obtained with pmalloc, and can
 *  be freed with pfree. If memory isn't available then pmalloc will panic.
 * \param in_str NULL terminated string to copy
 * \return A duplicate of \c in_str
 */
extern char * utils_strdup(const char * in_str);

#endif /* _UTILS_STRDUP_H_ */
