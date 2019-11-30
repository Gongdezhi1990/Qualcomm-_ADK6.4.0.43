/*************************************************************************
 * Copyright (c) 2018 Qualcomm Technologies International, Ltd.
 * All Rights Reserved.
 * Qualcomm Technologies International, Ltd. Confidential and Proprietary.
*************************************************************************/
/**
 * \defgroup extmem external memory manager 
 *
 * \file extmem.h
 * \ingroup extmem
 *
 * Public definitions for  extmem
 */
#ifndef _EXT_MEM_H_
#define _EXT_MEM_H_

/****************************************************************************
Include Files
*/
#include "types.h"


/* EXT_MEM Types */
typedef enum{
     EXTMEM_SPI_RAM,
     EXTMEM_UNKNOWN 
}EXTMEM_TYPE; 

/* EXT MEM Modes */
typedef enum{
      EXTMEM_OFF,   /* Turn OFF External memory */
      EXTMEM_ON,    /* Turn ON External memory with current clock setting*/
}EXTMEM_MODE;

/* Extrenal Memory clocks */
typedef enum{
       EXTMEM_CLK_PLL_OFF,      /* RAM and PLL are OFF */
       EXTMEM_CLK_OFF,          /* RAM is OFF */
       EXTMEM_CLK_32MHZ,        /* 32MHZ ext memory clock */
       EXTMEM_CLK_80MHZ         /* 80MHz ext memory clock */
}EXTMEM_CLK;


/**
 * Definition of callback type. Callback when an event change due to extmem_ctrl
 *
 * \param type External memory type
 * \param mode ON/OFF
 * \param clk  current external memory clock
 */
typedef void (*EXTMEM_CBACK)(EXTMEM_TYPE type, EXTMEM_MODE mode, EXTMEM_CLK clk);


/**
 *
 * \brief Power on or off the external memory. If it is already ON, this will 
 * return TRUE. If power on is pending this API return failure. To avoid this,
 * caller can grab the extmem lock before making this call. To avoid a low priority
 * task getting interrupted after acquiring exclusive lock and getting into a 
 * dealock situation due to some other higher priority task waitingg on the lock,
 * block the interrupts while acquiring the lock and doing the operation. 
 *
 * e.g: 
 *  bool done=FALSE;
 *  do{
 *        LOCK_INTERRUPTS;
 *        if((done=extmem_lock(EXTMEM_SPI_RAM, TRUE)))
 *           {
 *                 / * Do an external memory power on/power off * /
 *                if(!extmem_enable( EXTMEM_SPI_RAM, EXTMEM_ON/ EXTMEM_OFF, callback))
 *                {
 *                   / * unexpected failure - Panic/Fault * /
 *                }
 *                extmem_unlock(EXTMEM_SPI_RAM); 
 *           }
 *          UNLOCK_INTERRRUPTS;
 *     }while(!done)
 * 
 * \param type External memory type
 * \param mode power mode - ON or OFF
 * \param callback callback to return the enable status if present
 *
 * \return TRUE on initiating the operation.
 */   
extern bool extmem_enable(EXTMEM_TYPE type, EXTMEM_MODE mode, EXTMEM_CBACK cback);

/**
 * \brief This function re-attempts calling extme_enable for a finite time before 
 * returning the failure
 */
extern bool extmem_enable_with_retry(EXTMEM_TYPE type, EXTMEM_MODE mode,
                              EXTMEM_CBACK cback, TIME wait_time );


/**
 * \brief Grab an inusage lock. The exclusive lock is like a mutex and 
 * non-exclusive lock is like a countable semaphore.
 * Use the non-exclusive lock while doing power on or extCbuffer operations.
 * Use the exclusive lock while changing clock and doinh a power off.
 * 
 * e.g: 
 * bool done=FALSE;
 * do{
 *   LOCK_INTERRUPTS;
 *   if((done=extmem_lock(EXTMEM_SPI_RAM, TRUE)))
 *   {
 *       / * Do an external memory power on/power off / clock change * /
 *       extmem_unlock(EXTMEM_SPI_RAM); 
 *   }
 *   UNLOCK_INTERRRUPTS;
 * }while(!done)
 *
 * Use non-exclusive lock while doing an extCbuffer operation, so it will not
 * block other users to do 
 *
 * \param type External memory type
 * \param exclusive  Don't allow anyone else to use it.
 *
 * \return TRUE on successfully grabing the lock
 */   
extern bool extmem_lock(EXTMEM_TYPE type, bool exclusive);


/**
 * \brief  Release the  lock.
 *
 * \param type External memory type
 * 
 * \return TRUE on successfully grabing the lock
 */   
extern bool extmem_unlock(EXTMEM_TYPE type);

/**
 * \brief  Get the current mode
 *
 * \param type External memory type
 *
 * \return current mode
 */
extern EXTMEM_MODE extmem_get_mode(EXTMEM_TYPE type);


#endif /*_EXT_MEM_H_*/ 
