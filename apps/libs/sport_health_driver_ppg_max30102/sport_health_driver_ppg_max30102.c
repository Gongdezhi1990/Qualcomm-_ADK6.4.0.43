/****************************************************************
* Copyright (C) 2017 Qualcomm Technologies International, Ltd
*
* max30102.c
* Date: 19/06/2017
* Revision: 1.0.0
*
*
* Usage: (MAX30102) Sensor Device Driver file
*
****************************************************************/

/** Device specific: MAX30102 */
#include "sport_health_driver_ppg_max30102.h"
#include "sport_health_driver.h"

/** Crescendo specific */
//#include "imu_ppg_private.h"
#include "macros.h"
#include "hydra_log.h"
/***********************************************************
* PRIVATE VARIABLE DECLARATION
***********************************************************/
ppg_t *p_max30102;
static uint8 s_led_current_level = 0;
static uint8 out_counter = 0;

/**
 *	@brief  This API is used for calculate the average of FIFO data
 *          and compare it with a threshold setted for indicating in/out.
 *
 *  @param  fifo_data Pointer holding the FIFO data.
 *          fifo_data_length The value of FIFO byte counter
 *
 *	@return In/Out (0/1)
 */
static PPG_RETURN_FUNCTION_TYPE ppg_threshold_checking(uint8 *fifo_data, uint8 fifo_data_length)
{
    /** variable used to return the status of communication result*/
    PPG_RETURN_FUNCTION_TYPE com_rslt = PPG_STATUS_IN;
    uint8 v_fifo_index = PPG_INIT_VALUE;
    uint8 count = 0;
    uint32 sum = 0;
    uint32 average = 0;

    for(v_fifo_index = PPG_INIT_VALUE; v_fifo_index < fifo_data_length;)
    {
        sum += (uint16)((fifo_data[v_fifo_index + PPG_FIFO_X_MSB_DATA] << PPG_SHIFT_BIT_POSITION_BY_06_BITS))
                | (fifo_data[v_fifo_index + PPG_FIFO_X_MID_DATA]);
        count += PPG_FRAME_COUNT;
        v_fifo_index += PPG_SINGLE_LED_TOTAL_BYTES;
    }
    average = sum / fifo_data_length;

    if (average < PPG_PROX_INT_THRESH_VALUE && out_counter <= PPG_OUT_COUNTER_NUMBER)
        out_counter += 1;
    if (average >= PPG_PROX_INT_THRESH_VALUE)
        ppg_reset_out_counter();
    if (out_counter >= PPG_OUT_COUNTER_NUMBER)
        com_rslt = PPG_STATUS_OUT;

    return com_rslt;
}


/***********************************************************
* PUBLIC FUNCTION DECFINITION
***********************************************************/
/***
 *	@brief  This API is used to perform initialization steps
 *          for MAX30102. This include
 *          1. Trigger a reset including a reboot for MAX30102.
 *          2. Read and save the CHIP_ID
 *  @param p_ppg_info: Pointer to max30102 device specific
 *         information
 *	@return  Results of bus communication function
 *
 *
*/
PPG_RETURN_FUNCTION_TYPE ppg_init(ppg_t *p_ppg_info)
{
    /** variable used to return the status of communication result*/
    PPG_RETURN_FUNCTION_TYPE com_rslt = E_PPG_ERROR;
    uint8 v_data = PPG_INIT_VALUE;
    /** assign max30102 pointer */
    p_max30102 = p_ppg_info;
    /** RESET the MAX30102*/
    ppg_reset(PPG_CHIP_RESET_EN);
    /** reset to start up delay */
    p_max30102->delay_msec(PPG_CHIP_RESET_DELAY);
    com_rslt =
    p_max30102->PPG_BUS_READ_FUNC(p_max30102->dev_addr,
    PPG_USER_CHIP_ID__REG,
    &v_data, PPG_GEN_READ_WRITE_DATA_LENGTH);
    /** store the chip id which is read from the sensor */
    p_max30102->chip_id = v_data;
    return com_rslt;
}
/**
 *	@brief  This API is used to RESET the MAX30102
 *          register 0x09 bit 6
 *
 *  @param  v_reset The value to be set in reset field
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_reset(uint8 v_reset)
{
    /** variable used to return the status of communication result*/
    PPG_RETURN_FUNCTION_TYPE com_rslt = E_PPG_ERROR;
    uint8 v_data = PPG_INIT_VALUE;

    /** check the p_max30102 structure for NULL pointer assignment*/
    if (p_max30102 == PPG_NULL)
    {
        return E_PPG_NULL_PTR;
    }
    else
    {
        /** write in reset field */
        com_rslt = p_max30102->PPG_BUS_READ_FUNC(p_max30102->dev_addr,
                   PPG_CHIP_RESET__REG, &v_data, PPG_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == E_PPG_SUCCESS)
        {
            v_data = PPG_SET_BITSLICE(v_data, PPG_CHIP_RESET, v_reset);
            com_rslt += p_max30102->PPG_BUS_WRITE_FUNC(p_max30102->dev_addr,
                        PPG_CHIP_RESET__REG, &v_data, PPG_GEN_READ_WRITE_DATA_LENGTH);
        }

    }
    return com_rslt;
}
/**
 *	@brief  This API is used to SHUTDOWN the MAX30102
 *          register 0x09 bit 7
 *
 *  @param  v_shdn The value to be set in shutdown field
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_shutdown(uint8 v_shdn)
{
    /** variable used to return the status of communication result*/
    PPG_RETURN_FUNCTION_TYPE com_rslt = E_PPG_ERROR;
    uint8 v_data = PPG_INIT_VALUE;

    /** check the p_max30102 structure for NULL pointer assignment*/
    if (p_max30102 == PPG_NULL)
    {
        return E_PPG_NULL_PTR;
    }
    else
    {
        /** write in shutdown field */
        com_rslt = p_max30102->PPG_BUS_READ_FUNC(p_max30102->dev_addr,
                   PPG_SHDN_CONTROL__REG, &v_data, PPG_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == E_PPG_SUCCESS)
        {
            v_data = PPG_SET_BITSLICE(v_data, PPG_SHDN_CONTROL, v_shdn);
            com_rslt += p_max30102->PPG_BUS_WRITE_FUNC(p_max30102->dev_addr,
                        PPG_SHDN_CONTROL__REG, &v_data, PPG_GEN_READ_WRITE_DATA_LENGTH);
        }

    }
    return com_rslt;
}

/**
 *	@brief  This API is used to set the multi-LED mode control for slot1 and 2
 *          in MAX30102 register 0x11 bit 0 to 2 and 4 to 6
 *
 *  @param  slot1_mode The value to be set in mode field for slot1
 *  @param  slot2_mode The value to be set in mode field for slot2
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_set_multi_led_mode_control_slot1_2(uint8 slot1_mode,
                                                                uint8 slot2_mode)
{
    /** variable used to return the status of communication result*/
    PPG_RETURN_FUNCTION_TYPE com_rslt = E_PPG_ERROR;
    uint8 v_data = PPG_INIT_VALUE;

    /** check the p_max30102 structure for NULL pointer assignment*/
    if (p_max30102 == PPG_NULL)
    {
        return E_PPG_NULL_PTR;
    }
    else
    {
        /** write in mode field */
        com_rslt = p_max30102->PPG_BUS_READ_FUNC(p_max30102->dev_addr,
                   PPG_MULTI_LED_MODE_CONTROL1__REG, &v_data,
                   PPG_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == E_PPG_SUCCESS)
        {
            v_data = PPG_SET_BITSLICE(v_data, PPG_MULTI_LED_MODE_CONTROL1, slot1_mode);
            com_rslt += p_max30102->PPG_BUS_WRITE_FUNC(p_max30102->dev_addr,
                        PPG_MULTI_LED_MODE_CONTROL1__REG, &v_data,
                        PPG_GEN_READ_WRITE_DATA_LENGTH);
        }
        com_rslt += p_max30102->PPG_BUS_READ_FUNC(p_max30102->dev_addr,
                   PPG_MULTI_LED_MODE_CONTROL2__REG, &v_data,
                   PPG_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == E_PPG_SUCCESS)
        {
            v_data = PPG_SET_BITSLICE(v_data, PPG_MULTI_LED_MODE_CONTROL2, slot2_mode);
            com_rslt += p_max30102->PPG_BUS_WRITE_FUNC(p_max30102->dev_addr,
                        PPG_MULTI_LED_MODE_CONTROL2__REG, &v_data,
                        PPG_GEN_READ_WRITE_DATA_LENGTH);
        }
    }
    return com_rslt;
}

