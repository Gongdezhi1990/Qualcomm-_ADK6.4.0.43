#ifndef __SOURCE_H__
#define __SOURCE_H__
#include <source_.h>
#include <app/vm/vm_if.h>
#include <app/stream/stream_if.h>

/*! file  @brief Operations on sources of 8-bit data */
/*! Discard all bytes that are currently in the source */
#define SourceEmpty(s) do { Source _x=(s); SourceDrop(_x, SourceSize(_x)); } while(0)

#if TRAPSET_AUDIO

/**
 *  \brief Find the SCO handle corresponding to a source. 
 * @returns The handle, or 0 is the sink wasn't a SCO source
 *    
 *    @note
 *    If the source is an operator source stream then it always returns Zero
 *  because it is 
 *    not a sco source.
 *  \param source The source to get the handle for 
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_audio
 */
uint16 SourceGetScoHandle(Source source);
#endif /* TRAPSET_AUDIO */
#if TRAPSET_STREAM

/**
 *  \brief Reports the number of bytes available in the source.
 *  @note 
 *  Operator source stream mapping parameters must be configured by using 
 *  SourceMapInit before calling this function. If the source is an operator
 *  source stream with uninitialized mapping parameters, then it always returns
 *  zero(0), irrespective of whether the operator source stream is valid or not.
 *         
 *  \param source The source to fetch the size of. 
 *  \return Zero if the source is invalid.
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
uint16 SourceSize(Source source);

/**
 *  \brief Reports the number of words available in the first header associated with
 *  source.
 *           
 *   @note 
 *   Operator source stream mapping parameters must be configured by using 
 *   SourceMapInit before calling this function. If the source is an operator
 *   source stream with uninitialized mapping parameters, then it always returns
 *   zero(0), irrespective of whether the operator source stream is valid or not.
 *  
 *  \param source The Source whose header size is required.
 *  \return zero if the source is invalid or has no header.
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
uint16 SourceSizeHeader(Source source);

/**
 *  \brief Map the source into the address map, returning a pointer to the first byte in
 *  the source. 
 *   The number of accessible bytes is as given by SourceSize(). 
 *   At most one source can be mapped in at any time;
 *   pointers previously obtained from SourceMap() become invalid when
 *   another call to SourceMap() is made. Calls to SourceDrop() also
 *   invalidate previous pointers returned by SourceMap(). See the SourceDrop()
 *   documentation for more details.
 *   @note 
 *   Operator source stream mapping parameters must be configured by using 
 *   SourceMapInit before calling this function. If the source is an operator
 *   source stream with uninitialized mapping parameters, then it always returns
 *   zero(0), irrespective of whether the operator source stream is valid or not.
 *   
 *  \param source The source to map into the address map.
 *  \return zero if the source is invalid. 
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
const uint8 * SourceMap(Source source);

/**
 *  \brief Map the first header associated with a source into the address map.
 *   The number of accessible words is as given by SourceSizeHeader(). At most one
 *  header
 *   source can be mapped in at any time; pointers previously obtained
 *   from SourceMapHeader() become invalid when another call to
 *   SourceMapHeader() is made. Calls to SourceDrop() also
 *   invalidate previous pointers returned by SourceMapHeader().
 *   See the SourceDrop() documentation for more details.
 *   @note 
 *   Operator source stream mapping parameters must be configured by using 
 *   SourceMapInit before calling this function. If the source is an operator
 *   source stream with uninitialized mapping parameters, then it always returns
 *   zero(0), irrespective of whether the operator source stream is valid or not.
 *   
 *  \param source The Source whose header we wish to map. 
 *  \return A pointer to the first word in the header, or zero if the source is invalid or
 *  has no headers.
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
const void * SourceMapHeader(Source source);

/**
 *  \brief Discards the indicated number of bytes from the front of the source.
 *  @note
 *  An implementation detail first introduced in ADK6 means that pointers
 *  previously obtained from SourceMap or SourceMapHeader will not automatically
 *  point to the first non-dropped data item after SourceDrop is called.
 *  Effectively, SourceDrop invalidates pointers previously obtained from SourceMap
 *  or SourceMapHeader.
 *  @note 
 *  Operator source stream mapping parameters must be configured by using 
 *  SourceMapInit before calling this function. If the source is an operator
 *  source stream with uninitialized mapping parameters, data will not be 
 *  dropped, irrespective of whether the operator source stream is valid or not.
 *        
 *  \param source The Source to drop the data from. 
 *  \param amount The number of bytes to drop. 
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
void SourceDrop(Source source, uint16 amount);

/**
 *  \brief Return how many bytes in this source are before the next packet boundary (for
 *  non packet-based sources returns the same as SourceSize.)
 *   @note 
 *   Operator source stream mapping parameters must be configured by using 
 *   SourceMapInit before calling this function. If the source is an operator
 *   source stream with uninitialized mapping parameters, then it always returns
 *   zero(0), irrespective of whether the operator source stream is valid or not.
 *   
 *   
 *  \param source The source to evaluate.
 *  \return Zero if the source is invalid.
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
uint16 SourceBoundary(Source source);

/**
 *  \brief Reports the number of data bytes available in the source.
 *   This is a blocking version of the SourceSize function. 
 *   It can be used when reading from streams that have bounded transport delays 
 *   and where the data is known to exist (such as file streams). 
 *   It can be used as an alternative to having a task waiting for a
 *  MESSAGE_MORE_DATA message.
 *   @note
 *   Not all stream types support SourceSizeBlocking. If the given stream doesn't
 *   support SourceSizeBlocking this function will return SourceSize result.
 *   @note
 *   For writable filesystem if data read into source has failed then it will 
 *   return 0 indicating FAILURE and no data.
 *   
 *   
 *  \param source The source to evaluate.
 *  \return Zero if the source is invalid or if read data into source had failed. 
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
uint16 SourceSizeBlocking(Source source);

/**
 *  \brief Return TRUE if a source is valid, FALSE otherwise.
 *   @note
 *   Even if the source is an operator source, this trap should be able to check
 *   whether the supplied source stream is valid or not.
 *  \param source The source to check.
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
bool SourceIsValid(Source source);

/**
 *  \brief Configure a particular source.
 *     
 *     See \#stream_config_key for the possible keys and their meanings. Note that
 *     some keys apply only to specific kinds of source.
 *     @note
 *     This trap cannot configure operator source stream. So, it always returns 
 *     FALSE over operator source stream input.
 *  \param source The source to configure. 
 *  \param key The key to configure. 
 *  \param value The value to write to 'key'
 *  \return FALSE if the request could not be performed, TRUE otherwise.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
bool SourceConfigure(Source source, stream_config_key key, uint32 value);

/**
 *  \brief Request to close the source 
 *     Some sources, such as RFCOMM connections or the USB
 *     hardware, have a lifetime defined by other means, and cannot be
 *     closed using this call.
 *  @note 
 *  Operator source stream mapping parameters must be configured by using 
 *  SourceMapInit before calling this function. If the source is an operator
 *  source stream with uninitialized mapping parameters, then it always returns  
 *  FALSE, irrespective of whether the operator source stream is valid or not.
 *  @note
 *  For Pipe streams, a SourceClose(Src) call shall close the source of that
 *  stream and the corresponding sink of the other stream, with which this
 *  stream shares the common buffer. Also, any transform that exists on these
 *  source or sink shall get disconnected.
 *  \param source The source to close 
 *  \return TRUE if the source could be closed, and FALSE otherwise.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
bool SourceClose(Source source);

/**
 *  \brief Request to synchronise two Sources 
 *     Call this function to synchronise timing drifts between two
 *     source streams before calling a StreamConnect
 *  \param source1 The first Source to be synchronised 
 *  \param source2 The second Source to be synchronised 
 *  \return TRUE if the Sources are synchronised successfully, else FALSE.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
bool SourceSynchronise(Source source1, Source source2);

/**
 *  \brief Configures operator source mapping parameters before calling SourceMap or
 *  SourceMapHeader. 
 *   Source mapping parameters must be configured for an operator source 
 *   stream before calling SourceMap or SourceMapHeader. The configuration
 *   parameters are \#stream_device and header length.
 *   
 *   @note
 *   If the source is not an operator source then it always returns zero(0).
 *  \param source Identifies the operator source. 
 *  \param device Type of operator source device. 
 *  \param header_len Length of header in bytes to be used with operator source stream. 
 *  \return TRUE if source mapping parameters have been configured, otherwise FALSE.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
bool SourceMapInit(Source source, stream_device device, uint16 header_len);

/**
 *  \brief Unmap and reset mapping parameters for an operator source stream. 
 * Unmap and reset mapping parameters for an operator source stream.
 *   @note
 *   If the source is not an operator source then it always returns zero(0).
 *  \param source Identifies the operator source to unmap. 
 *  \return TRUE if source has been unmapped successfully, otherwise FALSE.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
bool SourceUnmap(Source source);
#endif /* TRAPSET_STREAM */
#endif
