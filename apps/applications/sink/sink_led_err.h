/*
Copyright (c) 2018 Qualcomm Technologies International, Ltd.
*/
/*!
\file
\ingroup sink_app
\brief   List the error codes to call the LedsIndicateError function with.
*/

#ifndef SINK_LED_ERR_H
#define SINK_LED_ERR_H


/*
    Error codes used by different modules when calling the LedsIndicateError function.
*/
typedef enum
{
    led_err_id_enum_no_error = 0,
    led_err_id_enum_anc,
    led_err_id_enum_ahi,
    led_err_id_enum_config,
    led_err_id_enum_downloadable_caps
} led_err_id_enum_t;


#endif /* SINK_LED_ERR_H */
