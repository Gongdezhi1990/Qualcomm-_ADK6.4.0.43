#ifndef __STATUS_H__
#define __STATUS_H__
#include <message_.h>
#include <app/status/status_if.h>

/*! file  @brief Access to chip status information.
** 
** 
Functions to access the chip status information.
*/

#if TRAPSET_STATUS

/**
 *  \brief Queries the value of the specified status fields.
 *   The current value of each of the fields will be written to the
 *   corresponding element of the results array.
 *  \param count The number of status fields to read. 
 *  \param fields count values each specifying one status value 
 *  \param results count values which will be written to
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_status
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
void StatusQuery(uint16 count, const status_field * fields, uint16 * results);
#endif /* TRAPSET_STATUS */
#endif