/**
 *	@brief  This API is used to get the multi-LED mode control for slot1 and 2
 *          in MAX30102 register 0x11 bit 0 to 2 and 4 to 6
 *
 *  @param  p_slot1_mode Stores value set in mode field for slot1
 *  @param  p_slot2_mode Stores value set in mode field for slot2
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_get_multi_led_mode_control_slot1_2(uint8 *p_slot1_mode,
                                                                uint8 *p_slot2_mode)
{
    /** variable used to return the status of communication result*/
    PPG_RETURN_FUNCTION_TYPE com_rslt = E_PPG_ERROR;
    uint8 v_data = PPG_INIT_VALUE;

    /** check the p_max30102 structure for NULL pointer assignment*/
    if (p_max30102 == PPG_NULL)
    {
        return E_PPG_NULL_PTR;
    }
    else
    {
        /** read the mode field */
        com_rslt = p_max30102->PPG_BUS_READ_FUNC(p_max30102->dev_addr,
                   PPG_MULTI_LED_MODE_CONTROL1__REG, &v_data,
                   PPG_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == E_PPG_SUCCESS)
        {
            *p_slot1_mode = PPG_GET_BITSLICE(v_data, PPG_MULTI_LED_MODE_CONTROL1);
        }
        com_rslt += p_max30102->PPG_BUS_READ_FUNC(p_max30102->dev_addr,
                   PPG_MULTI_LED_MODE_CONTROL2__REG, &v_data,
                   PPG_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == E_PPG_SUCCESS)
        {
            *p_slot2_mode = PPG_GET_BITSLICE(v_data, PPG_MULTI_LED_MODE_CONTROL2);
        }
    }
    return com_rslt;
}

/**
 *	@brief  This API is used to set the multi-LED mode control for slot3 and 4
 *          in MAX30102 register 0x12 bit 0 to 2 and 4 to 6
 *
 *  @param  slot3_mode The value to be set in mode field for slot3
 *  @param  slot4_mode The value to be set in mode field for slot4
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_set_multi_led_mode_control_slot3_4(uint8 slot3_mode,
                                                                uint8 slot4_mode)
{
    /** variable used to return the status of communication result*/
    PPG_RETURN_FUNCTION_TYPE com_rslt = E_PPG_ERROR;
    uint8 v_data = PPG_INIT_VALUE;

    /** check the p_max30102 structure for NULL pointer assignment*/
    if (p_max30102 == PPG_NULL)
    {
        return E_PPG_NULL_PTR;
    }
    else
    {
        /** write in mode field */
        com_rslt = p_max30102->PPG_BUS_READ_FUNC(p_max30102->dev_addr,
                   PPG_MULTI_LED_MODE_CONTROL3__REG, &v_data,
                   PPG_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == E_PPG_SUCCESS)
        {
            v_data = PPG_SET_BITSLICE(v_data, PPG_MULTI_LED_MODE_CONTROL3, slot3_mode);
            com_rslt += p_max30102->PPG_BUS_WRITE_FUNC(p_max30102->dev_addr,
                        PPG_MULTI_LED_MODE_CONTROL3__REG, &v_data,
                        PPG_GEN_READ_WRITE_DATA_LENGTH);
        }
        com_rslt += p_max30102->PPG_BUS_READ_FUNC(p_max30102->dev_addr,
                   PPG_MULTI_LED_MODE_CONTROL4__REG, &v_data,
                   PPG_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == E_PPG_SUCCESS)
        {
            v_data = PPG_SET_BITSLICE(v_data, PPG_MULTI_LED_MODE_CONTROL4, slot4_mode);
            com_rslt += p_max30102->PPG_BUS_WRITE_FUNC(p_max30102->dev_addr,
                        PPG_MULTI_LED_MODE_CONTROL4__REG, &v_data,
                        PPG_GEN_READ_WRITE_DATA_LENGTH);
        }
    }
    return com_rslt;
}

/**
 *	@brief  This API is used to get the multi-LED mode control for slot3 and 4
 *          in MAX30102 register 0x12 bit 0 to 2 and 4 to 6
 *
 *  @param  p_slot3_mode Stores value set in mode field for slot3
 *  @param  p_slot4_mode Stores value set in mode field for slot4
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_get_multi_led_mode_control_slot3_4(uint8 *p_slot3_mode,
                                                                uint8 *p_slot4_mode)
{
    /** variable used to return the status of communication result*/
    PPG_RETURN_FUNCTION_TYPE com_rslt = E_PPG_ERROR;
    uint8 v_data = PPG_INIT_VALUE;

    /** check the p_max30102 structure for NULL pointer assignment*/
    if (p_max30102 == PPG_NULL)
    {
        return E_PPG_NULL_PTR;
    }
    else
    {
        /** read the mode field */
        com_rslt = p_max30102->PPG_BUS_READ_FUNC(p_max30102->dev_addr,
                   PPG_MULTI_LED_MODE_CONTROL3__REG, &v_data,
                   PPG_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == E_PPG_SUCCESS)
        {
            *p_slot3_mode = PPG_GET_BITSLICE(v_data, PPG_MULTI_LED_MODE_CONTROL3);
        }
        com_rslt += p_max30102->PPG_BUS_READ_FUNC(p_max30102->dev_addr,
                   PPG_MULTI_LED_MODE_CONTROL4__REG, &v_data,
                   PPG_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == E_PPG_SUCCESS)
        {
            *p_slot4_mode = PPG_GET_BITSLICE(v_data, PPG_MULTI_LED_MODE_CONTROL4);
        }
    }
    return com_rslt;
}


/**
 *	@brief  This API is used to get the interrupt status for proximity in MAX30102
 *          register 0x00 bit 4
 *
 *  @param  p_status Stores value set in status field
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_get_proximity_interrupt_status(uint8 *p_status)
{
    /** variable used to return the status of communication result*/
    PPG_RETURN_FUNCTION_TYPE com_rslt = E_PPG_ERROR;
    uint8 v_data = PPG_INIT_VALUE;

    /** check the p_max30102 structure for NULL pointer assignment*/
    if (p_max30102 == PPG_NULL)
    {
        return E_PPG_NULL_PTR;
    }
    else
    {
        /** read the proximity interrupt status field */
        com_rslt = p_max30102->PPG_BUS_READ_FUNC(p_max30102->dev_addr,
                   PPG_PROX_INT_STATUS_PTR__REG, &v_data, PPG_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == E_PPG_SUCCESS)
        {
            *p_status = PPG_GET_BITSLICE(v_data, PPG_PROX_INT_STATUS_PTR);
        }
    }
    return com_rslt;
}
/**
 *	@brief  This API is used to set the interrupt enable for proximity in MAX30102
 *          register 0x02 bit 4
 *
 *  @param  v_enable The value to be set in enable field
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_set_proximity_interrupt_enable(uint8 v_enable)
{
    /** variable used to return the status of communication result*/
    PPG_RETURN_FUNCTION_TYPE com_rslt = E_PPG_ERROR;
    uint8 v_data = PPG_INIT_VALUE;

    /** check the p_max30102 structure for NULL pointer assignment*/
    if (p_max30102 == PPG_NULL)
    {
        return E_PPG_NULL_PTR;
    }
    else
    {
        /** write in enable field */
        com_rslt = p_max30102->PPG_BUS_READ_FUNC(p_max30102->dev_addr,
                   PPG_PROX_INT_EN_PTR__REG, &v_data, PPG_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == E_PPG_SUCCESS)
        {
            v_data = PPG_SET_BITSLICE(v_data, PPG_PROX_INT_EN_PTR, v_enable);
            com_rslt += p_max30102->PPG_BUS_WRITE_FUNC(p_max30102->dev_addr,
                        PPG_PROX_INT_EN_PTR__REG, &v_data, PPG_GEN_READ_WRITE_DATA_LENGTH);
        }
    }
    return com_rslt;
}

