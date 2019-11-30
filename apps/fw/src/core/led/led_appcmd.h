/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file 
 * Private header file for led appcmd interface.
 */

#ifndef LED_APPCMD_H
#define LED_APPCMD_H

#include "hydra/hydra_types.h"
#include "hydra/hydra_macros.h"
#include "hydra_log/hydra_log.h"
#include "sched/sched.h"
#include "appcmd/appcmd.h"


/**
 * Helper macro for reading bit fields from appcmd packets.
 */
#define read(s, p) ((s==1)?(*((uint8 *)(p))): \
                              ((s==2)?(*((uint16 *)(p))):((*((uint32 *)(p))))))
/**
 * Helper macro for writing bit fields in appcmd packets.
 */
#define write(s, p, v) ((s==1)?(*((uint8 *)(p)) = (uint8)((v) & 0xff)): \
                       ((s==2)?(*((uint16 *)(p)) = (uint16)((v) & 0xffff)): \
                       (*((uint32 *)(p)) = (uint32)((v) & 0xffffffffUL))))

/** 
 * Helper macro for getting and getting packet fields. 
 */
#define appcmd_get_value(module, field, buffer_ptr) \
    read(module ## _APPCMD_ ## field ## _SIZE, \
         ((uint8 *)buffer_ptr + module ## _APPCMD_ ## field ## _LSB_POSN))
/** 
 * Helper macro for setting a packet field. 
 */
#define appcmd_set_value(module, field, buffer_ptr, v) \
    write(module ## _APPCMD_ ## field ## _SIZE, \
          ((uint8 *)buffer_ptr + module ## _APPCMD_ ## field ## _LSB_POSN), v)

/**
 * Macro which generates the required enum members for appcmd packets. 
 */
#define field(module, name, size, prev) \
    module ## _APPCMD_ ## name ## _LSB_POSN = \
                                  ((module ## _APPCMD_ ## prev ## _SIZE) + \
                                   (module ## _APPCMD_ ## prev ## _LSB_POSN)),\
    module ## _APPCMD_ ## name ## _SIZE = size 

    #define led_field(name, size, prev) field(LED, name, size, prev)

/**
 * Enumeration which defines the bit field sizes and their position in an 
 * appcmd packet. These are chained together to calculate the positions based
 * on the previous field. 
 * Example: field(PARAMS_COMMAND, 1, PARAMS_TEST_ID) 
 * The field named PARAMS_COMMAND follows PARAMS_TEST_ID and occupies 1 byte.
 * Care must be taken with alignment since kalimba does not support unaligned 
 * memory accesses.
 */
typedef enum led_appcmd_fields
{
    LED_APPCMD_PARAMS_TEST_ID_LSB_POSN = -4,
    LED_APPCMD_PARAMS_TEST_ID_SIZE = 4,
    led_field(PARAMS_COMMAND, 1, PARAMS_TEST_ID),
    /* LED_APPCMD_INIT */
    led_field(PARAMS_INIT, 1, PARAMS_COMMAND),
    led_field(PARAMS_CLK_MUX, 1, PARAMS_INIT),
    /* LED_APPCMD_CONFIGURE */
    led_field(PARAMS_INITSTATE, 1, PARAMS_COMMAND),
    /* LED_APPCMD_SYNC */
    led_field(PARAMS_INSTANCES, 2, PARAMS_INITSTATE),
    led_field(PARAMS_PINCONF, 1, PARAMS_INSTANCES),
    led_field(PARAMS_SSEN, 1, PARAMS_PINCONF),
    led_field(PARAMS_LOGEN, 1, PARAMS_SSEN),
    led_field(PARAMS_LOG_OFFSET_LO_HI, 1, PARAMS_LOGEN),
    led_field(PARAMS_MIN_LO, 2, PARAMS_LOG_OFFSET_LO_HI),
    led_field(PARAMS_MIN_HI, 2, PARAMS_MIN_LO),
    led_field(PARAMS_MAX_LO, 2, PARAMS_MIN_HI),
    led_field(PARAMS_MAX_HI, 2, PARAMS_MAX_LO),
    led_field(PARAMS_HOLD_LO, 2, PARAMS_MAX_HI),
    led_field(PARAMS_HOLD_HI, 2, PARAMS_HOLD_LO),
    led_field(PARAMS_RAMP_CUR_LO, 2, PARAMS_HOLD_HI),
    led_field(PARAMS_RAMP_CUR_HI, 2, PARAMS_RAMP_CUR_LO),
    led_field(PARAMS_SS_LO, 2, PARAMS_RAMP_CUR_HI),
    led_field(PARAMS_SS_HI, 2, PARAMS_SS_LO),
    led_field(PARAMS_RAMP, 2, PARAMS_SS_HI),
    led_field(PARAMS_COUNT_HOLD, 2, PARAMS_RAMP),
    /* LED_APPCMD_ENABLE */
    led_field(PARAMS_EN, 1, PARAMS_INSTANCES),
    /* LED_APPCMD_RESET */
    
    /* LED_APPCMD_MAX_INSTANCES */
    LED_APPCMD_RESULTS_MAX_INSTANCES_LSB_POSN = 0,
    LED_APPCMD_RESULTS_MAX_INSTANCES_SIZE = 4
}led_appcmd_fields;


/**
 * Installs the led appcmd handler.
 * \return Whether the handler was successfully added.
 */
bool led_install_appcmd_handler(void);

/**
 * Appcmd handler function. This is the interface to appcmd and handler of 
 * appcmd messages. 
 * \param test_id The internal appcmd test ID.
 * \param params Pointer to parameters buffer. Here it is used as a message.
 * \param results Pointer to results buffer. Here it is used as a message.
 * \return Standard appcmd status code.
 */
APPCMD_RESPONSE led_appcmd_handler (APPCMD_TEST_ID test_id, uint32 *params,
                                                              uint32 *results);

#endif /* LED_APPCMD_H */
