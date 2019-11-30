#ifndef __PS_H__
#define __PS_H__
#include <app/ps/ps_if.h>

/*! file 
@brief Access to persistent store.
** 
A user application running on the VM can access the persistent store on 
BlueCore and Apps Subsystem. 
These provide a small amount of non-volatile storage.
** 
For the Apps Subsystem, up to two hundred and ninety keys can be used, 0 to 289. 
There are no predefined meanings for these keys; they can be used for any 
purpose required by the user application. 
The ADK example application assigns these keys the following functions:-
PSKEY_USR          Configuration of state for P1 applications
PSKEY_DSP          Formerly used on Bluecore for Audio DSP. Data now used for by
                   ANC tuning (Audio plugin)
PSKEY_CONNLIB      Storage reserved for connection library on P1
PSKEY_CUSTOMER     Reserved for customer use.
                   Not used by the ADK example application.
** 
These keys are divided into six blocks.
- User configuration data (0 to 49). 
- DSP configuration (50 to 99).
- VM Connection Library data (100 to 149) 
- User configuration data, 2nd block (150 to 199).
- Customer configuration data, (200 to 289).
- Customer configuration data, 2nd block (300-509).
** 
The total amount of data associated with all the keys should be kept 
as small as possible.
**
The maximum length of persistent store key that is guaranteed to be supported by
the BlueCore firmware and its supporting tools is 64 words. Attempts to use
longer persistent store keys may return failure or may cause undefined firmware
behaviour.
** 
@par Notes
The persistent store is held in flash memory. This means that only a finite
number of changes can be made to values held in persistent store without
performing a defragmentation of the flash. It is a time and memory
intensive operation, hence the number of changes to the value of a key should
be kept to a minimum. Refer to PsDefrag() trap for details.
** 
@par Notes
Doing error checks on a key's value costs code space and usually has no
effect.If the value of a key gives an error then it should be detected while the
module is being designed (or by the tools if you have simple keys) and so the
error checking is redundant in the field. The only exception is where it's
possible to set a key in such a way that the chip doesn't boot. In this case,
the user may be unable to recover the module once they've set the key. In this
case, you may want to consider issuing a fault and falling back to sane values
if the key is wrong.  It may be worth considering overriding certain settings
depending on other keys as a way of avoiding synchronised settings.  If you end
up overriding a whole key then the key is redundant and can be removed. If you
override part of a key, say a bit fielded key, then it costs little code to
simply apply a mask to the value read from persistent store and then set bits
later.
*/
/*!
@brief Deprecated trap, was used to fill PS store with keys to force reboot
and defragmentation.
New applications should use PsDefrag() trap instead, but for compatibility
with legacy code PsFlood() is still defined and points at PsDefrag().
*/
#define PsFlood PsDefrag 

#if TRAPSET_CORE

