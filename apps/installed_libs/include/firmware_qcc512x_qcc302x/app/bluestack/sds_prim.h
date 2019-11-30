/*!

Copyright (c) 2001 - 2016 Qualcomm Technologies International, Ltd.
  

\file   sdc_prim.h

\brief  This file contains Service Discovery Server specific type definitions.
        SDS Interface Primitives.

        The interface between the application and the SDS sub-system.

*/

#ifndef BLUESTACK__SDS_PRIM_H     /* Once is enough */
#define BLUESTACK__SDS_PRIM_H

/*============================================================================*
                    Header files on which this one depends
 *============================================================================*/
#include "bluestack/sdc_prim.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*
                       Interface #defined parameters
 *============================================================================*/

#define BT_BASE_UUID0  (0x00000000)
#define BT_BASE_UUID1  (0x00001000)
#define BT_BASE_UUID2  (0x80000080U)
#define BT_BASE_UUID3  (0x5f9b34fb)

#define SDS_RES_TABLE \
    SDS_RES_TABLE_X(SDS_SUCCESS,                        = 0x0000)               SDS_RES_TABLE_SEP \
    SDS_RES_TABLE_X(SDS_PENDING,                        = SDS_ERRORCODE_BASE)   SDS_RES_TABLE_SEP \
    SDS_RES_TABLE_X(SDS_ERROR_UNSPECIFIED,              SDS_RES_TABLE_DUMMY)    SDS_RES_TABLE_SEP \
    SDS_RES_TABLE_X(SDS_ERROR_INVALID_SR_DATA,          SDS_RES_TABLE_DUMMY)    SDS_RES_TABLE_SEP \
    SDS_RES_TABLE_X(SDS_ERROR_INVALID_SR_HANDLE,        SDS_RES_TABLE_DUMMY)    SDS_RES_TABLE_SEP \
    SDS_RES_TABLE_X(SDS_ERROR_INSUFFICIENT_RESOURCES,   SDS_RES_TABLE_DUMMY)

#define SDS_RES_TABLE_DUMMY
#define SDS_RES_TABLE_X(a, b) a b
#define SDS_RES_TABLE_SEP ,

typedef enum
{
    SDS_RES_TABLE
} SDS_RESPONSE_T;

#undef SDS_RES_TABLE_DUMMY
#undef SDS_RES_TABLE_X
#undef SDS_RES_TABLE_SEP


/*-------------------------------------------------------------
 Start of SDS primitives
---------------------------------------------------------------*/

#define SDS_PRIM_DOWN SDS_PRIM_BASE
#define SDS_PRIM_UP (SDS_PRIM_BASE | 0x0040)

/*-------------------------------------------------------------
 SDS Primitives
---------------------------------------------------------------*/

typedef enum sds_prim_tag
{
    ENUM_SDS_REGISTER_REQ = SDS_PRIM_DOWN,
    ENUM_SDS_UNREGISTER_REQ,
    ENUM_SDS_CONFIG_REQ,

    ENUM_SDS_REGISTER_CFM = SDS_PRIM_UP,
    ENUM_SDS_UNREGISTER_CFM,
    ENUM_SDS_CONFIG_CFM
} SDS_PRIM_T;

#define SDS_REGISTER_REQ              ((sds_prim_t)(ENUM_SDS_REGISTER_REQ))
#define SDS_REGISTER_CFM              ((sds_prim_t)(ENUM_SDS_REGISTER_CFM))
#define SDS_UNREGISTER_REQ            ((sds_prim_t)(ENUM_SDS_UNREGISTER_REQ))
#define SDS_UNREGISTER_CFM            ((sds_prim_t)(ENUM_SDS_UNREGISTER_CFM))
#define SDS_CONFIG_REQ                ((sds_prim_t)(ENUM_SDS_CONFIG_REQ))
#define SDS_CONFIG_CFM                ((sds_prim_t)(ENUM_SDS_CONFIG_CFM))

/*----------------------------------
 end of #define of primitives
------------------------------------*/

/*----------------------------------
 primitive type for sds
------------------------------------*/
typedef uint16_t        sds_prim_t;             /* same as l2cap_prim.h */

/*----------------------------------
 other parameter typedefs
------------------------------------*/

