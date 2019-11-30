/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file 
 * Implementation of bitserial HAL layer. This file contains functions which 
 * are interfaced directly to the HW, hence this should be the only place for 
 * changes when porting the bitserial FW on another platform.
 */

#include "hal/hal_bitserial.h"
#include "panic/panic.h"

#ifdef INSTALL_BITSERIAL

#ifdef HYDRA_BITSERIAL
#include "bus_interrupt/bus_interrupt.h"
#include "int/swint.h"
#elif defined(INSTALL_HAWKEYE) || defined(INSTALL_VULTAN) || \
                                                      defined (INSTALL_TRINITY)
#endif /* HYDRA_BITSERIAL */

/**
 * Array of function pointers to store the bitserial interrupt handlers
 */
static volatile void (*hal_int_handlers[HAVE_NUMBER_OF_BITSERIALS])(
                                                         bitserial_instance i);
/**
 * Function pointer which stores the software interrupt handler 
 */
static volatile void (*hal_swint_handler)(void);
/**
 * Stores the software interrupt ID. This is obtained following the software 
 * interrupt initialisation and is used to raise interrupts and for 
 * deinitialisation. 
 */
static swint_id swintid;

/**
 * Interrupt handler for bitserial software interrupts.
 */
void hal_bitserial_swint_handler(void);

#if HAVE_NUMBER_OF_BITSERIALS > 0
/**
 * Interrupt handler for bitserial0 hardware interrupts.
 */
void hal_bitserial0_int_handler(void);
#endif

#if HAVE_NUMBER_OF_BITSERIALS > 1
/**
 * Interrupt handler for bitserial1 hardware interrupts.
 */
void hal_bitserial1_int_handler(void);
#endif

void hal_bitserial_int_enable(bitserial_instance i, 
                                                void(*h)(bitserial_instance i))
{
#ifdef HYDRA_BITSERIAL
    bus_interrupt_configuration bitserial_int_config =
    {
        SYSTEM_BUS_HOST_SYS,
        0,
        TRUE,  /* enable */
        FALSE, /* clear_on_read */
        BITSERIAL_EVENT_ALL_MASK,
        INT_LEVEL_HOSTIO,
        NULL
    };
    
    /* Enable bitserial interrupts */
    hal_bitserial_interrupt_en_set(i, BITSERIAL_EVENT_ALL_MASK);
    
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            hal_int_handlers[0] = h;
            bitserial_int_config.block_id = HOST_SYS_BITSERIAL_0_BLOCK_ID;
            bitserial_int_config.handler = hal_bitserial0_int_handler;
            /* Enable bitserial bus interrupt */
            bus_interrupt_configure(BUS_INTERRUPT_ID_BITSERIAL0, 
                                                        &bitserial_int_config);
            break;
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            hal_int_handlers[1] = h;
            bitserial_int_config.block_id = HOST_SYS_BITSERIAL_1_BLOCK_ID;
            bitserial_int_config.handler = hal_bitserial1_int_handler;
            /* Enable bitserial bus interrupt */
            bus_interrupt_configure(BUS_INTERRUPT_ID_BITSERIAL1, 
                                                        &bitserial_int_config);
            break;
#endif
        default:
            UNUSED(h);
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
#elif defined(INSTALL_HAWKEYE) || defined(INSTALL_VULTAN) || \
                                                      defined (INSTALL_TRINITY)
#warning VULTAN(HAWKEYE) code should add hal_bitserial_int_handler as the \
default interrupt handler
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            hal_int_handlers[0] = h;
            /* Enable bitserial hardware interrupt */
            hal_set_interrupt_priority(INT_SOURCE_BITSERIAL0_EVENT, 
                                                             INTLEV_BITSERIAL);
            hal_enable_interrupt(INT_SOURCE_BITSERIAL0_EVENT, TRUE);
            break;
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            hal_int_handlers[1] = h;
            /* Enable bitserial hardware interrupt */
            hal_set_interrupt_priority(INT_SOURCE_BITSERIAL1_EVENT, 
                                                             INTLEV_BITSERIAL);
            hal_enable_interrupt(INT_SOURCE_BITSERIAL1_EVENT, TRUE);
            break;
#endif
        default:
            UNUSED(h);
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
#endif /* HYDRA_BITSERIAL */
}

