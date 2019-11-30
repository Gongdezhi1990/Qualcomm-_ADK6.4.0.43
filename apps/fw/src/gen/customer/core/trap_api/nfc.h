#ifndef __NFC_H__
#define __NFC_H__
/** \file */
#if TRAPSET_NFC

/**
 *  \brief  Send an NFC primitive from APP P1 to APP P0. It is 
 *             recommended not to use this function directly.  A separate function 
 *             is provided to send each primitive, use these instead as it 
 *             abstracts the details that may change. The implementation of these 
 *             function is provided in nfc.c and nfc.h.
 *              
 *  \param msg  Content of an NFC primitive.  The nfc primitives are 
 *             defined in the header file "nfc_prim.h".  
 *             
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_nfc
 */
void NfcSendPrim(void * msg);
#endif /* TRAPSET_NFC */
#endif
