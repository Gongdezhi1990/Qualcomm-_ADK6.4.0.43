/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
#ifndef ARCHITECTURE_H_INCLUDED
#define ARCHITECTURE_H_INCLUDED

#ifdef CHIP_AMBER
#include "amber.h"
#elif defined CHIP_GORDON
#include "gordon.h"
#elif defined CHIP_RICK
#include "rick.h"
#elif defined CHIP_A7DA_KAS
#include "a7da_kas.h"
#elif defined CHIP_CRESCENDO
#include "crescendo.h"
#if CHIP_BUILD_VER == CHIP_MINOR_VERSION_d00
#error Compiling for Crescendo D00 is not supported anymore.
#endif
#elif defined CHIP_STRE
#include "stre.h"
#elif defined CHIP_NAPIER
#include "napier.h"
#else
#error "Unknown or unsupported CHIP_XXXX"
#endif


#endif