void hal_bitserial_int_disable(bitserial_instance i)
{
    bus_interrupt_configuration bitserial_int_config =
    {
        SYSTEM_BUS_HOST_SYS,
        0,
        FALSE,  /* disable */
        FALSE, /* clear_on_read */
        0,
        INT_LEVEL_HOSTIO,
        NULL
    };
#ifdef HYDRA_BITSERIAL
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            hal_int_handlers[0] = NULL;
            bitserial_int_config.block_id = HOST_SYS_BITSERIAL_0_BLOCK_ID;
            /* Enable bitserial bus interrupt */
            bus_interrupt_configure(BUS_INTERRUPT_ID_BITSERIAL0, 
                                                        &bitserial_int_config);
            break;
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            hal_int_handlers[1] = NULL;
            bitserial_int_config.block_id = HOST_SYS_BITSERIAL_1_BLOCK_ID;
            /* Enable bitserial bus interrupt */
            bus_interrupt_configure(BUS_INTERRUPT_ID_BITSERIAL1, 
                                                        &bitserial_int_config);
            break;
#endif
        default:
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
#elif defined(INSTALL_HAWKEYE) || defined(INSTALL_VULTAN) || \
                                                      defined (INSTALL_TRINITY)
#warning VULTAN(HAWKEYE) code should add hal_bitserial_int_handler as the \
default interrupt handler
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            hal_int_handlers[0] = NULL;
            break;
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            hal_int_handlers[1] = NULL;
            break;
#endif
        default:
            UNUSED(h);
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
#endif /* HYDRA_BITSERIAL */
}

void hal_bitserial_swint_enable(void (*h)(void))
{
    hal_swint_handler = h;
#ifdef HYDRA_BITSERIAL
    swintid = configure_sw_interrupt(INT_LEVEL_FG,
                                                  hal_bitserial_swint_handler);
#elif defined(INSTALL_HAWKEYE) || defined(INSTALL_VULTAN) || \
                                                      defined (INSTALL_TRINITY)
#warning VULTAN(HAWKEYE) code should add hal_bitserial_swint_handler as the \
default interrupt handler
    enable_fg_int(FGINTID_BITSERIAL_FG_INT, INTLEV_BITSERIAL);
#endif /* HYDRA_BITSERIAL */
}

void hal_bitserial_swint_disable(void)
{
#ifdef HYDRA_BITSERIAL
    unconfigure_sw_interrupt(swintid);
#elif defined(INSTALL_HAWKEYE) || defined(INSTALL_VULTAN) || \
                                                      defined (INSTALL_TRINITY)
#warning VULTAN(HAWKEYE) code should remove hal_bitserial_swint_handler as the \
default interrupt handler
#endif /* HYDRA_BITSERIAL */
    hal_swint_handler = NULL;
}

void hal_bitserial_raise_swint(void)
{
    generate_sw_interrupt(swintid);
}

void hal_bitserial_interrupt_en_set(bitserial_instance i, uint16 v)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            hal_set_reg_bitserial0_interrupt_en(v);
            break;
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            hal_set_reg_bitserial1_interrupt_en(v);
            break;
#endif
        default:
            UNUSED(v);
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

uint16 hal_bitserial_interrupt_en_get(bitserial_instance i)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            return (uint16)hal_get_reg_bitserial0_interrupt_en();
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            return (uint16)hal_get_reg_bitserial1_interrupt_en();
#endif
        default:
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

