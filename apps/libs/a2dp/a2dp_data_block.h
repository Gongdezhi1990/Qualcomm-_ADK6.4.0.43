/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    a2dp_data_block.h

DESCRIPTION
    A set of functions to manage creation, deletion and indexing of multiple data blocks
    maintained in a single slot.

*/

#ifndef A2DP_DATA_BLOCK_H_
#define A2DP_DATA_BLOCK_H_

#include "a2dp_private.h"


/****************************************************************************
NAME
    blockInit

DESCRIPTION
    Initialise the data block manager
    
    Returns TRUE on successful initialisation.  Otherwise, returns FALSE.
*/
bool blockInit (void);


/****************************************************************************
NAME
    blockAdd

DESCRIPTION
    Add a data block, of a specified size, to the existing pool of data blocks.
    A pointer to the base of the data block is returned on success, otherwise NULL is returned.
    
    Note: The returned pointer will only be guaranteed to remain valid until a subsequent call to
          blocksAdd or blockRemove is made.
*/
uint8 *blockAdd (uint8 device_id, data_block_id id, uint8 element_count, uint8 element_size);


/****************************************************************************
NAME
    blockRemove

DESCRIPTION
    Remove an existing data block from the pool.
*/
void blockRemove (uint8 device_id, data_block_id id);


/****************************************************************************
NAME
    blockGetBase

DESCRIPTION
    Return pointer to base of requested data block.
    NULL is retured if the data block does not exist.
    
    Note: The base of a data block is equilalent to element zero.
*/
uint8 *blockGetBase (uint8 device_id, data_block_id id);


/****************************************************************************
NAME
    blockGetCurrent

DESCRIPTION
    Return a pointer to the current element within the data block.
    NULL is retured if the data block does not exist.
*/
uint8 *blockGetCurrent (uint8 device_id, data_block_id id);


/****************************************************************************
NAME
    blockSetCurrent

DESCRIPTION
    Set which element is marked as the current element in the sepcified data block.
    Returns TRUE on success or FALSE otherwise.
*/
uint8 *blockSetCurrent (uint8 device_id, data_block_id id, uint8 element);


/****************************************************************************
NAME
    blockGetIndexed

DESCRIPTION
    Return a pointer to the specified element within a data block.  
    NULL is returned if the data block does not exist or the element index is out of range.
    
    Note:  An element index of zero is equilalent to the base address of the data block.
*/
uint8 *blockGetIndexed (uint8 device_id, data_block_id id, uint8 element);


/****************************************************************************
NAME
    blockGetSize

DESCRIPTION
    Returns the size, in bytes, of the specified data block.
    A size of zero will be returned for blocks that do not exist.
*/
uint16 blockGetSize (uint8 device_id, data_block_id id);


#endif /* A2DP_DATA_BLOCK_H_ */
