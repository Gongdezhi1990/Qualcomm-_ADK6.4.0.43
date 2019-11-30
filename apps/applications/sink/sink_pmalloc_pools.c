/*
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.
*/

/*!
@file sink_pmalloc_pools.c
@ingroup sink_app
*/

#include <pmalloc.h>

#include "sink_pmalloc_pools.h"

#ifdef ENABLE_BROADCAST_AUDIO

#include "broadcast.h"

#endif /* ENABLE_BROADCAST_AUDIO */

#ifdef ACTIVITY_MONITORING

#include "sport_health_hub.h"

#endif /* ACTIVITY_MONITORING */

#ifdef ENABLE_AMA

#include "ama.h"

#endif /* ENABLE_AMA */
    
#ifdef ENABLE_BISTO

#include "bisto_pmalloc_pools.h"

#endif /* ENABLE_BISTO */

_Pragma ("unitsuppress Unused")

_Pragma ("datasection apppool")

/* Define pmalloc pools. This definition will be merged at run time with the
   base definitions from Hydra OS - see 'pmalloc_config_P1.h'. */

static const pmalloc_pool_config app_pools[] =
{
    {   4, 14 },
    {   8, 25 },
    {  12, 17 },
    {  16,  7 },
    {  20, 22 },
    {  24, 15 },
    {  28, 55 },
    {  32, 21 },
    {  36, 19 },
    {  40, 10 },
    {  56,  9 },
    {  64,  7 },
    {  80,  9 },
    { 120, 16 },
    { 140,  4 },
    { 180,  6 },
    { 220,  3 },
    { 288,  1 }, 
    { 512,  1 },
    { 692,  3 },
    
#ifdef ENABLE_BROADCAST_AUDIO /* Add extra BA pool configuration. */

PMALLOC_BROADCAST_EXTRAS

#endif /* ENABLE_BROADCAST_AUDIO */

#ifdef ACTIVITY_MONITORING /* Add extra Activity Monitoring pool configuration. */

PMALLOC_SPORT_HEALTH_EXTRAS

#endif /* ACTIVITY_MONITORING */

#ifdef ENABLE_AMA /* Add extra AMA pool configuration. */

PMALLOC_AMA_EXTRAS

#endif /* ENABLE_AMA */
    
#ifdef ENABLE_BISTO /* Add extra Bisto pool configuration. */

PMALLOC_BISTO_EXTRAS

#endif /* ENABLE_BISTO */

};

/****************************************************************************
NAME
    sink_pmalloc_init

DESCRIPTION
    Configure the pmalloc pools.
    
    The linker will discard any files in an archive library that do not contain
    an externally-referenced symbol. This function is used to guarantee that the 
    architecture-specific app_pools structure is included in the final executable
    when needed.
     
 */
void sink_pmalloc_init(void)
{
    /* Do nothing. */
}