void hal_bitserial_buffers_set(bitserial_instance i, mmu_handle rxh1, 
                             mmu_handle txh1, mmu_handle rxh2, mmu_handle txh2)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            if(rxh1 != MMU_INDEX_NULL)
            {
                hal_set_bitserial0_remote_ssid_rx_buffer(mmu_handle_ssid(rxh1));
                hal_set_bitserial0_buff_handle_rx_buffer(mmu_handle_index(rxh1));
            }
            if(txh1 != MMU_INDEX_NULL)
            {
                hal_set_bitserial0_remote_ssid_tx_buffer(mmu_handle_ssid(txh1));
                hal_set_bitserial0_buff_handle_tx_buffer(mmu_handle_index(txh1));
            }
            if(rxh2 != MMU_INDEX_NULL)
            {
                hal_set_bitserial0_remote_ssid_rx_buffer2(mmu_handle_ssid(rxh2));
                hal_set_bitserial0_buff_handle_rx_buffer2(mmu_handle_index(rxh2));
            }
            if(txh2 != MMU_INDEX_NULL)
            {
                hal_set_bitserial0_remote_ssid_tx_buffer2(mmu_handle_ssid(txh2));
                hal_set_bitserial0_buff_handle_tx_buffer2(mmu_handle_index(txh2));
            }
            break;
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            if(rxh1 != MMU_INDEX_NULL)
            {
                hal_set_bitserial1_remote_ssid_rx_buffer(mmu_handle_ssid(rxh1));
                hal_set_bitserial1_buff_handle_rx_buffer(mmu_handle_index(rxh1));
            }
            if(txh1 != MMU_INDEX_NULL)
            {
                hal_set_bitserial1_remote_ssid_tx_buffer(mmu_handle_ssid(txh1));
                hal_set_bitserial1_buff_handle_tx_buffer(mmu_handle_index(txh1));
            }
            if(rxh2 != MMU_INDEX_NULL)
            {
                hal_set_bitserial1_remote_ssid_rx_buffer2(mmu_handle_ssid(rxh2));
                hal_set_bitserial1_buff_handle_rx_buffer2(mmu_handle_index(rxh2));
            }
            if(txh2 != MMU_INDEX_NULL)
            {
                hal_set_bitserial1_remote_ssid_tx_buffer2(mmu_handle_ssid(txh2));
                hal_set_bitserial1_buff_handle_tx_buffer2(mmu_handle_index(txh2));
            }
            break;
#endif
        default:
            UNUSED(rxh1); UNUSED(txh1); UNUSED(rxh2); UNUSED(txh2);
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

void hal_bitserial_set_misc(bitserial_instance i)
{
#ifdef HYDRA_BITSERIAL
    UNUSED(i);
#elif defined(INSTALL_HAWKEYE) || defined(INSTALL_VULTAN) || \
                                                      defined (INSTALL_TRINITY)
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            hal_set_clkgen_active3_bitserial0(8); /* 16M MMU clock */
            hal_set_clkgen_activity_bitserial0(1);/* bitserial can wake up from
                                                               shallow sleep */
            break;
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            hal_set_clkgen_active3_bitserial1(8); /* 16M MMU clock */
            hal_set_clkgen_activity_bitserial1(1);/* bitserial can wake up from
                                                               shallow sleep */
            break;
#endif
        default:
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
#endif /* HYDRA_BITSERIAL */
}

void hal_bitserial_data_output_pio_set(bitserial_instance i, uint8 v)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            hal_set_reg_host_sys_bitserial_0_data_output_pio_ctrl(v);
            break;
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            hal_set_reg_host_sys_bitserial_1_data_output_pio_ctrl(v);
            break;
#endif
        default:
            UNUSED(v);
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

void hal_bitserial_clk_output_pio_set(bitserial_instance i, uint8 v)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            hal_set_reg_host_sys_bitserial_0_clk_output_pio_ctrl(v);
            break;
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            hal_set_reg_host_sys_bitserial_1_clk_output_pio_ctrl(v);
            break;
#endif
        default:
            UNUSED(v);
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

