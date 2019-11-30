/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup obpm Obpm adaptor
 * \ingroup adaptor
 *
 * \file obpm_msg_adaptor.h
 * \ingroup obpm
 *
 *
 */

#ifndef _OBPM_ADAPTOR_H_
#define _OBPM_ADAPTOR_H_

#include "obpm_prim.h"
#include "adaptor.h"

/* Function for sending message to OBPM */
extern bool obpm_adaptor_send_message(unsigned conn_id, ADAPTOR_MSGID msg_id, unsigned length, const unsigned *payload);

#endif /* _OBPM_ADAPTOR_H_ */
