/*!
\copyright  Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_handset_signalling.h
\brief	    Interface to module providing signalling to handset.
*/

#ifndef AV_HEADSET_HANDSET_SIGNALLING_H
#define AV_HEADSET_HANDSET_SIGNALLING_H

#include "av_headset.h"
#include "av_headset_role.h"
#include "av_headset_phy_state.h"

/*! Handset signalling module state. */
typedef struct
{
    /*! Handset signalling module task. */
    TaskData task;
    
    /* 
     * AVRCP signalling state.
     */
    /*! reference used to communicate using AVRCP with handset via the AV module. */
    avInstanceTaskData* av_inst;
    /*! flag that we're waiting for an AVRCP connection. */
    uint16 waiting_avrcp;
    /*! Type of AVRCP operation in progress. */
    uint16 current_avrcp_op;
    /*! Task to respond to with result of replace earbud operation. */
    Task replace_earbud_client_task;
    
    /* 
     * HFP signalling state.
     */
    /*! Handle when using HFP APIs */
    hfp_link_priority hfp_priority;
    /*! flag that we're waitingi for an SLC connection. */
    uint16 waiting_hfp_slc;
    /*! Type of current AT command operation. */
    uint16 current_slc_op;

    /*! Is the charger attached or not. */
    bool charger_attached;
    /*! Latest physical state of the earbud. */
    phyState current_phy_state;

} handsetSigTaskData;

/*! Handset signalling status codes. */
typedef enum
{
    /*! Operation succeeded. */
    handsetSigStatusSuccess,
    /*! Operation failed. */
    handsetSigStatusFail,
    /*! Failed to send Earbud State to handset. */
    handsetSigStatusEarbudStateFail,
    /*! Failed to send Earbud Role to handset. */
    handsetSigStatusEarbudRoleFail,
    /*! Failed to send Replace Earbud message to handset. */
    handsetSigStatusReplaceEarbudFail,
    /*! Failed to send Replace Earbud message to handset, as there is already
     *  an operation of this type in progress. */
    handsetSigStatusReplaceEarbudFailInProgress
} handsetSigStatus;

/*! Type of transport to use. */
typedef enum
{
    /*! Send this message using AVRCP */
    OPTYPE_AVRCP,
    /*! Send this message using AT commands over HFP SLC */
    OPTYPE_AT
} handsetSigOpType;

/*! Earbud states, as reported in Earbud State messages to handset. */
typedef enum
{
    /*! Powered by battery and discharging. */
    EARBUD_CHARGING_STATE_DISCHARGING,
    /*! Powered by battery and charging. */
    EARBUD_CHARGING_STATE_CHARGING,
    /*! Power fault detected. */
    EARBUD_CHARGING_STATE_FAULT
} handsetSigChargingState;

/*! Messages sent to clients of the handset signalling module. */
enum av_headset_handset_signalling_messages
{
                /*! Message confirming that peer earbud has been replaced */
    HANDSET_SIG_REPLACE_EARBUD_CFM = HANDSET_SIG_MESSAGE_BASE,
};

/*! Message sent to client with result of operation to send Replace Earbud message to handset. */
typedef struct
{
    handsetSigStatus status;        /*!< Signalling status */
} HANDSET_SIG_REPLACE_EARBUD_CFM_T;

/*! Internal messages used by the handset signalling module. */
typedef enum
{
    HANDSET_SIG_INTERNAL_EARBUD_PHYSTATE_AVRCP_REQ,
    HANDSET_SIG_INTERNAL_EARBUD_ROLE_AVRCP_REQ,
    HANDSET_SIG_INTERNAL_EARBUD_PHYSTATE_HFP_REQ,
    HANDSET_SIG_INTERNAL_EARBUD_ROLE_HFP_REQ,
    HANDSET_SIG_INTERNAL_REPLACE_EARBUD_REQ,
    HANDSET_SIG_INTERNAL_AT_MIC_QUALITY_REQ,
    HANDSET_SIG_INTERNAL_AT_CHARGING_STATE_REQ,
    HANDSET_SIG_INTERNAL_AT_MIC_DELAY_REQ,
    HANDSET_SIG_INTERNAL_AT_DSP_STATUS_REQ,
};

/*! Internal message to action sending of Earbud physical state to handset. */
typedef struct
{
    handsetSigOpType transport;     /*!< The transport to send the state over */
    phyState state;                 /*!< The earbud physical state */
} HANDSET_SIG_INTERNAL_EARBUD_PHYSTATE_REQ_T;

/*! Internal message to action sending of Earbud role to handset. */
typedef struct
{
    handsetSigOpType transport;     /*!< The transport to send the role over */
    earbudRole role;                /*!< The role to send */
} HANDSET_SIG_INTERNAL_EARBUD_ROLE_REQ_T;

/*! Message definition for action to send Replace Earbud message to handset. */
typedef struct
{
    bdaddr new_earbud;                  /*!< Address of replacement earbud for handset */
} HANDSET_SIG_INTERNAL_REPLACE_EARBUD_REQ_T;

