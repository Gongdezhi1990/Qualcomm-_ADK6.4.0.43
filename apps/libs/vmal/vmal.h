/****************************************************************************
Copyright (c) 2015 - 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    vmal.h

DESCRIPTION
    Header file for the VMAL library.
*/

/*!
@file   vmal.h
@brief  Header file for the VM Abstraction Layer library.
*/

#ifndef VMAL_H_
#define VMAL_H_

#include <transform.h>
#include <message.h>

typedef struct
{
    uint16 key;
    uint32 value;
} vmal_operator_keys_t;

/* Reset aptx codec payload header size when we configure transform for RTP_PAYLOAD_HEADER_SIZE,
   usage of this will be further handled in firmware update for aptx. */
#define APTX_PAYLOAD_HEADER_SIZE       0

/*
 *  The SIZEOF_OPERATOR_MESSAGE macro is used to determine the length of a message passed to the OperatorMessage API.
 *  On legacy (Gordon, Rick) systems sizeof() always returns the number of uint16 in a structure. On Crescendo,
 *  it returns the number of bytes. This is nort what is required for crescendo.
 *  The OperatorMessage API expects the message length to always be reported in number of uint16s, regardless of platform.
 *  When sending messages that contain no paramters always declare message id to be uint16, otherwise it is likely to
 *  default to a uint32.
 */
#define SIZEOF_OPERATOR_MESSAGE(msg) (uint16)(sizeof(msg)/sizeof(uint16))

/*
 *  The SIZEOF_OPERATOR_MESSAGE_ARRAY macro is used to determine the length of
 *  a message passed to the OperatorMessage API.
 *  This differs from SIZEOF_OPERATOR_MESSAGE in that it takes the allocated
 *  size of an array of uint16 values rather than an object.
 */
#define SIZEOF_OPERATOR_MESSAGE_ARRAY(s) (uint16)((s)/sizeof(uint16))

/*!
    \brief Create an RTP decode transform and configure for SBC

    \param source The RTP packet Source
    \param sink The Sink to receive the decoded SBC packets

    \return The Transform created (or NULL if the operation failed)
*/
Transform VmalTransformRtpSbcDecode(Source source, Sink sink);

/*!
    \brief Create an RTP encode transform and configure for SBC

    \param source The SBC packet Source
    \param sink The Sink to receive the RTP encoded packets

    \return The Transform created (or NULL if the operation failed)
*/
Transform VmalTransformRtpSbcEncode(Source source, Sink sink);
/*!
    \brief Create an RTP encode transform and configure for Aptx

    \param source The aptx, aptx LL and aptx HD  packet Source
    \param sink The Sink to receive the RTP encoded packets

    \return The Transform created (or NULL if the operation failed)
*/
Transform VmalTransformRtpAptxEncode(Source source, Sink sink);

/*!
    \brief Create an RTP decode transform and configure for MP3

    \param source The RTP packet Source
    \param sink The Sink to receive the decoded MP3 packets

    \return The Transform created (or NULL if the operation failed)
*/
Transform VmalTransformRtpMp3Decode(Source source, Sink sink);

/*!
    \brief Create an RTP encode transform and configure for MP3

    \param source The MP3 packet Source
    \param sink The Sink to receive the RTP encoded packets

    \return The Transform created (or NULL if the operation failed)
*/
Transform VmalTransformRtpMp3Encode(Source source, Sink sink);

/*!
    \brief Create an RTP decode transform and configure for AAC

    \param source The RTP packet Source
    \param sink The Sink to receive the decoded AAC packets

    \return The Transform created (or NULL if the operation failed)
*/
Transform VmalTransformRtpAacDecode(Source source, Sink sink);

/*!
    \brief Create an RTP decode transform and configure for AptxHD

    \param source The RTP packet Source
    \param sink The Sink to receive the decoded AptxHD packets

    \return The Transform created (or NULL if the operation failed)
*/
Transform VmalTransformRtpAptxHdDecode(Source source, Sink sink);

/*!
    \brief Create an RTP decode transform and configure for Aptx

    \param source The RTP packet Source
    \param sink The Sink to receive the decoded Aptx packets

    \return The Transform created (or NULL if the operation failed)
*/
Transform VmalTransformRtpAptxDecode(Source source, Sink sink);

/*!
    \brief This API is deprecated and will likely be removed from future adks.
           Please call the function MessageStreamTaskFromSink(...) directly

           Assign the Task which will receive messages relating to a Sink

    \param sink The Sink which will generate messages
    \param task The Task to which messages will be sent

    \return The Task previously associated with this Sink, or NULL if no Task
            was previously associated.
*/
Task VmalMessageSinkTask(Sink sink, Task task);

/*!
    \brief This API is deprecated and will likely be removed from future adks.
           Please call the function MessageStreamGetTaskFromSink(...) directly

           Get the Task associated with a Sink

    \param sink The Sink from which to get the associated Task

    \return The Task associated with this Sink, or NULL if no Task is
            associated.
*/
Task VmalMessageSinkGetTask(Sink sink);

/*!
    \brief This API is deprecated and will likely be removed from future adks.
           Please call the function MessageStreamTaskFromSink(StreamSinkFromSource(...)) directly

           Assign the Task which will receive messages relating to a Source

    \param source The Source which will generate messages
    \param task The Task to which messages will be sent

    \return The Task previously associated with this Source, or NULL if no Task
            was previously associated.
*/
Task VmalMessageSourceTask(Source source, Task task);

