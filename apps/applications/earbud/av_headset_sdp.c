/*!
\copyright  Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_sdp.c
\brief	    SDP definitions and help functions
*/

#include <region.h>
#include <service.h>
#include <bdaddr.h>
#include <panic.h>

#include "av_headset.h"
#include "av_headset_sdp.h"

/*! Offset of the peer Bluetooth address in a sink service record */
#define PEER_BDADDR_OFFSET (55)

static const uint8 tws_sink_service_record[] =
{
   /* Offset */ /* ServiceClassIDList(0x0001), Data Element Sequence */
    /*  0 */    SDP_ATTR_ID(UUID_SERVICE_CLASS_ID_LIST),
    /*  3 */        SDP_DATA_EL_SEQ(17),

    /*  UUID Qualcomm TWS+ Sink */
    /*  5 */        SDP_DATA_EL_UUID128(UUID_TWS_SINK_SERVICE),
    
    /*  BluetoothProfileDescriptorList (0x0009) */
    /* 22 */    SDP_ATTR_ID(UUID_BT_PROFILE_DESCRIPTOR_LIST),
    /* 25 */    SDP_DATA_EL_SEQ(22),
    /* 27 */        SDP_DATA_EL_SEQ(20),

    /* 29 */            SDP_DATA_EL_UUID128(UUID_TWS_PROFILE_SERVICE),

                        /* Version */
    /* 46 */            SDP_DATA_EL_UINT16(DEVICE_TWS_VERSION),

                /*  Bluetooth Address(0x0200) */
    /* 49 */    SDP_ATTR_ID(UUID_PEER_BDADDR),
    /* 52 */    SDP_DATA_EL_SEQ(10),

                    /* NAP */
    /* 54 */        SDP_DATA_EL_UINT16(0xA5A5),

                    /* UAP */
    /* 57 */        SDP_DATA_EL_UINT8(0x00),

                    /* LAP */
    /* 59 */        SDP_DATA_EL_UINT32(0x000000),

};

const uint8 *appSdpGetTwsSinkServiceRecord(void)
{
    return tws_sink_service_record;
}

uint16 appSdpGetTwsSinkServiceRecordSize(void)
{
    return sizeof(tws_sink_service_record);
}


static const uint8 tws_sink_service_search_request[] =
{
    SDP_DATA_EL_SEQ(17),                     /* type = DataElSeq, 17 bytes in DataElSeq */
        SDP_DATA_EL_UUID128(UUID_TWS_SINK_SERVICE)
};

const uint8 *appSdpGetTwsSinkServiceSearchRequest(void)
{
    return tws_sink_service_search_request;
}

uint16 appSdpGetTwsSinkServiceSearchRequestSize(void)
{
    return sizeof(tws_sink_service_search_request);
}

/* TWS+ Sink attribute search request */
static const uint8 tws_sink_attribute_list[] =
{
     SDP_DATA_EL_SEQ(3),                               /* Data Element Sequence of 3 */
        SDP_ATTR_ID(UUID_BT_PROFILE_DESCRIPTOR_LIST),  /* Current Version Attribute ID */
};

const uint8 *appSdpGetTwsSinkAttributeSearchRequest(void)
{
    return tws_sink_attribute_list;
}

uint16 appSdpGetTwsSinkAttributeSearchRequestSize(void)
{
    return sizeof(tws_sink_attribute_list);
}



/* TWS+ Source UUID search request */
#ifndef DISABLE_TWS_PLUS
static const uint8 tws_source_service_search_request[] =
{
    SDP_DATA_EL_SEQ(17),                     /* type = DataElSeq, 17 bytes in DataElSeq */
       SDP_DATA_EL_UUID128(UUID_TWS_SOURCE_SERVICE)
};
#endif

const uint8 *appSdpGetTwsSourceServiceSearchRequest(void)
{
#ifdef DISABLE_TWS_PLUS    
    Panic();
    return NULL;
#else
    return tws_source_service_search_request;
#endif
}

uint16 appSdpGetTwsSourceServiceSearchRequestSize(void)
{
#ifdef DISABLE_TWS_PLUS    
    Panic();
    return 0;
#else
    return sizeof(tws_source_service_search_request);
#endif
}



/* TWS+ Source attribute search request */
static const uint8 tws_source_attribute_list[] =
{
    SDP_DATA_EL_SEQ(3),                                /* Data Element Sequence of 3 */
        SDP_ATTR_ID(UUID_BT_PROFILE_DESCRIPTOR_LIST),  /* Current Version Attribute ID */
};

const uint8 *appSdpGetTwsSourceAttributeSearchRequest(void)
{
#ifdef DISABLE_TWS_PLUS    
    Panic();
#endif
    return tws_source_attribute_list;
}

uint16 appSdpGetTwsSourceAttributeSearchRequestSize(void)
{
#ifdef DISABLE_TWS_PLUS    
    Panic();
#endif
    return sizeof(tws_source_attribute_list);
}