/**
 *	@brief  This API is used to get the interrupt enable for proximity in MAX30102
 *          register 0x02 bit 4
 *
 *  @param  p_enable Stores value set in enable field
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_get_proximity_interrupt_enable(uint8 *p_enable)
{
    /** variable used to return the status of communication result*/
    PPG_RETURN_FUNCTION_TYPE com_rslt = E_PPG_ERROR;
    uint8 v_data = PPG_INIT_VALUE;

    /** check the p_max30102 structure for NULL pointer assignment*/
    if (p_max30102 == PPG_NULL)
    {
        return E_PPG_NULL_PTR;
    }
    else
    {
        /** read the proximity interrupt enable field */
        com_rslt = p_max30102->PPG_BUS_READ_FUNC(p_max30102->dev_addr,
                   PPG_PROX_INT_EN_PTR__REG, &v_data, PPG_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == E_PPG_SUCCESS)
        {
            *p_enable = PPG_GET_BITSLICE(v_data, PPG_PROX_INT_EN_PTR);
        }
    }
    return com_rslt;
}

/**
 *	@brief  This API writes Proximity Mode LED PA to register 0x10
 *
 *  @param  v_pilot_pa The value to write to Proximity Mode LED PA
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_set_proximity_mode_led_pa(uint8 v_pilot_pa)
{
    PPG_RETURN_FUNCTION_TYPE com_rslt  = E_PPG_ERROR;

    /** check the p_max30102 structure for NULL pointer assignment*/
    if (p_max30102 == PPG_NULL)
    {
        return E_PPG_NULL_PTR;
    }
    else
    {
        /** write Proximity Mode LED PA */
        com_rslt =  p_max30102->PPG_BUS_WRITE_FUNC(p_max30102->dev_addr,
                    PPG_PROX_MODE_LED_PA__REG, &v_pilot_pa, PPG_GEN_READ_WRITE_DATA_LENGTH);
    }
    return com_rslt;
}
/**
 *	@brief  This API reads Proximity Mode LED PA value from register 0x10
 *
 *  @param  v_pilot_pa Stores the value of Proximity Mode LED PA
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_get_proximity_mode_led_pa(uint8 *v_pilot_pa)
{
    PPG_RETURN_FUNCTION_TYPE com_rslt  = E_PPG_ERROR;

    /** check the p_max30102 structure for NULL pointer assignment*/
    if (p_max30102 == PPG_NULL)
    {
        return E_PPG_NULL_PTR;
    }
    else
    {
        /** read Proximity Mode LED PA */
         com_rslt = p_max30102->PPG_BUS_READ_FUNC(p_max30102->dev_addr,
                    PPG_PROX_MODE_LED_PA__REG, v_pilot_pa, PPG_GEN_READ_WRITE_DATA_LENGTH);
    }
    return com_rslt;
}
/**
 *	@brief  This API writes Proximity Interrupt Threshold to register 0x30
 *
 *  @param  v_prox_int_threshhold The value to write to Proximity Interrupt Threshold
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_set_proximity_interrupt_threshold(uint8 v_prox_int_threshhold)
{
    PPG_RETURN_FUNCTION_TYPE com_rslt  = E_PPG_ERROR;

    /** check the p_max30102 structure for NULL pointer assignment*/
    if (p_max30102 == PPG_NULL)
    {
        return E_PPG_NULL_PTR;
    }
    else
    {
        /** write Proximity Interrupt Threshold value */
        com_rslt =  p_max30102->PPG_BUS_WRITE_FUNC(p_max30102->dev_addr,
                    PPG_PROX_INT_THRESH__REG, &v_prox_int_threshhold,
                                                        PPG_GEN_READ_WRITE_DATA_LENGTH);
    }
    return com_rslt;
}
/**
 *	@brief  This API reads Proximity Interrupt Threshold value from register 0x30
 *
 *  @param  v_prox_int_threshhold Stores the value of Proximity Interrupt Threshold
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_get_proximity_interrupt_threshold(uint8 *v_prox_int_threshhold)
{
    PPG_RETURN_FUNCTION_TYPE com_rslt  = E_PPG_ERROR;

    /** check the p_max30102 structure for NULL pointer assignment*/
    if (p_max30102 == PPG_NULL)
    {
        return E_PPG_NULL_PTR;
    }
    else
    {
        /** read Proximity Interrupt Threshold value */
         com_rslt = p_max30102->PPG_BUS_READ_FUNC(p_max30102->dev_addr,
                    PPG_PROX_INT_THRESH__REG, v_prox_int_threshhold,
                                                       PPG_GEN_READ_WRITE_DATA_LENGTH);
    }
    return com_rslt;
}

