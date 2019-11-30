/****************************************************************************

        Copyright (c) 2015 - 2019 Qualcomm Technologies International, Ltd 

FILE
    operator_if.h

CONTAINS
    Definitions for the DSPManager subsystem from VM.

DESCRIPTION
    This file is seen by the stack, and VM applications, and
    contains things that are common between them.
*/

#ifndef __OPERATOR_IF_H__
#define __OPERATOR_IF_H__

/*!
    @brief DSP operator framework power state.
*/
typedef enum {
    MAIN_PROCESSOR_OFF = 0,    /*!< Power-off DSP main processor */
    MAIN_PROCESSOR_ON = 1,     /*!< Load DSP software on main processor */
    SECOND_PROCESSOR_OFF = 2,  /*!< Power-off DSP second processor */
    SECOND_PROCESSOR_ON = 3,   /*!< Power-on DSP second processor */
    MAIN_PROCESSOR_ONLY = 4    /*!< Disallow use of second processor */
} OperatorFrameworkPowerState;

/*!
    @brief key-value pair to specify certain parameters for 
    creating the operator via OperatorCreate() API.
*/
typedef struct
{
    uint16 key;         /*!< Key for OperatorCreate. */    
    uint32 value;       /*!< Value for the key. */
} OperatorCreateKeys;

/*!
    @brief DSP operator framework trigger notification type.
*/
typedef enum {
    TRIGGER_ON_VAD = 0,    /*!< Voice activity detection */
    TRIGGER_ON_VTD = 1,    /*!< Voice trigger phrase detection */
    TRIGGER_ON_VCI = 2,    /*!< Voice command interpreter */
    TRIGGER_ON_GM = 3      /*!< Graph Manager */
} OperatorFrameworkTriggerType;

/*!
    @brief Type of operator data file stored in file-system.
*/
typedef enum {
    DATAFILE_BIN = 1    /*!< ".bin" file type */
} OperatorDataFileType;


/*!
    @brief Type of storage used by audio-DSP framework for storing operator data.
*/
typedef enum {
    STORAGE_INTERNAL = 1,    /*!< Internal storage such as Internal RAM */
    STORAGE_EXTERNAL = 2     /*!< External storage such as SPI RAM */
} OperatorDataStorageType;

#endif /* __OPERATOR_IF_H__  */