bool appSdpFindTwsVersion(const uint8 *ptr, const uint8 *end, uint16 *tws_version)
{
    static const uint8 tws_profile_uuid[] = {UUID_TWS_PROFILE_SERVICE};
    ServiceDataType type;
    Region record, protocols, protocol;
    Region value;

    PanicNull(tws_version);

    record.begin = ptr;
    record.end   = end;

    while (ServiceFindAttribute(&record, UUID_BT_PROFILE_DESCRIPTOR_LIST, &type, &protocols))
    {
        if(type == sdtSequence)
        {
            while (ServiceGetValue(&protocols, &type, &protocol))
            {
                if (type == sdtSequence)
                {
                    if (ServiceGetValue(&protocol, &type, &value))
                    {
                        if (type == sdtUUID)
                        {
                            if (RegionMatchesUUID128(&value, tws_profile_uuid))
                            {
                                if (ServiceGetValue(&protocol, &type, &value))
                                {
                                    if (type == sdtUnsignedInteger)
                                    {
                                        *tws_version = (uint16)RegionReadUnsigned(&value);
                                        return TRUE;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return FALSE; /* Failed */
}


void appSdpSetTwsSinkServiceRecordPeerBdAddr(uint8 *record, const bdaddr *bd_addr)
{
    record[PEER_BDADDR_OFFSET + 0] = (bd_addr->nap >> 8) & 0xFF;
    record[PEER_BDADDR_OFFSET + 1] = (bd_addr->nap >> 0) & 0xFF;
    record[PEER_BDADDR_OFFSET + 3] = (bd_addr->uap) & 0xFF;
    record[PEER_BDADDR_OFFSET + 5] = 0x00;
    record[PEER_BDADDR_OFFSET + 6] = (bd_addr->lap >> 16) & 0xFF;
    record[PEER_BDADDR_OFFSET + 7] = (bd_addr->lap >> 8) & 0xFF;
    record[PEER_BDADDR_OFFSET + 8] = (bd_addr->lap >> 0) & 0xFF;
}



static const uint8 sco_fwd_service_record[] =
{
   /* Offset */ /* ServiceClassIDList(0x0001), Data Element Sequence */
    /*  0 */    SDP_ATTR_ID(UUID_SERVICE_CLASS_ID_LIST),
    /*  3 */        SDP_DATA_EL_SEQ(17),

    /*  UUID Qualcomm SCO Forwarding (0000eb03-d102-11e1-9b23-00025b00a5a5) */
    /*  5 */        SDP_DATA_EL_UUID128(UUID_SCO_FWD_SERVICE),

    /* 22 */    SDP_ATTR_ID(UUID_PROTOCOL_DESCRIPTOR_LIST),
    /* 25 */        SDP_DATA_EL_SEQ(8),
    /* 27 */            SDP_DATA_EL_SEQ(6),
    /* 29 */                SDP_DATA_EL_UUID16(UUID16_L2CAP),
    /* 32 */                SDP_DATA_EL_UINT16(0x9999),

    /* 35 */    SDP_ATTR_ID(UUID_BT_SUPPORTED_FEATURES),
    /* 38 */        SDP_DATA_EL_UINT16(0x0000)
};

void appSdpSetScoFwdPsm(uint8 *record, uint16 psm)
{
    record[33 + 0] = (psm >> 8) & 0xFF;
    record[33 + 1] = (psm >> 0) & 0xFF;
}

void appSdpSetScoFwdFeatures(uint8 *record, uint16 features)
{
    record[39 + 0] = (features >> 8) & 0xFF;
    record[39 + 1] = (features >> 0) & 0xFF;
}

const uint8 *appSdpGetScoFwdServiceRecord(void)
{
    return sco_fwd_service_record;
}

uint16 appSdpGetScoFwdServiceRecordSize(void)
{
    return sizeof(sco_fwd_service_record);
}


/* TWS+ Source attribute search request */
static const uint8 sco_fwd_service_search_request[] =
{
    SDP_DATA_EL_SEQ(17),                     /* type = DataElSeq, 17 bytes in DataElSeq */
        SDP_DATA_EL_UUID128(UUID_SCO_FWD_SERVICE),
};

const uint8 *appSdpGetScoFwdServiceSearchRequest(void)
{
    return sco_fwd_service_search_request;
}

uint16 appSdpGetScoFwdServiceSearchRequestSize(void)
{
    return sizeof(sco_fwd_service_search_request);
}


/* TWS+ Source attribute search request */
static const uint8 sco_fwd_attribute_list[] =
{
    SDP_DATA_EL_SEQ(3),                                /* Data Element Sequence of 3 */
        SDP_ATTR_ID(UUID_PROTOCOL_DESCRIPTOR_LIST),
};


const uint8 *appSdpGetScoFwdAttributeSearchRequest(void)
{
    return sco_fwd_attribute_list;
}

uint16 appSdpGetScoFwdAttributeSearchRequestSize(void)
{
    return sizeof(sco_fwd_attribute_list);
}

/* TWS+ Source attribute search request */
static const uint8 sco_fwd_features_attribute_list[] =
{
    SDP_DATA_EL_SEQ(3),                                /* Data Element Sequence of 3 */
        SDP_ATTR_ID(UUID_BT_SUPPORTED_FEATURES),
};

const uint8 *appSdpGetScoFwdFeaturesAttributeSearchRequest(void)
{
    return sco_fwd_features_attribute_list;
}

uint16 appSdpGetScoFwdFeaturesAttributeSearchRequestSize(void)
{
    return sizeof(sco_fwd_features_attribute_list);
}


bool appSdpGetScoFwdSupportedFeatures(const uint8 *begin, const uint8 *end, uint16 *supported_features)
{
    ServiceDataType type;
    Region record, value;
    record.begin = begin;
    record.end   = end;

    while (ServiceFindAttribute(&record, UUID_BT_SUPPORTED_FEATURES, &type, &value))
    {
        if (type == sdtUnsignedInteger)
        {
            *supported_features = (uint16)RegionReadUnsigned(&value);
            return TRUE;
        }
    }

    return FALSE;
}


