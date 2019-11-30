#ifndef __OS_H__
#define __OS_H__
/** \file */
#if TRAPSET_CORE

/**
 *  \brief Initialises the P1 Operating System. Must be called before doing anything else.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
void OsInit(void );
#endif /* TRAPSET_CORE */
#endif
