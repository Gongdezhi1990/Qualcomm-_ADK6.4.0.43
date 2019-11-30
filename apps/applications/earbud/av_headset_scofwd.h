/*!
\copyright  Copyright (c) 2015 - 2018 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       
\brief      Interface to implementation of SCO forwarding.

            This file contains code for management and use of an L2CAP link
            to the peer device, used for forwarding timestamped SCO audio.

    @startuml

    [*] -down-> NULL
    NULL : Initialising SCO forwarding module
    NULL -down-> INITIALISING : appScoFwdInit()

    INITIALISING : L2CAP and SDP registration
    INITIALISING --> SDP_SEARCH : appScoFwdConnectPeer()

    note "Common states" as N3 #PapayaWhip
    note "Master states" as N1 #LightGreen
    note "Slave states" as N2 #LightBlue

    SDP_SEARCH : Query SCO and MIC forward features of peer
    SDP_SEARCH --> IDLE : SDP search complete

    IDLE : ready for connections
    IDLE --> CONNECTING_MASTER : SFWD_INTERNAL_LINK_CONNECT_REQ
    IDLE --> CONNECTING_SLAVE : CL_L2CAP_CONNECT_IND
    CONNECTING_MASTER -[hidden]l-> CONNECTING_SLAVE

    state l2cap_connected_in_these_states {

        CONNECTING_MASTER : waiting to connect as link master
        CONNECTING_MASTER -d-> CONNECTED : CL_L2CAP_CONNECT_CFM

        CONNECTING_SLAVE : waiting to connect as link slave
        CONNECTING_SLAVE -d-> CONNECTED : CL_L2CAP_CONNECT_CFM

        CONNECTED : L2CAP link up, but no audio traffic
        CONNECTED --> CONNECTED_ACTIVE : SCO Connected (Not Forwarding)\nSlave Out Of Ear
        CONNECTED --> CONNECTED_ACTIVE_SEND : SCO Connected (Forwarding)\nSlave In Ear
        CONNECTED --> CONNECTED_ACTIVE_RECEIVE : OTA_SETUP (START_RX_CHAIN)
        CONNECTED_ACTIVE #LightGreen --> CONNECTED : SCO Disconnected
        CONNECTED_ACTIVE_SEND -u-> CONNECTED : SCO Disconnected

        CONNECTED_ACTIVE : L2CAP Link up and SCO audio on master, but not forwarding to slave
        CONNECTED_ACTIVE --> CONNECTED_ACTIVE_SEND : Start Forwarding\nSlave In Ear

        CONNECTED_ACTIVE_RECEIVE : Audio chain and link up - receiving data
        CONNECTED_ACTIVE_RECEIVE : Also sending MIC data to master with MICFWD enabled
        CONNECTED_ACTIVE_RECEIVE #LightBlue --> CONNECTED : OTA_TEARDOWN (STOP_RX_CHAIN)

        CONNECTED_ACTIVE_SEND : Audio chain and link up - sending data
        CONNECTED_ACTIVE_SEND : Also receiving MIC data from slave with MICFWD enabled
        CONNECTED_ACTIVE_SEND  #LightGreen --> CONNECTED_ACTIVE : Stop Forwarding\nSlave Out Of Ear

        l2cap_connected_in_these_states -u-> IDLE : CON_MANAGER_DISCONNECT_IND
        l2cap_connected_in_these_states -u-> IDLE : CL_L2CAP_DISCONNECT_IND
    }
    @enduml
*/

#ifndef _AV_HEADSET_SCO_FWD_H_
#define _AV_HEADSET_SCO_FWD_H_

#define SFWD_FEATURE_MIC_FWD    (1 << 0)
#define SFWD_FEATURE_RING_FWD   (1 << 1)

#define SFWD_STATE_LOCK_MASK    (1 << 4)

