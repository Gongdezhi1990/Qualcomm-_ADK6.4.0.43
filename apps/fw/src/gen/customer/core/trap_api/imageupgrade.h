#ifndef __IMAGEUPGRADE_H__
#define __IMAGEUPGRADE_H__
#include <app/image_upgrade/image_upgrade_if.h>

/*! file  @brief Image upgrade traps specifically for DFU  */

#if TRAPSET_IMAGEUPGRADE

/**
 *  \brief Get information about an image section. 
 *   @note
 *   It's expected that the application will only invoke this trap when the image
 *  is running from the boot image bank in the input QSPI device.
 *   
 *   
 *  \param qspi QSPI device number where the image section is written. 
 *  \param image_section Image section.
 *  \param key Image upgrade info key. 
 *  \param value Address to return the value of the key. 
 *  \return Successful in getting the requested information about an image section. 
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_imageupgrade
 */
bool ImageUpgradeGetInfo(uint16 qspi, image_section_id image_section, image_info_key key, uint32 * value);

/**
 *  \brief Get the stream sink handle for the image section. 
 *   @note
 *   The sink handle returned should be used to write
 *   the image section data into the QSPI device using the stream traps.
 *   Application shouldn't use this sink handle to connect to any other stream
 *   types using StreamConnect() trap.
 *   It's expected that the application will only invoke this trap when the image
 *  is running from the boot image bank in the input QSPI device.
 *   
 *  \param qspi QSPI device number where the image section is written. 
 *  \param image_section Image section.
 *  \param first_word First 4 bytes of the image section which is written when the stream is closed. 
 *  \return Stream sink handle for writing the image section in the QSPI device. 
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_imageupgrade
 */
Sink ImageUpgradeStreamGetSink(uint16 qspi, image_section_id image_section, uint32 first_word);

/**
 *  \brief Get the current sink write position (in bytes) of the image section sink stream.
 *   @note
 *   This API is used to get the sink position of the specified image upgrade sink
 *  in 
 *   octets. Firmware returns a sink position from which, each word has a value of 
 *   0xFFFF till the end of the image section. Firmware assumes that the image
 *  section 
 *   contents before the sink position are successfully written.
 *   
 *  \param sink Sink handle for the image upgrade stream. 
 *  \param offset  Sink write position of the specified image upgrade sink. 
 *  \return  TRUE if Successful in getting the sink write position of the specified image
 *  upgrade sink, otherwise FALSE 
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_imageupgrade
 */
bool ImageUpgradeSinkGetPosition(Sink sink, uint32 * offset);

/**
 *  \brief Erase the other image bank in all QSPI devices.
 *   @note
 *   It's expected that the application will only invoke this trap when images are
 *  running from the boot image bank in all QSPI devices.
 *   
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_imageupgrade
 */
void ImageUpgradeErase(void );

/**
 *  \brief Reset the chip, load and run using the images (if present) from the other image
 *  bank in all QSPI devices.
 *   @note
 *   It's expected that the application will only invoke this trap when images are
 *  running from the boot image bank in all QSPI devices, and there are 
 *   valid images in the other (new) image bank in all QSPI devices.
 *   
 *   
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_imageupgrade
 */
void ImageUpgradeSwapTry(void );

/**
 *  \brief Get the status of images (if present) running from the other image bank in all
 *  QSPI devices.
 *   
 *  \return Images successfully running from the other image bank. 
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_imageupgrade
 */
bool ImageUpgradeSwapTryStatus(void );

/**
 *  \brief Swap the image bank by modifying the boot image in the QSPI device.
 *   @note
 *   It's expected that the application will only invoke this trap when images are
 *  running from the other (new) image bank in all QSPI devices.
 *   
 *   
 *  \return Boot image successfully updated. 
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_imageupgrade
 */
bool ImageUpgradeSwapCommit(void );

/**
 *  \brief Initialise a hash algorithm context for image upgrade. 
 *   @note
 *   The returned context doesn't need to be interpreted by the application, but
 *  should be
 *   passed in the subsequent image upgrade hash traps to update and finalise the
 *  hash data to the same context.
 *   The created context can be freed by invoking ImageUpgradeHashFinalise trap
 *  with NULL hash pointer.
 *   
 *  \param algo Hash algorithm. 
 *  \return Hash context pointer. 
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_imageupgrade
 */
hash_context_t ImageUpgradeHashInitialise(hash_algo_t algo);

/**
 *  \brief Update the image upgrade hash context with the image section data read from the
 *  other image bank. 
 *   
 *  \param context Hash context. 
 *  \param image_section Image section.
 *  \return TRUE if the hash context is successfully updated, else FALSE. 
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_imageupgrade
 */
bool ImageUpgradeHashSectionUpdate(hash_context_t context, image_section_id image_section);

