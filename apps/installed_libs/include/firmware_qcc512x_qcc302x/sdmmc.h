#ifndef __SDMMC_H__
#define __SDMMC_H__
#include <app/sd_mmc/sd_mmc_if.h>



#if TRAPSET_SD_MMC

/**
 *  \brief Initialise/deinitialise SD host slot driver.
 *         Initialisation will be done of the slots for which MIB key is set
 *         (SDHostSlot0 and SDHostSlot1).
 *         If none of the key is set, no operation will be performed but trap will
 *         return TRUE. It will return FALSE if initialisation operation fails.
 *         For deinitialisation; all initialised slots will be deinitialised.
 *         If no slot was initialised, no operation will be performed but trap will
 *         return TRUE. It will return FALSE if deinitialisation operation fails.
 *         On QCC512x, QCC302x and QCC303x, once SD host is initialised, \c
 *  VmRequestRunTimeProfile()
 *         changes will pend until the SD host slot driver is deinitialised by
 *  this API.
 *   
 *  \param init Initialise (TRUE) or Deinitialise (FALSE) slot driver.
 *  \return True if operation is successful, otherwise FALSE.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_sd_mmc
 */
bool SdMmcSlotInit(bool init);

/**
 *  \brief Read data from an SD card.
 *   
 *  \param slot SD slot number, counting from 0
 *  \param buff Pointer to a block of memory with a minimum size of 512*blocks_count bytes.
 *  \param start_block Number of the first 512-byte block to be read from the card.
 *  \param blocks_count Total number of 512-byte blocks to be read from the card.
 *  \return True if operation is successful, otherwise FALSE.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_sd_mmc
 */
bool SdMmcReadData(uint8 slot, uint8 * buff, uint32 start_block, uint32 blocks_count);

/**
 *  \brief Fetch information of an SD card.
 *   
 *  \param slot SD slot number of the card, counting from 0. 
 *  \param device_info Pointer to structure (sd_mmc_dev_info) where device
 *             information will be written if operation succeeds.
 *  \return True if operation is successful, otherwise FALSE.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_sd_mmc
 */
bool SdMmcDeviceInfo(uint8 slot, sd_mmc_dev_info * device_info);
#endif /* TRAPSET_SD_MMC */
#endif