/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Request to register a service with the service discovery server.
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    sds_prim_t          type;           /* Always SDS_REGISTER_REQ */
    phandle_t           phandle;        /* routing handle */
    uint16_t            reg_ctx;        /* Reserved - shall be set to 0 */
    uint16_t            num_rec_bytes;  /* Number of bytes in the service */
                                        /* record data */
    uint8_t             *service_rec;   /* Pointer to service record data */
} SDS_REGISTER_REQ_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Confirm the registration of a service with the service discovery
 *      server.
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    sds_prim_t          type;           /* Always SDS_REGISTER_CFM */
    phandle_t           phandle;        /* routing handle */
    uint16_t            reg_ctx;        /* Reserved - shall be set to 0 */
    uint32_t            svc_rec_hndl;   /* ServiceRecordHandle of the */
                                        /* registered service */
    uint16_t            result;         /* Success or failure */
} SDS_REGISTER_CFM_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Request to unregister a service with the service discovery server.
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    sds_prim_t          type;           /* Always SDS_UNREGISTER_REQ */
    phandle_t           phandle;        /* routing handle */
    uint32_t            svc_rec_hndl;   /* ServiceRecordHandle of the */
                                        /* registered service */
} SDS_UNREGISTER_REQ_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Confirm the unregistration of a service with the service discovery
 *      server.
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    sds_prim_t          type;           /* Always SDS_UNREGISTER_CFM */
    phandle_t           phandle;        /* routing handle */
    uint16_t            reg_ctx;        /* Reserved - shall be set to 0 */
    uint32_t            svc_rec_hndl;   /* ServiceRecordHandle of the */
                                        /* unregistered service */
    uint16_t            result;         /* Success or failure */
} SDS_UNREGISTER_CFM_T;

/*----------------------------------------------------------------------------*
 * PURPOSE
 *       Configure SDP parameters
 *
 *       Apply L2CAP MTU and configure optional behaviour.
 *
 *       Currently there is no optional behaviour to configure,
 *       so all bits in the flags bitfield shall be 0.
 *
 *       If you just want to set optional behaviour and leave the L2CAP MTU
 *       unchanged then set the MTU to 0, which will always be ignored.
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    sds_prim_t          type;       /* Always SDS_CONFIG_REQ */
    phandle_t           phandle;    /* routing handle */
    uint16_t            l2cap_mtu;  /* L2CAP MTU value */
    uint16_t            flags;      /* Bitfield - currently all bits reserved (zero) */
} SDS_CONFIG_REQ_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *      Response to SDS_CONFIG_REQ
 *
 *      Returns the values of l2cap_mtu and flags in effect as a result of
 *      the SDS_CONFIG_REQ. If SDS_CONFIG_REQ was completely successful then
 *      the values of l2cap_mtu and flags will match those in SDS_CONFIG_REQ.
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
    sdc_prim_t          type;       /* Always SDS_CONFIG_CFM */
    phandle_t           phandle;    /* routing handle */
    uint16_t            l2cap_mtu;  /* Actual L2CAP MTU value used */
    uint16_t            flags;      /* Bitfield - currently all bits reserved (zero) */
} SDS_CONFIG_CFM_T;


/*----------------------------------------------------------------------------*
 * PURPOSE
 *              Common elements of "upstream" SDS primitives
 *
 *---------------------------------------------------------------------------*/
typedef SDC_SDS_UPSTREAM_COMMON_T SDS_UPSTREAM_COMMON_T;

/*------------------------------------------------
 Union of the primitives
-------------------------------------------------*/

typedef union
{
    sds_prim_t              type;
    SDS_UPSTREAM_COMMON_T   sds_upstream_common;

    SDS_REGISTER_REQ_T      sds_register_req;
    SDS_REGISTER_CFM_T      sds_register_cfm;
    SDS_UNREGISTER_REQ_T    sds_unregister_req;
    SDS_UNREGISTER_CFM_T    sds_unregister_cfm;
    SDS_CONFIG_REQ_T        sds_config_req;
    SDS_CONFIG_CFM_T        sds_config_cfm;
} SDS_UPRIM_T;

/*============================================================================*
 *                               PUBLIC FUNCTIONS
 *===========================================================================*/
/* None */

#ifdef __cplusplus
}
#endif

#endif /* ndef _SDS_PRIM_H */