/**
 *	@brief  This API is used to set the operating state in MAX30102
 *          register 0x09 bit 0 to 2
 *
 *  @param  v_mode The value to be set in mode field
 *          value | Mode
 *         -------|------------
 *            010 | HR
 *            011 | SPO2
 *            111 | Muli LED
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_set_operating_mode(uint8 v_mode)
{
    /** variable used to return the status of communication result*/
    PPG_RETURN_FUNCTION_TYPE com_rslt = E_PPG_ERROR;
    uint8 v_data = PPG_INIT_VALUE;

    /** check the p_max30102 structure for NULL pointer assignment*/
    if (p_max30102 == PPG_NULL)
    {
        return E_PPG_NULL_PTR;
    }
    else
    {
        /** write in mode field */
        com_rslt = p_max30102->PPG_BUS_READ_FUNC(p_max30102->dev_addr,
                   PPG_MODE_CONFIG__REG, &v_data, PPG_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == E_PPG_SUCCESS)
        {
            v_data = PPG_SET_BITSLICE(v_data, PPG_MODE_CONFIG, v_mode);
            com_rslt += p_max30102->PPG_BUS_WRITE_FUNC(p_max30102->dev_addr,
                        PPG_MODE_CONFIG__REG, &v_data, PPG_GEN_READ_WRITE_DATA_LENGTH);
        }

    }
    return com_rslt;
}
/**
 *	@brief  This API is used to get the operating state in MAX30102
 *          register 0x09 bit 0 to 2
 *
 *  @param  p_mode Stores value set in mode field
 *          value | Mode
 *         -------|------------
 *            010 | HR
 *            011 | SPO2
 *            111 | Muli LED
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_get_operating_mode(uint8 *p_mode)
{
    /** variable used to return the status of communication result*/
    PPG_RETURN_FUNCTION_TYPE com_rslt = E_PPG_ERROR;
    uint8 v_data = PPG_INIT_VALUE;

    /** check the p_max30102 structure for NULL pointer assignment*/
    if (p_max30102 == PPG_NULL)
    {
        return E_PPG_NULL_PTR;
    }
    else
    {
        /** read the mode field */
        com_rslt = p_max30102->PPG_BUS_READ_FUNC(p_max30102->dev_addr,
                   PPG_MODE_CONFIG__REG, &v_data, PPG_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == E_PPG_SUCCESS)
        {
            *p_mode = PPG_GET_BITSLICE(v_data, PPG_MODE_CONFIG);
        }

    }
    return com_rslt;
}
/**
 *	@brief  This API writes LED1 PA to the register 0x0C bit 0 to 7
 *
 *  @param  v_led_pa The value to write to LED PA
 *           value  |  Typical LED current (mA)
 *         ---------|------------------------------------------------
 *	         0x00	|	0.0
 *           0x01	|	0.2
 *           0x02   |   0.4
 *	         ...   	|	...
 *	         0x0F	|	3.1
 *	         ...   	|	...
 *	         0x1F	|	6.4
 *	         ...   	|	...
 *	         0x3F	|	12.5
 *	         ...   	|	...
 *	         0x7F	|	25.4
 *	         ...   	|	...
 *	         0xFF	|	50.0
 *
 *	@return  Success/Failure (0/1)
*/
PPG_RETURN_FUNCTION_TYPE ppg_set_led1_pa(uint8 v_led_pa)
{
    PPG_RETURN_FUNCTION_TYPE com_rslt  = E_PPG_ERROR;

    /** check the p_max30102 structure for NULL pointer assignment*/
    if (p_max30102 == PPG_NULL)
    {
        return E_PPG_NULL_PTR;
    }
    else
    {
        /** write LED1 PA */
        com_rslt =  p_max30102->PPG_BUS_WRITE_FUNC(p_max30102->dev_addr,
                    PPG_LED1_PA__REG, &v_led_pa, PPG_GEN_READ_WRITE_DATA_LENGTH);
    }
    return com_rslt;
}
/**
 *	@brief  This API writes LED2 PA to the register 0x0D bit 0 to 7
 *
 *  @param  v_led_pa The value to write to LED PA
 *           value  |  Typical LED current (mA)
 *         ---------|------------------------------------------------
 *	         0x00	|	0.0
 *           0x01	|	0.2
 *           0x02   |   0.4
 *	         ...   	|	...
 *	         0x0F	|	3.1
 *	         ...   	|	...
 *	         0x1F	|	6.4
 *	         ...   	|	...
 *	         0x3F	|	12.5
 *	         ...   	|	...
 *	         0x7F	|	25.4
 *	         ...   	|	...
 *	         0xFF	|	50.0
 *
 *	@return  Success/Failure (0/1)
*/
PPG_RETURN_FUNCTION_TYPE ppg_set_led2_pa(uint8 v_led_pa)
{
    PPG_RETURN_FUNCTION_TYPE com_rslt  = E_PPG_ERROR;

    /** check the p_max30102 structure for NULL pointer assignment*/
    if (p_max30102 == PPG_NULL)
    {
        return E_PPG_NULL_PTR;
    }
    else
    {
        /** write LED1 PA */
        com_rslt =  p_max30102->PPG_BUS_WRITE_FUNC(p_max30102->dev_addr,
                    PPG_LED2_PA__REG, &v_led_pa, PPG_GEN_READ_WRITE_DATA_LENGTH);
    }
    return com_rslt;
}
/**
 *	@brief  This API reads LED1 PA from the register 0x0C bit 0 to 7
 *
 *  @param  p_led_pa Stores the value of LED1 PA
 *
 *	@return  Success/Failure (0/1)
*/
PPG_RETURN_FUNCTION_TYPE ppg_get_led1_pa(uint8 *p_led_pa)
{
    PPG_RETURN_FUNCTION_TYPE com_rslt  = E_PPG_ERROR;

    /** check the p_max30102 structure for NULL pointer assignment*/
    if (p_max30102 == PPG_NULL)
    {
        return E_PPG_NULL_PTR;
    }
    else
    {
        /** read LED1 PA */
         com_rslt = p_max30102->PPG_BUS_READ_FUNC(p_max30102->dev_addr,
                    PPG_LED1_PA__REG, p_led_pa, PPG_GEN_READ_WRITE_DATA_LENGTH);
    }
    return com_rslt;
}
/**
 *	@brief  This API reads LED2 PA from the register 0x0D bit 0 to 7
 *
 *  @param  p_led_pa Stores the value of LED1 PA
 *
 *	@return  Success/Failure (0/1)
*/
PPG_RETURN_FUNCTION_TYPE ppg_get_led2_pa(uint8 *p_led_pa)
{
    PPG_RETURN_FUNCTION_TYPE com_rslt  = E_PPG_ERROR;

    /** check the p_max30102 structure for NULL pointer assignment*/
    if (p_max30102 == PPG_NULL)
    {
        return E_PPG_NULL_PTR;
    }
    else
    {
        /** read LED1 PA */
         com_rslt = p_max30102->PPG_BUS_READ_FUNC(p_max30102->dev_addr,
                    PPG_LED2_PA__REG, p_led_pa, PPG_GEN_READ_WRITE_DATA_LENGTH);
    }
    return com_rslt;
}
/**
 *	@brief  This API is used to set the fifo full water mark in MAX30102
 *          register 0x08 bit 0 to 3
 *
 *  @param  v_fifo_a_full Number of empty spaces in fifo before triggering
 *                        fifo full interrupt
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_set_fifo_a_full(uint8 v_fifo_a_full)
{
    /** variable used to return the status of communication result*/
    PPG_RETURN_FUNCTION_TYPE com_rslt = E_PPG_ERROR;
    uint8 v_data = PPG_INIT_VALUE;

    /** check the p_max30102 structure for NULL pointer assignment*/
    if (p_max30102 == PPG_NULL)
    {
        return E_PPG_NULL_PTR;
    }
    else
    {
        /** write in mode field */
        com_rslt = p_max30102->PPG_BUS_READ_FUNC(p_max30102->dev_addr,
                   PPG_FIFO_A_FULL__REG, &v_data, PPG_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == E_PPG_SUCCESS)
        {
            v_data = PPG_SET_BITSLICE(v_data, PPG_FIFO_A_FULL, v_fifo_a_full);
            com_rslt += p_max30102->PPG_BUS_WRITE_FUNC(p_max30102->dev_addr,
                        PPG_FIFO_A_FULL__REG, &v_data, PPG_GEN_READ_WRITE_DATA_LENGTH);
        }

    }
    return com_rslt;
}
/**
 *	@brief  This API is used to set the fifo roll over in MAX30102
 *          register 0x08 bit 4
 *
 *  @param  v_roll_over_en The value to be set in fifo roll over field
 *          value | roll over enable
 *         -------|---------------------------
 *            0   |     No fifo roll over
 *            1   |     fifo roll over
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_set_fifo_roll_over(uint8 v_roll_over_en)
{
    /** variable used to return the status of communication result*/
    PPG_RETURN_FUNCTION_TYPE com_rslt = E_PPG_ERROR;
    uint8 v_data = PPG_INIT_VALUE;

    /** check the p_max30102 structure for NULL pointer assignment*/
    if (p_max30102 == PPG_NULL)
    {
        return E_PPG_NULL_PTR;
    }
    else
    {
        /** write in mode field */
        com_rslt = p_max30102->PPG_BUS_READ_FUNC(p_max30102->dev_addr,
                   PPG_ROLL_OVER__REG, &v_data, PPG_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == E_PPG_SUCCESS)
        {
            v_data = PPG_SET_BITSLICE(v_data, PPG_ROLL_OVER, v_roll_over_en);
            com_rslt += p_max30102->PPG_BUS_WRITE_FUNC(p_max30102->dev_addr,
                        PPG_ROLL_OVER__REG, &v_data, PPG_GEN_READ_WRITE_DATA_LENGTH);
        }

    }
    return com_rslt;
}
/**
 *	@brief  This API is used to set the sample averaging in MAX30102
 *          register 0x08 bit 5 to 7
 *
 *  @param  v_sample_avg The value to be set in sample average field
 *          value | Number of samples averaged
 *         -------|---------------------------
 *            000 |     1 (no averaging)
 *            001 |     2
 *            010 |     4
 *            011 |     8
 *            ... |    ...
 *            111 |     32
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_set_sample_avg(uint8 v_sample_avg)
{
    /** variable used to return the status of communication result*/
    PPG_RETURN_FUNCTION_TYPE com_rslt = E_PPG_ERROR;
    uint8 v_data = PPG_INIT_VALUE;

    /** check the p_max30102 structure for NULL pointer assignment*/
    if (p_max30102 == PPG_NULL)
    {
        return E_PPG_NULL_PTR;
    }
    else
    {
        /** write in mode field */
        com_rslt = p_max30102->PPG_BUS_READ_FUNC(p_max30102->dev_addr,
                   PPG_SAMPLE_AVG__REG, &v_data, PPG_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == E_PPG_SUCCESS)
        {
            v_data = PPG_SET_BITSLICE(v_data, PPG_SAMPLE_AVG, v_sample_avg);
            com_rslt += p_max30102->PPG_BUS_WRITE_FUNC(p_max30102->dev_addr,
                        PPG_SAMPLE_AVG__REG, &v_data, PPG_GEN_READ_WRITE_DATA_LENGTH);
        }

    }
    return com_rslt;
}
/**
 *	@brief  This API is used to get the sample averaging in MAX30102
 *          register 0x08 bit 5 to 7
 *
 *  @param  p_sample_avg Stores value to set in sample average field
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_get_sample_avg(uint8 *p_sample_avg)
{
    /** variable used to return the status of communication result*/
    PPG_RETURN_FUNCTION_TYPE com_rslt = E_PPG_ERROR;
    uint8 v_data = PPG_INIT_VALUE;

    /** check the p_max30102 structure for NULL pointer assignment*/
    if (p_max30102 == PPG_NULL)
    {
        return E_PPG_NULL_PTR;
    }
    else
    {
        /** read the mode field */
        com_rslt = p_max30102->PPG_BUS_READ_FUNC(p_max30102->dev_addr,
                   PPG_SAMPLE_AVG__REG, &v_data, PPG_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == E_PPG_SUCCESS)
        {
            *p_sample_avg = PPG_GET_BITSLICE(v_data, PPG_SAMPLE_AVG);
        }

    }
    return com_rslt;
}
/**
 *	@brief  This API is used to set the SPO2 sensor ADC full
 *          scale range in MAX30102 using register 0x0A bit 5 to 6
 *
 *  @param  v_adc_range The value to be set in mode field
 *          value | full scale (nA)
 *         -------|-----------------
 *            00  | 2048
 *            01  | 4096
 *            10  | 8192
 *            11  | 16384
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_set_spo2_adc_range(uint8 v_adc_range)
{
    /** variable used to return the status of communication result*/
    PPG_RETURN_FUNCTION_TYPE com_rslt = E_PPG_ERROR;
    uint8 v_data = PPG_INIT_VALUE;

    /** check the p_max30102 structure for NULL pointer assignment*/
    if (p_max30102 == PPG_NULL)
    {
        return E_PPG_NULL_PTR;
    }
    else
    {
        /** write in ADC range field */
        com_rslt = p_max30102->PPG_BUS_READ_FUNC(p_max30102->dev_addr,
                   PPG_SPO2_ADC_RGE__REG, &v_data, PPG_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == E_PPG_SUCCESS)
        {
            v_data = PPG_SET_BITSLICE(v_data, PPG_SPO2_ADC_RGE, v_adc_range);
            com_rslt += p_max30102->PPG_BUS_WRITE_FUNC(p_max30102->dev_addr,
                        PPG_SPO2_ADC_RGE__REG, &v_data, PPG_GEN_READ_WRITE_DATA_LENGTH);
        }

    }
    return com_rslt;
}
/**
 *	@brief  This API is used to get the SPO2 sensor ADC full
 *          scale range in MAX30102 using register 0x0A bit 5 to 6
 *
 *  @param  p_adc_range Stores value set in ADC range field
 *          value | full scale (nA)
 *         -------|-----------------
 *            00  | 2048
 *            01  | 4096
 *            10  | 8192
 *            11  | 16384
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_get_spo2_adc_range(uint8 *p_adc_range)
{
    /** variable used to return the status of communication result*/
    PPG_RETURN_FUNCTION_TYPE com_rslt = E_PPG_ERROR;
    uint8 v_data = PPG_INIT_VALUE;

    /** check the p_max30102 structure for NULL pointer assignment*/
    if (p_max30102 == PPG_NULL)
    {
        return E_PPG_NULL_PTR;
    }
    else
    {
        /** read the mode field */
        com_rslt = p_max30102->PPG_BUS_READ_FUNC(p_max30102->dev_addr,
                   PPG_SPO2_ADC_RGE__REG, &v_data, PPG_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == E_PPG_SUCCESS)
        {
            *p_adc_range = PPG_GET_BITSLICE(v_data, PPG_SPO2_ADC_RGE);
        }

    }
    return com_rslt;
}
/**
 *	@brief  This API is used to set the SPO2 sensor sample rate in MAX30102
 *          using register 0x0A bit 2 to 4
 *
 *  @param  v_sample_rate The value to be set in sample rate field
 *          value | samples per sec
 *         -------|-----------------
 *            000 | 50
 *            001 | 100
 *            010 | 200
 *            011 | 400
 *            100 | 800
 *            101 | 1000
 *            110 | 1600
 *            111 | 3200
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_set_spo2_sample_rate(uint8 v_sample_rate)
{
    /** variable used to return the status of communication result*/
    PPG_RETURN_FUNCTION_TYPE com_rslt = E_PPG_ERROR;
    uint8 v_data = PPG_INIT_VALUE;

    /** check the p_max30102 structure for NULL pointer assignment*/
    if (p_max30102 == PPG_NULL)
    {
        return E_PPG_NULL_PTR;
    }
    else
    {
        /** write in sample rate field */
        com_rslt = p_max30102->PPG_BUS_READ_FUNC(p_max30102->dev_addr,
                   PPG_SPO2_SAMPLE_RATE__REG, &v_data, PPG_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == E_PPG_SUCCESS)
        {
            v_data = PPG_SET_BITSLICE(v_data, PPG_SPO2_SAMPLE_RATE, v_sample_rate);
            com_rslt += p_max30102->PPG_BUS_WRITE_FUNC(p_max30102->dev_addr,
                        PPG_SPO2_SAMPLE_RATE__REG, &v_data, PPG_GEN_READ_WRITE_DATA_LENGTH);
        }

    }
    return com_rslt;
}
/**
 *	@brief  This API is used to get the SPO2 sensor sample rate
 *          in MAX30102 using register 0x0A bit 5 to 6
 *
 *  @param  p_sample_rate Stores value set in ADC range field
 *          value | samples per sec
 *         -------|-----------------
 *            000 | 50
 *            001 | 100
 *            010 | 200
 *            011 | 400
 *            100 | 800
 *            101 | 1000
 *            110 | 1600
 *            111 | 3200
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_get_spo2_sample_rate(uint8 *p_sample_rate)
{
    /** variable used to return the status of communication result*/
    PPG_RETURN_FUNCTION_TYPE com_rslt = E_PPG_ERROR;
    uint8 v_data = PPG_INIT_VALUE;

    /** check the p_max30102 structure for NULL pointer assignment*/
    if (p_max30102 == PPG_NULL)
    {
        return E_PPG_NULL_PTR;
    }
    else
    {
        /** read the sample rate field */
        com_rslt = p_max30102->PPG_BUS_READ_FUNC(p_max30102->dev_addr,
                   PPG_SPO2_SAMPLE_RATE__REG, &v_data, PPG_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == E_PPG_SUCCESS)
        {
            *p_sample_rate = PPG_GET_BITSLICE(v_data, PPG_SPO2_SAMPLE_RATE);
        }

    }
    return com_rslt;
}
/**
 *	@brief  This API is used to set the SPO2 sensor LED pulse width
 *          and ADC resolution in MAX30102 using register 0x0A bit 0 to 1
 *
 *  @param  v_pulse_width The value to be set in LED pulse width field
 *          value | pulse width(us)| ADC resolution (bits)
 *         -------|----------------|-----------------------
 *            00  |      69        |      15
 *            01  |      118       |      16
 *            10  |      215       |      17
 *            11  |      411       |      18
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_set_led_pulse_width(uint8 v_pulse_width)
{
    /** variable used to return the status of communication result*/
    PPG_RETURN_FUNCTION_TYPE com_rslt = E_PPG_ERROR;
    uint8 v_data = PPG_INIT_VALUE;

    /** check the p_max30102 structure for NULL pointer assignment*/
    if (p_max30102 == PPG_NULL)
    {
        return E_PPG_NULL_PTR;
    }
    else
    {
        /** write in sample rate field */
        com_rslt = p_max30102->PPG_BUS_READ_FUNC(p_max30102->dev_addr,
                   PPG_SPO2_LED_PW__REG, &v_data, PPG_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == E_PPG_SUCCESS)
        {
            v_data = PPG_SET_BITSLICE(v_data, PPG_SPO2_LED_PW, v_pulse_width);
            com_rslt += p_max30102->PPG_BUS_WRITE_FUNC(p_max30102->dev_addr,
                        PPG_SPO2_LED_PW__REG, &v_data, PPG_GEN_READ_WRITE_DATA_LENGTH);
        }

    }
    return com_rslt;
}
/**
 *	@brief  This API is used to set the SPO2 sensor LED pulse width
 *          and ADC resolution in MAX30102 using register 0x0A bit 0 to 1
 *
 *  @param  p_pulse_width Stores value set in LED pulse width field
 *          value | pulse width(us)| ADC resolution (bits)
 *         -------|----------------|-----------------------
 *            00  |      69        |      15
 *            01  |      118       |      16
 *            10  |      215       |      17
 *            11  |      411       |      18
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_get_led_pulse_width(uint8 *p_pulse_width)
{
    /** variable used to return the status of communication result*/
    PPG_RETURN_FUNCTION_TYPE com_rslt = E_PPG_ERROR;
    uint8 v_data = PPG_INIT_VALUE;

    /** check the p_max30102 structure for NULL pointer assignment*/
    if (p_max30102 == PPG_NULL)
    {
        return E_PPG_NULL_PTR;
    }
    else
    {
        /** read the sample rate field */
        com_rslt = p_max30102->PPG_BUS_READ_FUNC(p_max30102->dev_addr,
                   PPG_SPO2_LED_PW__REG, &v_data, PPG_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == E_PPG_SUCCESS)
        {
            *p_pulse_width = PPG_GET_BITSLICE(v_data, PPG_SPO2_LED_PW);
        }

    }
    return com_rslt;
}
/**
 *	@brief  This API gets the length of FIFO of in MAX30102 using
 *	        register 0x04 and 0x06.
 *
 *  @param p_fifo_length The value of FIFO byte counter
 *  @param led_mode Enum indicates the led being used
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE PPG_FIFO_length(uint16 *p_fifo_length, ppg_led_mode_t led_mode)
{
    /** variable used to return the status of communication result*/
    PPG_RETURN_FUNCTION_TYPE com_rslt = E_PPG_ERROR;
    /** Variables to stores read and write ptr location*/
    uint8 v_fifo_wr_ptr = PPG_INIT_VALUE;
    uint8 v_fifo_rd_ptr = PPG_INIT_VALUE;
    uint8 v_num_available_samples = PPG_INIT_VALUE;
    uint8 v_mode = PPG_INIT_VALUE;

    /** check the p_max30102 structure for NULL pointer assignment*/
    if (p_max30102 == PPG_NULL)
    {
        return E_PPG_NULL_PTR;
    }
    else
    {
        /** read FIFO length*/
        com_rslt  = p_max30102->PPG_BUS_READ_FUNC(p_max30102->dev_addr,
                   PPG_FIFO_RD_PTR__REG, &v_fifo_rd_ptr, PPG_GEN_READ_WRITE_DATA_LENGTH);
        com_rslt += p_max30102->PPG_BUS_READ_FUNC(p_max30102->dev_addr,
                   PPG_FIFO_WR_PTR__REG, &v_fifo_wr_ptr, PPG_GEN_READ_WRITE_DATA_LENGTH);
        com_rslt += ppg_get_operating_mode(&v_mode);

        if (com_rslt == E_PPG_SUCCESS)
        {
            v_num_available_samples = (v_fifo_wr_ptr < v_fifo_rd_ptr)?
                                      ((v_fifo_wr_ptr + PPG_FIFO_DEPTH) - v_fifo_rd_ptr):
                                      (v_fifo_wr_ptr - v_fifo_rd_ptr);

            switch (v_mode){
            case HR_MODE_E:
                *p_fifo_length = v_num_available_samples * PPG_SINGLE_LED_TOTAL_BYTES;
                break;
            case SPO2_MODE_E:
                *p_fifo_length = v_num_available_samples * PPG_BOTH_LED_TOTAL_BYTES;
                break;
            case MULTI_LED_MODE_E:
                if (led_mode == IR_LED_ONLY)
                {
                    *p_fifo_length = v_num_available_samples * PPG_SINGLE_LED_TOTAL_BYTES;
                    break;
                }
                else
                {
                    SH_DRIVER_DEBUG_PANIC(("Unsupported LED mode"));
                }

            default:
                com_rslt = E_PPG_OUT_OF_RANGE;
            }
        }
    }
    return com_rslt;
}
/**
 *	@brief  This API reads the FIFO data in MAX30102 from the register 0x07
 *
 *  @param p_fifodata Pointer holding the FIFO data
 *  @param v_fifo_length Number of bytes to be read from FIFO.
 *                             The maximum size of FIFO is 192 bytes.
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE PPG_FIFO_data(uint8 *p_fifodata, uint16 v_fifo_length)
{
    /** variable used to return the status of communication result*/
    PPG_RETURN_FUNCTION_TYPE com_rslt = E_PPG_ERROR;

    /** check the p_max30102 structure for NULL pointer assignment*/
    if (p_max30102 == PPG_NULL)
    {
        return E_PPG_NULL_PTR;
    }
    else
    {
        /** read FIFO data*/
        com_rslt = p_max30102->PPG_BURST_READ_FUNC(p_max30102->dev_addr,
                   PPG_FIFO_DATA__REG, p_fifodata, v_fifo_length);
    }
    return com_rslt;
}
/**
 *	@brief  This API reads the FIFO data in MAX30102 from the register 0x07
 *          as per the user defined length and reorganizes the data according to LED types.
 *
 *  @param p_ppg_sensor_data Pointer holding the FIFO data defined as per algorithm
 *                              interface.
 *  @param v_fifo_length Number of bytes to be read from FIFO.
 *                             The maximum size of FIFO is 192 bytes.
 *  @param led_mode Enum indicates the led being used
 *  @param in_out pointer to store in and out status
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_read_fifo_data_user_defined_length(
        ppg_driver_sensor_data_t *p_ppg_sensor_data, uint16 v_fifo_length,
        ppg_led_mode_t led_mode, uint8 *in_out)
{
    /** variable used to return the status of communication result*/
    PPG_RETURN_FUNCTION_TYPE com_rslt = E_PPG_ERROR;
    uint8 v_mode = PPG_INIT_VALUE;
    uint8 v_fifo_index = PPG_INIT_VALUE;
    uint8 v_led_index = PPG_INIT_VALUE;

    /** check the p_max30102 structure for NULL pointer assignment*/
    if (p_max30102 == PPG_NULL)
    {
        return E_PPG_NULL_PTR;
    }
    else
    {
        memset(p_fifo_data, 0, FIFO_DEPTH);
        com_rslt = PPG_FIFO_data(p_fifo_data, v_fifo_length);
        #if DISABLE_SATURATION_CHECK
        p_ppg_sensor_data->red.saturation_flag = PPG_FIFO_check_data_saturation(v_fifo_length);
        if(p_ppg_sensor_data->red.saturation_flag)
        {
            /** Flush the FIFO */
            com_rslt += ppg_flush_fifo();
            return com_rslt;
        }
        #endif
        com_rslt += ppg_get_operating_mode(&v_mode);
        *in_out = ppg_threshold_checking(p_fifo_data, v_fifo_length);

        switch (v_mode){
        case HR_MODE_E:
            for(v_fifo_index = PPG_INIT_VALUE; v_fifo_index < v_fifo_length;)
            {
                // ADC 18 bit resolution converted to ADC 16 bit resolution by shifting to right by 2 bits
                p_ppg_sensor_data->red.p_ppg_data[v_led_index] =
                        (uint16)((p_fifo_data[v_fifo_index + PPG_FIFO_X_MSB_DATA] << PPG_SHIFT_BIT_POSITION_BY_14_BITS)
                          | (p_fifo_data[v_fifo_index + PPG_FIFO_X_MID_DATA] << PPG_SHIFT_BIT_POSITION_BY_06_BITS)
                          | (p_fifo_data[v_fifo_index + PPG_FIFO_X_LSB_DATA] >> PPG_SHIFT_BIT_POSITION_BY_02_BITS));

                p_ppg_sensor_data->red.frame_count += PPG_FRAME_COUNT;
                v_fifo_index += PPG_SINGLE_LED_TOTAL_BYTES;
                v_led_index += PPG_DATA_BUFFER_PTR_INCREMENT;
            }
            break;
        case SPO2_MODE_E:
            p_ppg_sensor_data->IR.p_ppg_data = p_ppg_sensor_data->red.p_ppg_data + (v_fifo_length >> 1);
            for(v_fifo_index = PPG_INIT_VALUE; v_fifo_index < v_fifo_length;)
            {
                /** ADC 18 bit resolution converted to ADC 16 bit resolution by shifting to right by 2 bits */
                p_ppg_sensor_data->red.p_ppg_data[v_led_index] =
                      (uint16)((p_fifo_data[v_fifo_index + PPG_FIFO_X_MSB_DATA] << PPG_SHIFT_BIT_POSITION_BY_14_BITS)
                        | (p_fifo_data[v_fifo_index + PPG_FIFO_X_MID_DATA] << PPG_SHIFT_BIT_POSITION_BY_06_BITS)
                        | (p_fifo_data[v_fifo_index + PPG_FIFO_X_LSB_DATA] >> PPG_SHIFT_BIT_POSITION_BY_02_BITS));

                p_ppg_sensor_data->red.frame_count += PPG_FRAME_COUNT;
                v_fifo_index += PPG_SINGLE_LED_TOTAL_BYTES;

                /** ADC 18 bit resolution converted to ADC 16 bit resolution by shifting to right by 2 bits */
                p_ppg_sensor_data->IR.p_ppg_data[v_led_index] =
                      (uint16)((p_fifo_data[v_fifo_index + PPG_FIFO_X_MSB_DATA] << PPG_SHIFT_BIT_POSITION_BY_14_BITS)
                        | (p_fifo_data[v_fifo_index + PPG_FIFO_X_MID_DATA] << PPG_SHIFT_BIT_POSITION_BY_06_BITS)
                        | (p_fifo_data[v_fifo_index + PPG_FIFO_X_LSB_DATA] >> PPG_SHIFT_BIT_POSITION_BY_02_BITS));

                p_ppg_sensor_data->IR.frame_count += PPG_FRAME_COUNT;
                v_fifo_index += PPG_SINGLE_LED_TOTAL_BYTES;

                v_led_index  += PPG_DATA_BUFFER_PTR_INCREMENT;
            }
            break;
        case MULTI_LED_MODE_E:
            switch (led_mode)
            {
            case RED_LED_ONLY:
                for(v_fifo_index = PPG_INIT_VALUE; v_fifo_index < v_fifo_length;)
                {
                    /** ADC 18 bit resolution converted to ADC 16 bit resolution by shifting to right by 2 bits */
                    p_ppg_sensor_data->red.p_ppg_data[v_led_index] =
                            (uint16)((p_fifo_data[v_fifo_index + PPG_FIFO_X_MSB_DATA] << PPG_SHIFT_BIT_POSITION_BY_14_BITS)
                              | (p_fifo_data[v_fifo_index + PPG_FIFO_X_MID_DATA] << PPG_SHIFT_BIT_POSITION_BY_06_BITS)
                              | (p_fifo_data[v_fifo_index + PPG_FIFO_X_LSB_DATA] >> PPG_SHIFT_BIT_POSITION_BY_02_BITS));

                    p_ppg_sensor_data->red.frame_count += PPG_FRAME_COUNT;
                    v_fifo_index += PPG_SINGLE_LED_TOTAL_BYTES;
                    v_led_index += PPG_DATA_BUFFER_PTR_INCREMENT;
                }
                break;
            case IR_LED_ONLY:
                for(v_fifo_index = PPG_INIT_VALUE; v_fifo_index < v_fifo_length;)
                {
                    /** ADC 18 bit resolution converted to ADC 16 bit resolution by shifting to right by 2 bits */
                    p_ppg_sensor_data->IR.p_ppg_data[v_led_index] =
                          (uint16)((p_fifo_data[v_fifo_index + PPG_FIFO_X_MSB_DATA] << PPG_SHIFT_BIT_POSITION_BY_14_BITS)
                            | (p_fifo_data[v_fifo_index + PPG_FIFO_X_MID_DATA] << PPG_SHIFT_BIT_POSITION_BY_06_BITS)
                            | (p_fifo_data[v_fifo_index + PPG_FIFO_X_LSB_DATA] >> PPG_SHIFT_BIT_POSITION_BY_02_BITS));

                    p_ppg_sensor_data->IR.frame_count += PPG_FRAME_COUNT;
                    v_led_index  += PPG_DATA_BUFFER_PTR_INCREMENT;
                    v_fifo_index += PPG_SINGLE_LED_TOTAL_BYTES;
                }
                break;
            case RED_IR_LED:
            default:
                p_ppg_sensor_data->IR.p_ppg_data = p_ppg_sensor_data->red.p_ppg_data + (v_fifo_length >> 1);
                for(v_fifo_index = PPG_INIT_VALUE; v_fifo_index < v_fifo_length;)
                {
                    /** ADC 18 bit resolution converted to ADC 16 bit resolution by shifting to right by 2 bits */
                    p_ppg_sensor_data->red.p_ppg_data[v_led_index] =
                          (uint16)((p_fifo_data[v_fifo_index + PPG_FIFO_X_MSB_DATA] << PPG_SHIFT_BIT_POSITION_BY_14_BITS)
                            | (p_fifo_data[v_fifo_index + PPG_FIFO_X_MID_DATA] << PPG_SHIFT_BIT_POSITION_BY_06_BITS)
                            | (p_fifo_data[v_fifo_index + PPG_FIFO_X_LSB_DATA] >> PPG_SHIFT_BIT_POSITION_BY_02_BITS));

                    p_ppg_sensor_data->red.frame_count += PPG_FRAME_COUNT;
                    v_fifo_index += PPG_SINGLE_LED_TOTAL_BYTES;

                    /** ADC 18 bit resolution converted to ADC 16 bit resolution by shifting to right by 2 bits */
                    p_ppg_sensor_data->IR.p_ppg_data[v_led_index] =
                          (uint16)((p_fifo_data[v_fifo_index + PPG_FIFO_X_MSB_DATA] << PPG_SHIFT_BIT_POSITION_BY_14_BITS)
                            | (p_fifo_data[v_fifo_index + PPG_FIFO_X_MID_DATA] << PPG_SHIFT_BIT_POSITION_BY_06_BITS)
                            | (p_fifo_data[v_fifo_index + PPG_FIFO_X_LSB_DATA] >> PPG_SHIFT_BIT_POSITION_BY_02_BITS));

                    p_ppg_sensor_data->IR.frame_count += PPG_FRAME_COUNT;
                    v_fifo_index += PPG_SINGLE_LED_TOTAL_BYTES;

                    v_led_index  += PPG_DATA_BUFFER_PTR_INCREMENT;
                    v_fifo_index += PPG_SINGLE_LED_TOTAL_BYTES;

                    /** ADC 18 bit resolution converted to ADC 16 bit resolution by shifting to right by 2 bits */
                    p_ppg_sensor_data->IR.p_ppg_data[v_led_index] =
                          (uint16)((p_fifo_data[v_fifo_index + PPG_FIFO_X_MSB_DATA] << PPG_SHIFT_BIT_POSITION_BY_14_BITS)
                            | (p_fifo_data[v_fifo_index + PPG_FIFO_X_MID_DATA] << PPG_SHIFT_BIT_POSITION_BY_06_BITS)
                            | (p_fifo_data[v_fifo_index + PPG_FIFO_X_LSB_DATA] >> PPG_SHIFT_BIT_POSITION_BY_02_BITS));

                    p_ppg_sensor_data->IR.frame_count += PPG_FRAME_COUNT;
                    v_led_index  += PPG_DATA_BUFFER_PTR_INCREMENT;
                    v_fifo_index += PPG_SINGLE_LED_TOTAL_BYTES;
                }
                break;
            }
            break;
        default:
            com_rslt += E_PPG_OUT_OF_RANGE;
            break;
        }
    }
    return com_rslt;
}
/**
 *	@brief  This API handles the data saturation in SPO2/HR photodiode
 *          If a data saturation is detected, the current level is reduced by 0.02 mA.
 *
 *  @param v_fifo_length Number of bytes to be read from FIFO.
 *                             The maximum size of FIFO is 192 bytes.
 *
 *	@return  saturation_flag_b: True/False
 */
