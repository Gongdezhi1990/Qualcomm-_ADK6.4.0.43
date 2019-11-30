#ifndef __STREAM_H__
#define __STREAM_H__
#include <bdaddr_.h>
#include <source_.h>
#include <sink_.h>
#include <transform_.h>
#include <app/vm/vm_if.h>
#include <app/file/file_if.h>
#include <app/uart/uart_if.h>
#include <app/stream/stream_if.h>
#include <app/usb/usb_if.h>
#include <app/partition/partition_if.h>
#include <app/voltsense/voltsense_if.h>
#include <operator_.h>
#include <app/audio/audio_if.h>
#include <app/ringtone/ringtone_if.h>

/*! file  @brief Sources and sinks of 8-bit data 
** 
** 
The Stream API provides functions for efficiently processing streams of eight bit data.
Available streams include RFCOMM, L2CAP, the serial port, USB endpoints and files.
** 
Streams are classified into sources (which can be read) and sinks (which can be written).
*/
/*!
@brief Get the Source for the specified stream-based BCSP\#13 channel.
@param channel The channel to fetch the Source for.
*/
#define StreamHostSource(channel) StreamSourceFromSink(StreamHostSink(channel))
/*!
@brief Find the Source associated with the raw UART.
** 
** 
Returns zero if it is unavailable (for example the appropriate
transport has not been configured.)
*/
#define StreamUartSource() StreamSourceFromSink(StreamUartSink())
/*!
@brief Find the Source corresponding to an RFCOMM connection.
*/
#define StreamRfcommSource(conn_id) StreamSourceFromSink(StreamRfcommSink(conn_id))
/*!
@brief Find the Source corresponding to an L2CAP connection
** 
** 
@param cid The connection ID to fetch the Source for.
*/
#define StreamL2capSource(cid) StreamSourceFromSink(StreamL2capSink(cid))
/*!
@brief The Source connected to the port passed on Kalimba.
@param port In the range 0..3 (BC3-MM) or 0..7 (BC5-MM)
*/
#define StreamKalimbaSource(port) StreamSourceFromSink(StreamKalimbaSink(port))
/*!
@brief Return the USB Class Request Source associated with 'interface'. 
@param interface The USB interface (returned by UsbAddInterface) to fetch the Source for.
*/
#define StreamUsbClassSource(interface) StreamSourceFromSink(StreamUsbClassSink(interface))
/*!
@brief Return the USB Request Source associated with the USB transport.
@param end_point The USB endpoint (bEndPointAddress field in EndPointInfo structure) to fetch the Source for.
*/
#define StreamUsbEndPointSource(end_point) StreamSourceFromSink(StreamUsbEndPointSink(end_point))
/*!
@brief Return the USB Vendor Source associated with the USB transport.
*/
#define StreamUsbVendorSource() StreamSourceFromSink(StreamUsbVendorSink())
/*!
@brief Return the FastPipe Source for the pipe requested.
@param id The ID of the pipe needed.
*/
#define StreamFastPipeSource(id) StreamSourceFromSink(StreamFastPipeSink(id))

#if TRAPSET_SPIFLASH

/**
 *  \brief Return a source with the contents of the specified SPI flash address. 
 *  \param address The array address to read data from. 
 *  \param size  The amount of data (in octets )to read. 
 *  \return The source associated with the SPI flash stream.
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_spiflash
 * 
 * WARNING: This trap is unless HIDE_SPIFLASH
 */
Source StreamSpiflashSource(uint32 address, uint16 size);
#endif /* TRAPSET_SPIFLASH */
#if TRAPSET_STREAM