/*!
    \brief This API is deprecated and will likely be removed from future adks.
           Please call the function MessageStreamGetTaskFromSink(StreamSinkFromSource(...)) directly

           Get the Task associated with a Source

    \param source The Source from which to get the associated Task

    \return The Task associated with this Source, or NULL if no Task is
            associated.
*/
Task VmalMessageSourceGetTask(Source source);

/*!
    \brief Create an operator

    \param cap_id The ID of the capability to create

    \return The operator ID (or zero if it failed)
*/
uint16 VmalOperatorCreate(uint16 cap_id);

/*!
    \brief Create an operator with operator create keys

    \param capability_id The ID of the capability to create
	\param keys The operator create keys
	\param num_keys The number of operator create keys specified

    \return The operator ID (or zero if it failed)
*/
uint16 VmalOperatorCreateWithKeys(uint16 capability_id, vmal_operator_keys_t* keys, uint16 num_keys);

/*!
    \brief Enable/Disable the main processor

    \param enable TRUE to turn main processor on, FALSE to turn it off

    \return TRUE if successful, otherwise FALSE.
*/
bool VmalOperatorFrameworkEnableMainProcessor(bool enable);

/*!
    \brief Enable/Disable the second processor

    \param enable TRUE to turn second processor on, FALSE to turn it off

    \return TRUE if successful, otherwise FALSE.
*/
bool VmalOperatorFrameworkEnableSecondProcessor(bool enable);

/*!
    \brief Send a message to an operator

    \param opid The Operator ID
    \param send_msg The message to send
    \param send_len The size of the message to send
    \param recv_msg Pointer to populate with response message if required
    \param recv_len The expected length of the response message (or zero if not
                    required)

    \return TRUE if the message was sent successfully, otherwise FALSE.
*/
bool VmalOperatorMessage(uint16 opid, const void * send_msg, uint16 send_len,
                                            void * recv_msg, uint16 recv_len);


/*!
  \brief Read the product id of the chip
 
  \return The product id of the device if the read was successful, zero otherwise. 
*/
uint32 VmalVmReadProductId(void);

/*!
  \brief Report whether systems uses a DSP which is in patchable ROM
 
  \return TRUE if patchable ROM used, FALSE otherwise
*/
bool VmalDspInPatchableRom(void);

/*!
    \brief Set the direction of a given pio

    \param bank This refers to the bank that the given pio is on
    \param mask Bitmask indicating which pio to configure
    \param direction 0 to set the pin as an input, 1 for output

    \return If any bit in this mask is high then that PIO could not be set to the direction specified
*/
uint32 VmalPioSetDirection(uint16 bank, uint32 mask, uint32 direction);

/*!
    \brief Modifies the contents of the PIO data output register

    \param bank This refers to the bank of pios to be set
    \param mask Bits set to 1 in this mask will be modified. Bits set to 0 in this mask will not be modified
    \param bits Bits set to 1 in this value will result in that PIO line being driven high. Bits set to 0
           in this value will result in that PIO line being driven low.

    \return If any bit in this mask is high then that PIO could not be driven to the level specified;
            note that no action will have been taken on any PIOs.
*/
uint32 VmalPioSet(uint16 bank, uint32 mask, uint32 bits);

/*!
    \brief Returns the contents of one of the PIO data input registers.

    \param bank This refers to the bank of pios to read from

    \return The value last written to that particular bank using VmalPioSet()
*/
uint32 VmalPioGet(uint16 bank);

/*!
    \brief configures a simple debounce engine for PIO input pins.

    \param bank This refers to the bank that the given pio is on
    \param mask Bitmask indicating which pins to monitor. Setting this to zero disables PIO monitoring.
    \param count How many times the monitored pins' state must be observed to be consistent before it
           is considered stable.
    \param period The delay in milliseconds between successive reads of the pins.

    \return If any bit in this mask is high then monitoring could not be set up for that PIO;
            no action will have been taken on any PIOs
*/
uint32 VmalPioDebounce(uint16 bank, uint32 mask, uint16 count, uint16 period);

/*!
    \brief configures a simple debounce engine for PIO input pins.

    \param bank This refers to the bank that the given pio is on
    \param mask Bitmask indicating which pins to monitor. Setting this to zero disables PIO monitoring.
    \param count How many times the monitored pins' state must be observed to be consistent before it
           is considered stable.
    \param period The delay in milliseconds between successive reads of the pins.

    \return If any bit in this mask is high then monitoring could not be set up for that PIO;
            no action will have been taken on any PIOs
*/
uint32 VmalPioDebounceGroup(uint16 group, uint16 bank, uint32 mask, uint16 count, uint16 period);

/*!
    \brief Cause the usual function of chip pins to be suppressed, and instead make them behave as PIOs

    \param bank This refers to the bank that the given pio is on
    \param mask Bits set to 1 in this mask will be modified. Bits set to 0 in this mask will not be modified.
    \param bits A bit set to 1 will cause a (non-PIO) chip pin to be behave as the corresponding PIO.
                A bit set to 0 will result in any mapped pin being returned to its original function.

    \return If any bit in this mask is high then that PIO could not be mapped or unmapped;
            note that no action will have been taken on any PIOs.
*/
uint32 VmalPioSetMap(uint16 bank, uint32 mask, uint32 bits);

/*!
    \brief Get the number of pio banks

    \return number of pio banks
*/
uint16 VmalGetNumberOfPioBanks(void);
                          
/*!
    \brief Get the number of debounce groups

    \return number of debounce groups
*/
uint16 VmalGetNumberOfDebounceGroups(void);
                          
#endif /*VMAL_H_*/
