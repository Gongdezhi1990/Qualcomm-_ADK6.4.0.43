/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/*
 * \file
 *
 * Configuration for the dorm subsystem.
*/

#include "dorm/dorm_private.h"
#ifndef DORM_KIP_TABLE_DEFAULT
/** Use 0 for the externally accessible kip table entry if not set elsewhere */
#define DORM_KIP_TABLE_DEFAULT 0
#endif /* DORM_KIP_TABLE_DEFAULT */

volatile uint16 kip_table[DORMID_COUNT] = {DORM_KIP_TABLE_DEFAULT};
/* Flag indicating whether there is an outstanding wakeup request */
volatile uint16 dorm_wake_requested = 0;

#ifndef __KALIMBA__
/** To be called when the MIB has been set and the system fully initialised.
 */
void dorm_config(void)
{
    /* TBD: not clear anything is needed here for Hydra */
    dorm_cfg.initialised = TRUE;
}
#endif