/**
 *  \brief Move the specified number of bytes from the start of 'source' to the end of
 *  'sink'. 
 *   The count must be no more than both SinkSlack() and SourceBoundary().
 *   @note
 *   An implementation detail first introduced in ADK6 means that pointers returned
 *   by SourceMap, SourceMapHeader, SinkMap or SinkMapHeader will be invalidated
 *   after a call to StreamMove.
 *   @note
 *   If either source or sink is an operator source/sink stream then it always
 *   returns zero(0), irrespective of whether the operator source/sink stream
 *   is valid or not.
 *  \param sink The Sink to move data from. 
 *  \param source The Source to move data from. 
 *  \param count The number of bytes to move. 
 *  \return Zero on failure and the count on success.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
uint16 StreamMove(Sink sink, Source source, uint16 count);

/**
 *  \brief Make an automatic connection between a source and sink 
 *   
 *   @note
 *   Transform created via this call is started implicitly. It is not 
 *   desired to start or stop such transforms by invoking @a TransformStart()
 *   or @a TransformStop() respectively from the application.
 *  \param source The Source data will be taken from. 
 *  \param sink The Sink data will be written to.
 *  \return An already started transform on success, or zero on failure.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
Transform StreamConnect(Source source, Sink sink);

/**
 *  \brief Dispose of all data arriving on the specified source by throwing it away.
 *   
 *   On success the source is effectively connected using StreamConnect();
 *   you can stop discarding data from the source by calling
 *   StreamDisconnect(source, 0).
 *  \param source The source whose data is to be disposed of.
 *  \return FALSE on failure, TRUE on success.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
bool StreamConnectDispose(Source source);

/**
 *  \brief Break any existing automatic connection involving the source *or* sink.
 *   Source or sink may be zero.
 *  \param source The Source to check for connections. 
 *  \param sink The Sink to check for connections.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
void StreamDisconnect(Source source, Sink sink);

/**
 *  \brief Configure the stream subsystem. 
 *     
 *     Reasons for a FALSE return value include attempting to use an invalid \e
 *  key,
 *     attempting to use a \e key not supported in the current firmware build and 
 *     attempting to use a \e value that is not suitable for the \e key being used.
 *     The application will be panicked if it attempts to enable or
 *     disable streams when any corresponding L2CAP/RFCOMM connection is
 *     open.
 *     Note that this trap is also used to enable/disable an application's interest
 *     in receiving certain messages (eg.see keys \#VM_STREAM_USB_ALT_IF_MSG_ENABLED
 *     and \#VM_STREAM_USB_ATTACH_MSG_ENABLED).
 *     This trap is also used to enable L2CAP large buffers for particular PSM
 *     using key \#VM_STREAM_L2CAP_ADD_LARGE_BUFFER_ON_PSM and remove all large 
 *     buffer information using key \#VM_STREAM_L2CAP_REMOVE_ALL_LARGE_BUFFER.
 *     \#VM_STREAM_L2CAP_ADD_LARGE_BUFFER_ON_PSM key should be used to configure 
 *     before sending or accepting L2CAP connection request.
 *  \param key Keys are defined in \#vm_stream_config_key. 
 *  \param value The value to set \e key to.
 *  \return TRUE if the configure worked, FALSE if the configure failed.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
bool StreamConfigure(vm_stream_config_key key, uint16 value);

/**
 *  \brief Find the Source from its Sink. 
 *  \param sink The Sink whose source is required. @note If the sink is an operator sink stream
 *  then this trap always returns zero(0), because BlueCore firmware can not
 *  convert operator sink stream into operator source stream.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
Source StreamSourceFromSink(Sink sink);

/**
 *  \brief Find the Sink from its Source. 
 *  \param source The source whose sink is required. @note If the source is an operator source
 *  stream then this trap always returns zero(0), because BlueCore firmware can
 *  not convert operator source stream into an operator sink stream.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
Sink StreamSinkFromSource(Source source);

/**
 *  \brief Create a source from a region of memory. 
 *   This function allows a region of memory to be treated as a source.
 *   This is useful when there is a requirement to handle data (held in a
 *   known region of memory) using functions that expect a source, e.g.
 *   StreamConnect(), in order to efficiently transfer the data without
 *   having to copy it.
 *   It is important that the memory being treated as a source persists
 *   long enough for the stream operation to complete, i.e., long enough
 *   for the source to be read. The source created using this function
 *   only exists while the data is being read. However, the memory block
 *   being treated as a source is not freed by the stream subsystem once
 *   the data has been read. It remains the caller's responsibility to
 *   manage the memory and free it when it is appropriate to do so.
 *   If length is zero then 0 is returned.
 *  \param data The memory that the source will be created from. 
 *  \param length The size of the memory region.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
Source StreamRegionSource(const uint8 * data, uint16 length);

/**
 *  \brief Find all the sinks connected to a given Bluetooth address. 
 *   
 *  \param max Stores up to *max sinks in the array given, and updates *max. 
 *  \param sinks The array of sinks to store into. 
 *  \param tpaddr The Bluetooth address to use.
 *  \return TRUE if there was enough space, FALSE if some had to be discarded.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_stream
 */
