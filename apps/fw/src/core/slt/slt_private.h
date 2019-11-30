/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file 
 * Symbol LookUp Table
 *
 */

#ifndef SLT_PRIVATE_H
#define SLT_PRIVATE_H

#include "hydra/hydra_types.h"
#include "hydra/hydra_macros.h"
#include "slt/slt.h"

/** Things of this type will never get into a firmware image.  They are
 * used as a place holder between the linking step and the building of
 * the static slt. */
typedef struct static_slt_entry
{
    apps_slt_id                         symbol_id;
    const void                          *data_ptr;
} static_slt_entry;


#endif /* SLT_PRIVATE_H */
