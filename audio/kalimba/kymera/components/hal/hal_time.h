/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup HAL Hardware Abstraction Layer
 * \file  hal_time.h
 *
 * Public header file for HAL get time function.
 *
 * \ingroup HAL
 *
 */
#ifndef _HAL_TIME_H_
#define _HAL_TIME_H_

#define get_time() hal_get_time()
#define time_get_time() hal_get_time()

/**
 * \brief  Get current system time
 */
extern TIME hal_get_time(void);

#endif /* _HAL_TIME_H_ */