bool StreamSinksFromBdAddr(uint16 * max, Sink * sinks, const tp_bdaddr * tpaddr);
#endif /* TRAPSET_STREAM */
#if TRAPSET_RFCOMM

/**
 *  \brief Find the Sink corresponding to an RFCOMM connection.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_rfcomm
 */
Sink StreamRfcommSink(uint16 conn_id);
#endif /* TRAPSET_RFCOMM */
#if TRAPSET_UART

/**
 *  \brief Find the Sink associated with the raw UART.
 *   Returns zero if it is unavailable (for example the appropriate
 *   transport has not been configured.)
 *   If the Source buffer overflows it will cause the chip to panic.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_uart
 */
Sink StreamUartSink(void );

/**
 *  \brief Dynamically configure the UART settings. 
 *         PIOs are not handled by this but can be assigned to the UART at any
 *         point. This is done by using PioSetMapPins32Bank to put the PIO under
 *         HW control and then PioSetFunction to set the function to one of the
 *         UART signals.
 *         Please note that one PIO can be assigned for multiple signals at the
 *         same time. All UART signals are initially muxed to PIO 96 (which
 *         doesn't exist) to make sure there are no conflicts. Once a PIO is muxed
 *         to a UART signal, that signal cannot be muxed back to PIO 96.
 *         This trap is ignored unless raw access to the UART is enabled. UART is
 *         enabled by adding TRANSPORT_USER to the firmware defines (enabled by
 *         default). 
 *  \param rate The UART rate to use. Custom baud rates can be calculated as
 *  round(baud_rate_in_mbps*4096).
 *             Consult the data sheet for part specific information.
 *  \param stop The UART stop to use. 
 *  \param parity The UART parity to use.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_uart
 */
void StreamUartConfigure(vm_uart_rate rate, vm_uart_stop stop, vm_uart_parity parity);
#endif /* TRAPSET_UART */

/**
 *  \brief Make an automatic connection between a source and sink, or dispose it.
 *   Like StreamConnect(), but if the connection could not be made then the
 *   source will be passed to StreamConnectDispose(). Similarly, if the
 *   connection is subsequently broken using StreamDisconnect() or by the
 *   sink being closed the source will be passed to StreamConnectDispose().
 *   The end result is that the source will be tidied up correctly, no
 *   matter what happens after this call.
 *   Note that the task associated with the source will be
 *   changed. Messages related to the source will no longer be sent to
 *   the task previously associated with it.
 *  \param source The Source data will be taken from. 
 *  \param sink The Sink data will be written to. 
 *  \return TRUE if the connection was made between \e source and \e sink; FALSE if the
 *  initial connection failed (in which case, if \e source was valid, it will have
 *  been immediately passed to StreamConnectDispose()).
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset___special_inline
 */
bool StreamConnectAndDispose(Source source, Sink sink);
#if TRAPSET_FASTPIPE

/**
 *  \brief Return the FastPipe Sink for the pipe requested. 
 *  \param id The ID of the pipe needed.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_fastpipe
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
Sink StreamFastPipeSink(uint16 id);
#endif /* TRAPSET_FASTPIPE */
#if TRAPSET_PARTITION

/**
 *  \brief Open a sink to erase and write to an external flash partition. This function
 *  will perform a flash erase on the entire partition specified and then provide
 *  a Sink to allow it to be written from the start.
 *   
 *   \note
 *   If the VM application uses the VM software watchdog functionality, BlueCore
 *   firmware automatically extends the VM software watchdog before the erase of
 *   an external serial flash memory. Erasing of the external serial flash memory
 *   is time consuming. This ensures that the VM application is given enough time
 *   to kick the VM software watchdog when the operation has completed.
 *   \note
 *   This trap expects all the partitions in the \#PARTITION_SERIAL_FLASH 
 *   device to be sector aligned. If partition is not sector aligned then
 *   firmware will erase shared sectors (i.e., end of previous partition's 
 *   sector or start of next partition's sector).
 *  \param device device to which to write, cannot be internal flash, see
 *  \#partition_filesystem_devices
 *  \param partition partition number to overwrite
 *  \return Sink if partition found and erased successfully, otherwise zero
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_partition
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
Sink StreamPartitionOverwriteSink(partition_filesystem_devices device, uint16 partition);

/**
 *  \brief Resume external flash sink partition after a controlled power failure.
 *   
 *   This VM trap is used to resume sink stream to write into external flash
 *   partition which got interrupted while writing previously. This trap returns
 *   the sink stream by reopening the stream for the interrupted external flash
 *   partition. This trap will not erase the contents of the partition.
 *   See \#PartitionSinkGetPosition trap description to know how to retrieve the
 *   sink position from which the data can be written.
 *  \param device device to which to write, cannot be internal flash, see
 *  \#partition_filesystem_devices. 
 *  \param partition_no partition number of the sink partition to be resumed. 
 *  \param first_word first word of the sink partition to be resumed.
 *  \return Sink if partition found or sink already exists, else return NULL 
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_partition
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
Sink StreamPartitionResumeSink(partition_filesystem_devices device, uint16 partition_no, uint16 first_word);
#endif /* TRAPSET_PARTITION */
#if TRAPSET_OPERATOR

