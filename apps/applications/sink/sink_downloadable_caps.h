/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_downloadable_caps.h
    
DESCRIPTION
    Passes the downloadable capabilities configuration data from the Config Tool to the libraries.
*/

#ifndef _SINK_DOWNLOADABLE_CAPS_H_
#define _SINK_DOWNLOADABLE_CAPS_H_

#ifdef ENABLE_DOWNLOADABLE_CAPS

/*!
 * @brief Definition of available processor(s) to which a downloadable capability/ bundle file is applicable.
 */
typedef enum
{
    available_to_processor_P0 = 1,
    available_to_processors_P0_and_P1 = 2
}config_processor_t;

/*!
 * @brief Reads in the downloadable capabilities configuration and the initialises the file_list library with 
 * downloadable capability file and processor data.
 *
 */
void sinkDownloadableCapsInit(void);
#else
#define sinkDownloadableCapsInit() ((void) (0))
#endif

#endif /* _SINK_DOWNLOADABLE_CAPS_H_ */