void hal_bitserial_sel_output_pio_set(bitserial_instance i, uint8 v)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            hal_set_reg_host_sys_bitserial_0_sel_output_pio_ctrl(v);
            break;
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            hal_set_reg_host_sys_bitserial_1_sel_output_pio_ctrl(v);
            break;
#endif
        default:
            UNUSED(v);
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

void hal_bitserial_data_input_pio_set(bitserial_instance i, uint8 v)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            hal_set_reg_host_sys_bitserial_0_data_input_pio_ctrl(v);
            break;
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            hal_set_reg_host_sys_bitserial_1_data_input_pio_ctrl(v);
            break;
#endif
        default:
            UNUSED(v);
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

void hal_bitserial_clk_input_pio_set(bitserial_instance i, uint8 v)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            hal_set_reg_host_sys_bitserial_0_clk_input_pio_ctrl(v);
            break;
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            hal_set_reg_host_sys_bitserial_1_clk_input_pio_ctrl(v);
            break;
#endif
        default:
            UNUSED(v);
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

void hal_bitserial_sel_input_pio_set(bitserial_instance i, uint8 v)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            hal_set_reg_host_sys_bitserial_0_sel_input_pio_ctrl(v);
            break;
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            hal_set_reg_host_sys_bitserial_1_sel_input_pio_ctrl(v);
            break;
#endif
        default:
            UNUSED(v);
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

void hal_bitserial_enable_set(bitserial_instance i, bool v)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            hal_set_register(*(volatile uint32 *)&HOST_SYS_CONFIG_ENABLES,
                             ((v) ? (hal_get_reg_host_sys_config_enables() | 
                                    (1 << HOST_SYS_BITSERIAL_0_BLOCK_ID)) 
                                  : (hal_get_reg_host_sys_config_enables() & 
                                    ~(1 << HOST_SYS_BITSERIAL_0_BLOCK_ID))
                             ), 1);
            break;
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            hal_set_register(*(volatile uint32 *)&HOST_SYS_CONFIG_ENABLES,
                             ((v) ? (hal_get_reg_host_sys_config_enables() | 
                                    (1 << HOST_SYS_BITSERIAL_1_BLOCK_ID)) 
                                  : (hal_get_reg_host_sys_config_enables() & 
                                    ~(1 << HOST_SYS_BITSERIAL_1_BLOCK_ID))
                             ), 1);
            break;
#endif
        default:
            UNUSED(v);
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

void hal_bitserial_config_set(bitserial_instance i, uint16 v)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            hal_set_reg_bitserial0_config(v);
            break;
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            hal_set_reg_bitserial1_config(v);
            break;
#endif
        default:
            UNUSED(v);
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

uint16 hal_bitserial_config_get(bitserial_instance i)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            return (uint16)hal_get_reg_bitserial0_config();
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            return (uint16)hal_get_reg_bitserial1_config();
#endif
        default:
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

void hal_bitserial_config2_set(bitserial_instance i, uint16 v)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            hal_set_reg_bitserial0_config2(v);
            break;
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            hal_set_reg_bitserial1_config2(v);
            break;
#endif
        default:
            UNUSED(v);
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

void hal_bitserial_config3_set(bitserial_instance i, uint16 v)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            hal_set_reg_bitserial0_config3(v);
            break;
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            hal_set_reg_bitserial1_config3(v);
            break;
#endif
        default:
            UNUSED(v);
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

void hal_bitserial_slave_underflow_byte_set(bitserial_instance i, uint8 v)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            hal_set_reg_bitserial0_slave_underflow_byte(v);
            break;
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            hal_set_reg_bitserial1_slave_underflow_byte(v);
            break;
#endif
        default:
            UNUSED(v);
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

uint16 hal_bitserial_config2_get(bitserial_instance i)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            return (uint16)hal_get_reg_bitserial0_config2();
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            return (uint16)hal_get_reg_bitserial1_config2();
#endif
        default:
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