/**
 *  \brief Gets operator source stream for the operator 
 *   This trap checks validity of the operator and its specified source terminal
 *   number. If the supplied input parameters are valid then it provides the
 *   operator source stream.
 *   @note
 *   This API returns zero(0) in either one of the below mentioned scenarios:
 *   1. Operator is not valid
 *   2. Source terminal ID is not valid with regards to the supplied operator.
 *  \param opid Operator, which was created in the DSP 
 *  \param terminal Source stream connection number for the specified operator 
 *  \return Operator source stream, if it is successful, otherwise zero(0).
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_operator
 */
Source StreamSourceFromOperatorTerminal(Operator opid, uint16 terminal);

/**
 *  \brief Gets operator sink stream for the operator 
 *   This API checks validity of the operator and its specified sink terminal
 *   number. If the supplied input parameters are valid then it provides the
 *   operator sink stream.
 *   @note
 *   This API returns zero(0) in either one of the below specified scenarios:
 *   1. Operator is not valid
 *   2. Sink terminal ID is not valid with regards to the supplied operator.
 *  \param opid Operator, which was created in the DSP 
 *  \param terminal Sink stream connection number for the specified operator 
 *  \return Operator sink stream, if it is successful, otherwise zero(0).
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_operator
 */
Sink StreamSinkFromOperatorTerminal(Operator opid, uint16 terminal);
#endif /* TRAPSET_OPERATOR */
#if TRAPSET_ATT

/**
 *  \brief Find the Source corresponding to an ATT connection with a specific connection
 *  id and attribute handle. 
 *   
 *  \param cid The channel id to get the connection source id for. 
 *  \param handle The attribute handle to get the connection source id for.
 *  \return Source on success or zero on failure.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_att
 */
Source StreamAttSource(uint16 cid, uint16 handle);

/**
 *  \brief Add an attribute handle corresponding to an ATT connection with a specific
 *  connection id.
 *  \param cid The channel id to get the connection source id for. 
 *  \param handle The attribute handle to get the connection source id for. 
 *  \return Source in case of successful addition or zero on failure.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_att
 */
Source StreamAttSourceAddHandle(uint16 cid, uint16 handle);

/**
 *  \brief Remove all attribute handles corresponding to an ATT connection registered with
 *  StreamAttSourceAddHandle. 
 *   
 *  \param cid  The channel id of the ATT connection
 *  \return bool TRUE in case of successful deletion otherwise FALSE
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_att
 */
bool StreamAttSourceRemoveAllHandles(uint16 cid);

/**
 *  \brief Find the Sink corresponding to an ATT connection with a specific connection id
 *  and attribute handle. 
 *         
 *  \param cid The channel id to get the connection sink id for. 
 *  \return Sink on success or zero on failure.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_att
 */
Sink StreamAttSink(uint16 cid);

/**
 *  \brief Find the ATT Server Source corresponding to an ATT connection with a specific
 *  connection id. 
 *         
 *  \param cid The channel id to get the connection source id for. 
 *  \return Source on success or zero on failure.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_att
 */
Source StreamAttServerSource(uint16 cid);

/**
 *  \brief Find the ATT Client Source corresponding to an ATT connection with a specific
 *  connection id. 
 *         
 *  \param cid The channel id to get the connection source id for. 
 *  \return Source on success or zero on failure.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_att
 */
Source StreamAttClientSource(uint16 cid);

