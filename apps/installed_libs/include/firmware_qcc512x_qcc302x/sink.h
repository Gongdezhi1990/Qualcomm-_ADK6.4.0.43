#ifndef __SINK_H__
#define __SINK_H__
#include <sink_.h>
#include <bdaddr_.h>
#include <app/vm/vm_if.h>
#include <app/stream/stream_if.h>

/*! file    @brief Operations on sinks which take 8-bit data */

#if TRAPSET_AUDIO

/**
 *  \brief Find the SCO handle corresponding to a sink. 
 * @returns The handle, or 0 is the sink wasn't a SCO sink
 *    
 *    @note
 *    If the sink is an operator sink stream then it always returns Zero because
 *  it is 
 *    not a sco sink.
 *  \param sink The Sink to get the handle for 
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_audio
 */
uint16 SinkGetScoHandle(Sink sink);
#endif /* TRAPSET_AUDIO */
#if TRAPSET_RFCOMM

/**
 *  \brief Find the RFCOMM connection corresponding to a sink. 
 *  \param sink The Sink to get the connection identifier for. @note If the sink is an operator
 *  sink stream then it always returns Zero because BlueCore firmware can not get
 *  RFCOMM connection ID from operator sink stream.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_rfcomm
 */
uint16 SinkGetRfcommConnId(Sink sink);
#endif /* TRAPSET_RFCOMM */
#if TRAPSET_STREAM

