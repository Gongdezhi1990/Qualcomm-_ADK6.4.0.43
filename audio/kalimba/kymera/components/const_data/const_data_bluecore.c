/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file const_data_bluecore.c
 * \ingroup const_data
 *
 * BlueCore platform implementation of constant data access API.
 */

#include "const_data_private.h"

/****************************************************************************
Public function definitions
*/

/** \brief Copy constant data.
 * Constant data will be copied from the source location to the 'destination'.
 * \param source - the source descriptor containing information about the constant data location and format
 * \param offset - the offset in the source memory where the data read should start from
 * \param destination - the destination memory block pointer, should be at least 'size' memory units long
 * \param size - the size of the data to be transferred
 * Note: the user is responsible for allocation and freeing of the memory pointed by 'destination'.
 */
bool const_data_copy ( const_data_descriptor * source, unsigned offset,
                        void * destination,  unsigned size )
{
   char *src = ((char*)source->address)+offset;

   /* TODO Handle 16-bit sign extension and zero padding */

   if(size==0)
   {
      return FALSE;
   }
   else if (source->type==MEM_TYPE_DMCONST_WINDOWED16)
   {
      if (source->format==FORMAT_PACKED16)
      {
         mem_ext_window_unpack_to_ram(src,size,(unsigned *)destination);
      }
      else
      {
         mem_ext_window_copy_to_ram(src,size,(unsigned *)destination);
      }
   }
   else
   {
      memcpy(destination,src,size);
   }

   return TRUE;
}



/** \brief Access constant data.
 * A pointer to a block of constant data described by 'source' will be returned.
 * \param source - the source descriptor containing information about the constant data location and format
 * \param offset - the offset in the source memory where the data read should start from
 * \param work_buffer - the memory block which will act as a working buffer; if this parameter is NULL a new buffer will be allocated,
          on successive calls this parameter should contain the returned value from the first call to const_data_access. This is required
          due to Kalimba platform differences where locations may or may not be memory mapped (or directly accessible).
 * \param size - the size of the data to be transferred
 */
void * const_data_access ( const_data_descriptor * source, unsigned offset,
                            void * work_buf, unsigned size )
{
  
   /* TODO Handle 16-bit sign extension and zero padding */

    if(source->type==MEM_TYPE_DMCONST_WINDOWED16)
    {
       /* TO DO */
       return NULL;
    }
    else
    {
       return   (void*) (((uint8*)source->address) + offset);
    }
}

/** \brief Release (free) memory block allocated by const_data_access function.
 * \param ptr - pointer to the memory as returned by const_data_access.
 */
void const_data_release ( void * ptr )
{
   /* No Memory is ever allocated */
}