/*! Message definition for action to send AT MIC Quality message to handset. */
typedef struct
{
    uint8 mic_quality;                  /*!< Mic quality value to send to handset */
} HANDSET_SIG_INTERNAL_AT_MIC_QUALITY_REQ_T;

/*! Message definition for action to send AT Charging State message to handset. */
typedef struct
{
    handsetSigChargingState charging_state;     /*!< charging state to send to handset */
    uint8 battery_level_percent;                /*!< battery level to send to handet */
} HANDSET_SIG_INTERNAL_AT_CHARGING_STATE_REQ_T;

/*! Message definition for action to send AT MIC Delay message to handset. */
typedef struct
{
    uint16 mic_delay_us;        /*!< Mic delay to be reported in &mu;s */
} HANDSET_SIG_INTERNAL_AT_MIC_DELAY_REQ_T;

/*! Message definition for action to send AT DSP Status message to handset. */
typedef struct
{
    uint8 dsp_status;           /*!< Status of the feature requested i.e. #AT_TWS_DSP_FEATURE_NR */
} HANDSET_SIG_INTERNAL_AT_DSP_STATUS_REQ_T;

/*! @brief Initialise the handset signalling module.
 */
void appHandsetSigInit(void);

/*! @brief Request an AVRCP_TWS_EARBUD_STATE message is sent to the handset.
 
    Client do not receive confirmation of this message, it is a notification
    of current state only. If a subsequent call to this API is made before
    the previous state has been sent, the previous will be deleted and
    replaced with the latest state.

    @param[in] state The current physical state of the earbud.
 */
void appHandsetSigSendEarbudStateReq(phyState state);

/*! @brief Request an AVRCP_TWS_EARBUD_ROLE message is sent to the handset.

    Behaviour as for appHandsetSigSendEarbudStateReq()

    @param[in] role The role of the earbud.
 */
void appHandsetSigSendEarbudRoleReq(earbudRole role);

/*! @brief Request an AVRCP_TWS_REPLACE_EARBUD message is sent to the handset.
    
    Only a single AVRCP_TWS_REPLACE_EARBUD message can be in progress at one time.
    Following a successful call to this API clients must wait to receive a
    #HANDSET_SIG_REPLACE_EARBUD_CFM message before requesting another. If called
    before receipt of this message clients will receive an error code of
    handsetSigStatusReplaceEarbudFailInProgress.

    @param[in] task  The task to which #HANDSET_SIG_REPLACE_EARBUD_CFM message will be sent.
    @param[in] new_earbud  Pointer to BT address of replacement earbud.

    @return handsetSigStatus handsetSigStatusSuccess message queued to be sent.
                             handsetSigStatusReplaceEarbudFailInProgress request rejected.
 */
handsetSigStatus appHandsetSigSendReplaceEarbudReq(Task task, const bdaddr* new_earbud);

/*! @brief Handle request to transmit mic quality AT command to handset.
 
    @param[in] mic_quality Microphone quality value 0 (worst) to 15 (best). 
 
    @return handsetSigStatus handsetSigStatusSuccess message queued to be sent.
                             handsetSigStatusFail request rejected.
 */
handsetSigStatus appHandsetSigSendMicQualityReq(uint8 mic_quality);

/*! @brief Handle request to transmit battery charging state AT command to handset.
 
    @param[in] charging_state 0.255
                               0 Earbud is powered by the battery, discharging.
                               1 Earbud is powered by the battery, charging.
                               2 Earbud power fault detected.
                               3..255 RFU
    @param[in] battery_level_percent Values in the range 0 to 100 represent the
    battery level as a percentage:
        0 represents a battery that is fully discharged;
        100 represents a battery that is fully charged.
        101 to 255 = RFU

 
    @return handsetSigStatus handsetSigStatusSuccess message queued to be sent.
                             handsetSigStatusFail request rejected.
 */
handsetSigStatus appHandsetSigSendChargingStateReq(handsetSigChargingState charging_state,
                                                   uint8 battery_level_percent);

/*! @brief Handle request to transmit mic delay AT command to handset.
 
    @param[in] mic_delay Earbud microphone path delay variation, 0..65535 (us).

    @return handsetSigStatus handsetSigStatusSuccess message queued to be sent.
                             handsetSigStatusFail request rejected.
 */
handsetSigStatus appHandsetSigSendMicDelayReq(uint16 mic_delay);

/*! @brief Handle request to transmit DSP status AT command to handset.
 
    @param[in] dsp_status Status of Earbud DSP features.
                           Bitmask where 0 disabled and 1 enabled.
                           Bit 0 : Noise Reduction (NR)
                           Bit 1 : Echo Cancellation (EC)

    @return handsetSigStatus handsetSigStatusSuccess message queued to be sent.
                             handsetSigStatusFail request rejected.
 */
handsetSigStatus appHandsetSigSendDspStatusReq(uint8 dsp_status);

#endif /* AV_HEADSET_HANDSET_SIGNALLING_H */

