/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 *
 * Utilities associated with patching.
 */

#ifndef UTILS_PATCH_H
#define UTILS_PATCH_H

/**
 * Called by conman after patches are done.
 *
 * This has no other purpose than to be the first patch ID, so
 * we can guarantee what we are patching, and can also guarantee
 * that it will be run immediately.
 */
extern uint16 utils_patch_init(void);


#endif /* UTILS_PATCH_H */
