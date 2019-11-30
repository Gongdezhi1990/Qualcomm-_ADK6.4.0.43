/* Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd. */
/*  */
/*!
  @file uuid.h
  @brief Implementation of UUID library.
*/

#include <csrtypes.h>
#include <stddef.h>
#include <string.h>

#include "uuid.h"

#define SWAP(T, a, b) do { T t = a; a = b; b = t; } while (0)

/** Base UUID value common to all standardized Bluetooth services */
static const uint16 base_uuid[] = {0x0000U, 0x1000U, 0x8000U, 0x0080U, 0x5F9BU, 0x34FBU};

void Uuid16Init(uuid_t *uuid, uint16 uuid_data)
{
    uuid->type = UUID_16;
    uuid->uuid[0] = uuid_data;
}

void Uuid32Init(uuid_t *uuid, uint32 uuid_data)
{
    uuid->type = UUID_32;
    uuid->uuid[0] = uuid_data >> 16;
    uuid->uuid[1] = uuid_data & 0xFFFFU;
}

void Uuid128Init(uuid_t *uuid, uint32 uuid_data[4])
{
    int index;

    uuid->type = UUID_128;
    for (index = 0; index < 4; index++)
    {
        uuid->uuid[index * 2]     = uuid_data[index] >> 16;
        uuid->uuid[1 + index * 2] = uuid_data[index] & 0xFFFFU;
    }
}

void Uuid128InitWithBase(uuid_t *uuid, const uuid_base_t* uuid_base, uint32 first32)
{
    int index;

    Uuid32Init(uuid, first32);
    uuid->type = UUID_128;
    for (index = 2; index < 8; index++)
    {
        uuid->uuid[index] = uuid_base->base[index-2];
    }
}

int UuidSize(const uuid_t *uuid)
{
    return (int)uuid->type;
}

uuid_type_t UuidType(const uuid_t *uuid)
{
    return uuid->type;
}

bool UuidHasBase(const uuid_t *uuid, const uuid_base_t *uuid_base)
{
    if (uuid->type != UUID_128)
        return FALSE;
    else
        return !memcmp(&uuid->uuid[2], uuid_base, sizeof(base_uuid));
}

bool Uuid16IsSame(const uuid_t *uuid_a, const uuid_t *uuid_b)
{
    /* Both should be of type UUID_16 */
    if ((uuid_a->type != UUID_16) || (uuid_b->type != UUID_16))
        return FALSE;
    else
        return uuid_a->uuid[0] == uuid_b->uuid[0];
}

bool UuidIsSame(const uuid_t *uuid_a, const uuid_t *uuid_b)
{
    uuid_type_t uuid_a_type = uuid_a->type;
    uuid_type_t uuid_b_type = uuid_b->type;
    const uint16 *uuid_a_data = uuid_a->uuid;
    const uint16 *uuid_b_data = uuid_b->uuid;

    /* Cheap check to see if UUIDs are actually the same structure */
    if (uuid_a == uuid_b)
        return TRUE;

    for (;;)
    {
        /* Easy compare if UUIDs are the same type */
        if (uuid_a_type == uuid_b_type)
        {
            return !memcmp(uuid_a_data, uuid_b_data, (int)uuid_a_type);
        }

        /* Make sure A > B */
        if (uuid_a_type < uuid_b_type)
        {
            SWAP(const uint16 *, uuid_a_data, uuid_b_data);
            SWAP(uuid_type_t, uuid_a_type, uuid_b_type);
        }

        /* Reduce A until it's the same size as B */
        if (uuid_a_type == UUID_128)
        {
            /* Check for Bluetooth Base UUID */
            if (memcmp(uuid_a_data + 2, base_uuid, sizeof(base_uuid)))
            {
                /* No match to Bluetooth Base UUID */
                return 0;
            }
            else
            {
                /* Contains Bluetooth Base UUID, so can reduce down to 32 bit UUID */
                uuid_a_type = UUID_32;
            }
        }
        else if (uuid_a_type == UUID_32)
        {
            /* Check top 16 bits of 32 bit UUID are 0 or not */
            if (uuid_a_data[0])
            {
                /* Non zero top 16 bits */
                return 0;
            }
            else
            {
                /* Top 16 bits are 0, so can reduce down to 16 bit UUID */
                uuid_a_type = UUID_16;
                uuid_a_data += 1;
            }
        }
        
    }
}


void UuidCopy(uuid_t *uuid_a, const uuid_t *uuid_b)
{    
    uuid_a->type = uuid_b->type;
    memcpy(uuid_a->uuid, uuid_b->uuid, sizeof(uuid_a->uuid));
}


bool Uuid128HasBaseGet32(const uuid_t* uuid, uint32* out, const uuid_base_t* uuid_base)
{
    if (uuid->type != UUID_128)
        return FALSE;

    if (!UuidHasBase(uuid, uuid_base))
        return FALSE;

    *out = (((uint32)uuid->uuid[0]) << 16) | (uint32)uuid->uuid[1];
    return TRUE;
}
