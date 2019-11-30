/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file stream_endpoint_usb_audio.h
 * \ingroup stream
 *
 */

#ifndef STREAM_ENDPOINT_USB_AUDIO_H_INCLUDED
#define STREAM_ENDPOINT_USB_AUDIO_H_INCLUDED

/*****************************************************************************
 * The packets are expected to arrive at 1ms intervals, considering jitters we
 * will try to buffer up to 2ms. In terms of number of words required this depends
 * on the sample rate and number of channels. We expect to know this info at the
 * connection points, so we won't allocate memory for more than needed. Here we
 * define some typical and max values for the buffer size required.
 ********************************************************************************/
#define USB_TYPICAL_SAMPLE_RATE           48000
#define USB_TYPICAL_NUMBER_OF_CHANNELS    2
#define USB_MAX_NUMBER_OF_CHANNELS        6
#define USB_MAX_SAMPLE_RATE               192000
#define USB_TYPICAL_AUDIO_BUFFER_SIZE     ((USB_TYPICAL_SAMPLE_RATE*USB_TYPICAL_NUMBER_OF_CHANNELS*2)/1000)
#define USB_MAX_AUDIO_BUFFER_SIZE         ((USB_MAX_SAMPLE_RATE*USB_MAX_NUMBER_OF_CHANNELS*2)/1000)
#define USB_AUDIO_TX_METADATA_SIZE (2)       /* just tag length and boundary flag, no private info */
#define USB_AUDIO_MAX_PACKETS_IN_BUFFER (24) /* Normally should be 3 or less */
#define USB_AUDIO_TX_METADATA_BUFFER_SIZE    (USB_AUDIO_MAX_PACKETS_IN_BUFFER*USB_AUDIO_TX_METADATA_SIZE)
#define USB_AUDIO_PACKET_RATE_HZ        1000 /* Number of packets to send in one second */
#define USB_AUDIO_TX_PACKETS_PER_KICK   2 /* Expected number of packets to send each kick, set to 2 or 1 only */
#define USB_AUDIO_TX_KICK_PERIOD_US     ((SECOND*USB_AUDIO_TX_PACKETS_PER_KICK)/USB_AUDIO_PACKET_RATE_HZ) /* Timer period required */
#define USB_AUDIO_TX_MAX_PACKET_IN_OUTPUT_BUFFER     3 /* This number of packets will be in output buffer after each service */
#define USB_AUDIO_TX_MAX_LATENCY_HIGH_US 7000          /* TTP error Threshold to switch from discarding/silence insertion to real audio */
#define USB_AUDIO_TX_MAX_LATENCY_LOW_US  3500          /* TTP error Threshold to switch from real audio to discarding/silence insertion */

/* adjusting timer period */
#define USB_AUDIO_TX_KICK_PERIOD_US_MAX (USB_AUDIO_TX_KICK_PERIOD_US + USB_AUDIO_TX_PACKETS_PER_KICK) /* max timer period */
#define USB_AUDIO_TX_KICK_PERIOD_US_MIN (USB_AUDIO_TX_KICK_PERIOD_US - USB_AUDIO_TX_PACKETS_PER_KICK) /* min timer period */
#define USB_AUDIO_TX_KICK_PERIOD_STALL_DETECTION_THRESHOLD 3   /* number of consecutive times we couldn't send any packet */
#define USB_AUDIO_TX_KICK_PERIOD_NORMAL_DETECTION_THRESHOLD 10 /* At least 10 times without stall to start adjustment */
#define USB_AUDIO_TX_KICK_PERIOD_MAX_PACKETS_DRIFT 10          /* maximum accumulated drift from expected value */

/* For ttp error control */
#define USB_AUDIO_TX_TTP_CONTROL_MAX_EXPECTED_ERROR_JUMP_US   50 /* Maximum error change compared to previous kick */
#define USB_AUDIO_TX_TTP_CONTROL_ERROR_AVARAGING_PERIOD       4  /* number of kicks to average the error */

