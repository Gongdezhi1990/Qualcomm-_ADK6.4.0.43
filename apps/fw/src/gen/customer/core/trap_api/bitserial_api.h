#ifndef __BITSERIAL_API_H__
#define __BITSERIAL_API_H__
#include <app/bitserial/bitserial_if.h>

/*! file  @brief Control of Bitserial. 
** 
**
Bitserial traps.
*/

#if TRAPSET_BITSERIAL

/**
 *  \brief Init and configure Bitserial hardware.
 *         Init and configure Bitserial hardware, preparing it for Bitserial
 *         operations.
 *         PIOs can be assigned to the Bitserial at any point. This is done by
 *         using PioSetMapPins32Bank to put the PIO under HW control and then
 *         PioSetFunction to set the function to one of the Bitserial signals.
 *         Please note that one PIO can be assigned for multiple signals at the
 *         same time. All Bitserial signals are initially muxed to
 *         PIO 96 (which doesn't exist) to make sure there are no conflicts. Once a
 *         PIO is muxed to a Bitserial signal, that signal cannot be muxed back to
 *         PIO 96.
 *         If the PIOs are assigned to the Bitserial before the Bitserial is opened
 *         then the state of the PIOs is undetermined during the execution of
 *         BitserialOpen. It is therefore recommended that the call to
 *         PioSetFunction happens some time after the call to BitserialOpen to
 *         avoid glitches.
 *         
 *  \param block_index Bitserial hardware instance to work with.
 *  \param config Bitserial hardware configuration.
 *  \return Bitserial handle to be used with all Bitserial traps.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_bitserial
 */
bitserial_handle BitserialOpen(bitserial_block_index block_index, const bitserial_config * config);

/**
 *  \brief Close and deconfigure Bitserial instance.
 *         Close and deconfigure Bitserial hardware and resources. Please note that
 *         this does not affect PIO settings.
 *         During the execution of BitserialClose, the state of any PIOs still
 *         assigned to the Bitserial is undetermined. It is therefore recommended
 *         to re-configure the PIOs using PioSetMapPins32Bank and/or
 *         PioSetFunction some time before the call to BitserialClose to avoid
 *         glitches.
 *         
 *  \param handle Bitserial handle obtained with BitserialOpen().
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_bitserial
 */
void BitserialClose(bitserial_handle handle);

/**
 *  \brief Perform a combined Bitserial write + read operation.
 *         Peforms Bitserial write followed by read without deasserting CS (SPI) / 
 *         sending STOP (I2C) in between.
 *         
 *         Depending on parameters, one of the following three sequences can be
 *  requested:
 *         TX RX: rx_data/tx_data != NULL, rx_size/tx_size != 0;
 *         RX: rx_data != NULL, rx_size != 0, tx_data == NULL, tx_size == 0; 
 *         TX: tx_data != NULL, tx_size != 0, rx_data == NULL, rx_size == 0.
 *         
 *         "transfer_handle_ptr" controls whether MESSAGE_BITSERIAL_EVENT message
 *         is sent on transfer completion. If transfer_handle_ptr ==
 *         BITSERIAL_NO_MSG ("NULL"), then no message is sent on completion.
 *         
 *         For this trap "transfer_handle_ptr" has additional effect: if
 *         transfer_handle_ptr == BITSERIAL_NO_MSG, then blocking operation
 *         is performed, otherwise the operation is non-blocking.
 *         \note
 *         Tx Data is copied into a buffer in BitserialTransfer function.
 *         As soon as BitserialTransfer trap returns, tx_data buffer can be freed.
 *         
 *  \param handle Bitserial handle
 *  \param transfer_handle_ptr Location for returning transfer handle.
 *  \param tx_data Data for transmission
 *  \param tx_size Transmission size.
 *  \param rx_data Location for received data.
 *  \param rx_size Amount of data to receive.
 *  \return Result of transfer queueing or completion.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_bitserial
 */
bitserial_result BitserialTransfer(bitserial_handle handle, bitserial_transfer_handle * transfer_handle_ptr, const uint8 * tx_data, uint16 tx_size, uint8 * rx_data, uint16 rx_size);

/**
 *  \brief Perform a Bitserial write operation
 *         Performs a Bitserial write operation. The "flags" argument allows
 *         control over transfer parameters.
 *         
 *         Blocking operation can be requested by setting BITSERIAL_FLAG_BLOCK
 *         in "flags". Otherwise the operation is non-blocking.
 *         
 *         "transfer_handle_ptr" controls whether MESSAGE_BITSERIAL_EVENT message
 *         is sent on transfer completion. If transfer_handle_ptr ==
 *         BITSERIAL_NO_MSG ("NULL"), then no message is sent on completion.
 *         \note
 *         Data is copied into a buffer in BitserialWrite function. 
 *         As soon as BitserialWrite trap returns, data buffer can be freed.
 *         
 *  \param handle Bitserial handle
 *  \param transfer_handle_ptr Location for returning transfer handle.
 *  \param data Data for transmission
 *  \param size Transmission size.
 *  \param flags Bitserial transfer flags.
 *  \return Result of transfer queueing or completion.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_bitserial
 */
bitserial_result BitserialWrite(bitserial_handle handle, bitserial_transfer_handle * transfer_handle_ptr, const uint8 * data, uint16 size, bitserial_transfer_flags flags);

/**
 *  \brief Perform a Bitsrial read operation
 *         Performs a Bitserial read operation. The "flags" argument allows
 *         control over transfer parameters.
 *         
 *         Blocking operation can be requested by setting BITSERIAL_FLAG_BLOCK
 *         in "flags". Otherwise the operation is non-blocking.
 *         
 *         "transfer_handle_ptr" controls whether MESSAGE_BITSERIAL_EVENT message
 *         is sent on transfer completion. If transfer_handle_ptr ==
 *         BITSERIAL_NO_MSG ("NULL"), then no message is sent on completion.
 *         
 *  \param handle Bitserial handle
 *  \param transfer_handle_ptr Location for returning transfer handle.
 *  \param data Location for received data.
 *  \param size Amount of data to receive.
 *  \param flags Bitserial transfer flags.
 *  \return Result of transfer queueing or completion.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_bitserial
 */
bitserial_result BitserialRead(bitserial_handle handle, bitserial_transfer_handle * transfer_handle_ptr, uint8 * data, uint16 size, bitserial_transfer_flags flags);

/**
 *  \brief Perform a Bitserial parameter change operation
 *         Queues a Bitserial parameter change operation. This will take effect
 *         after all the previous operations finish. There is no Bitserial event
 *         message sent for this operation.
 *         
 *  \param handle Bitserial handle
 *  \param key Bitserial parameter to change
 *  \param value The new value of the Bitserial parameter
 *  \param flags Bitserial operation flags.
 *  \return Result of operation queueing or completion.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_bitserial
 */
bitserial_result BitserialChangeParam(bitserial_handle handle, bitserial_changeable_params key, uint16 value, bitserial_transfer_flags flags);
#endif /* TRAPSET_BITSERIAL */
#endif