/*! \brief SCO Forwarding task state machine states */
typedef enum
{
    SFWD_STATE_NULL                 = 0,                        /*!< Startup state */
    SFWD_STATE_INITIALISING         = 1 + SFWD_STATE_LOCK_MASK, /*!< Awaiting L2CAP registration */
    SFWD_STATE_IDLE                 = 2,                        /*!< Initialised and ready for connections */
    SFWD_STATE_SDP_SEARCH           = 3 + SFWD_STATE_LOCK_MASK,
    SFWD_STATE_CONNECTING           = 4 + SFWD_STATE_LOCK_MASK,
    SFWD_STATE_CONNECTED            = 5,                        /*!< L2CAP link up, but no traffic */
        SFWD_STATE_CONNECTED_ACTIVE         = 6,                /*!< Chain supporting SCO forwarding active, but not forwarding */
        SFWD_STATE_CONNECTED_ACTIVE_RECEIVE = 7,                /*!< Chain supporting SCO forwarding active, receiving data */
        SFWD_STATE_CONNECTED_ACTIVE_SEND_PENDING_ROLE_IND = 8,
        SFWD_STATE_CONNECTED_ACTIVE_SEND    = 9,                /*!< Chain supporting SCO forwarding active, sending data */
    SFWD_STATE_DISCONNECTING        = 10 + SFWD_STATE_LOCK_MASK,
} scoFwdState;

#define appScoFwdIsConnectedActive(s) \
    (((s) >= SFWD_STATE_CONNECTED_ACTIVE) && ((s) <= SFWD_STATE_CONNECTED_ACTIVE_SEND))

/*! Rudimentary statistics about forwarded audio packets received over the L2CAP link
*/
typedef struct
{
    unsigned        lost_packets;               /*!< Number of incoming forwarded packets lost or late */
    uint32          packet_history;             /*!< Bit mask showing missed packets in the last 32 */
    bool            audio_missing;              /*!< Consecutive section of audio missing */
} scoFwdReceivedPacketStats;

/*! \brief SCO Forwarding task data
*/
typedef struct
{
    TaskData        task;
    uint16          lock;
    scoFwdState     state;                      /*!< Current state of the state machine */
    uint16          local_psm;                  /*!< L2CAP PSM registered */
    uint16          remote_psm;                 /*!< L2CAP PSM registered by peer device */
    uint16          pending_connects;

    Sink            link_sink;                  /*!< The sink of the L2CAP link */
    Source          link_source;                /*!< The source of the L2CAP link */

    Source          forwarding_source;          /*!< The chain output - SCO/MIC to transmit */
    Sink            forwarded_sink;             /*!< The chain input - received SCO/MIC */
    Sink            wallclock_sink;             /*!< The sink to be used for the wallclock synchronization */

    wallclock_state_t wallclock;                /*!< Structure giving us a common timebase with the peer */
    rtime_t         ttp_of_last_received_packet;/*!< Tracking information for packets over the air */ 

    bool            peer_incoming_call;         /*!< We are slave, and peer has incoming call */

    scoFwdReceivedPacketStats   stats;          /*!< Statistics for received packets */

    bool            vol_muted;                  /*!< SCOFWD muted the local volume */
#ifdef INCLUDE_SCOFWD_TEST_MODE
    unsigned        percentage_to_drop;         /*!< Percentage of packets to not transmit */
    int             drop_multiple_packets;      /*!< A negative number indicates the number of consecutive 
                                                     packets to drop. 0, or a positive number indicates the
                                                     percentage chance a packet should drop after a packet
                                                     was dropped. */
#endif
} scoFwdTaskData;


/*! \brief Message IDs from SCO Forwarding to main application task */
enum av_headset_scofwd_messages
{
    SFWD_INIT_CFM = SFWD_MESSAGE_BASE,

};


#define appScoFwdGetSink()  (appGetScoFwd()->link_sink)


extern void appScoFwdInit(void);

extern void appScoFwdConnectPeer(void);
extern void appScoFwdDisconnectPeer(void);

extern void appScoFwdInitScoPacketising(Source audio_source);

extern bool appScoFwdIsConnected(void);
extern bool appScoFwdIsDisconnected(void);
extern bool appScoFwdIsStreaming(void);
extern bool appScoFwdIsReceiving(void);
extern bool appScoFwdIsSending(void);

extern void appScoFwdNotifyIncomingSink(Sink sco_sink);

extern bool appScoFwdIsCallIncoming(void);

extern void appScoFwdCallAccept(void);
extern void appScoFwdCallReject(void);
extern void appScoFwdCallHangup(void);
extern void appScoFwdCallVoice(void);
extern void appScoFwdRing(void);
extern void appScoFwdRingCancel(void);
extern void appScoFwdVolumeStart(int16 step);
extern void appScoFwdVolumeStop(int16 step);

extern void appScoFwdClearForwarding(void);
extern void appScoFwdClearForwardingReceive(void);

/*! \brief Start forwarding SCO to slave earbud.
    The Earbud must have an active SCO for this to work.
*/
extern void appScoFwdEnableForwarding(void);

