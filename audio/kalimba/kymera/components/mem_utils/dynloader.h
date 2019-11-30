/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file dynloader.h
 *
 * \ingroup mem_util
 *
 * Handle the creation,management,and initialization of capability
 * data and scratch memory.   Also, handles shared constant variables.   
 */

#ifndef DYN_LOADER_HDR
#define DYN_LOADER_HDR
/****************************************************************************
Include Files
*/
#include "types.h"

/****************************************************************************
Public Function Definitions
*/


/**
 * \brief  Reference a constant shared variable 
 * 
 * \param address   Pointer to location to save address to shared variable
 * \param desc      Source Handle for main descriptor
 * \param ext_desc  Source Handle for External descriptor (may be NULL)
 * \param share_id  Idenifier for shared variable
 *
 * \return    Non-Zero on success
 *
 * Given the descriptor handles, this function will create
 *   The variable in data memory.  If it already exists it will increment
 *   The reference count.   This function may not be called while interupts
 *   are disabled.
 */
int DynLoaderProcessSharedAllocations(uintptr_t **address,void *desc,void *ext_desc,unsigned share_id);

/**
 * \brief  Release a reference to constant shared variable
 * 
 * \param share_id  Idenifier for shared variable
 * \param desc      Source Handle for main descriptor
 * \return    Non-Zero on success
 *
 * Given the descriptor handles, this function will release a reference to
 *   the shared vraible.  If this is the last reference the memory will be
 *   freed. This function may not be called while interupts
 *   are disabled.
 */
int DynLoaderReleaseSharedAllocations(unsigned share_id,void *desc);

/**
 * \brief  Reserve Scratch memory for a capability
 * 
 * \param root  Pointer to data array in capability.   
 *                Used as interface to allocated memory.
 * \param desc  Source Handle for main descriptor
 * \param variant  Variant Identifier for memory allocation
 * \return    Non-Zero on success
 *
 * Given the descriptor handles, this function will reserve scratch memory 
 *   for the capability This function may not be called while interupts
 *   are disabled.
 */
int  DynLoaderScratchReserve(uintptr_t *root,void *desc,unsigned variant);

/**
 * \brief  Access Scratch memory for a capability
 * 
 * \param inst_ptr  Pointer to persistent memory control block.
 *                The control block is created when persistent memory
 *                is allocated.
 * \param scr_ptr  Pointer to scratch memory control block.
 *                The control block is created when scratch memory
 *                is reserved.
 * \return    None.  Always succeeds
 *
 */
void DynLoaderScratchCommit(void *inst_ptr,void *scr_ptr);

/**
 * \brief  Release reserved scratch memory
 * 
 * \param ptr  Pointer to scratch memory control block.
 *                The control block is created when scratch memory
 *                is reserved.
 * \return    None.  Always succeeds
 *
 */
void DynLoaderDeregisterScratch(void *ptr);


/**
 * \brief  Allocate Persistent data for a capability
 * 
 * \param root  Pointer to data array in capability.   
 *                Used as interface to allocated memory.
 * \param desc      Source Handle for main descriptor
 * \param ext_desc  Source Handle for External descriptor (may be NULL)
 * \param variant  Variant Identifier for memory allocation
 * \return    Non-Zero on success
 *
 * Given the descriptor handles, this function will allocate persistent memory 
 *   for the capability This function may not be called while interupts
 *   are disabled.
 */
int  DynLoaderProcessDynamicAllocations(uintptr_t *root,void *desc,void *ext_desc,unsigned variant);


/**
 * \brief  Release Persistent data for a capability
 * 
 * \param ptr  Pointer to persistent memory control block.
 *                The control block is created when persistent memory
 *                is allocated.
 * \return    Non-Zero on success
 *
 * Given the descriptor handles, this function will allocate persistent memory 
 *   for the capability This function may not be called while interupts
 *   are disabled.
 */
void DynLoaderReleaseDynamicAllocations(void *ptr);


/**
 * Does the referenced table start with an entry indicating exported to filesystem 
 * 
 * \param ptr  Pointer to main descriptor
 * \return     TRUE if descriptor starts with DYN_SECTION_EXPORT_CONSTANTS
 */
#ifdef INSTALL_CAPABILITY_CONSTANT_EXPORT
bool DynLoaderHasExternalRedirect(void *ptr);
#else
#define DynLoaderHasExternalRedirect(ptr,pFile) (FALSE)
#endif


/**
 * Return the file ID for the referenced file.
 * 
 * \param ptr      Pointer to the descriptor
 * \param file_id  Pointer to variable to hold file ID
 * \returns        TRUE if the descriptor references a file, file_id if supplied will
 *                 have been updated.
 */
#ifdef INSTALL_CAPABILITY_CONSTANT_EXPORT
bool DynLoaderGetExternalRedirectFile(void *ptr,uint16 *pFile);
#else
#define DynLoaderGetExternalRedirectFile(ptr,file) (FALSE)

#endif

#endif
