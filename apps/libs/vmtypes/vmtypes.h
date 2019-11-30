/*******************************************************************************
Copyright (c) 2015-2018 Qualcomm Technologies International, Ltd.
 
*******************************************************************************/

#ifndef _VMTYPES_H
#define _VMTYPES_H

#ifndef STATIC_ASSERT
#define STATIC_ASSERT(COND, MSG) typedef char static_assertion_##MSG[(COND)?1:-1]
#endif

#ifndef BITFIELD
#define BITFIELD    unsigned
#endif

#define BITFIELD_CAST(bit_width, value) (((1 << bit_width) - 1) & (unsigned)value)

#ifndef PACK_STRUCT
#define PACK_STRUCT
#endif

#ifndef UNUSED
#define UNUSED(var)     (void)(var)
#endif

#ifndef MAX
#define MAX(a,b)        (((a) < (b)) ? (b) : (a))
#endif

#ifndef MIN
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#endif

/* Make sure that any sizes (in uint16 resolution) of structures mapping
   directly into PSKeys are passed as the correct number of uint16s.
   ie round up sizes rather than round down if sizeof() has octet resolution */
#define PS_SIZE_ADJ(X) (((X) + sizeof(uint16) - 1) / sizeof(uint16))

#ifndef MIN
#define MIN(a,b)    ( ((a)<(b)) ? (a) : (b) )
#endif

#ifndef MAX
#define MAX(a,b)    ( ((a)>(b)) ? (a) : (b) )
#endif

#ifdef HYDRACORE

/*
    Crescendo enums have variable sizes, the following macro ensures an emums size will be at least 16 bits.
    The reason being in some cases, to ensure compatibility, enums must have a size of 16 bits.

    This macro should be the last entry in an enums definition.
    It needs the tag parameter to create unique symbols in order to avoid compiler errors.
*/
#define FORCE_ENUM_TO_MIN_16BIT(tag) dummy_enum_entry__##tag##__ = 0xFFFF

#else /* HYDRACORE */

/*
    For xap we want to leave things alone so define the end enum to be 0
*/
#define FORCE_ENUM_TO_MIN_16BIT(tag) dummy_enum_entry__##tag##__ = 0

#endif /* HYDRACORE */

#endif
