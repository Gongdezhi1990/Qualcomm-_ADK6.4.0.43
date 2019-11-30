/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    broadcast_context.h

DESCRIPTION
    A utility library to hold broadcast related runtime data so that
    it can be accessed from within an audio plugin.
*/


/*!
@file    broadcast_context.h
@brief   A utility library to hold broadcast related runtime data so that
         it can be accessed from within an audio plugin.

         It also contains miscellaneous CSB and CELT configuration values
         that are needed by both kymera and non-kymera audio plugins. At the
         time of writing there isn't a better place to put them.
*/


#ifndef BROADCAST_CONTEXT_
#define BROADCAST_CONTEXT_

#include <broadcast.h>
#include <scm.h>
#include <sink.h>


/*! The CSB packet interval to use */
#define CSB_INTERVAL_SLOTS 22

/*! The number of channels supported by the CELT codec in the DSP. */
#define CELT_FRAME_CHANNELS 2
/*! The number of samples per frame supported by the CELT codec in the DSP. */
#define CELT_FRAME_SAMPLES 512

/*! Number of samples represented in each CELT frame at 44100Hz. */
#define CELT_CODEC_FRAME_SAMPLES_44100HZ  0x0200   /* 512 samples */
/*! Number of samples represented in each CELT frame at 48kHz. */
#define CELT_CODEC_FRAME_SAMPLES_48KHZ  0x0200   /* 512 samples */

/*! Number of bytes in each CELT frame at 44100Hz. */
#define CELT_CODEC_FRAME_SIZE_44100HZ   220
/*! Number of bytes in each CELT frame at 48kHz. */
#define CELT_CODEC_FRAME_SIZE_48KHZ   186

/*! Number of bytes in each CELT frame at 44100Hz, for EC(3,9). 124kbps */
#define CELT_CODEC_FRAME_SIZE_44100HZ_EC3_9   180
/*! Number of bytes in each CELT frame at 48kHz, for EC(3,9). 123kbps */
#define CELT_CODEC_FRAME_SIZE_48KHZ_EC3_9   164

/*! Size in words of the encryption key data stored in PS. */
#define ENCR_KEY_SIZE       9
/*! Size in words of the fixed IV data stored in PS. */
#define ENCR_FIXED_IV_SIZE  3

/*! Configuration for using encrypted links. */
typedef struct
{
    uint16 seckey[ENCR_KEY_SIZE];
    uint16 fixed_iv[ENCR_FIXED_IV_SIZE];
    uint16 variant_iv;
} broadcast_encr_config_t;

typedef enum
{
    EC_SCHEME_2_5,
    EC_SCHEME_3_9
} erasure_coding_scheme_t;

/*!
    @brief Store the broadcast library handle.
*/
void BroadcastContextSetBroadcastHandle(BROADCAST *broadcast);

/*!
    @brief Retrieve the broadcast library handle.
*/
BROADCAST *BroadcastContextGetBroadcastHandle(void);

/*!
    @brief Store the scm broadcaster library handle.
*/
void BroadcastContextSetBroadcasterScm(SCMB *scm);

/*!
    @brief Retrieve the scm broadcaster library handle.
*/
SCMB *BroadcastContextGetBroadcasterScm(void);

/*!
    @brief Store the scm receiver library handle.
*/
void BroadcastContextSetReceiverScm(SCMR *scm);

/*!
    @brief Retrieve the scm receiver library handle.
*/
SCMR *BroadcastContextGetReceiverScm(void);

/*!
    @brief Store the encryption config values.
*/
void BroadcastContextSetEncryptionConfig(broadcast_encr_config_t *encr);

/*!
    @brief Retrieve the encryption config values.
*/
broadcast_encr_config_t *BroadcastContextGetEncryptionConfig(void);

/*!
    @brief Store the current stream id.
*/
void BroadcastContextSetStreamId(uint16 stream_id);

/*!
    @brief Retrieve the current stream id.
*/
uint16 BroadcastContextGetStreamId(void);

/*!
    @brief Store a pointer to the BSSR configuration data.
*/
void BroadcastContextSetBssrConfig(const uint8 *bssr, uint16 bssr_len_words);

/*!
    @brief Retrieve the BSSR configuration data.
*/
bool BroadcastContextGetBssrConfig(const uint8 **bssr, uint16 *bssr_len_words);

/*!
    @brief Store the current sink to be used by the packetiser.
*/
void BroadcastContextSetSink(Sink sink);

/*!
    @brief Retrieve the current sink to be used by the packetiser.
*/
Sink BroadcastContextGetSink(void);