PPG_RETURN_FUNCTION_TYPE PPG_FIFO_check_data_saturation(uint16 v_fifo_length)
{

    uint32 fifo_adc_18bit_data = PPG_INIT_VALUE;
    uint8 v_fifo_index = PPG_INIT_VALUE;
    bool saturation_flag_b = FALSE;

    for(v_fifo_index = PPG_INIT_VALUE; v_fifo_index < v_fifo_length;)
    {
        fifo_adc_18bit_data =
              (uint32)((p_fifo_data[v_fifo_index + PPG_FIFO_X_MSB_DATA] << PPG_SHIFT_BIT_POSITION_BY_16_BITS)
                | (p_fifo_data[v_fifo_index + PPG_FIFO_X_MID_DATA] << PPG_SHIFT_BIT_POSITION_BY_08_BITS)
                | (p_fifo_data[v_fifo_index + PPG_FIFO_X_LSB_DATA]));

        fifo_adc_18bit_data = (0x0003ffff & fifo_adc_18bit_data) >> PPG_SHIFT_BIT_POSITION_BY_02_BITS;

        if(fifo_adc_18bit_data > PPG_ADC_16BIT_SATURATION_VALUE)
        {
            saturation_flag_b = TRUE; /** set the data saturation flag to true */
            s_led_current_level -= PPG_CURRENT_DECREMENT_STEP; /** decrement the current by 1 level = 0.02 mA */
            ppg_set_led_pulse_width(s_led_current_level);
            break;
        }
        v_fifo_index += PPG_SINGLE_LED_TOTAL_BYTES; /** increment to start of next 18 bit sample */
    }
    return saturation_flag_b;
}
/**
 *	@brief  This API handles the current control of LED based on feedback from algorithms
 *          If in the request, algos report HR_DATA_BAD status current is incremented by
 *          0.2mA and FIFO is flushed. If HR_DATA_GOOD, FIFO is read and current is
 *          decremented by 0.2mA. If HR_OK, current level is unchanged and FIFO is read.
 *
 *  @param  p_ppg_sensor_data Pointer to store PPG sensor data.
 *  @param  ppg_header_feedback HR data quality status from algorithms.
 *  @param  led_mode Led mode value
 *  @param  in_out Pointer to store in and out status.
 *
 *	@return  saturation_flag_b: True/False
 */
