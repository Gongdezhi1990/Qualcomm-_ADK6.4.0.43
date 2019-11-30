/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    broadcast_context.c

DESCRIPTION
    A utility library to hold broadcast related runtime data so that
    it can be accessed from within an audio plugin.
*/

#include <panic.h>
#include <print.h>
#include <vmtypes.h>

#include <broadcast_stream_service_record.h>

#include "broadcast_context.h"


typedef struct {
    /*! Reference to the broadcast library */
    BROADCAST* broadcast;

    /*! SCM broadcaster */
    SCMB *scmb;

    /*! SCM receiver */
    SCMR *scmr;

    /*! AESCCM configuration */
    broadcast_encr_config_t encr_config;

    /*! Has the AESCCM configuration been set? */
    bool encr_config_set;

    /*! The current stream id */
    uint16 stream_id;

    /*! Pointer to the BSSR config data */
    const uint8 *bssr;

    /*! Length of the BSSR config data in 16bit words */
    uint16 bssr_len_words;

    /*! The current sink to be used by the packetiser */
    Sink sink;

    /*! Latest value of the TTP extension returned by the DSP */
    uint16      prev_ttp_ext;

    /*! Stream source from which broadcast data may be received */
    Source receive_source;

    int16 *volume_table;

    /*! Set when broadcast packets are being processed. */
    uint16 broadcast_busy;

    /*! Is the the valid variant IV set? */
    bool is_variant_iv_invalid;

    /*! Erasure coding TX handle */
    void *ec_tx_handle;

    /*! Broadcast packetiser TX handle */
    void *bp_tx_handle;

    /*! Erasure coding scheme */
    erasure_coding_scheme_t ec_scheme;
} broadcast_ctx_t;

static broadcast_ctx_t b_ctx;



void BroadcastContextSetBroadcastHandle(BROADCAST *broadcast)
{
    b_ctx.broadcast = broadcast;
}

BROADCAST *BroadcastContextGetBroadcastHandle(void)
{
    return b_ctx.broadcast;
}

void BroadcastContextSetBroadcasterScm(SCMB *scm)
{
    b_ctx.scmb = scm;
}

SCMB *BroadcastContextGetBroadcasterScm(void)
{
    return b_ctx.scmb;
}

void BroadcastContextSetReceiverScm(SCMR *scm)
{
    b_ctx.scmr = scm;
}

SCMR *BroadcastContextGetReceiverScm(void)
{
    return b_ctx.scmr;
}

void BroadcastContextSetEncryptionConfig(broadcast_encr_config_t *encr)
{
    if (encr)
    {
        b_ctx.encr_config = *encr;
        b_ctx.encr_config_set = TRUE;
    }
    else
    {
        b_ctx.encr_config_set = FALSE;
    }
}

broadcast_encr_config_t *BroadcastContextGetEncryptionConfig(void)
{
    return (b_ctx.encr_config_set ? &b_ctx.encr_config : NULL);
}

void BroadcastContextSetStreamId(uint16 stream_id)
{
    b_ctx.stream_id = stream_id;
}

uint16 BroadcastContextGetStreamId(void)
{
    return b_ctx.stream_id;
}

void BroadcastContextSetBssrConfig(const uint8 *bssr, uint16 bssr_len_words)
{
    b_ctx.bssr = bssr;
    b_ctx.bssr_len_words = bssr_len_words;
}

bool BroadcastContextGetBssrConfig(const uint8 **bssr, uint16 *bssr_len_words)
{
    if (!b_ctx.bssr)
        return FALSE;

    *bssr = b_ctx.bssr;
    *bssr_len_words = b_ctx.bssr_len_words;

    return TRUE;
}

void BroadcastContextSetSink(Sink sink)
{
    b_ctx.sink = sink;
}

Sink BroadcastContextGetSink(void)
{
    return b_ctx.sink;
}

void BroadcastContextSetTtpExtension(uint16 ttp_extension)
{
    b_ctx.prev_ttp_ext = ttp_extension;
}