/**
 *  \brief Add an attribute handle corresponding to an ATT source stream specified
 *          with source id.
 *          The number available streams is limited, and can change in the future,
 *  and 
 *          therefore the application shall always check the return value for
 *  being FALSE
 *         
 *  \param source The source stream id of the ATT stream. 
 *  \param handle The attribute handle to be added to the specifed source stream. 
 *  \return TRUE in case of successful addition or FALSE on failure.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_att
 */
bool StreamAttAddHandle(Source source, uint16 handle);

/**
 *  \brief Remove all attribute handles corresponding to an ATT source stream registered
 *  with StreamAttAddHandle or StreamAttSourceAddHandle.
 *         
 *  \param source The source stream id of the ATT stream. 
 *  \return TRUE in case of successful deletion otherwise FALSE.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_att
 */
bool StreamAttRemoveAllHandles(Source source);

/**
 *  \brief Find the Client Sink corresponding to an ATT connection with a specific
 *  connection id. 
 *         
 *  \param cid The channel id to get the connection sink id for. 
 *  \return Sink on success or zero on failure.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_att
 */
Sink StreamAttClientSink(uint16 cid);

/**
 *  \brief Find the Server Sink corresponding to an ATT connection with a specific
 *  connection id. 
 *         
 *  \param cid The channel id to get the connection sink id for. 
 *  \return Sink on success or zero on failure.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_att
 */
Sink StreamAttServerSink(uint16 cid);
#endif /* TRAPSET_ATT */
#if TRAPSET_CSB

/**
 *  \brief Find the Sink corresponding to a CSB transmitter stream.
 *  \param lt_addr The logical transport address used for CSB link.
 *  \return  Sink if CSB transmitter stream exists for a given lt_addr otherwise 
 * NULL.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_csb
 */
Sink StreamCsbSink(uint16 lt_addr);

/**
 *  \brief Find the Source corresponding to a CSB receiver stream.
 *  \param remote_addr The remote device Bluetooth address.
 *  \param lt_addr The logical transport address used for CSB link.
 *  \return  Source if CSB receiver stream exists for a given parameters otherwise 
 * NULL.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_csb
 */
Source StreamCsbSource(const bdaddr * remote_addr, uint16 lt_addr);
#endif /* TRAPSET_CSB */
#if TRAPSET_SHUNT

/**
 *  \brief Return the sink corresponding to the shunt for the given L2CAP CID on the given
 *  ACL 
 *  \param acl the ACL connection handle (from the host) 
 *  \param cid the L2CAP connection id (from the host)
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_shunt
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
Sink StreamShuntSink(uint16 acl, uint16 cid);
#endif /* TRAPSET_SHUNT */
#if TRAPSET_KALIMBA

/**
 *  \brief The Sink connected to the port passed on Kalimba. 
 *  \param port In the range 0..3 (BC3-MM) or 0..7 (BC5-MM)
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_kalimba
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
Sink StreamKalimbaSink(uint16 port);
#endif /* TRAPSET_KALIMBA */
#if TRAPSET_SD_MMC

/**
 *  \brief Return a source with data on an SD/MMC card.
 *   
 *  \param slot SD slot number, counting from 0
 *  \param start_block number of the first 512-byte block to be read from a card.
 *  \param blocks_count total number of 512-byte blocks to be read from a card
 *  \return 0 if params are not correct or there is no SD/MMC card in the slot.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_sd_mmc
 */
Source StreamSdMmcSource(uint8 slot, uint32 start_block, uint32 blocks_count);

/**
 *  \brief Return a sink that writes data on an SD/MMC card.
 *   
 *  \param slot SD slot number, counting from 0
 *  \param start_block number of the first 512-byte block to write data to.
 *  \param blocks_count total number of 512-byte blocks that can be written.
 *  \return 0 if params are not correct or there is no SD/MMC card in the slot.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_sd_mmc
 */
Sink StreamSdMmcSink(uint8 slot, uint32 start_block, uint32 blocks_count);
#endif /* TRAPSET_SD_MMC */
#if TRAPSET_HOSTSTREAM

/**
 *  \brief Get the Sink for the specified stream-based BCSP\#13 channel. 
 *  \param channel The channel to fetch the Sink for.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_hoststream
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
Sink StreamHostSink(uint16 channel);
#endif /* TRAPSET_HOSTSTREAM */
#if TRAPSET_REFORMATSQIF

