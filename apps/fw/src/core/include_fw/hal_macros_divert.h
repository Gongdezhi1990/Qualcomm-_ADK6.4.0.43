/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
#ifndef HAL_MACROS_DIVERTS_FIRMWARE_H_
#define HAL_MACROS_DIVERTS_FIRMWARE_H_

#include "hal/hal_data_conv_access.h"

#define hal_get_reg_timer_time_lsw() (hal_get_reg_timer_time() & 0xffff)



/* HAL macros are not correct for host registers that are more
 * than 16-bits wide: B-234061.
 *
 * Workaround for USB2_EP_CONFIG_RX_FREE_SPACE_IN_BUFFER: we know that 
 * despite being 24-bit wide it wraps around at 64KB, so it is OK
 * to drop the upper bits. */
#ifdef hal_get_reg_usb2_ep_config_rx_free_space_in_buffer
#undef hal_get_reg_usb2_ep_config_rx_free_space_in_buffer
#define hal_get_reg_usb2_ep_config_rx_free_space_in_buffer() \
        hal_get_register( \
                *(uint32 *)(((uint8 *)&USB2_EP_CONFIG_RX_FREE_SPACE_IN_BUFFER)+4), 1)
#endif

#endif /* HAL_MACROS_DIVERTS_FIRMWARE_H_ */