uint16 BroadcastContextGetTtpExtension(void)
{
    return b_ctx.prev_ttp_ext;
}

uint16 BroadcastContextGetBssrConfigLength(void)
{
    uint16 length = 0;

    if (!b_ctx.bssr)
        return length;

    length = b_ctx.bssr_len_words;

    return length;
}

Source BroadcastContextGetReceiverSource(void)
{
    return b_ctx.receive_source;
}

void BroadcastContextSetReceiverSource(Source source)
{
    b_ctx.receive_source = source;
}

void BroadcastContextSetVolumeTable(int16 *volume_conversion_table)
{
    b_ctx.volume_table = volume_conversion_table;
}

int16 *BroadcastContextGetVolumeTable(void)
{
    return b_ctx.volume_table;
}

void BroadcastContextSetBroadcastBusy(bool broadcast_busy)
{
    b_ctx.broadcast_busy = broadcast_busy;
}

uint16 *BroadcastContextGetBroadcastBusy(void)
{
    return &b_ctx.broadcast_busy;
}

void BroadcastContextSetEcTxHandle(void *handle)
{
    b_ctx.ec_tx_handle = handle;
}

void *BroadcastContextGetEcTxHandle(void)
{
    return b_ctx.ec_tx_handle;
}

void BroadcastContextSetBpTxHandle(void *handle)
{
    b_ctx.bp_tx_handle = handle;
}

void *BroadcastContextGetBpTxHandle(void)
{
    return b_ctx.bp_tx_handle;
}