void hal_bitserial_word_config_set(bitserial_instance i, uint16 v)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            hal_set_reg_bitserial0_word_config(v);
            break;
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            hal_set_reg_bitserial1_word_config(v);
            break;
#endif
        default:
            UNUSED(v);
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

uint16 hal_bitserial_word_config_get(bitserial_instance i)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            return (uint16)hal_get_reg_bitserial0_word_config();
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            return (uint16)hal_get_reg_bitserial1_word_config();
#endif
        default:
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

uint8 hal_bitserial_bytes_per_word_get(bitserial_instance i)
{
    uint16 value;

    value = hal_bitserial_word_config_get(i);
    value &= HAL_BITSERIAL_WORD_CONFIG_NUM_BYTES_MASK;
    value >>= HAL_BITSERIAL_WORD_CONFIG_NUM_BYTES_POSN;

    return (uint8)(value + 1);
}

void hal_bitserial_interbyte_spacing_set(bitserial_instance i, uint16 v)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            hal_set_reg_bitserial0_interbyte_spacing(v);
            break;
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            hal_set_reg_bitserial1_interbyte_spacing(v);
            break;
#endif
        default:
            UNUSED(v);
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

void hal_bitserial_sel_time_set(bitserial_instance i, uint32 v)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            hal_set_reg_bitserial0_config_sel_time((uint16)(v & 0xffff));
            hal_set_reg_bitserial0_config_sel_time2((uint16)((v >> 16) & 0xf));
            break;
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            hal_set_reg_bitserial1_config_sel_time((uint16)(v & 0xffff));
            hal_set_reg_bitserial1_config_sel_time2((uint16)((v >> 16) & 0xf));
            break;
#endif
        default:
            UNUSED(v);
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

void hal_bitserial_clk_rate_set(bitserial_instance i, uint16 hi, uint16 lo)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            hal_set_reg_bitserial0_clk_rate_hi(hi);
            hal_set_reg_bitserial0_clk_rate_lo(lo);
            break;
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            hal_set_reg_bitserial1_clk_rate_hi(hi);
            hal_set_reg_bitserial1_clk_rate_lo(lo);
            break;
#endif
        default:
            UNUSED(hi);
            UNUSED(lo);
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

void hal_bitserial_clk_samp_offset_set(bitserial_instance i, uint16 v)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            hal_set_reg_bitserial0_clk_samp_offset(v);
            break;
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            hal_set_reg_bitserial1_clk_samp_offset(v);
            break;
#endif
        default:
            UNUSED(v);
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

void hal_bitserial_i2c_address_set(bitserial_instance i, uint16 v)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            hal_set_reg_bitserial0_i2c_address(v);
            break;
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            hal_set_reg_bitserial1_i2c_address(v);
            break;
#endif
        default:
            UNUSED(v);
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

void hal_bitserial_clear_sticky_ack(bitserial_instance i)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            hal_set_reg_bitserial0_config(hal_get_reg_bitserial0_config() | 
                                   HAL_BITSERIAL_CONFIG_CLEAR_STICKY_ACK_MASK);
            hal_set_reg_bitserial0_config(hal_get_reg_bitserial0_config() & 
                                (~HAL_BITSERIAL_CONFIG_CLEAR_STICKY_ACK_MASK));
            break;
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            hal_set_reg_bitserial1_config(hal_get_reg_bitserial1_config() | 
                                   HAL_BITSERIAL_CONFIG_CLEAR_STICKY_ACK_MASK);
            hal_set_reg_bitserial1_config(hal_get_reg_bitserial1_config() & 
                                (~HAL_BITSERIAL_CONFIG_CLEAR_STICKY_ACK_MASK));
            break;
#endif
        default:
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