/**
 *  \brief Open a sink to erase and write to an external serial flash. This function will
 *  perform a chip erase on the entire serial flash including partition table and
 *  then provide a Sink to allow it to be written from the start.
 *   
 *   \note
 *   If the VM software watchdog is not disabled, BlueCore firmware
 *   extends the VM software watchdog before chip erase operation. Erase
 *   to an external serial flash is time consuming. This ensures that the
 *   VM application is given enough time to kick the VM software watchdog
 *   once erase to an external serial flash is over.
 *  \param device device which needs to be reformatted, cannot be internal flash
 *  \return The sink associated with reformatting of the serial flash.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_reformatsqif
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
Sink StreamReformatSerialFlashSink(partition_filesystem_devices device);
#endif /* TRAPSET_REFORMATSQIF */
#if TRAPSET_FILE

/**
 *  \brief Return a source with the contents of the specified file.
 *   
 *  \param index the file whose contents are requested
 *  \return 0 if index is \#FILE_NONE, or does not correspond to a narrow file.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_file
 */
Source StreamFileSource(FILE_INDEX index);

/**
 *  \brief Open the file in writable filesystem and return a sink structure to that file.
 *   
 *  \param index the name (possibly including a path) of the item to find 
 *  \return The index of the file created, or \#FILE_NONE if could not.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_file
 */
Sink StreamFileSink(FILE_INDEX index);

/**
 *  \brief Return a source with the contents of the specified filesystem.
 *   The StreamFilesystemEnable MIB key must be set to 'true' for this trap to be
 *  used. This MIB key should be disabled before release as this trap
 *   allows anyone with access to the Apps P1 traps to read the contents of
 *  filesystems which may contain sensitive information.
 *   This sink does not support being connected to stream sources, i.e. calling
 *  StreamConnect on a Filesystem source will return 0.
 *   Example:
 *   \code
 *   uint16 size;
 *   Source source = StreamFilesystemSource(FILESYSTEM_ID_DEVICE_RO_FS);
 *   while ((size = SourceBoundary(source)) != 0)
 *   {
 *     const uint8 *data = SourceMap(source);
 *     // do something with the data here
 *     SourceDrop(source, size);
 *   }
 *   SourceClose(source);
 *   \endcode
 *         
 *  \param filesystem_id The filesystem whose contents are requested. Currently
 *  FILESYSTEM_ID_DEVICE_RO_FS is the only supported value.
 *  \return 0 if the provided filesystem_id is not supported or if the
 *  StreamFilesystemEnable MIB key is false. The contents of the filesystem
 *  otherwise.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_file
 */
Source StreamFilesystemSource(FILESYSTEM_ID filesystem_id);

/**
 *  \brief Return a sink for writing an entire filesystem.
 *   The entire filesystem must fit into the available space in the stream so that
 *  it can be written in a single flush.
 *   This is required so that the entire filesystem can be validated before any
 *  erase or write operations happen.
 *   Once the entire filesystem has been written into this stream
 *  SinkFlushBlocking should be called to validate and write the data.
 *   SinkFlushBlocking will return false if any stage of the validate / erase /
 *  write / initialise process fails.
 *   Alternatively, SinkFlush can be used to write the filesystem, but the
 *  operation should only be considered successfully complete when
 *   MESSAGE_MORE_SPACE is received by the sink's associated task. When using
 *  SinkFlush, if any stage of the process fails then, whilst SinkFlush
 *   may have returned TRUE, MESSAGE_MORE_SPACE will not be sent to the sink's
 *  task.
 *   This sink does not support being connected to stream sources, i.e. calling
 *  StreamConnect on a Filesystem sink will return 0.
 *   This sink is automatically closed when a flush completes successfully.
 *   The StreamFilesystemEnable MIB key must be set to 'true' for this trap to be
 *  used. This MIB key should be disabled before release as this trap
 *   allows anyone with access to the Apps P1 traps to modify the contents of
 *  filesystems which may contain sensitive information.
 *   Example:
 *   \code
 *   // Create a Filesystem Sink
 *   Sink sink = PanicNull(StreamFilesystemSink(FILESYSTEM_ID_DEVICE_RO_FS));
 *   uint8 *mapped = SinkMap(sink);
 *   uint16 total = 0;
 *   // Copy each packet to the sink before flushing
 *   while((data = userDefinedGetPacketFunction(&length)))
 *   {
 *     uint16 offset = SinkClaim(sink, length);
 *     memcpy(mapped + offset, data, length);
 *     total += length;
 *   }
 *   // Blocking flush to validate, erase, write and initialise the filesystem.
 *   SinkFlushBlocking(sink, total);
 *   \endcode
 *         
 *  \param filesystem_id The filesystem whose contents are to be written. Currently
 *  FILESYSTEM_ID_DEVICE_RO_FS is the only supported value.
 *  \return 0 if the provided filesystem_id is not supported or if the
 *  StreamFilesystemEnable MIB key is false. A sink for writing to the filesystem
 *  otherwise.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_file
 */