/*!
    @brief Store the Previous TTP extension
*/
void BroadcastContextSetTtpExtension(uint16 ttp_extension);

/*!
    @brief Retrieve the TTP extension
*/
uint16 BroadcastContextGetTtpExtension(void);

/*!
    @brief Retrieve the BSSR configuration data length.
*/
uint16 BroadcastContextGetBssrConfigLength(void);

/*!
    @brief Retrieve the Receiver Stream Source.
*/
Source BroadcastContextGetReceiverSource(void);

/*!
    @brief Set the Receiver Stream Source.
*/
void BroadcastContextSetReceiverSource(Source source);

/*!
    @brief Set pointer to the broadcast volume conversion table.
*/
void BroadcastContextSetVolumeTable(int16 *volume_conversion_table);

/*!
    @brief Get pointer to the broadcast volume conversion table.
*/
int16 *BroadcastContextGetVolumeTable(void);

/*! @brief Set whether the audio plugin is processing broadcast packets.

    On kymera this flag must be set when the erasure_coding and broadcast_packetiser 
    are in use.
    
    On Bluecore this flag must be set when the broadcast or receive kap file is loaded and active.
*/
void BroadcastContextSetBroadcastBusy(bool broadcast_plugin);

/*! @brief Get whether the audio plugin is processing broadcast packets.

    On kymera this flag must be set when the erasure_coding and broadcast_packetiser 
    are in use.
    
    On Bluecore this flag must be set when the broadcast or receive kap file is loaded and active.
    
    @return TRUE if audio plugin is processing broadcast packets, FALSE otherwise.
*/
uint16 *BroadcastContextGetBroadcastBusy(void);

/*! @brief Set the erasure coding handle.

    It is useful only on kymera broadcaster.
    void * is used becasue actual type is platfrom specific.
*/
void BroadcastContextSetEcTxHandle(void *handle);

/*! @brief Get the erasure coding handle.

    It is useful only on kymera broadcaster.

    @return Erasure coding handle.
*/
void *BroadcastContextGetEcTxHandle(void);

/*! @brief Set the broadcast packetiser handle.

    It is useful only on kymera broadcaster.
    void * is used because actual type is platform specific.
*/
void BroadcastContextSetBpTxHandle(void *handle);

/*! @brief Get the broadcast packetiser handle.

    It is useful only on kymera broadcaster.

    @return Broadcast packetiser handle.
*/
void *BroadcastContextGetBpTxHandle(void);

/*!
    @brief Check if a given stream id is valid.
    
    This function looks up the stream id in the given bssr data table and checks
    if it is valid for this device.

    @param bssr bssr [IN] Pointer to the stream records array.
    @param bssr_len [IN] Number of bytes in the stream records array.
    @param stream_id [IN] The stream id to check.
*/
bool BroadcastContextStreamCanBeReceived(const uint8 *bssr, uint16 bssr_len, uint8 stream_id);

/*!
    @brief Get variant IV
    
    This function can be used to know whether Variant IV is set in broadcast context
    If encryption config is set it & variant IV is set then return TRUE else FALSE
    @param void
*/
bool BroadcastContextGetVariantIVIsSet(void);

/*!
    @brief Set variant IV
    
    This function can be used to update whether Variant IV is set in broadcast context
    If encryption config is set it updates the flag variant_iv_not_set

    @param bool set [IN] Variant iv.
*/
void BroadcastContextSetValidVariantIV(void);

/*!
    @brief Set variant IV
    
    This function can be used to update whether Variant IV is set in broadcast context
    If encryption config is set it updates the flag variant_iv_not_set

    @param bool set [IN] Variant iv.
*/
void BroadcastContextSetInvalidVariantIV(void);

/*!
    @brief Get reference of flag variant_iv_not_set
    
    This function can be used to get the reference of flag variant_iv_not_set

    @param void
*/
uint16* BroadcastContextGetVariantIVReference(void);

/*!
    @brief Set the erasure coding scheme to be used.

    Example of erasure coding schemes are 2:5 or 3:9.

    @param ec_scheme [IN] Erasure coding scheme.
*/
void BroadcastContextSetEcScheme(erasure_coding_scheme_t ec_scheme);

/*!
    @brief Get the erasure coding scheme to be used.

    @param void.
*/
erasure_coding_scheme_t BroadcastContextGetEcScheme(void);

#ifdef HOSTED_TEST_ENVIRONMENT
/*!
    @brief Reset any static variables
    
    This is only intended for unit test and will panic if called in a release build.
*/
void BroadcastContextTestReset(void);
#endif

#endif /* BROADCAST_CONTEXT_ */
