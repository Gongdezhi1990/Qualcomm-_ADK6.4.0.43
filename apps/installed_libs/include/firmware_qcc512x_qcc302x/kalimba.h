#ifndef __KALIMBA_H__
#define __KALIMBA_H__
#include <app/file/file_if.h>

/*! file  @brief Control of the Kalimba DSP */

#if TRAPSET_KALIMBA

/**
 *  \brief Loads the specified DSP code into Kalimba and sets it running at full speed
 *  with Kalimba in control of the clock.
 *   
 *   \note
 *   The time taken to start the DSP application depends on the details of the DSP
 *   application, which can not be predicted by the BlueCore firmware. If the VM
 *   software watchdog is in use, the VM application should consider the time
 *   taken to start the DSP application when deciding on the timeout value. The
 *   BlueCore firmware will not automatically extend the timeout.
 *   @note
 *   It does not load DSP code when the operators are in use. In this case, 
 *   it returns FALSE. Application needs to destroy running operators to
 *   load the DSP code successfully.
 *  \param file The DSP code to load.
 *  \return TRUE if the DSP was successfully loaded and started, FALSE otherwise.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_kalimba
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
bool KalimbaLoad(FILE_INDEX file);

/**
 *  \brief Turns off power to Kalimba 
 *    @note
 *    This functionality fails when the operator(s) are running.
 *  \return TRUE if the kalimba is powered-off, otherwise FALSE
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_kalimba
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
bool KalimbaPowerOff(void );

/**
 *  \brief Sends a four word message to Kalimba. 
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_kalimba
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
bool KalimbaSendMessage(uint16 message, uint16 a, uint16 b, uint16 c, uint16 d);

/**
 *  \brief Send a long message to Kalimba
 *   
 *  \param message the id of the message 
 *  \param len  the length of the data (limited to 64) 
 *  \param data  the actual data to be sent
 *  \return TRUE if the message was sent, FALSE if the send failed.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_kalimba
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
bool KalimbaSendLongMessage(uint16 message, uint16 len, const uint16 * data);
#endif /* TRAPSET_KALIMBA */
#endif