void hal_bitserial_clock_enable_set(bitserial_instance i, bool v)
{
#ifdef HYDRA_BITSERIAL
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            /*if e == false we can go to sleep*/
            UNUSED(v);
            break;
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            /*if e == false we can go to sleep*/
            UNUSED(v);
            break;
#endif
        default:
            UNUSED(v);
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
#elif defined(INSTALL_HAWKEYE) || defined(INSTALL_VULTAN) || \
                                                      defined (INSTALL_TRINITY)
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            bitserial_block_int();
            dorm_sleep_info(DORMID_BITSERIAL, 
                                         v?DORM_STATE_NO_DEEP:DORM_STATE_ANY); 
            hal_set_clkgen_bitserial_en(v);
            bitserial_unblock_int();
            break;
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            bitserial_block_int();
            dorm_sleep_info(DORMID_BITSERIAL, 
                                         v?DORM_STATE_NO_DEEP:DORM_STATE_ANY); 
            hal_set_clkgen_bitserial_en(v);
            bitserial_unblock_int();
            break;
#endif
        default:
            UNUSED(v);
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
#endif /* HYDRA_BITSERIAL */
}

bitserial_events hal_bitserial_events_get(bitserial_instance i)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            return (bitserial_events)hal_get_reg_bitserial0_interrupt_status();
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            return (bitserial_events)hal_get_reg_bitserial1_interrupt_status();
#endif
        default:
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

uint16 hal_bitserial_event_get(bitserial_instance i)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            return hal_get_bitserial0_event();
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            return hal_get_bitserial1_event();
#endif
        default:
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

uint16 hal_bitserial_status_get(bitserial_instance i)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            return (uint16)hal_get_reg_bitserial0_status();
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            return (uint16)hal_get_reg_bitserial1_status();
#endif
        default:
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

uint16 hal_bitserial_error_status_get(bitserial_instance i)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            return (uint16)hal_get_reg_bitserial0_error_status();
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            return (uint16)hal_get_reg_bitserial1_error_status();
#endif
        default:
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

uint16 hal_bitserial_part_2_get(bitserial_instance i)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            return hal_get_bitserial0_part_2();
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            return hal_get_bitserial1_part_2();
#endif
        default:
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

uint16 hal_bitserial_failed_get(bitserial_instance i)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            return hal_get_bitserial0_failed();
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            return hal_get_bitserial1_failed();
#endif
        default:
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

void hal_bitserial_event_clear(bitserial_instance i, uint32 event_mask)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            hal_set_reg_bitserial0_interrupt_clear(event_mask);
            hal_set_reg_bitserial0_interrupt_clear(0);
            hal_set_reg_bitserial0_interrupt_source_clear(event_mask);
            hal_set_reg_bitserial0_interrupt_source_clear(0);
            break;
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            hal_set_reg_bitserial1_interrupt_clear(event_mask);
            hal_set_reg_bitserial1_interrupt_clear(0);
            hal_set_reg_bitserial1_interrupt_source_clear(event_mask);
            hal_set_reg_bitserial1_interrupt_source_clear(0);
            break;
#endif
        default:
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

void hal_bitserial_clk_force_enable(bitserial_instance i, bool enable)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            hal_set_host_sys_bitserial_0_clk_force_enable(enable ? 1:0);
            break;
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            hal_set_host_sys_bitserial_1_clk_force_enable(enable ? 1:0);
            break;
#endif
        default:
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}


bool hal_bitserial_busy_get(bitserial_instance i)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            return hal_get_bitserial0_busy();
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            return hal_get_bitserial1_busy();
#endif
        default:
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

uint16 hal_bitserial_i2c_acks_get(bitserial_instance i)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            return (uint16)(hal_get_reg_bitserial0_i2c_acks() & 0x7fff);
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            return (uint16)(hal_get_reg_bitserial1_i2c_acks() & 0x7fff);
#endif
        default:
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

void hal_bitserial_rwb_set(bitserial_instance i, bitserial_rw_modes v)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            hal_set_reg_bitserial0_rwb(v);
            break;
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            hal_set_reg_bitserial1_rwb(v);
            break;
#endif
        default:
            UNUSED(v);
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