/**
 *  \brief Update the image upgrade hash context with the input message. 
 *   
 *  \param context Hash context. 
 *  \param msg Pointer to the input message.
 *  \param msg_len_bytes Length (in bytes) of the input message.
 *  \return TRUE if the image upgrade hash context is successfully updated, else FALSE. 
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_imageupgrade
 */
bool ImageUpgradeHashMsgUpdate(hash_context_t context, const uint8 * msg, uint16 msg_len_bytes);

/**
 *  \brief Calculate the hash for the image upgrade hash context. 
 *   @note
 *   The context can be freed by invoking ImageUpgradeHashFinalise trap with NULL
 *  hash pointer.
 *   
 *  \param context Hash context. 
 *  \param hash Pointer to return the hash.
 *  \param hash_len_bytes Length (in bytes) of the expected hash.
 *  \return TRUE if the image upgrade hash is successfully calculated, else FALSE. 
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_imageupgrade
 */
bool ImageUpgradeHashFinalise(hash_context_t context, uint8 * hash, uint16 hash_len_bytes);

/**
 *  \brief Copy unmodified image sections to the new image bank.
 *       
 *    @note
 *    The application should invoke this trap to copy unmodified image sections
 *  during an image upgrade from the boot image bank 
 *    to the new image bank before resetting the chip using the new image bank.
 *    It's expected that the application shall abort copying of the image sections
 *  by invoking ImageUpgradeAbortCommand() trap
 *    if the Host application aborts the image update in between.
 *    It's expected that the application will only invoke this trap when images
 *  are running from the boot image bank in all QSPI devices.
 *     
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_imageupgrade
 */
void ImageUpgradeCopy(void );

/**
 *  \brief Abort an ongoing image upgrade trap activity.
 *       
 *    @note
 *    For now, only an ongoing ImageUpgradeCopy trap execution will be aborted by
 *  invoking ImageUpgradeAbortCommand trap.
 *     
 *  \return TRUE if aborting an ongoing image upgrade activity is successful, else FALSE. 
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_imageupgrade
 */
bool ImageUpgradeAbortCommand(void );

/**
 *  \brief Erase the other image bank in the Audio QSPI and copy the new audio image from
 *  Apps QSPI to Audio QSPI device.
 *       
 *    @note
 *    The application should invoke this trap before invoking the trap to try the
 *  new image (ImageUpgradeSwapTry) to copy the new audio image 
 *    from the Apps QSPI to the Audio QSPI on platforms in which the audio is
 *  running from the external QSPI (flash) device.
 *    As Apps P0 has to get exclusive access to the Audio QSPI to do the copy,
 *  it's expected that the all the
 *    audio services are stopped before invoking this trap. 
 *    Also, no new audio services should be started before the Apps P0 returns the
 *  status of the trap in 
 *    \#MessageImageUpgradeAudioStatus.
 *     
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_imageupgrade
 */
void ImageUpgradeAudio(void );

/**
 *  \brief Update the image upgrade hash validation context data with all the unerased
 *  image sections in the other image bank.
 *       
 *    @note
 *    The application should invoke this trap to add all the image sections
 *  included in an image upgrade to the hash validation context created in
 *    ImageUpgradeHashInitialise() trap. Apps P0 shall update the hash context in
 *  the background and shall return the status of 
 *    the trap in \#MessageImageUpgradeHashAllSectionsUpdateStatus.
 *    This trap should be invoked before copying (ImageUpgradeCopy trap) the
 *  missing image sections from the boot image bank to 
 *    the other image bank during a partial image upgrade.
 *    This trap should be invoked as an alternative to the
 *  ImageUpgradeHashSectionUpdate() trap in which the Apps P0 updates the input
 *  image section blocking the Apps P1. 
 *    It's not expected that the application will invoke both
 *  ImageUpgradeHashSectionUpdate() and ImageUpgradeHashAllSectionsUpdate() to add
 *  image sections to the 
 *    same hash context.
 *     
 *  \param context Hash context. 
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_imageupgrade
 */
void ImageUpgradeHashAllSectionsUpdate(hash_context_t context);

/**
 *  \brief Get the stream source handle for the image section. 
 *   @note
 *   The source handle returned should be used to read the image section 
 *   data in the QSPI device using the stream traps. The source will contain
 *   first_word (written using ImageUpgradeStreamGetSink api) followed by 
 *   rest of image section data. Application shouldn't use this source handle 
 *   to connect to any other stream types using StreamConnect() trap.
 *   It's expected that the application will only invoke this trap when 
 *   the image is running from the boot image bank in the input QSPI device.
 *   
 *  \param qspi QSPI device number from where image section is read. 
 *  \param image_section Image section.
 *  \return Stream source handle for reading the image section in the QSPI device. 
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_imageupgrade
 */
Source ImageUpgradeStreamGetSource(uint16 qspi, image_section_id image_section);
#endif /* TRAPSET_IMAGEUPGRADE */
#endif