/*! \brief Stop forwarding SCO to slave earbud.
    Can only be called on the SCO forwarding master, and only
    effective if Earbud is already SCO forwarding. */
extern void appScoFwdDisableForwarding(void);
extern void appScoFwdSetWallclock(Sink sink);

extern void appScoFwdNotifyRole(hci_role role);
extern void appScoFwdNotifyIncomingMicSink(Sink mic_sink);
extern void appScoFwdInitMicPacketising(Source mic_source);
extern void appScoFwdMicForwardingEnable(bool enable);


#define SFWD_MICFWD_RECV_CHAIN_BUFFER_SIZE     1536



/*! \todo PSM to be retrieved from the seid */
#define SFWD_L2CAP_LINK_PSM (0x1043) /* has to be odd */

/*! Number of attempts we will make to connect to our peer at
    any one time before giving up */
#define SFWD_L2CAP_MAX_ATTEMPTS     5


    /*! Time to play delay in &mu;s */
#define SFWD_TTP_DELAY_US           (appConfigScoFwdVoiceTtpMs() * 1000)


/*! Size of buffer to use in the send chain. The buffer is required
    to compensate for Time To Play data being backed up before the 
    AEC & speaker output */
#define SFWD_SEND_CHAIN_BUFFER_SIZE     2048

/*! Size of buffer to use in the recv chain. The buffer is required
    to compensate for Time To Play data being backed up. */
#define SFWD_RECV_CHAIN_BUFFER_SIZE     2048


/*! Size of SCO metadata at the start of a SCO block */
#define SFWD_SCO_METADATA_SIZE    (5 * sizeof(uint16))

/*! Amount of data to claim if sending an incorrect packet into the
    WBS decoder, so as to generate a PLC frame */
#define SFWD_WBS_DEC_KICK_SIZE    SFWD_SCO_METADATA_SIZE

/*! Size of header used on the forwarding link. 24 bit TTP only */
#define SFWD_TX_PACKETISER_FRAME_HDR_SIZE   3

/*! Time before the last TTP that we want to feed in "missing" metadata 
 *  Note we have 7500us of data 
 */
#define SFWD_RX_PROCESSING_TIME_NORMAL_US 8000
#define SFWD_PACKET_INTERVAL_US           7500
#define SFWD_PACKET_INTERVAL_MARGIN_US    ((SFWD_PACKET_INTERVAL_US)/2)

/*! Size of bitpool to use for the asynchronouse WBS.
    26 is the same quality of encoding use for wideband SCO (mSBC),
    but won't allow the use of single slot packets */
#define SFWD_MSBC_BITPOOL                22

/*! Number of octets produced by the WBS Encoding for an audio frame */
#define SFWD_AUDIO_FRAME_OCTETS          (((11 + ((15*SFWD_MSBC_BITPOOL + 7)/8)) / 2) * 2 )

/*! Number of octets of WBS header that can be stripped over the air */
#define SFWD_STRIPPED_HEADER_SIZE           5

/*! Size of an audio frame over the air (accounts for header removal) */
#define SFWD_STRIPPED_AUDIO_FRAME_OCTETS    (SFWD_AUDIO_FRAME_OCTETS - SFWD_STRIPPED_HEADER_SIZE)

/* The implementation was based on keeping the audio down to a single slot
   packet. Ensure that the bitpool selected will work with a 2DH1 packet
   allowing for the 4 octet L2CAP header */
#if (SFWD_STRIPPED_AUDIO_FRAME_OCTETS + SFWD_TX_PACKETISER_FRAME_HDR_SIZE) > (54 -4)
#error "bitpool makes SCO forwarding packet too large for 2DH1"
#endif


/*! Although there is an accurate time source between each end of
    the SCO forwarding link, processing offsets arise between each end
    of the link.

    This is made up of 3 parts.

    - The WBS decoder, which includes Packet Loss Concealment,
      buffers some data before output. This offsets the Time To 
      Play (TTP) that is output compared to the TTP that is input. 
      The eventual TTP is earlier by 6063us. Although this is 
      compensated for in the decoder, SCO forwarding needs to be
      aware to help calculate transit times.
    - Small offset due to the alignment of samples to the local clock 
      This is not compensated for as direction and value vary based on
      the alignment of clocks.
    - Unexplained, but repeatable offset (not compensated, ~13us)

    The receive end of the link will remove \ref SFWD_WBS_UNCOMPENSATED_OFFSET_US
    from the received TTP value.
    */
