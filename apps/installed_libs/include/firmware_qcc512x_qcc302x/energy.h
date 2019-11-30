#ifndef __ENERGY_H__
#define __ENERGY_H__
#include <sink_.h>

/*! file  @brief Estimate the energy in a SCO connection */

#if TRAPSET_ENERGY

/**
 *  \brief Turn on energy estimation with both upper and lower thresholds.
 *    One \#MESSAGE_ENERGY_CHANGED message will be sent to the task associated
 *    with the SCO stream when estimated energy content goes outside the bounds.
 *    The message payload will indicate if the energy content went above the
 *    upper threshold, or below the lower one. The task must then
 *    reenable the energy estimation if it wishes to receive further messages.
 *    Estimation is automatically stopped when the SCO connection is closed.
 *    It can be stopped under program control using EnergyEstimationOff().
 *             
 *  \param sco  The SCO stream to enable energy estimation on. 
 *  \param lower  The lower bound on energy 
 *  \param upper  The upper bound on energy
 *  \return FALSE if the sink does not correspond to a SCO connection.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_energy
 */
bool EnergyEstimationSetBounds(Sink sco, uint16 lower, uint16 upper);

/**
 *  \brief Disable estimation on the specified SCO connection. 
 *  \param sco The SCO connection to disable energy estimation on. 
 *  \return FALSE if the sink does not correspond to a SCO connection.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_energy
 */
bool EnergyEstimationOff(Sink sco);
#endif /* TRAPSET_ENERGY */
#endif
