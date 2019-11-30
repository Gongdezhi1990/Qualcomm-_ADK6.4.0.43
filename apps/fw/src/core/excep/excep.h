/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
#ifndef EXCEP_H_
#define EXCEP_H_

/**
 * Set up memory exception handling.  This function does not have options
 * because callers (i.e. Apps main) aren't meant to worry about the details -
 * \c excep knows what to do.
 *
 * This function simply enables the default exception handling and sets it up
 * to fire a special interrupt which panics, reporting the error type in the
 * diatribe.  Note that we don't use the explicit memory region access controls
 * as the DM ones aren't very useful and the PM one gives us less information
 * than the ad hoc software method.
 */
extern void excep_enable(void);

/**
 * Called from the crt to indicate that this wasn't a real start but was instead
 * likely to have been an erroneous attempt to execute code at address zero.
 * This function calls a panic that will be communicated to the curator so the
 * subsystem can be restarted if necessary.
 */
extern void excep_branch_through_zero(void);

/**
 * Switch off the exception handling.  The configuration will remain set, but
 * the enable bit will be cleared.
 */
extern void excep_disable(void);

#endif /* EXCEP_H_ */
/*@}*/