#define SFWD_WBS_PROCESSING_OFFSET_US           (6063)

#define SFWD_WBS_UNCOMPENSATED_OFFSET_US        (SFWD_WBS_PROCESSING_OFFSET_US - 6063)

/*! Minimum amount of time for packet to get to receiver and still 
    be processed */
#define SFWD_MIN_TRANSIT_TIME_US  (SFWD_PACKET_INTERVAL_US \
                                   + SFWD_WBS_PROCESSING_OFFSET_US \
                                   + SFWD_RX_PROCESSING_TIME_NORMAL_US)


/*! Set a limit on the number of frames we can be behind on
    in the transmit processor. If there appear to be more than 
    this number of frames waiting, then the earliest ones
    will be discarded */
#define SFWD_TX_PACKETISER_MAX_FRAMES_BEHIND    3

    /*! Preferred flush setting.
        Flush should be set lower than the Time to Play delay, as there is
        a processing delay before the packets are queued and after receipt.
        Packets may also contain more than 1 frame so although the first 
        frame in a packet might be late the later ones could 
        still be usable. */
#define SFWD_FLUSH_TARGET_MS    (appConfigScoFwdVoiceTtpMs() / 2)
#define SFWD_FLUSH_TARGET_SLOTS ((SFWD_FLUSH_TARGET_MS * 1000)/ US_PER_SLOT)

#define SFWD_FLUSH_MIN_MS    (SFWD_FLUSH_TARGET_MS / 2)
#define SFWD_FLUSH_MAX_MS    (SFWD_FLUSH_TARGET_MS * 3 / 2)
#define SFWD_FLUSH_MIN_US    (SFWD_FLUSH_MIN_MS * 1000)
#define SFWD_FLUSH_MAX_US    (SFWD_FLUSH_MAX_MS * 1000)

    /*! Time before expected Time To Play to decide that a packet is late.
    
        Allow for additional processing time
     */
#define SFWD_LATE_PACKET_OFFSET_TIME_US ((SFWD_RX_PROCESSING_TIME_NORMAL_US * 3)/2)

/*! Messages sent between devices for SCO forwarding */
enum av_headset_scofwd_ota_messages
{
        /*! Forwarded SCO is about to start. Sent to receiving peer. */
    SFWD_OTA_MSG_SETUP = 0x01,
        /*! Forwarded SCO is about to end. Sent to receiving peer.*/
    SFWD_OTA_MSG_TEARDOWN,
        /*! Notify peer that there is an incoming SCO call */
    SFWD_OTA_MSG_INCOMING_CALL,
        /*! Notify peer that incoming SCO call has terminated */
    SFWD_OTA_MSG_INCOMING_ENDED,
        /*! Notify peer of a MIC set-up */
    SFWD_OTA_MSG_MIC_SETUP,
        /*! Notify slave of new volume level */
    SFWD_OTA_MSG_SET_VOLUME,

        /*! Notify the earbud with the call, that volume changed started on the peer */
    SFWD_OTA_MSG_VOLUME_START = 0x41,
        /*! Notify the earbud with the call, that volume change stopped on the peer */
    SFWD_OTA_MSG_VOLUME_STOP,
        /*! Notify the earbud with the call, that the used requested answer of incoming call */
    SFWD_OTA_MSG_CALL_ANSWER,
        /*! Notify the earbud with the call, that the peer requested reject of incoming call */
    SFWD_OTA_MSG_CALL_REJECT,
        /*! Notify the earbud with the call, that the peer requested hangup of current call */
    SFWD_OTA_MSG_CALL_HANGUP,
        /*! Notify the earbud that the peer requested a voice dial */
    SFWD_OTA_MSG_CALL_VOICE,
       /*! Notify the earbud that the peer requested a RING */
    SFWD_OTA_MSG_RING,
    /*! Notify the earbud to start MIC forwarding */
    SFWD_OTA_MSG_MICFWD_START,
    /*! Notify the earbud to stop MIC forwarding */
    SFWD_OTA_MSG_MICFWD_STOP
};

extern void appScoFwdHandleHfpAudioConnectConfirmation(const HFP_AUDIO_CONNECT_CFM_T *cfm);
extern void appScoFwdHandleHfpAudioDisconnectIndication(const HFP_AUDIO_DISCONNECT_IND_T *ind);

#endif /* _AV_HEADSET_SCO_FWD_H_ */

