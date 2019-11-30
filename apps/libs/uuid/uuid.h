/* Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd. */
/*  */
/*!
  @file uuid.h
  @brief Interface to UUID library.
*/

#ifndef __UUID_H
#define __UUID_H

#include <csrtypes.h>

/*! @brief Enumeration of type of UUID.
 */
typedef enum uuid_type
{
    UUID_16 = 2,
    UUID_32 = 4,
    UUID_128 = 16
} uuid_type_t;

/*! @brief Definition of a UUID.
 */
typedef struct uuid
{
    uuid_type_t type;
    uint16 uuid[8];
} uuid_t;

/*! @brief Definition of the 96-bit base of a 128-bit UUID.
 */
typedef struct uuid_base
{
    uint16 base[6];
} uuid_base_t;

/*! @brief Initialise a 16-bit UUID.
 */
void Uuid16Init(uuid_t *uuid, uint16 uuid_data);

/*! @brief Initialise a 32-bit UUID.
 */
void Uuid32Init(uuid_t *uuid, uint32 uuid_data);

/*! @brief Initialise a 128-bit UUID.
 */
void Uuid128Init(uuid_t *uuid, uint32 uuid_data[4]);

/*! @brief Initialise a 128-bit UUID with a base and most significant 32-bits.
 */
void Uuid128InitWithBase(uuid_t *uuid, const uuid_base_t* uuid_base, uint32 first32);

/*! @brief Get the size of a UUID.
 */
int UuidSize(const uuid_t *uuid);

/*! @brief Get the type of a UUID.
 */
uuid_type_t UuidType(const uuid_t *uuid);

/*! @brief Compare a UUID with a base.
 */
bool UuidHasBase(const uuid_t *uuid, const uuid_base_t *uuid_base);

/*! @brief Compare two 16-bit UUIDs.
 */
bool Uuid16IsSame(const uuid_t *uuid_a, const uuid_t *uuid_b);

/*! @brief Compare any two UUIDs.
 */
bool UuidIsSame(const uuid_t *uuid_a, const uuid_t *uuid_b);

/*! @brief Copy a UUID.
 */
void UuidCopy(uuid_t *uuid_a, const uuid_t *uuid_b);

/*! @brief Return the most significant 32-bits of a 128-bit UUID.
 */
bool Uuid128HasBaseGet32(const uuid_t* uuid, uint32* out, const uuid_base_t* uuid_base);

#endif