PPG_RETURN_FUNCTION_TYPE ppg_led_current_control(ppg_driver_sensor_data_t *p_ppg_sensor_data,
                                                 uint8 ppg_header_feedback, ppg_led_mode_t led_mode,
                                                 uint8 *in_out)
{
    /** variable used to return the status of communication result*/
    PPG_RETURN_FUNCTION_TYPE com_rslt = E_PPG_ERROR;
    uint16 v_fifo_length = PPG_INIT_VALUE;
    uint8 v_led_current_level  =  PPG_INIT_VALUE;

    switch(ppg_header_feedback)
    {
    case HR_DATA_BAD:
        /** increment the current by 0.2 mA */
        switch(led_mode)
        {
        case IR_LED_ONLY:
            com_rslt += ppg_get_led2_pa(&v_led_current_level);
            com_rslt += ppg_set_led2_pa(v_led_current_level + PPG_CURRENT_INCREMENT_STEP);
            break;
        case PILOT_RED_LED_ONLY:
        case PILOT_IR_LED_ONLY:
            com_rslt += ppg_get_proximity_mode_led_pa(&v_led_current_level);
            com_rslt += ppg_set_proximity_mode_led_pa(v_led_current_level + PPG_CURRENT_INCREMENT_STEP);
            break;
        default:
            com_rslt += ppg_get_led1_pa(&v_led_current_level);
            com_rslt += ppg_set_led1_pa(v_led_current_level + PPG_CURRENT_INCREMENT_STEP);
            break;
        }
        com_rslt += ppg_get_led1_pa(&v_led_current_level);
        com_rslt += ppg_set_led1_pa(v_led_current_level + PPG_CURRENT_INCREMENT_STEP);
        /** Flush the FIFO */
        com_rslt += ppg_flush_fifo();
        break;
    case HR_DATA_GOOD:
        /** Read the FIFO if FIFO Length greater than 0 */
        com_rslt += PPG_FIFO_length(&v_fifo_length, led_mode);
        if (v_fifo_length > MAX_SENSOR_RSLT_BUFFER_SIZE)
        {
            SH_DRIVER_DEBUG_PANIC("FIFO length greater than result buffer size");
        }
        if(v_fifo_length)
        {
            com_rslt += ppg_read_fifo_data_user_defined_length(p_ppg_sensor_data, v_fifo_length, led_mode, in_out);
        }
        /** decrement the current by 0.2 mA */
        switch(led_mode)
        {
        case IR_LED_ONLY:
            com_rslt += ppg_get_led2_pa(&v_led_current_level);
            com_rslt += ppg_set_led2_pa(v_led_current_level - PPG_CURRENT_DECREMENT_STEP);
            break;
        case PILOT_RED_LED_ONLY:
        case PILOT_IR_LED_ONLY:
            com_rslt += ppg_get_proximity_mode_led_pa(&v_led_current_level);
            com_rslt += ppg_set_proximity_mode_led_pa(v_led_current_level - PPG_CURRENT_DECREMENT_STEP);
            break;
        default:
            com_rslt += ppg_get_led1_pa(&v_led_current_level);
            com_rslt += ppg_set_led1_pa(v_led_current_level - PPG_CURRENT_DECREMENT_STEP);
            break;
        }
        com_rslt += ppg_get_led1_pa(&v_led_current_level);
        com_rslt += ppg_set_led1_pa(v_led_current_level - PPG_CURRENT_DECREMENT_STEP);
        /** Flush the FIFO */
        com_rslt += ppg_flush_fifo();
        break;
    case HR_DATA_OK:
        /** Read the FIFO if FIFO Length greater than 0 */
        com_rslt += PPG_FIFO_length(&v_fifo_length, led_mode);
        if (v_fifo_length > MAX_SENSOR_RSLT_BUFFER_SIZE)
        {
            SH_DRIVER_DEBUG_PANIC("FIFO length greater than result buffer size");
        }
        if(v_fifo_length)
        {
            com_rslt += ppg_read_fifo_data_user_defined_length(p_ppg_sensor_data, v_fifo_length, led_mode, in_out);
        }
        break;
    case HR_DATA_VERY_BAD:
        break;
    }
    return com_rslt;
}
/**
 *	@brief  This APIis used to flush the FIFO.
 *
 *  @param
 *
 *	@return
 */