bool BroadcastContextStreamCanBeReceived(const uint8 *bssr, uint16 bssr_len, uint8 stream_id)
{
   uint8 ec_type = 0, codec_type = 0, sample_size = 0, afh_channel_map_update_method = 0;
    uint16 security = 0, instance = 0, channels = 0, count44100 = 0, count48000 = 0;
    codec_config_celt celt_config;

    PRINT(("BA: BroadcastContextStreamCanBeReceived:"));

    /* Check the erasure code type is supported */
    PRINT(("\t Erasure code type="));
    if (!bssrGetErasureCode(bssr, bssr_len, stream_id, &ec_type))
    {
        PRINT(("Failed to read ec_type !OK\n"));
        return FALSE;
    }

    if(!(ec_type == BSSR_ERASURE_CODE_2_5 || ec_type == BSSR_ERASURE_CODE_3_9))
    {
        PRINT(("%u !OK\n", ec_type));
        return FALSE;
    }
    PRINT(("%u OK\n", ec_type));

    /* Check the security type is supported */
    PRINT(("\t: Security type="));
    if (!bssrGetSecurity(bssr, bssr_len, stream_id, &security))
    {
        PRINT(("%u !OK\n", security));
        return FALSE;
    }
    /* Don't care if forwarding is enabled (because it is not possible to
       forward with the current design) - clear the bit. */
    security &= ~BSSR_SECURITY_FORWARD_ENABLED;
    /* Only associate with private devices supporting AESCCM encryption */
    if (security != (BSSR_SECURITY_KEY_TYPE_PRIVATE | BSSR_SECURITY_ENCRYPT_TYPE_AESCCM))
    {
        PRINT(("%u !OK\n", security));
        return FALSE;
    }
    PRINT(("%u OK\n", security));

    /* Check the codec type is supported */
    PRINT(("\t: Codec type="));
    if (!bssrGetCodecType(bssr, bssr_len, stream_id, &codec_type) ||
        codec_type != BSSR_CODEC_TYPE_CELT)
    {
        PRINT(("%u !OK\n", codec_type));
        return FALSE;
    }
    PRINT(("%u OK\n", codec_type));

    /* Read the celt codec config. Only associate with devices supporting
       either or both 44100 and 48000. Check that the BSSR only defines one
       config at each sample rate  */
    PRINT(("\t: CELT codec config="));
    for (instance = 0, count44100 = 0, count48000 = 0;
         bssrGetNthCodecConfigCelt(bssr, bssr_len, stream_id, instance, &celt_config);
         instance++)
    {
        /* CELT currently only supports 512 samples per frame */
        if (celt_config.frame_samples == CELT_FRAME_SAMPLES)
        {
            if (celt_config.frequencies & BSSR_CODEC_FREQ_44100HZ)
            {
                count44100++;
            }
            if (celt_config.frequencies & BSSR_CODEC_FREQ_48KHZ)
            {
                count48000++;
            }
        }
    }
    if (count44100 > 1 || count48000 > 1 || !(count44100 + count48000))
    {
        PRINT(("!OK\n"));
        return FALSE;
    }
    PRINT(("OK\n"));

    /* Check the channels config is supported */
    PRINT(("\t: Channels type="));
    if (!bssrGetChannels(bssr, bssr_len, stream_id, &channels) || channels != BSSR_CHANNELS_STEREO)
    {
        PRINT(("%u !OK\n", channels));
        return FALSE;
    }
    PRINT(("%u OK\n", channels));

    /* Check the sample size is supported */
    PRINT(("\t: Sample size config="));
    if (!bssrGetSampleSize(bssr, bssr_len, stream_id, &sample_size) ||
        sample_size != BSSR_SAMPLE_SIZE_16_BIT)
    {
        PRINT(("%u !OK\n", sample_size));
        return FALSE;
    }
    PRINT(("%u OK\n", sample_size));

    /* Check the AFH channel map update method is supported. This configuration was added in the ADK6.0 release.
       The Broadcaster's BSSR in releases prior to ADK6.0 will not contain this configuration. To be backwards
       compatible, assume the lack of this configuration means either there is no update method, or the SCM
       update method is used.

       If the configuration is present, then no update method, or both SCM and Sync Train triggered update
       methods are supported by this release.
    */
    if (bssrGetAFHChannelMapUpdateMethod(bssr, bssr_len, stream_id, &afh_channel_map_update_method))
    {
        PRINT(("\t: AFH channel map update method="));
        if ((afh_channel_map_update_method != BSSR_AFH_CHANNEL_MAP_UPDATE_METHOD_NONE) &&
            (afh_channel_map_update_method != BSSR_AFH_CHANNEL_MAP_UPDATE_METHOD_SCM) &&
            (afh_channel_map_update_method != BSSR_AFH_CHANNEL_MAP_UPDATE_METHOD_TRIGGERED_SYNC_TRAIN))
        {
            PRINT(("%u !OK\n", afh_channel_map_update_method));
            return FALSE;
        }
        PRINT(("%u OK\n", afh_channel_map_update_method));
    }

    return TRUE;
}

/*****************************************************************************/
bool BroadcastContextGetVariantIVIsSet(void)
{
    if((b_ctx.encr_config_set == TRUE) && (b_ctx.is_variant_iv_invalid == FALSE))
        return TRUE;
    else
        return FALSE;
}

/*****************************************************************************/
void BroadcastContextSetInvalidVariantIV(void)
{
    if(b_ctx.encr_config_set)
        b_ctx.is_variant_iv_invalid = TRUE;
}

/*****************************************************************************/
void BroadcastContextSetValidVariantIV(void)
{
    if(b_ctx.encr_config_set)
        b_ctx.is_variant_iv_invalid = FALSE;
}

/*****************************************************************************/
uint16* BroadcastContextGetVariantIVReference(void)
{
    return (uint16*) &b_ctx.is_variant_iv_invalid;
}

void BroadcastContextSetEcScheme(erasure_coding_scheme_t ec_scheme)
{
    b_ctx.ec_scheme = ec_scheme;
}

erasure_coding_scheme_t BroadcastContextGetEcScheme(void)
{
    return b_ctx.ec_scheme;
}

/*****************************************************************************/
#ifdef HOSTED_TEST_ENVIRONMENT
void BroadcastContextTestReset(void)
{
    memset(&b_ctx, 0, sizeof(b_ctx));
}
#endif