Sink StreamFilesystemSink(FILESYSTEM_ID filesystem_id);
#endif /* TRAPSET_FILE */
#if TRAPSET_USB

/**
 *  \brief Return the USB Class Request Sink. 
 *  \param interface The USB interface (returned by UsbAddInterface) to fetch the Sink for. 
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_usb
 */
Sink StreamUsbClassSink(UsbInterface interface);

/**
 *  \brief Return the USB Request Sink associated with the USB transport. 
 *  \param end_point The USB endpoint (bEndPointAddress field in EndPointInfo structure) to fetch
 *  the Sink for.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_usb
 */
Sink StreamUsbEndPointSink(uint16 end_point);

/**
 *  \brief Return the USB Vendor Sink associated with the USB transport.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_usb
 */
Sink StreamUsbVendorSink(void );
#endif /* TRAPSET_USB */
#if TRAPSET_NFC

/**
 *  \brief  Create an NFC Stream object and return the associated
 *             sink. This is used for the data information to be sent to the NFC
 *             module on P0.
 *              
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_nfc
 */
Sink StreamNfcSink(void );
#endif /* TRAPSET_NFC */
#if TRAPSET_AUDIO

/**
 *  \brief Returns a source for a synthesised sequence of notes.
 *   If the ringtone_note* passed is invalid, the function returns 0.
 *  \param ringtone This must be a pointer to an array of ringtone notes.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_audio
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
Source StreamRingtoneSource(const ringtone_note* ringtone);

/**
 *  \brief Request to create an audio source 
 *    Note that the DSP software/operator framework must be loaded before creating
 *  an audio source.
 *    Check OperatorFrameworkEnable documentation for more details. 
 *         
 *  \param hardware The audio hardware which would be reserved as a source 
 *  \param instance The audio hardware instance (meaning depends on \e hardware) 
 *  \param channel The audio channel (meaning depends on \e hardware) 
 *  \return The Source ID associated with the audio hardware.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_audio
 */
Source StreamAudioSource(audio_hardware hardware, audio_instance instance, audio_channel channel);

/**
 *  \brief Request to create an audio sink 
 *    Note that the DSP software/operator framework must be loaded before creating
 *  an audio sink.
 *    Check OperatorFrameworkEnable documentation for more details.
 *         
 *  \param hardware The audio hardware which would be reserved as a sink 
 *  \param instance The audio hardware instance (meaning depends on \e hardware) 
 *  \param channel The audio channel (meaning depends on \e hardware) 
 *  \return The Sink ID associated with the audio hardware.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_audio
 */
Sink StreamAudioSink(audio_hardware hardware, audio_instance instance, audio_channel channel);

/**
 *  \brief Returns a Sink from the SCO stream passed. 
 *  \param handle The SCO stream from which to fetch the Sink.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_audio
 */
Sink StreamScoSink(uint16 handle);

/**
 *  \brief Returns a Source from the SCO stream passed. 
 *  \param handle The SCO stream from which to fetch the Source.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_audio
 */
Source StreamScoSource(uint16 handle);
#endif /* TRAPSET_AUDIO */
#if TRAPSET_IICSTREAM

/**
 *  \brief Return a source with the contents of the specified I2C address. 
 *   
 *  \param slave_addr The slave address of the device to read data from. 
 *  \param array_addr The array address to read data from. 
 *  \param size The amount of data (in bytes) to read.
 *  \return The source associated with the I2C stream.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_iicstream
 */
Source StreamI2cSource(uint16 slave_addr, uint16 array_addr, uint16 size);
#endif /* TRAPSET_IICSTREAM */
#if TRAPSET_BLUESTACK

/**
 *  \brief Find the Sink corresponding to an L2CAP connection
 *  \param cid The connection ID to fetch the Sink for.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_bluestack
 */
Sink StreamL2capSink(uint16 cid);
#endif /* TRAPSET_BLUESTACK */
#endif