PPG_RETURN_FUNCTION_TYPE ppg_flush_fifo(void)
{
    /** variable used to return the status of communication result*/
    PPG_RETURN_FUNCTION_TYPE com_rslt = E_PPG_ERROR;

    /** Flush the FIFO */
    com_rslt += PPG_FIFO_data(p_fifo_data, PPG_FIFO_LENGTH);
    return com_rslt;
}

/**
 *	@brief  This API is used to reset the out of ear counter to zero.
 *
 *  @param
 *
 *	@return
 */
void ppg_reset_out_counter(void)
{
    // L0_DBG_MSG("Reset out counter");
    out_counter = 0;
}

/**
 *	@brief  This API reads the FIFO data in MAX30102 from the register 0x07
 *          as per the user defined length and reorganizes the data according to LED types.
 *
 *  @param led_mode mode of led in multi led configuration.
 *  @param in_out pointer to store in and out status
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE prox_read_fifo_data(ppg_led_mode_t led_mode, uint8 *in_out)
{
    /** variable used to return the status of communication result*/
    PPG_RETURN_FUNCTION_TYPE com_rslt = E_PPG_ERROR;

    uint16 v_fifo_length = PPG_INIT_VALUE;

    /** check the p_max30102 structure for NULL pointer assignment*/
    if (p_max30102 == PPG_NULL)
    {
        return E_PPG_NULL_PTR;
    }
    else
    {
        com_rslt = PPG_FIFO_length(&v_fifo_length, led_mode);
        memset(p_fifo_data, 0, FIFO_DEPTH);
        com_rslt += PPG_FIFO_data(p_fifo_data, v_fifo_length);
        *in_out = ppg_threshold_checking(p_fifo_data, v_fifo_length);
    }
    return com_rslt;
}