void hal_bitserial_txrx_length_set(bitserial_instance i, uint16 v)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            hal_set_reg_bitserial0_txrx_length(v);
            break;
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            hal_set_reg_bitserial1_txrx_length(v);
            break;
#endif
        default:
            UNUSED(v);
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

void hal_bitserial_txrx_length2_set(bitserial_instance i, uint16 v)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            hal_set_reg_bitserial0_txrx_length2(v);
            break;
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            hal_set_reg_bitserial1_txrx_length2(v);
            break;
#endif
        default:
            UNUSED(v);
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

void hal_bitserial_num_protocol_words_set(bitserial_instance i, uint16 v)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            hal_set_reg_bitserial0_slave_num_protocol_words(v);
            break;
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            hal_set_reg_bitserial1_slave_num_protocol_words(v);
            break;
#endif
        default:
            UNUSED(v);
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

void hal_bitserial_slave_read_command_byte_set(bitserial_instance i, uint16 v)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            hal_set_reg_bitserial0_slave_read_command_byte(v);
            break;
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            hal_set_reg_bitserial1_slave_read_command_byte(v);
            break;
#endif
        default:
            UNUSED(v);
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

uint16 hal_bitserial_slave_read_get(bitserial_instance i)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            return (uint16)hal_get_reg_bitserial0_slave_read_count();
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            return (uint16)hal_get_reg_bitserial1_slave_read_count();
#endif
        default:
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

uint16 hal_bitserial_slave_write_get(bitserial_instance i)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            return (uint16)hal_get_reg_bitserial0_slave_write_count();
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            return (uint16)hal_get_reg_bitserial1_slave_write_count();
#endif
        default:
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
} 

uint16 hal_bitserial_clk_control_get(bitserial_instance i)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            return (uint16)hal_get_reg_bitserial0_clk_control();
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            return (uint16)hal_get_reg_bitserial1_clk_control();
#endif
        default:
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

void hal_bitserial_clk_control_set(bitserial_instance i, uint16 v)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            hal_set_reg_bitserial0_clk_control(v);
            break;
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            hal_set_reg_bitserial1_clk_control(v);
            break;
#endif
        default:
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

uint16 hal_bitserial_debug_sel_get(bitserial_instance i)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            return (uint16)hal_get_bitserial0_config2_debug_sel();
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            return (uint16)hal_get_bitserial1_config2_debug_sel();
#endif
        default:
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

void hal_bitserial_debug_sel_set(bitserial_instance i, uint16 v)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            hal_set_bitserial0_config2_debug_sel(v);
            break;
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            hal_set_bitserial1_config2_debug_sel(v);
            break;
#endif
        default:
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

void hal_bitserial_slave_debug_sel_set(bitserial_instance i, uint16 v)
{
    switch(i)
    {
#if HAVE_NUMBER_OF_BITSERIALS > 0
        case BITSERIAL_INSTANCE_0:
            hal_set_reg_bitserial0_debug_sel(v);
            break;
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
        case BITSERIAL_INSTANCE_1:
            hal_set_reg_bitserial1_debug_sel(v);
            break;
#endif
        default:
            panic(PANIC_BITSERIAL_HAL_ERROR); /* Does not return */
    }
}

#if HAVE_NUMBER_OF_BITSERIALS > 0
void hal_bitserial0_int_handler(void)
{
    if(hal_int_handlers[0])
    {
        hal_int_handlers[0](BITSERIAL_INSTANCE_0);
    }
}
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
void hal_bitserial1_int_handler(void)
{
    if(hal_int_handlers[1])
    {
        hal_int_handlers[1](BITSERIAL_INSTANCE_1);
    }
}
#endif

void hal_bitserial_swint_handler(void)
{
    if(hal_swint_handler)
    {
        hal_swint_handler();
    }
}

PRESERVE_TYPE_FOR_DEBUGGING(bitserial_status_bits)

#endif /* INSTALL_BITSERIAL */