/**
 *  \brief Report how many bytes can successfully be claimed in the corresponding sink.
 *   @note 
 *   Operator sink stream mapping parameters must be configured by using 
 *   SinkMapInit before calling this function. If the sink is an operator
 *   sink stream with uninitialized mapping parameters, then it always returns
 *   zero(0), irrespective of whether the operator sink stream is valid or not.
 *  \param sink The Sink to check. 
 *  \return Zero if the sink is not valid.
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
uint16 SinkSlack(Sink sink);

/**
 *  \brief Attempt to claim the indicated number of extra bytes in a sink.
 *   Claims will certainly fail if the sink is invalid, 
 *   or if SinkSlack indicates that the space is unavailable.
 *   SinkClaim(sink, 0) will return the number of octets that are currently
 *   claimed in the sink.
 *   @note 
 *   Operator sink stream mapping parameters must be configured by using 
 *   SinkMapInit before calling this function. If the sink is an operator
 *   sink stream with uninitialized mapping parameters, then it always returns
 *   0xFFFF, irrespective of whether the operator sink stream is valid or not.
 *  \param sink The sink to claim. 
 *  \param extra The number of bytes to attempt to claim. 
 *  \return The offset, from the address returned by SinkMap, of the newly claimed region
 *  if the claim was successful, 0xFFFF otherwise.
 *           This value is also the number of octets claimed at the point
 *  SinkClaim is called.
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
uint16 SinkClaim(Sink sink, uint16 extra);

/**
 *  \brief Map the sink into the address map, returning a pointer to the first claimed
 *  byte in the sink. 
 *   Only the total number of claimed bytes (as
 *   returned by SinkClaim(sink,0)) are accessible. At most one sink can be
 *   mapped in at any time; pointers previously obtained from SinkMap become
 *   invalid when another call to SinkMap is made.
 *   @note
 *   An implementation detail first introduced in ADK6 means that the return value
 *   of this function for a given sink is no longer a constant. Therefore, any code
 *   that relies on the value returned by SinkMap to remain the same between
 *   SinkFlush calls may fail.
 *   @note
 *   Operator sink stream mapping parameters must be configured by using
 *   SinkMapInit before calling this function. If the sink is an operator
 *   sink stream with uninitialized mapping parameters, then it always returns
 *   zero(0), irrespective of whether the operator sink stream is valid or not.
 *  \param sink The sink to map into the address map. 
 *  \return zero if the sink is invalid.
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
uint8 * SinkMap(Sink sink);

/**
 *  \brief Flush the indicated number of bytes out of the sink. 
 *   The specified bytes of data are passed to the corresponding byte stream,
 *   for example out to the UART, or into BlueStack as if sent by a
 *   RFC_DATA_IND for UART/RFCOMM sinks respectively.
 *   Pointers previously obtained from SinkMap or SinkMapHeader become invalid
 *   after a call to SinkFlush.
 *   @note
 *   Operator sink stream mapping parameters must be configured by using 
 *   SinkMapInit before calling this function. If the sink is an operator
 *   sink stream with uninitialized mapping parameters, then it always returns
 *   FALSE, irrespective of whether the operator sink stream is valid or not.
 *   @note
 *   If the sink is a StreamFileSink, StreamFilesystemSink or an
 *  ImageUpgradeStreamGetSink
 *   then further data must not be written to the sink before MESSAGE_MORE_SPACE
 *  has been
 *   received. Alternatively the SinkFlushBlocking trap can be called instead of
 *  SinkFlush
 *   on the above sink types. SinkFlushBlocking provides a guarantee that the data
 *  has been
 *   written by the time the trap returns.
 *   @note
 *   The claimed number of bytes in the sink must be mapped and initialised 
 *   before flush operation is called into.
 *  \param sink The Sink to flush. 
 *  \param amount The number of bytes to flush.
 *  \return TRUE on success, or FALSE if the operation failed because the sink was invalid
 *  or amount exceeded the size of the sink as reported by SinkClaim(sink, 0).
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
bool SinkFlush(Sink sink, uint16 amount);

/**
 *  \brief Flush the indicated number of bytes out of the sink, with a header.
 *   Associates the header with the message.
 *   The specified bytes of data are then passed to the corresponding byte stream,
 *  for
 *   example out to the UART, or into BlueStack as if sent by a
 *   RFC_DATA_IND for UART/RFCOMM sinks respectively.
 *  @note
 *  Operator sink stream mapping parameters must be configured by using 
 *  SinkMapInit before calling this function. If the sink is an operator
 *  sink stream with uninitialized mapping parameters, then it always returns
 *  FALSE, irrespective of whether the operator sink stream is valid or not.
 *  
 *   
 *  \param sink The Sink to flush data from. 
 *  \param amount The number of bytes of data to flush. 
 *  \param header The header to use. 
 *  \param length The size of the header.
 *  \return TRUE on success, or FALSE if the operation failed because the sink was invalid
 *  or amount exceeded the size of the sink as reported by SinkClaim(sink, 0).
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
bool SinkFlushHeader(Sink sink, uint16 amount, const void * header, uint16 length);

/**
 *  \brief Return TRUE if a sink is valid, FALSE otherwise.
 *   @note
 *   Even if the sink is an operator sink, this trap should able to check whether
 *   the supplied sink is valid or not. 
 *  \param sink The sink to check.
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
bool SinkIsValid(Sink sink);

/**
 *  \brief Configure a particular sink. 
 *     
 *     See \#stream_config_key for the possible keys and their meanings. Note that
 *     some keys apply only to specific kinds of sink.
 *     
 *     @note
 *     This trap can not configure an operator sink stream. So, it always returns 
 *     FALSE when passed an operator sink stream input.
 *  \param sink The Sink to configure. 
 *  \param key The key to configure. 
 *  \param value The value to write to 'key'
 *  \return FALSE if the request could not be performed, TRUE otherwise.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
bool SinkConfigure(Sink sink, stream_config_key key, uint32 value);

/**
 *  \brief Request to close the sink 
 *     Some sinks, such as RFCOMM connections or the USB
 *     hardware, have a lifetime defined by other means, and cannot be
 *     closed using this call.
 *   @note 
 *   Operator sink stream mapping parameters must be configured by using 
 *   SinkMapInit before calling this function. If the sink is an operator
 *   sink stream with uninitialized mapping parameters, then it always returns
 *   FALSE, irrespective of whether the operator sink stream is valid or not.
 *   @note
 *   For Pipe streams, a SinkClose(Sink) call shall close the sink of that
 *   stream and the corresponding source of the other stream, with which this
 *   stream shares the common buffer. Also, any transform that exists on these
 *   source or sink shall get disconnected.
 *  \param sink The sink to close 
 *  \return TRUE if the source could be closed, and FALSE otherwise.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
bool SinkClose(Sink sink);

/**
 *  \brief Request to alias two Sinks 
 *  \param sink1 The first Sink to be aliased 
 *  \param sink2 The second Sink to be aliased 
 *  \return TRUE if the sinks are aliased successfully, else FALSE.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
bool SinkAlias(Sink sink1, Sink sink2);

/**
 *  \brief Request to synchronise two Sinks 
 *     Call this function to synchronise timing drifts between two
 *     sink streams before calling a StreamConnect
 *  \param sink1 The first Sink to be synchronised 
 *  \param sink2 The second Sink to be synchronised 
 *  \return TRUE if the Sinks are synchronised successfully, else FALSE.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
bool SinkSynchronise(Sink sink1, Sink sink2);

/**
 *  \brief Configures operator sink mapping parameters before calling SinkMap. 
 *   Sink mapping parameters must be configured for an operator sink stream
 *   before calling SinkMap. The configuration parameters are \#stream_device
 *   and header length.
 *   
 *   @note
 *   If the sink is not an operator sink then it always returns zero(0).
 *  \param sink Identifies the operator sink stream. 
 *  \param device Type of operator sink device. 
 *  \param header_len Length of header in bytes to be used with operator sink stream. 
 *  \return TRUE if sink mapping parameters have been configured, FALSE otherwise.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
bool SinkMapInit(Sink sink, stream_device device, uint16 header_len);

/**
 *  \brief Unmap and reset mapping parameters for an operator sink stream. 
 *   Unmap and reset mapping parameters for an operator sink stream.
 *   @note
 *   If the sink is not an operator sink then it always returns zero(0).
 *  \param sink Identifies the operator sink to unmap. 
 *  \return TRUE if sink has been unmapped successfully, FALSE otherwise.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
bool SinkUnmap(Sink sink);

/**
 *  \brief Execute a blocking flush operation to flush the indicated number of bytes out
 *  of the sink and only return once they've left. 
 *   The specified bytes of data are passed to the corresponding byte stream
 *   and the function waits for confirmation that the bytes have been processed
 *   before returning. For example an image upgrade sink would pass bytes
 *   to the QSPI device to write and wait until the write is complete before
 *   returning.
 *   Not all stream types support SinkFlushBlocking. If the given stream doesn't
 *   support SinkFlushBlocking this function will return FALSE.
 *   Pointers previously obtained from SinkMap or SinkMapHeader become invalid
 *   after a call to SinkFlushBlocking.
 *   This is a blocking version of the SinkFlush function.
 *   @note
 *   Operator sink stream mapping parameters must be configured by using
 *   SinkMapInit before calling this function. If the sink is an operator
 *   sink stream with uninitialized mapping parameters, then it always returns
 *   FALSE, irrespective of whether the operator sink stream is valid or not.
 *   @note
 *   The claimed number of bytes in the sink must be mapped and initialised 
 *   before flush operation is called into.
 *  \param sink The Sink to flush. 
 *  \param amount The number of bytes to flush.
 *  \return TRUE on success, or FALSE if the operation failed because the sink was invalid,
 *  or doesn't support blocking flushes, or amount exceeded the size of the sink
 *  as reported by SinkClaim(sink, 0).
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
bool SinkFlushBlocking(Sink sink, uint16 amount);

/**
 *  \brief Get the Bluetooth address from a sink.
 *   
 *  \param sink The Sink to fetch the Bluetooth address from. 
 *  \param tpaddr If the address is found it will be returned to the location pointed at by this
 *  value.
 *  \return TRUE if such an address was found, FALSE otherwise. @note If the sink is an
 *  operator sink stream then it always returns FALSE because BlueCore firmware
 *  can not get Bluetooth address from operator sink stream.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
bool SinkGetBdAddr(Sink sink, tp_bdaddr * tpaddr);

/**
 *  \brief Get the RSSI for the ACL for a sink.
 *   
 *  \param sink The Sink which uses the ACL, 
 *  \param rssi If the sink corresponds to an ACL the RSSI in dBm will be written to this
 *  location.
 *  \return TRUE if the RSSI was obtained, FALSE otherwise. @note If the sink is an
 *  operator sink stream then it always returns FALSE because it is not a ACL sink.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
bool SinkGetRssi(Sink sink, int16 * rssi);

/**
 *  \brief Read Bluetooth wallclock information of ACL link for sink. 
 *   The wallclock information contains the bluetooth piconet clock counter value  
 *   and system timestamp at which the piconet clock value was updated.
 *   BT clock counter ticks in units of 312.5 microseconds (i.e. half a Bluetooth
 *  slot)
 *   and system timestamp is reported in microseconds. The wallclock information 
 *   also contains change counter which is incremented on each change in 
 *   wallclock information due to ACL role switch. 
 *   
 *   @note
 *   The sink must be a L2CAP sink stream otherwise it always returns Zero.
 *   The application must enable the wallclock feature by using SinkConfigure or 
 *   SourceConfigure before calling this function.
 *  
 *  \param sink Identifies the underlying ACL link
 *  \param wallclock If wallclock information could be read, it will be returned
 *             to the location pointed at by this value 
 *  \return TRUE if the sink identifies an ACL link and the wallclock information could be
 *  read, FALSE otherwise.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
bool SinkGetWallclock(Sink sink, bt_wallclock_info * wallclock);

/**
 *  \brief Create a bi-directional Pipe Stream. 
 *   @note
 *   This trap creates two streams with buffers. sink of stream a and source of
 *   stream b share a common buffer and forms one Pipe. Similarly, sink of stream
 *   b and source of stream a share another common buffer and forms the second
 *   Pipe.
 *   Data arriving at one end of the pipe will be automatically available at the
 *   other end of the pipe. For example: Data arriving at sink of stream a will be
 *   automatically available at source of stream b and data arriving at sink of
 *   stream b will be automatically available at source of stream a. Application
 *   must pass a valid buffer size for both the interconnected buffers of stream a
 *   and stream b or else this trap will return FALSE. Passing the size as either
 *   Zero or greater than the supported buffer memory size is considered as
 *   invalid size. Typically, the maximum supported buffer memory size is lesser
 *   than 4KB. It is also strongly recommended not to use buffer memory with
 *   large size.
 *   @note Destroying the entire pipe stream requires the application to
 *   individually invoke close (on either the sink or source) of both the pipes.
 *   In order to destroy the Pipe Stream, Application can call either
 *   SourceClose() or SinkClose() trap. Closing one of the Pipe ends
 *   (source or sink) shall also close the source or sink of the other end.
 *   However, the other pipe and hence its corresponding source and sink will
 *   remain active. In order to destroy the other pipe, application needs to call
 *   either SourceClose() or SinkClose() again on the other pipe. Closing any one
 *   pipe using SourceClose() or SinkClose() trap will not invalidate the source
 *   and sink ids since these are required to retrieve the sink or source of the
 *   other pipe using either StreamSourceFromSink() or StreamSinkFromSource() trap.
 *     
 *  \param sink_a Address of the sink of stream a 
 *  \param sink_b Address of the sink of stream b 
 *  \param size_a_to_b Size of the buffer to transfer data from stream a to stream b
 *  \param size_b_to_a Size of the buffer to transfer data from stream b to stream a
 *  \return TRUE if the Pipe Stream is created successfully, else FALSE.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
bool StreamPipePair(Sink * sink_a, Sink * sink_b, uint16 size_a_to_b, uint16 size_b_to_a);
#endif /* TRAPSET_STREAM */
#if TRAPSET_BLUESTACK

/**
 *  \brief Find the L2CAP channel id corresponding to a sink. 
 *   @note
 *   If the sink is an operator sink stream then it always returns Zero because 
 *   BlueCore firmware can not get L2CAP channel id from operator
 *   sink stream.
 *  \param sink The Sink to get the connection identifier for.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_bluestack
 */
uint16 SinkGetL2capCid(Sink sink);

/**
 *  \brief Read the away time on the underlying ACL. 
 *   
 *   The away time is the time since any packet was received on that ACL
 *   and is reported in milliseconds. If the time exceeds 0xFFFF, 0xFFFF will
 *   be returned (this is unlikely with sensible link supervision
 *   timeouts.)
 *   @note
 *   If the sink is an operator sink stream then it always returns Zero because 
 *   BlueCore firmware can not get ACL connections from operator
 *   sink stream.
 *  \param sink identifies the underlying ACL 
 *  \param msec receives the away time if the call succeeds (unmodified otherwise)
 *  \return TRUE if the sink identifies an ACL and the away time on that link could be
 *  read, FALSE otherwise.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_bluestack
 */
bool SinkPollAwayTime(Sink sink, uint16 * msec);
#endif /* TRAPSET_BLUESTACK */
#endif
