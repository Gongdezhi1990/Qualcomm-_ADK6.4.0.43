/*******************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.
 
*******************************************************************************/

/*
  Process regions of uint8 memory

  Used by the service library

  JBS, 23 June 2004
*/

/*!
\defgroup region region
\ingroup vm_libs

\brief Processes regions of uint8 memory

\section region_intro INTRODUCTION
  This library is used by the service library.
*/

/** @{ */

#ifndef REGION_H_
#define REGION_H_

#include <csrtypes.h>

/*!
    @brief A memory region
*/
typedef struct 
{
    const uint8 *begin;  /*!< The end of the region.*/
    const uint8 *end;    /*!< The begining of the region.*/
} Region;

/*!
    @brief The size of the region.
    @param r The region.
*/
#define RegionSize(r) ((uint16)((r)->end - ((r)->begin)))

/*!
    @brief Write an unsigned value to a region.
    @param r The region to write to.
    @param value The value to write.
*/
void RegionWriteUnsigned(const Region *r, uint32 value);

/*!
    @brief Read an unsigned value from a region.
    @param r The region to read from.
*/
uint32 RegionReadUnsigned(const Region *r);

#define RegionReadUint16(region) (uint16)(RegionReadUnsigned(region))
#define RegionReadUint32(region) (RegionReadUnsigned(region))

/*!
    @brief Checks that the contents of a region matches the UUID32 passed.
    @param r The region to compare.
    @param uuid The uuid to compare.
*/
bool RegionMatchesUUID32(const Region *r, uint32 uuid);

/*!
    @brief Checks that the contents of a region matches the UUID128 passed.
    @param r The region to compare.
    @param uuid The uuid to compare.
*/
bool RegionMatchesUUID128(const Region *r, const uint8 *uuid);

#endif /* REGION_H_ */

/** @} */
