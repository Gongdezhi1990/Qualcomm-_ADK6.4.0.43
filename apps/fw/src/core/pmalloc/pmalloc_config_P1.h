/* Copyright (c) 2016 - 2018 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Internal header file holding the fixed pool configurations
 * known for each subsystem on processor P1 builds
 * This configuration provides the *minimum* allocation, which 
 * is topped up with sane allocations for the remaining heap space
 * for each module or subsystem, feel free to edit *your* allocation.
 * The pool size put in needs to be multiple of 4.
 */
#ifndef PMALLOC_CONFIG_H
#define PMALLOC_CONFIG_H

/*****************************************************************************
 * DEFAULT MEMORY POOL CONFIGURATION
 *****************************************************************************/
static const pmalloc_pool_config pools_reqd[] =
{ 
    /* Minimal set of pools required to boot. Other pools are under the
     * control of the application OS library.
     */
    {  4, 4 },
    {  8, 2 },
    { 12, 2 },
    { 16, 8 },
    { 20, 2 },
    { 28, 8 },
    { 36, 2 }
};
  
#endif /* PMALLOC_CONFIG_H */