/**
 *  \brief Copy the specified memory buffer to persistent store. 
 *   A Persistent store user key can be deleted by calling the PsStore() function
 *   with \e key to be deleted and \e words argument being zero.
 *   
 *   PsStore() trap can trigger defragmentation if PS store is filled to over 90%.
 *   Defragmentation runs in the background and can cause some
 *   disruption to time-sensitive activities (e.g. streaming audio) depending
 *   on overall system load. It is recommended that the application monitors the
 *   PS store level with PsFreeCount() and calls PsDefrag() when idle to avoid 
 *   potential disruption.
 *   
 *   \note
 *   The \e key is an Apps Subsystem key and is interpreted as an index into 
 *   PSKEY_USR0 ... PSKEY_USR49,  PSKEY_DSP0 ... PSKEY_DSP49, 
 *   PSKEY_CONNLIB0 ... PSKEY_CONNLIB49,  PSKEY_USR50 ... PSKEY_USR99,   
 *   PSKEY_CUSTOMER0 ... PSKEY_CUSTOMER89, PSKEY_CUSTOMER90 ... PSKEY_CUSTOMER299
 *   No other keys can be written from an application.
 *  \param key The persistent store key to copy data to. 
 *  \param buff The memory buffer to copy data from,
 *             must be word aligned. 
 *  \param words The number of uint16 words to copy.
 *  \return The number of words actually written.
 *           Zero if either \e words is zero or the store failed.
 *           Store can fail for the following reasons:
 *           - \e key is greater than \#PSKEY_MAX_APPKEY_INDEX.
 *           - \e buff pointer is not uint16 word aligned.
 *           - PS store is locked by ImageUpgradeSwapTry() during DFU.
 *           - PS store does not have enough space for the key, defragmentation
 *           needs to be started with PsDefrag() trap.
 *           
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
uint16 PsStore(uint16 key, const void * buff, uint16 words);

/**
 *  \brief Copy to specified memory buffer from persistent store. 
 *   \note
 *   The key is an Apps Subsystem key and is interpreted as an index into 
 *   PSKEY_USR0 ... PSKEY_USR49,  PSKEY_DSP0 ... PSKEY_DSP49, 
 *   PSKEY_CONNLIB0 ... PSKEY_CONNLIB49,  PSKEY_USR50 ... PSKEY_USR99,   
 *   PSKEY_CUSTOMER0 ... PSKEY_CUSTOMER89, PSKEY_CUSTOMER90 ... PSKEY_CUSTOMER299
 *   No other keys can be read by an application using this function.
 *  \param key The persistent store key to copy data from. 
 *  \param buff The memory buffer to copy data to,
 *             must be word aligned.
 *  \param words The number of uint16 words to copy. 
 *  \return If \e buff is NULL and \e words is zero,
 *           returns the minimum length in words of the buffer necessary to hold
 *           the contents of \e key. Otherwise, returns the number of words
 *           actually read, or zero if the key does not exist or is longer
 *           than \e words.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
uint16 PsRetrieve(uint16 key, void * buff, uint16 words);

/**
 *  \brief Start PS store defragmentation in the background
 *   
 *   Changing a key with PsStore() trap adds a new instance of the key to PS store.
 *   After some time the store becomes filled with multiple instances of the same
 *   keys that consume space. To remove the old data and recover space in
 *   the store, defragmentation needs to be performed from time to time.
 *   Defragmentation is started automatically if after a PsStore request the store
 *   is more than 90% full or can be started manually at any time using this trap.
 *   
 *   Unlike previous chips, defragmentation runs in the background and normally
 *   is completely transparent to the application. However it can cause some
 *   disruption to time-sensitive activities (e.g. streaming audio) depending
 *   on overall system load. It is recommended for the application to monitor
 *   PS store level with PsFreeCount() and call PsDefrag() preventively when idle.
 *   
 *   For the reference with 64KB store, PsFreeCount(16) returns 142 when
 *   store is 90% full and 1055 when store is 90% empty.
 *   
 *   Defragmentation can take up to several seconds to complete depending on
 *   the store content, system load and flash chip performance. Application
 *   can use PsFreeCount() trap to determine when defragmentation has finished and
 *   more space becomes available.
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
void PsDefrag(void );

/**
 *  \brief Return how many keys of this size in words we could write. 
 *  \param words The key size in words to use.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
uint16 PsFreeCount(uint16 words);

/**
 *  \brief Read any persistent store key which could be accessed from off-chip using
 *  BCCMD. 
 *   Note this function can be used to obtain values both for Bluecore PS keys and
 *   for Apps Subsystem PS keys. Unlike PsStore() and PsRetrieve() it does not use
 *   a PS key index in the range [0, 289]. It can be used for three different
 *   purposes.
 *   
 *   1. To obtain legacy Bluecore PS keys now stored as MIB keys in the Apps
 *  Subsystem.
 *   2. To obtain Apps Subsystem keys.
 *   3. To obtain Bluetooth Subsystem keys.
 *   
 *   For 1) the following keys may be used:
 *   
 *   PS Key Name                           | PS Key ID | Associated MIB key
 *   ------------------------------------- | --------- | -------------------
 *   PSKEY_THERMAL_SHUTDOWN_PERIOD         | 9704      | 255
 *   PSKEY_SHUTDOWN_TEMPERATURE            | 9705      | 256
 *   PSKEY_THERMAL_SHUTDOWN_LONG_PERIOD    | 9706      | 257
 *   PSKEY_RAMPDOWN_TEMPERATURE            | 32000     | 258
 *   PSKEY_REENABLE_TEMPERATURE            | 32001     | 259
 *   PSKEY_CHARGER_MAXIMUM_CURRENT_TRICKLE | 32002     | 260
 *   PSKEY_CHARGER_MAXIMUM_CURRENT_PRE     | 32003     | 261
 *   PSKEY_CHARGER_MAXIMUM_CURRENT_FAST    | 32004     | 262
 *   PSKEY_CHARGER_MAXIMUM_VOLTAGE         | 9691      | 263
 *   PSKEY_USB_BCD_DEVICE                  | 125       | 14
 *   PSKEY_USB_VERSION                     | 700       | 11
 *   PSKEY_USB_DEVICE_CLASS_CODES          | 701       | 16
 *   PSKEY_USB_VENDOR_ID                   | 702       | 12
 *   PSKEY_USB_PRODUCT_ID                  | 703       | 13
 *   PSKEY_USB_MANUF_STRING                | 705       | 7
 *   PSKEY_USB_PRODUCT_STRING              | 706       | 8
 *   PSKEY_USB_SERIAL_NUMBER_STRING        | 707       | 9
 *   PSKEY_USB_CONFIG_STRING               | 708       | 10
 *   PSKEY_USB_MAX_POWER                   | 709       | 17
 *   PSKEY_USB_LANGID                      | 713       | 15
 *   
 *   The ten PSKEY_READONLY(n) keys map onto the equivalent CustomerReadOnlyKey(n) 
 *   MIBKEYs. These can be read but not written by the application.
 *   
 *   PS Key Name       | PS Key ID | Associated MIB key
 *   ----------------- | --------- | -------------------
 *   PSKEY_READONLY0   | 10282     | 295
 *   PSKEY_READONLY1   | 10283     | 296
 *   PSKEY_READONLY2   | 10284     | 297
 *   PSKEY_READONLY3   | 10285     | 298
 *   PSKEY_READONLY4   | 10286     | 299
 *   PSKEY_READONLY5   | 10287     | 300
 *   PSKEY_READONLY6   | 10288     | 301
 *   PSKEY_READONLY7   | 10289     | 302
 *   PSKEY_READONLY8   | 10290     | 303
 *   PSKEY_READONLY9   | 10291     | 304
 *   
 *   
 *   For 2) these keys may be used:
 *   
 *   PS Key ID Name       | PS Key ID Value | 0..509 space as used by PsRetrieve
 *   -------------------- | --------------- | -----------------------------------
 *   [PSKEY_USR0, PSKEY_USR49]             | [650, 699]       | [0, 49]
 *   [PSKEY_DSP0, PSKEY_DSP49]             | [8792, 8841]     | [50, 99]
 *   [PSKEY_CONNLIB0, PSKEY_CONNLIB49]     | [9742, 9791]     | [100, 149]
 *   [PSKEY_USR50, PSKEY_USR99]            | [10142, 10191]   | [150, 199]
 *   [PSKEY_CUSTOMER0, PSKEY_CUSTOMER89 ]  | [10192, 10281]   | [200, 289]
 *   [PSKEY_CUSTOMER90, PSKEY_CUSTOMER300 ]| [10292, 10502]   | [300, 509]
 *   
 *   For 3) any known Bluetooth Subsystem PS key may be used.    
 *   
 *   Note that you can override any value with a zero-length value,
 *   so testing for a zero-length value does not have this problem.
 *   Usually this means you can obey this rule by allowing a zero length key
 *   to behave the same as an absent key.
 *   
 *   The correct way to read a key which may be a zero length pskey
 *   is as follows
 *   
 *       uint16 pio = 0xffff;
 *       if (PsFullRetrieve(PSKEY_*., &pio, sizeof(pio)) && pio <= 15)
 *   
 *   PsFullRetrieve leaves the initialisation of pio unchanged if the key isn't
 *  present,
 *   PsFullRetrieve returns FALSE if the key doesn't exist.
 *  \param key The persistent store key to copy data from. 
 *  \param buff The memory buffer to copy data to,
 *             must be word aligned. 
 *  \param words The number of uint16 words to copy.
 *  \return If \e buff is NULL and \e words is zero,
 *           returns the minimum length in words of the buffer necessary
 *           to hold the contents of \e key. Otherwise, returns the number
 *           of words actually read, or zero if the key does not exist or
 *           is longer than \e words.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
uint16 PsFullRetrieve(uint16 key, void * buff, uint16 words);

/**
 *  \brief Copy the specified memory buffer to audio persistent store. 
 *   A Persistent store audio key can be changed by the application by invoking
 *  this trap.
 *   
 *   Since audio keys can be long, this API allows writing the new
 *   value in several steps using the \p offset field.
 *   
 *   Note, that keys can't be partialy updated, the whole new value of a key
 *   must be written. Also, blocks of data can't be written out of order.
 *   The write is finalised and the new key value becomes effective
 *   once the last block of data is written.
 *         
 *  \param key The persistent store audio key to copy data to. 
 *  \param buffer The memory buffer to copy data from. 
 *  \param words Number of word of key data to write.
 *  \param offset Number of words from the start of the key to write data.
 *   Block of data will be written at this offset. This must always be equal
 *   to the number of words written so far, out-of-order data writes are not
 *  supported.
 *             
 *  \param key_length Length of the key data in uint16 words.
 *   This value gives overall key length and must be the same when calling the trap
 *   several times to write blocks of key data. The key update completes and
 *   the new value becomes effective once exactly this many words have been
 *  written.
 *   
 *  \return The number of words actually written.
 *           Zero if either \p words is zero or the update failed.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
uint16 PsUpdateAudioKey(uint32 key, const uint16 * buffer, uint16 words, uint16 offset, uint16 key_length);

/**
 *  \brief Copy to specified memory buffer from audio persistent store. 
 *   A Persistent store audio key can be read by the application by invoking this
 *  trap.
 *   
 *   The trap can be used to read any part of the key data using the
 *   \p offset and \p words fields.
 *   
 *   The trap can also be used to find the key length without copying any data: 
 *   
 *       PsReadAudioKey(key, NULL, 0, 0, &key_len);
 *   
 *         
 *  \param key The persistent store audio key to copy data from. 
 *  \param buffer The memory buffer to copy data to. 
 *  \param words The number of words to copy. 
 *  \param offset Number of words from the start of the key to read data.
 *  \param key_length_addr Pointer to return the length of the key in uint16 words.
 *   Can be "NULL" if the length is not needed.
 *  \return Returns 0 if input \e buffer is NULL.
 *           Otherwise, returns the number of words actually read, or zero if the
 *  key does not exist.
 *           
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
uint16 PsReadAudioKey(uint32 key, uint16 * buffer, uint16 words, uint16 offset, uint16 * key_length_addr);

/**
 *  \brief Set the PS Store that are used for subsequent PS operations 
 *   \note
 *   Any store in the BlueCore firmware can be set as default using this trap.
 *  \param store The PS store to be used
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
void PsSetStore(PsStores store);

/**
 *  \brief Returns the current PS store used for PS operations. 
 *  \return The PS store currently used.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
PsStores PsGetStore(void );
#endif /* TRAPSET_CORE */
#if TRAPSET_PARTITION

/**
 *  \brief Copy the specified memory buffer into PSKEY_FSTAB within the persistent store
 *   The function operates in a similar manner to \see PsStore but only operates
 *   on the PSKEY_FSTAB persistent store key. The parameter commit is used to set
 *   which store the value will be written to. The data to write must always be
 *   passed to the function.
 *   
 *  \param buff The memory buffer to copy data from. 
 *  \param words The number of words to copy. 
 *  \param commit Write the new FSTAB to non-volatile memory or not. If FALSE then the new FSTAB
 *  will only be stable across warm reboots; this feature can be used to enable
 *  the newly upgrade application to perform system checks before setting the
 *  FSTAB in the non-volatile store.
 *  \return TRUE if the operation succeeded or FALSE if it failed
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_partition
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
bool PsStoreFsTab(const void * buff, uint16 words, bool commit);
#endif /* TRAPSET_PARTITION */
#endif
