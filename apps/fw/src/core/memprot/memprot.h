/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
#ifndef MEMPROT_H_
#define MEMPROT_H_

/**
 * Set up memory protection handling.  This function does not have options
 * because callers (i.e. Apps main) aren't meant to worry about the details -
 * \c memprot knows what to do.
 *
 * This function sets up the interrupt handler for restricted memory accesses 
 * by P1. The default behaviour is to report details of the access into the 
 * firmware log and then clear the fault allowing P1 to continue
 */
extern void memprot_enable(void);


/**
 * Switch off the CPU1 access interrupt handling. 
 */
extern void memprot_disable(void);


/**
 * Update memory protection handling just before P1 starts. This is
 * delayed so that MIB values can be read and used.
 */
extern void memprot_update_protection(void);

#endif /* MEMPROT_H_ */
/*@}*/