/**
 * stream_usb_audio_read_audio_subframes
 * \brief reads and unpack audio subframes from packed usb audio raw buffer
 * \param sink_buffer pointer to local cbuffer for writing subframes into
 * \param source_buffer pointer to source cbuffer, this is normally a local mmu buffer
 * \param subframe_size size of subframes in bits, can be 16, 24 or 32
 * \param max_copy maximum amount of data to read from source buffer in OCTETs.
 * \return number of subframes read
 */
extern unsigned stream_usb_audio_read_audio_subframes(tCbuffer *sink_buffer,
                                        tCbuffer *source_buffer,
                                        unsigned subframe_size,
                                        unsigned max_copy);

/**
 * stream_usb_audio_read_audio_subframes
 * \brief packs and writes audio subframes into usb audio raw buffer
 * \param sink_buffer pointer to cbuffer for destination buffer, this is normally
 *        a local mmu buffer
 * \param source_buffer pointer to source cbuffer containing audio subframes
 * \param subframe_size size of subframes in bits, can be 16, 24 or 32
 * \param max_copy maximum amount of data to write to sink buffer in OCTETs.
 * \return number of subframes written
 */
extern unsigned stream_usb_audio_write_audio_subframes(tCbuffer *sink_buffer,
                                         tCbuffer *source_buffer,
                                         unsigned subframe_size,
                                         unsigned max_copy);

/**
 * stream_usb_audio_discard_source_buffer
 * \brief discard source buffer
 * \param source_buffer source buffer containing packed usb audio subframes
 * \param octets number of octets to discard (must be multiple of subframe size)
 */
extern void stream_usb_audio_discard_octets_from_source_buffer(tCbuffer *source_buffer, unsigned octets);

/**
 * stream_usb_audio_write_silence_packet
 * \brief writes a silence packet into output
 * \param sink_buffer pointer to cbuffer for destination buffer, this is normally
 *        a local mmu buffer
 * \param packet_size_in_octets size of the packet in octets
 *  return number of silence octets written, this will be the same as packet_size_in_octets if there
 *  was enough space in the sink buffer otherwise will be 0
 */
extern unsigned stream_usb_audio_write_silence_packet(tCbuffer *sink_buffer, unsigned packet_size_in_octets);

#ifdef STREAM_USB_AUDIO_ENABLE_DEBUG_MSG
#define STREAM_USB_AUDIO_DBG_MSG(x)                 L0_DBG_MSG(x)
#define STREAM_USB_AUDIO_DBG_MSG1(x, a)             L0_DBG_MSG1(x, a)
#define STREAM_USB_AUDIO_DBG_MSG2(x, a, b)          L0_DBG_MSG2(x, a, b)
#define STREAM_USB_AUDIO_DBG_MSG3(x, a, b, c)       L0_DBG_MSG3(x, a, b, c)
#define STREAM_USB_AUDIO_DBG_MSG4(x, a, b, c, d)    L0_DBG_MSG4(x, a, b, c, d)
#define STREAM_USB_AUDIO_DBG_MSG5(x, a, b, c, d, e) L0_DBG_MSG5(x, a, b, c, d, e)
#else  /* STREAM_USB_AUDIO_DEBUG */
#define STREAM_USB_AUDIO_DBG_MSG(x)                 ((void)0)
#define STREAM_USB_AUDIO_DBG_MSG1(x, a)             ((void)0)
#define STREAM_USB_AUDIO_DBG_MSG2(x, a, b)          ((void)0)
#define STREAM_USB_AUDIO_DBG_MSG3(x, a, b, c)       ((void)0)
#define STREAM_USB_AUDIO_DBG_MSG4(x, a, b, c, d)    ((void)0)
#define STREAM_USB_AUDIO_DBG_MSG5(x, a, b, c, d, e) ((void)0)
#endif /* USB_AUDIO_ENABLE_DEBUG_MSG */

#endif /* #ifndef STREAM_ENDPOINT_USB_AUDIO_H_INCLUDED */
