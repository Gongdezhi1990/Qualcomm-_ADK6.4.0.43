/****************************************************************
* Copyright (C) 2017 Qualcomm Technologies International, Ltd
*
* bmi160.c
* Date: 23/05/2017
* Revision: 1.0.0
*
*
* Usage: (BMI160) Sensor Device Driver file
*
****************************************************************/

/** Device specific: BMI60 */
#include "sport_health_driver_imu_bmi160.h"
#include "sport_health_driver.h"
/** Crescendo specific */
//#include "imu_ppg_private.h"
#include "macros.h"
#include "hydra_log.h"

/***********************************************************
* PRIVATE VARIABLE DECLARATION
***********************************************************/
imu_t *p_imu;
/** Power mode monitoring variable used to introduce delays
 * after primary interface write in low power and suspend
 * modes of sensor */
uint8 imu_power_mode_status_g;

/***********************************************************
* PUBLIC FUNCTION DECFINITION
***********************************************************/
/**
 *	@brief  This API is used to perform initialization steps
 *          for BMI160. This include
 *          1. Trigger a reset including a reboot of BMI160.
 *          2. Read and save the CHIP_ID
 *          3. Prevent wake up of GYRO
 *  @param p_imu_info Pointer to bmi160 device specific
 *         information
 *	@return  Results of bus communication function
 *
 *
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_init(imu_t *p_imu_info)
{
    /** variable used to return the status of communication result*/
        IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
    uint8 v_data = IMU_DRIVER_INIT_VALUE;
    uint8 v_pmu_data = IMU_DRIVER_INIT_VALUE;
    /** assign bmi160 pointer */
    p_imu = p_imu_info;
    /** RESET the BMI160*/
    imu_set_command_register(RESET_IMU_DRIVER);
    /**reset to start up delay */
    p_imu->delay_msec(IMU_DRIVER_CHIP_RESET_DELAY);
	com_rslt =
        p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->dev_addr,
        IMU_DRIVER_USER_CHIP_ID__REG,
    &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
    /** store the chip id which is read from the sensor */
    p_imu->chip_id = v_data;
    /** To avoid gyro wakeup it is required to write 0x00 to 0x6C*/
    com_rslt = p_imu->IMU_DRIVER_BUS_WRITE_FUNC(p_imu->dev_addr,
    IMU_DRIVER_USER_PMU_TRIGGER_ADDR,
    &v_pmu_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
	return com_rslt;
}
/**
 *	@brief  This API is used to get the Accel bandwidth using the
 *          register 0x40 bit 4 to 6
 *
 *  @param  p_bw Stores the value of Accel bandwidth
 *          accel_us  | Under sampling mode
 *         -----------|---------------------
 *            0x00    |  no averaging
 *            0x01    |  average 2 samples
 *            0x02    |  average 4 samples
 *            0x03    |  average 8 samples
 *            0x04    |  average 16 samples
 *            0x05    |  average 32 samples
 *            0x06    |  average 64 samples
 *            0x07    |  average 128 samples
 *
 *	@return  DRIVER_SUCCESS/Failure (0/1)
 */
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_accel_bw(uint8 *p_bw)
{
    /** variable used to return the status of communication result*/
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
    uint8 v_data = IMU_DRIVER_INIT_VALUE;
    /** check the p_imu structure for NULL pointer assignment*/
    if (p_imu == IMU_DRIVER_NULL) {
        return E_IMU_DRIVER_NULL_PTR;
        } else {
            /** read the Accel bandwidth */
            com_rslt =
            p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->dev_addr,
            IMU_DRIVER_USER_ACCEL_CONFIG_ACCEL_BW__REG, &v_data,
            IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            *p_bw = IMU_DRIVER_GET_BITSLICE(v_data,
            IMU_DRIVER_USER_ACCEL_CONFIG_ACCEL_BW);
        }
    return com_rslt;
}
/**
 *	@brief  This API is used to set the Accel bandwidth using the
 *          register 0x40 bit 4 to 6
 *
 *  @param  v_bw The value to be set of Accel bandwidth
 *          accel_us  | Under sampling mode
 *         -----------|---------------------
 *            0x00    |  no averaging
 *            0x01    |  average 2 samples
 *            0x02    |  average 4 samples
 *            0x03    |  average 8 samples
 *            0x04    |  average 16 samples
 *            0x05    |  average 32 samples
 *            0x06    |  average 64 samples
 *            0x07    |  average 128 samples
 *
 *	@return  DRIVER_SUCCESS/Failure (0/1)
 */
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_accel_bw(uint8 v_bw)
{
    /** variable used to return the status of communication result*/
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
    uint8 v_data = IMU_DRIVER_INIT_VALUE;
    /** check the p_imu structure for NULL pointer assignment*/
    if (p_imu == IMU_DRIVER_NULL) {
        return E_IMU_DRIVER_NULL_PTR;
        } else {
        /** select Accel bandwidth*/
        if (v_bw <= IMU_DRIVER_MAX_ACCEL_BW) {
            /** write Accel bandwidth*/
            com_rslt =
            p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->dev_addr,
            IMU_DRIVER_USER_ACCEL_CONFIG_ACCEL_BW__REG, &v_data,
            IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            if (com_rslt == DRIVER_SUCCESS) {
                v_data = IMU_DRIVER_SET_BITSLICE(v_data,
                IMU_DRIVER_USER_ACCEL_CONFIG_ACCEL_BW,
                v_bw);
                com_rslt +=
                p_imu->IMU_DRIVER_BUS_WRITE_FUNC(
                p_imu->dev_addr,
                IMU_DRIVER_USER_ACCEL_CONFIG_ACCEL_BW__REG,
                &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);

                /**Accel and Gyro power mode check*/
                if (imu_power_mode_status_g !=
                IMU_DRIVER_NORMAL_MODE)
                    /**interface idle time delay */
                    p_imu->delay_msec(
                    IMU_DRIVER_GEN_READ_WRITE_DELAY);
            }
        } else {
        com_rslt = E_IMU_DRIVER_OUT_OF_RANGE;
        }
    }
    return com_rslt;
}
/**
 *	@brief  This API is used to set the Accel output date rate using the
 *          register 0x40 bit 0 to 3
 *
 *
 *  @param   v_output_data_rate The value of Accel output date rate
 *           value |  output data rate
 *          -------|--------------------------
 *	          0    |	IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_RESERVED
 *	          1	   |	IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_0_78HZ
 *	          2    |	IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_1_56HZ
 *	          3    |	IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_3_12HZ
 *	          4    |    IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_6_25HZ
 *	          5    |	IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_12_5HZ
 *	          6	   |	IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_25HZ
 *	          7	   |	IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_50HZ
 *	          8	   |	IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_100HZ
 *	          9	   |	IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_200HZ
 *	         10	   |	IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_400HZ
 *	         11	   |	IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_800HZ
 *	         12	   |	IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_1600HZ
 *
 *  @param  v_accel_bw :The value of selected Accel bandwidth
 *	@return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_accel_output_data_rate(
uint8 v_output_data_rate, uint8 v_accel_bw)
{
    /** variable used to return the status of communication result*/
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
    uint8 v_data = IMU_DRIVER_INIT_VALUE;
    /** check the p_imu structure for NULL pointer assignment*/
    if (p_imu == IMU_DRIVER_NULL) {
        return E_IMU_DRIVER_NULL_PTR;
        } else {
        if ((v_accel_bw >= IMU_DRIVER_ACCEL_RES_NO_AVG) &&
        (v_accel_bw <= IMU_DRIVER_ACCEL_RES_AVG128)) {
            /** enable the under sampling */
            /**com_rslt = imu_set_accel_under_sampling_parameter(
            IMU_DRIVER_US_ENABLE);*/
        } else if (((v_accel_bw > IMU_DRIVER_ACCEL_OSR4_AVG1) &&
        (v_accel_bw <= IMU_DRIVER_ACCEL_CIC_AVG8))
        || (v_accel_bw == IMU_DRIVER_ACCEL_OSR4_AVG1)) {
            /** disable the under sampling */
            com_rslt = imu_set_accel_under_sampling_parameter(
            IMU_DRIVER_US_DISABLE);
        }
        /** write Accel output data rate */
        com_rslt =
        p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->dev_addr,
        IMU_DRIVER_USER_ACCEL_CONFIG_OUTPUT_DATA_RATE__REG,
        &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == DRIVER_SUCCESS) {
            v_data = IMU_DRIVER_SET_BITSLICE(v_data,
            IMU_DRIVER_USER_ACCEL_CONFIG_OUTPUT_DATA_RATE,
            v_output_data_rate);
            com_rslt +=
            p_imu->IMU_DRIVER_BUS_WRITE_FUNC(
            p_imu->dev_addr,
            IMU_DRIVER_USER_ACCEL_CONFIG_OUTPUT_DATA_RATE__REG,
            &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);

            /** Accel and Gyro power mode check */
            if (imu_power_mode_status_g !=
            IMU_DRIVER_NORMAL_MODE)
                /** interface idle time delay */
                p_imu->delay_msec(
                IMU_DRIVER_GEN_READ_WRITE_DELAY);
        }
    }
    return com_rslt;
}
/**
 *	@brief  This API is used to get the Accel output date rate using the
 *          register 0x40 bit 0 to 3
 *
 *
 *  @param   p_output_data_rate Stores the value of Accel output
 *           date rate
 *           value |  output data rate
 *          -------|--------------------------
 *	          0    |	IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_RESERVED
 *	          1	   |	IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_0_78HZ
 *	          2    |	IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_1_56HZ
 *	          3    |	IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_3_12HZ
 *	          4    |    IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_6_25HZ
 *	          5    |	IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_12_5HZ
 *	          6	   |	IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_25HZ
 *	          7	   |	IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_50HZ
 *	          8	   |	IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_100HZ
 *	          9	   |	IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_200HZ
 *	         10	   |	IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_400HZ
 *	         11	   |	IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_800HZ
 *	         12	   |	IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_1600HZ
 *
 *	@return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_accel_output_data_rate(
uint8 *p_output_data_rate)
{
    /** variable used to return the status of communication result*/
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
    uint8 v_data = IMU_DRIVER_INIT_VALUE;
    /** check the p_imu structure for NULL pointer assignment*/
    if (p_imu == IMU_DRIVER_NULL) {
        return E_IMU_DRIVER_NULL_PTR;
        } else {
            /** read the Accel output data rate*/
            com_rslt =
            p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->dev_addr,
            IMU_DRIVER_USER_ACCEL_CONFIG_OUTPUT_DATA_RATE__REG,
            &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            *p_output_data_rate = IMU_DRIVER_GET_BITSLICE(v_data,
            IMU_DRIVER_USER_ACCEL_CONFIG_OUTPUT_DATA_RATE);
        }
    return com_rslt;
}
/**
 *	@brief  This API is used to get the Accel under sampling parameter
 *          using the register 0x40 bit 7
 *
 *	@param  p_accel_under_sampling Stores the value of Accel
 *          under sampling
 *	        value    | under_sampling
 *         ----------|---------------
 *            1      |  IMU_DRIVER_ENABLE
 *            0      |  IMU_DRIVER_DISABLE
 *
 *	@return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_accel_under_sampling_parameter(
uint8 *p_accel_under_sampling)
{
    /** variable used to return the status of communication result*/
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
    uint8 v_data = IMU_DRIVER_INIT_VALUE;
    /** check the p_imu structure for NULL pointer assignment*/
    if (p_imu == IMU_DRIVER_NULL) {
        return E_IMU_DRIVER_NULL_PTR;
        } else {
            /** read the Accel under sampling parameter */
            com_rslt =
            p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->dev_addr,
            IMU_DRIVER_USER_ACCEL_CONFIG_ACCEL_UNDER_SAMPLING__REG,
            &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            *p_accel_under_sampling =
            IMU_DRIVER_GET_BITSLICE(v_data,
            IMU_DRIVER_USER_ACCEL_CONFIG_ACCEL_UNDER_SAMPLING);
        }
    return com_rslt;
}
/**
 *	@brief  This API is used to set the Accel under sampling parameter
 *          using the register 0x40 bit 7
 *
 *	@param  v_accel_under_sampling The value of Accel under sampling
 *	        value    | under_sampling
 *         ----------|---------------
 *            1      |  IMU_DRIVER_ENABLE
 *            0      |  IMU_DRIVER_DISABLE
 *
 *	@return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_accel_under_sampling_parameter(
uint8 v_accel_under_sampling)
{
/** variable used to return the status of communication result*/
IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
uint8 v_data = IMU_DRIVER_INIT_VALUE;
/** check the p_imu structure for NULL pointer assignment*/
if (p_imu == IMU_DRIVER_NULL) {
    return E_IMU_DRIVER_NULL_PTR;
    } else {
    if (v_accel_under_sampling <= IMU_DRIVER_MAX_UNDER_SAMPLING) {
        com_rslt =
        p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->dev_addr,
        IMU_DRIVER_USER_ACCEL_CONFIG_ACCEL_UNDER_SAMPLING__REG,
        &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == DRIVER_SUCCESS) {
            /** write the Accel under sampling parameter */
            v_data = IMU_DRIVER_SET_BITSLICE(v_data,
            IMU_DRIVER_USER_ACCEL_CONFIG_ACCEL_UNDER_SAMPLING,
            v_accel_under_sampling);
            com_rslt +=
            p_imu->IMU_DRIVER_BUS_WRITE_FUNC(
            p_imu->dev_addr,
            IMU_DRIVER_USER_ACCEL_CONFIG_ACCEL_UNDER_SAMPLING__REG,
            &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);

            /**Accel and Gyro power mode check*/
            if (imu_power_mode_status_g !=
            IMU_DRIVER_NORMAL_MODE)
                /**interface idle time delay */
                p_imu->delay_msec(
                IMU_DRIVER_GEN_READ_WRITE_DELAY);
        }
    } else {
    com_rslt = E_IMU_DRIVER_OUT_OF_RANGE;
    }
}
return com_rslt;
}
/**
 *	@brief  This API is used to read the range (g values) of the
 *          Accel using the register 0x41 bit 0 to 3
 *
 *  @param p_range The value of Accel g range
 *         value    | g_range
 *        ----------|-----------
 *          0x03    | IMU_DRIVER_ACCEL_RANGE_2G
 *          0x05    | IMU_DRIVER_ACCEL_RANGE_4G
 *          0x08    | IMU_DRIVER_ACCEL_RANGE_8G
 *          0x0C    | IMU_DRIVER_ACCEL_RANGE_16G
 *
 *	@return  DRIVER_SUCCESS/Failure (0/1)
 */
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_accel_range(uint8 *p_range)
{
    /** variable used to return the status of communication result*/
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
    uint8 v_data = IMU_DRIVER_INIT_VALUE;
    /** check the p_imu structure for NULL pointer assignment*/
    if (p_imu == IMU_DRIVER_NULL) {
        return E_IMU_DRIVER_NULL_PTR;
        } else {
            /** read the Accel range*/
            com_rslt =
            p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->dev_addr,
            IMU_DRIVER_USER_ACCEL_RANGE__REG, &v_data,
            IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            *p_range = IMU_DRIVER_GET_BITSLICE(v_data,
            IMU_DRIVER_USER_ACCEL_RANGE);
        }
    return com_rslt;
}
/**
 *	@brief  This API is used to set the range (g values) of the
 *          Accel using the register 0x41 bit 0 to 3
 *
 *  @param v_range The value of Accel g range
 *         value    | g_range
 *        ----------|-----------
 *          0x03    | IMU_DRIVER_ACCEL_RANGE_2G
 *          0x05    | IMU_DRIVER_ACCEL_RANGE_4G
 *          0x08    | IMU_DRIVER_ACCEL_RANGE_8G
 *          0x0C    | IMU_DRIVER_ACCEL_RANGE_16G
 *
 *	@return  DRIVER_SUCCESS/Failure (0/1)
 */
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_accel_range(uint8 v_range)
{
    /** variable used to return the status of communication result*/
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
    uint8 v_data = IMU_DRIVER_INIT_VALUE;
    /** check the p_imu structure for NULL pointer assignment*/
    if (p_imu == IMU_DRIVER_NULL) {
        return E_IMU_DRIVER_NULL_PTR;
        } else {
        if ((v_range == IMU_DRIVER_ACCEL_RANGE0) ||
            (v_range == IMU_DRIVER_ACCEL_RANGE1) ||
            (v_range == IMU_DRIVER_ACCEL_RANGE2) ||
            (v_range == IMU_DRIVER_ACCEL_RANGE3)) {
            com_rslt =
            p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->dev_addr,
            IMU_DRIVER_USER_ACCEL_RANGE__REG,
            &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            if (com_rslt == DRIVER_SUCCESS) {
                v_data  = IMU_DRIVER_SET_BITSLICE(
                v_data, IMU_DRIVER_USER_ACCEL_RANGE,
                v_range);
                /** write the Accel range*/
                com_rslt +=
                p_imu->IMU_DRIVER_BUS_WRITE_FUNC(
                p_imu->dev_addr,
                IMU_DRIVER_USER_ACCEL_RANGE__REG,
                &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
#if 0
                /**Accel and Gyro power mode check*/
                if (imu_power_mode_status_g !=
                IMU_DRIVER_NORMAL_MODE)
                    /**interface idle time delay */
                    p_imu->delay_msec(
                    IMU_DRIVER_GEN_READ_WRITE_DELAY);
#endif
            }
        } else {
        com_rslt = E_IMU_DRIVER_OUT_OF_RANGE;
        }
    }
    return com_rslt;
}
/**
 *	@brief  This API reads the Accel power mode using PMU status
 *          register 0x03 bit 4 and 5
 *
 *  @param p_accel_power_mode_stat Stores the value of Accel
 *         power mode
 *	       accel_powermode  |   value
 *        ------------------|----------
 *           SUSPEND        |   0x00
 *           NORMAL         |   0x01
 *         LOW POWER        |   0x02
 *
 *	@return  DRIVER_SUCCESS/Failure (0/1)
 *
 *
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_accel_power_mode_stat(uint8
*p_accel_power_mode_stat)
{
    /** variable used to return the status of communication result*/
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
    uint8 v_data = IMU_DRIVER_INIT_VALUE;
    /** check the p_imu structure for NULL pointer assignment*/
    if (p_imu == IMU_DRIVER_NULL) {
        return E_IMU_DRIVER_NULL_PTR;
        } else {
            com_rslt =
            p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->dev_addr,
            IMU_DRIVER_USER_ACCEL_POWER_MODE_STAT__REG,
            &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            *p_accel_power_mode_stat =
            IMU_DRIVER_GET_BITSLICE(v_data,
            IMU_DRIVER_USER_ACCEL_POWER_MODE_STAT);
        }
    return com_rslt;
}
/**
 *	@brief This API is used to read the Gyro output data rate
 *         from the register 0x42 bit 0 to 3
 *
 *  @param  p_output_data_rate Stores the value of gyro output data rate
 *  value     |      gyro output data rate
 * -----------|-----------------------------
 *   0x00     | IMU_DRIVER_GYRO_OUTPUT_DATA_RATE_RESERVED
 *   0x01     | IMU_DRIVER_GYRO_OUTPUT_DATA_RATE_RESERVED
 *   0x02     | IMU_DRIVER_GYRO_OUTPUT_DATA_RATE_RESERVED
 *   0x03     | IMU_DRIVER_GYRO_OUTPUT_DATA_RATE_RESERVED
 *   0x04     | IMU_DRIVER_GYRO_OUTPUT_DATA_RATE_RESERVED
 *   0x05     | IMU_DRIVER_GYRO_OUTPUT_DATA_RATE_RESERVED
 *   0x06     | IMU_DRIVER_GYRO_OUTPUT_DATA_RATE_25HZ
 *   0x07     | IMU_DRIVER_GYRO_OUTPUT_DATA_RATE_50HZ
 *   0x08     | IMU_DRIVER_GYRO_OUTPUT_DATA_RATE_100HZ
 *   0x09     | IMU_DRIVER_GYRO_OUTPUT_DATA_RATE_200HZ
 *   0x0A     | IMU_DRIVER_GYRO_OUTPUT_DATA_RATE_400HZ
 *   0x0B     | IMU_DRIVER_GYRO_OUTPUT_DATA_RATE_800HZ
 *   0x0C     | IMU_DRIVER_GYRO_OUTPUT_DATA_RATE_1600HZ
 *   0x0D     | IMU_DRIVER_GYRO_OUTPUT_DATA_RATE_3200HZ
 *   0x0E     | IMU_DRIVER_GYRO_OUTPUT_DATA_RATE_RESERVED
 *   0x0F     | IMU_DRIVER_GYRO_OUTPUT_DATA_RATE_RESERVED
 *
 *	@return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_gyro_output_data_rate(uint8 *p_output_data_rate)
{
    /** variable used to return the status of communication result*/
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
    uint8 v_data = IMU_DRIVER_INIT_VALUE;
    /** check the p_imu structure for NULL pointer assignment*/
    if (p_imu == IMU_DRIVER_NULL) {
        return E_IMU_DRIVER_NULL_PTR;
        } else {
            /** read the gyro output data rate*/
            com_rslt =
            p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->dev_addr, IMU_DRIVER_USER_GYRO_CONFIG_OUTPUT_DATA_RATE__REG,
            &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            *p_output_data_rate = IMU_DRIVER_GET_BITSLICE(v_data, IMU_DRIVER_USER_GYRO_CONFIG_OUTPUT_DATA_RATE);
        }
    return com_rslt;
}
/**
 *	@brief This API is used to set the Gyro output data rate
 *         from the register 0x42 bit 0 to 3
 *
 *  @param  v_output_data_rate The value of gyro output data rate
 *
 *	@return  DRIVER_SUCCESS/Failure (0/1)
 *
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_gyro_output_data_rate(uint8 v_output_data_rate)
{
    /** variable used to return the status of communication result*/
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
    uint8 v_data = IMU_DRIVER_INIT_VALUE;
    /** check the p_imu structure for NULL pointer assignment*/
    if (p_imu == IMU_DRIVER_NULL) {
        return E_IMU_DRIVER_NULL_PTR;
        } else {
        /** select the gyro output data rate*/
        if ((v_output_data_rate <  IMU_DRIVER_OUTPUT_DATA_RATE6) &&
        (v_output_data_rate != IMU_DRIVER_INIT_VALUE)
        && (v_output_data_rate !=  IMU_DRIVER_OUTPUT_DATA_RATE1)
        && (v_output_data_rate !=  IMU_DRIVER_OUTPUT_DATA_RATE2)
        && (v_output_data_rate !=  IMU_DRIVER_OUTPUT_DATA_RATE3)
        && (v_output_data_rate !=  IMU_DRIVER_OUTPUT_DATA_RATE4)
        && (v_output_data_rate !=  IMU_DRIVER_OUTPUT_DATA_RATE5)
        && (v_output_data_rate !=  IMU_DRIVER_OUTPUT_DATA_RATE6)
        && (v_output_data_rate !=  IMU_DRIVER_OUTPUT_DATA_RATE7)) {
            /** write the gyro output data rate */
            com_rslt = p_imu->IMU_DRIVER_BUS_READ_FUNC
            (p_imu->dev_addr,
            IMU_DRIVER_USER_GYRO_CONFIG_OUTPUT_DATA_RATE__REG,
            &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            if (com_rslt == DRIVER_SUCCESS) {
                v_data = IMU_DRIVER_SET_BITSLICE(v_data,
                IMU_DRIVER_USER_GYRO_CONFIG_OUTPUT_DATA_RATE,
                v_output_data_rate);
                com_rslt += p_imu->IMU_DRIVER_BUS_WRITE_FUNC
                (p_imu->dev_addr,
                IMU_DRIVER_USER_GYRO_CONFIG_OUTPUT_DATA_RATE__REG,
                &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);

                /**Accel and Gyro power mode check*/
                if (imu_power_mode_status_g !=
                IMU_DRIVER_NORMAL_MODE)
                    /**interface idle time delay */
                    p_imu->delay_msec(
                    IMU_DRIVER_GEN_READ_WRITE_DELAY);
            }
        } else {
        com_rslt = E_IMU_DRIVER_OUT_OF_RANGE;
        }
    }
    return com_rslt;
}
/**
 *	@brief This API is used to read the bandwidth of gyro from
 *         the register 0x42 bit 4 to 5
 *
 *  @param  p_bw Stores the value of gyro bandwidth
 *  value     | gyro bandwidth
 *  ----------|----------------
 *   0x00     | IMU_DRIVER_GYRO_OSR4_MODE
 *   0x01     | IMU_DRIVER_GYRO_OSR2_MODE
 *   0x02     | IMU_DRIVER_GYRO_NORMAL_MODE
 *   0x03     | IMU_DRIVER_GYRO_CIC_MODE
 *
 *	@return  DRIVER_SUCCESS/Failure (0/1)
 *
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_gyro_bw(uint8 *p_bw)
{
    /** variable used to return the status of communication result*/
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
    uint8 v_data = IMU_DRIVER_INIT_VALUE;
    /** check the p_imu structure for NULL pointer assignment*/
    if (p_imu == IMU_DRIVER_NULL) {
        return E_IMU_DRIVER_NULL_PTR;
        } else {
            /** read gyro bandwidth*/
            com_rslt =
            p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->dev_addr,
            IMU_DRIVER_USER_GYRO_CONFIG_BW__REG, &v_data,
            IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            *p_bw = IMU_DRIVER_GET_BITSLICE(v_data,
            IMU_DRIVER_USER_GYRO_CONFIG_BW);
        }
    return com_rslt;
}
/**
 *	@brief This API is used to set the bandwidth of gyro from
 *         the register 0x42 bit 4 to 5
 *
 *  @param  v_bw Stores The value of gyro bandwidth
 *
 *	@return  DRIVER_SUCCESS/Failure (0/1)
 *
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_gyro_bw(uint8 v_bw)
{
    /** variable used to return the status of communication result*/
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
    uint8 v_data = IMU_DRIVER_INIT_VALUE;
    /** check the p_imu structure for NULL pointer assignment*/
    if (p_imu == IMU_DRIVER_NULL) {
        return E_IMU_DRIVER_NULL_PTR;
        } else {
        if (v_bw <= IMU_DRIVER_MAX_GYRO_BW) {
            /** write the gyro bandwidth*/
            com_rslt =
            p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->dev_addr,
            IMU_DRIVER_USER_GYRO_CONFIG_BW__REG,
            &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            if (com_rslt == DRIVER_SUCCESS) {
                v_data = IMU_DRIVER_SET_BITSLICE(v_data,
                IMU_DRIVER_USER_GYRO_CONFIG_BW, v_bw);
                com_rslt +=
                p_imu->IMU_DRIVER_BUS_WRITE_FUNC(
                p_imu->dev_addr,
                IMU_DRIVER_USER_GYRO_CONFIG_BW__REG,
                &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);

                /**Accel and Gyro power mode check*/
                if (imu_power_mode_status_g !=
                IMU_DRIVER_NORMAL_MODE)
                    /**interface idle time delay */
                    p_imu->delay_msec(
                    IMU_DRIVER_GEN_READ_WRITE_DELAY);
            }
        } else {
        com_rslt = E_IMU_DRIVER_OUT_OF_RANGE;
        }
    }
    return com_rslt;
}
/**
 *	@brief This API reads the range of Gyro from the register
 *         0x43 bit 0 to 2
 *
 *  @param  p_range Stores the value of gyro range
 *   value    |    range
 *  ----------|-------------------------------
 *    0x00    | IMU_DRIVER_GYRO_RANGE_2000_DEG_SEC
 *    0x01    | IMU_DRIVER_GYRO_RANGE_1000_DEG_SEC
 *    0x02    | IMU_DRIVER_GYRO_RANGE_500_DEG_SEC
 *    0x03    | IMU_DRIVER_GYRO_RANGE_250_DEG_SEC
 *    0x04    | IMU_DRIVER_GYRO_RANGE_125_DEG_SEC
 *
 *	@return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_gyro_range(uint8 *p_range)
{
    /** variable used to return the status of communication result*/
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
    uint8 v_data = IMU_DRIVER_INIT_VALUE;
    /** check the p_imu structure for NULL pointer assignment*/
    if (p_imu == IMU_DRIVER_NULL) {
        return E_IMU_DRIVER_NULL_PTR;
        } else {
            /** read the gyro range */
            com_rslt = p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->dev_addr,
            IMU_DRIVER_USER_GYRO_RANGE__REG, &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            *p_range = IMU_DRIVER_GET_BITSLICE(v_data, IMU_DRIVER_USER_GYRO_RANGE);
        }
    return com_rslt;
}
/**
 *	@brief This API sets the range of Gyro from the register
 *         0x43 bit 0 to 2
 *
 *  @param  v_range The value of gyro range
 *
 *	@return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_gyro_range(uint8 v_range)
{
    /** variable used to return the status of communication result*/
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
    uint8 v_data = IMU_DRIVER_INIT_VALUE;
    /** check the p_imu structure for NULL pointer assignment*/
    if (p_imu == IMU_DRIVER_NULL) {
        return E_IMU_DRIVER_NULL_PTR;
        } else {
        if (v_range <= IMU_DRIVER_MAX_GYRO_RANGE) {
            /** write the gyro range value */
            com_rslt = p_imu->IMU_DRIVER_BUS_READ_FUNC
            (p_imu->dev_addr,
            IMU_DRIVER_USER_GYRO_RANGE__REG,
            &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            if (com_rslt == DRIVER_SUCCESS) {
                v_data = IMU_DRIVER_SET_BITSLICE(v_data,
                IMU_DRIVER_USER_GYRO_RANGE,
                v_range);
                com_rslt += p_imu->IMU_DRIVER_BUS_WRITE_FUNC
                (p_imu->dev_addr,
                IMU_DRIVER_USER_GYRO_RANGE__REG,
                &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);

                /**Accel and Gyro power mode check*/
                if (imu_power_mode_status_g !=
                IMU_DRIVER_NORMAL_MODE)
                    /**interface idle time delay */
                    p_imu->delay_msec(
                    IMU_DRIVER_GEN_READ_WRITE_DELAY);
            }
        } else {
        com_rslt = E_IMU_DRIVER_OUT_OF_RANGE;
        }
    }
    return com_rslt;
}
/**
 *	@brief This API is used to enable gyro data in FIFO (all 3 axes)
 *         from the register 0x47 bit 7
 *
 *
 *  @param v_fifo_gyro The value of FIFO gyro enable
 *	value    | FIFO gyro
 * ----------|-------------------
 *  0x00     |  no gyro data is stored
 *  0x01     |  gyro data is stored
 *
 *
 *	@return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_fifo_gyro_enable(uint8 v_fifo_gyro)
{
    /** variable used to return the status of communication result*/
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
    uint8 v_data = IMU_DRIVER_INIT_VALUE;
    /** check the p_imu structure for NULL pointer assignment*/
    if (p_imu == IMU_DRIVER_NULL) {
        return E_IMU_DRIVER_NULL_PTR;
        } else {
        if (v_fifo_gyro <= IMU_DRIVER_MAX_VALUE_FIFO_GYRO) {
            /** write FIFO gyro enable*/
            com_rslt =
            p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->dev_addr,
            IMU_DRIVER_USER_FIFO_GYRO_ENABLE__REG, &v_data,
            IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            if (com_rslt == DRIVER_SUCCESS) {
                v_data = IMU_DRIVER_SET_BITSLICE(v_data,
                IMU_DRIVER_USER_FIFO_GYRO_ENABLE, v_fifo_gyro);
                com_rslt +=
                p_imu->IMU_DRIVER_BUS_WRITE_FUNC(
                p_imu->dev_addr,
                IMU_DRIVER_USER_FIFO_GYRO_ENABLE__REG,
                &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            }
        } else {
        com_rslt = E_IMU_DRIVER_OUT_OF_RANGE;
        }
    }
    return com_rslt;
}
/**
 *	@brief  This API writes the value to the register 0x7E bit 0 to 7
 *
 *  @param  v_command_reg The value to write command register
 *           value  |  Description
 *         ---------|------------------------------------------------
 *	         0x00	|	Reserved
 *           0x03	|	Starts fast offset calibration for the Accel
 *                  |   and gyro
 *	         0x10	|	Sets the PMU mode for the Accel to suspend
 *	         0x11	|	Sets the PMU mode for the Accel to normal
 *	         0x12	|	Sets the PMU mode for the Accel Lowpower
 *           0x14	|	Sets the PMU mode for the Gyro to suspend
 *	         0x15	|	Sets the PMU mode for the Gyro to normal
 *	         0x16	|	Reserved
 *	         0x17	|	Sets the PMU mode for the Gyro to fast start-up
 *           0x18	|	Sets the PMU mode for the Mag to suspend
 *	         0x19	|	Sets the PMU mode for the Mag to normal
 *	         0x1A	|	Sets the PMU mode for the Mag to Lowpower
 *	         0xB0	|	Clears all data in the FIFO
 *           0xB1	|	Resets the interrupt engine
 *	         0xB2	|	step_cnt_clr Clears the step counter
 *	         0xB6	|	Triggers a reset
 *
 *	@return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_command_register(uint8 v_command_reg)
{
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt  = E_IMU_DRIVER_COMM_RES;
    /** check the p_imu structure for NULL pointer assignment*/
    if (p_imu == IMU_DRIVER_NULL) {
        return E_IMU_DRIVER_NULL_PTR;
        } else {
            /** write command register */
            com_rslt = p_imu->IMU_DRIVER_BUS_WRITE_FUNC(
            p_imu->dev_addr,
            IMU_DRIVER_CMD_COMMANDS__REG,
            &v_command_reg, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
                /**interface idle time delay */
            p_imu->delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
            /**power mode status of Accel and gyro is stored in the
            global variable imu_power_mode_status_g */
            com_rslt += p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->dev_addr,
            IMU_DRIVER_USER_PMU_STAT_ADDR,
            &imu_power_mode_status_g, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            imu_power_mode_status_g &=
            IMU_DRIVER_ACCEL_GYRO_PMU_MASK;
        }
    return com_rslt;
}
#ifdef FIFO_ENABLE
/**
 *	@brief This API reads FIFO frame header enable using
 *         the register 0x47 bit 4
 *
 *  @param p_fifo_header Stores the value of FIFO header
 *	        value   | FIFO header
 *        ----------|-------------------
 *            1     |  IMU_DRIVER_ENABLE
 *            0     |  IMU_DRIVER_DISABLE
 *
 *	@return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_fifo_header_enable(
uint8 *p_fifo_header)
{
    /** variable used to return the status of communication result*/
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
    uint8 v_data = IMU_DRIVER_INIT_VALUE;
    /** check the p_imu structure for NULL pointer assignment*/
    if (p_imu == IMU_DRIVER_NULL) {
        return E_IMU_DRIVER_NULL_PTR;
        } else {
            /** read FIFO header */
            com_rslt =
            p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->dev_addr,
            IMU_DRIVER_USER_FIFO_HEADER_ENABLE__REG, &v_data,
            IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            *p_fifo_header = IMU_DRIVER_GET_BITSLICE(v_data,
            IMU_DRIVER_USER_FIFO_HEADER_ENABLE);
        }
    return com_rslt;
}
/**
 *	@brief This API sets FIFO frame header enable using
 *         the register 0x47 bit 4
 *
 *  @param v_fifo_header The value of FIFO header
 *	        value   | FIFO header
 *        ----------|-------------------
 *            1     |  IMU_DRIVER_ENABLE
 *            0     |  IMU_DRIVER_DISABLE
 *
 *	@return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_fifo_header_enable(
uint8 v_fifo_header)
{
    /** variable used to return the status of communication result*/
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
    uint8 v_data = IMU_DRIVER_INIT_VALUE;
    /** check the p_imu structure for NULL pointer assignment*/
    if (p_imu == IMU_DRIVER_NULL) {
        return E_IMU_DRIVER_NULL_PTR;
        } else {
        if (v_fifo_header <= IMU_DRIVER_MAX_VALUE_FIFO_HEADER) {
            /** write the FIFO header */
            com_rslt =
            p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->dev_addr,
            IMU_DRIVER_USER_FIFO_HEADER_ENABLE__REG, &v_data,
            IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            if (com_rslt == DRIVER_SUCCESS) {
                v_data = IMU_DRIVER_SET_BITSLICE(v_data,
                IMU_DRIVER_USER_FIFO_HEADER_ENABLE,
                v_fifo_header);
                com_rslt +=
                p_imu->IMU_DRIVER_BUS_WRITE_FUNC(
                p_imu->dev_addr,
                IMU_DRIVER_USER_FIFO_HEADER_ENABLE__REG,
                &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            }
        } else {
        com_rslt = E_IMU_DRIVER_OUT_OF_RANGE;
        }
    }
    return com_rslt;
}
/**
 *	@brief  This API is used to check whether Accel data is stored
 *          in FIFO (all 3 axes) or not using the register 0x47 bit 6
 *
 *  @param p_fifo_accel Stores the value of FIFO Accel enable
 *	       value    | FIFO Accel
 *        ----------|-------------------
 *           0      |  no Accel data is stored
 *           1      |  Accel data is stored
 *
 *	@return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_fifo_accel_enable(
uint8 *p_fifo_accel)
{
    /** variable used to return the status of communication result*/
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
    uint8 v_data = IMU_DRIVER_INIT_VALUE;
    /** check the p_imu structure for NULL pointer assignment*/
    if (p_imu == IMU_DRIVER_NULL) {
        return E_IMU_DRIVER_NULL_PTR;
        } else {
            /** read the Accel FIFO enable*/
            com_rslt =
            p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->dev_addr,
            IMU_DRIVER_USER_FIFO_ACCEL_ENABLE__REG, &v_data,
            IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            *p_fifo_accel =
            IMU_DRIVER_GET_BITSLICE(v_data,
            IMU_DRIVER_USER_FIFO_ACCEL_ENABLE);
        }
    return com_rslt;
}
/**
 *	@brief  This API is used to enable Accel data in FIFO (all 3 axes)
 *          using the register 0x47 bit 6
 *
 *  @param v_fifo_accel Stores the value of FIFO Accel enable
 *	       value    | FIFO Accel
 *        ----------|-------------------
 *           0      |  no Accel data is stored
 *           1      |  Accel data is stored
 *
 *	@return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_fifo_accel_enable(
uint8 v_fifo_accel)
{
    /** variable used to return the status of communication result*/
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
    uint8 v_data = IMU_DRIVER_INIT_VALUE;
    /** check the p_imu structure for NULL pointer assignment*/
    if (p_imu == IMU_DRIVER_NULL) {
        return E_IMU_DRIVER_NULL_PTR;
        } else {
        if (v_fifo_accel <= IMU_DRIVER_MAX_VALUE_FIFO_ACCEL) {
            /** write the FIFO Mag enables*/
            com_rslt =
            p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->dev_addr,
            IMU_DRIVER_USER_FIFO_ACCEL_ENABLE__REG, &v_data,
            IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            if (com_rslt == DRIVER_SUCCESS) {
                v_data = IMU_DRIVER_SET_BITSLICE(v_data,
                IMU_DRIVER_USER_FIFO_ACCEL_ENABLE, v_fifo_accel);
                com_rslt +=
                p_imu->IMU_DRIVER_BUS_WRITE_FUNC(
                p_imu->dev_addr,
                IMU_DRIVER_USER_FIFO_ACCEL_ENABLE__REG,
                &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            }
        } else {
        com_rslt = E_IMU_DRIVER_OUT_OF_RANGE;
        }
    }
    return com_rslt;
}
/**
 *	@brief  This API reads the FIFO length of the sensor
 *	from the register 0x23 and 0x24 bit 0 to 7 and 0 to 2
 *
 *  @param p_fifo_length The value of FIFO byte counter
 *	@return  DRIVER_SUCCESS/Failure (0/1)
 */
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_d_fifo_length(uint16 *p_fifo_length)
{
    /** variable used to return the status of communication result*/
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
    /** Array contains the FIFO length data
    v_data[0] - FIFO length
    v_data[1] - FIFO length*/
    uint8 a_datar[IMU_DRIVER_FIFO_DATA_SIZE] = {IMU_DRIVER_INIT_VALUE,
    IMU_DRIVER_INIT_VALUE};
    /** check the p_imu structure for NULL pointer assignment*/
    if (p_imu == IMU_DRIVER_NULL) {
        return E_IMU_DRIVER_NULL_PTR;
        } else {
            /** read FIFO length*/
            com_rslt =
            p_imu->IMU_DRIVER_BURST_READ_FUNC(p_imu->dev_addr,
            IMU_DRIVER_USER_FIFO_BYTE_COUNTER_LSB__REG, a_datar,
             IMU_DRIVER_FIFO_DATA_LENGTH);

            a_datar[IMU_DRIVER_FIFO_LENGTH_MSB_BYTE] =
            IMU_DRIVER_GET_BITSLICE(
            a_datar[IMU_DRIVER_FIFO_LENGTH_MSB_BYTE],
            IMU_DRIVER_USER_FIFO_BYTE_COUNTER_MSB);

            *p_fifo_length =
            (uint16)(((uint16)((uint8) (
            a_datar[IMU_DRIVER_FIFO_LENGTH_MSB_BYTE]) <<
            IMU_DRIVER_SHIFT_BIT_POSITION_BY_08_BITS))
            | a_datar[IMU_DRIVER_FIFO_LENGTH_LSB_BYTE]);
        }
    return com_rslt;
}
/**
 *	@brief  This API reads the FIFO data of the sensor from the
 *          register 0x24
 *
 *  @param p_fifodata Pointer holding the FIFO data
 *  @param v_fifo_length Number of bytes to be read from
 *         FIFO. The maximum size of FIFO is 1024 bytes.
 *
 *	@return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_d_fifo_data(
uint8 *p_fifodata, uint16 v_fifo_length)
{
    /** variable used to return the status of communication result*/
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
    UNUSED(v_fifo_length);

    /** check the p_imu structure for NULL pointer assignment*/
    if (p_imu == IMU_DRIVER_NULL) {
        return E_IMU_DRIVER_NULL_PTR;
        } else {
            /** read FIFO data*/
            com_rslt =
            p_imu->IMU_DRIVER_BURST_READ_FUNC(p_imu->dev_addr,
            IMU_DRIVER_USER_FIFO_DATA__REG,
            p_fifodata, v_fifo_length);
        }
    return com_rslt;
}
/**
 *	@brief  This function is used to read the fifo data when
 *  configured in header mode
 *
 *  @param p_imu_d_sensor_data Pointer to 3D sensor data structure
 *
 *	@return  DRIVER_SUCCESS/Failure (0/1)
 */
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_read_fifo_header_data(
imu_driver_sensor_data_t *p_imu_d_sensor_data, uint16 v_fifo_length)
{
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
    /** read the whole FIFO data*/
    com_rslt =
    imu_read_fifo_header_data_user_defined_length(
    v_fifo_length, p_imu_d_sensor_data);
    return com_rslt;
}
/**
 *	@brief  This function is used to read the fifo data in header
 *          mode for user defined length
 *
 *  @param p_fifo_header_data Pointer to 3D sensor data structure
 *  @param v_fifo_user_length Number of bytes to be read
 *
 *	@return  DRIVER_SUCCESS/Failure (0/1)
 */
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_read_fifo_header_data_user_defined_length(
uint16 v_fifo_user_length, imu_driver_sensor_data_t *p_fifo_header_data)
{
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
    uint8  v_temp = v_fifo_user_length / MAX_DATA_TRANSFER_LENGTH;
    uint16 v_fifo_index = IMU_DRIVER_INIT_VALUE;
    uint16 v_accel_data_index = IMU_DRIVER_INIT_VALUE;
    uint16 v_gyro_data_index = IMU_DRIVER_INIT_VALUE;
    uint16 v_fifo_length = IMU_DRIVER_INIT_VALUE;
    int8 v_last_return_stat = IMU_DRIVER_INIT_VALUE;
    uint8 v_frame_head = IMU_DRIVER_INIT_VALUE;
    uint8 num_i2c_transfer = IMU_DRIVER_INIT_VALUE;

    /** Fetch the FIFO data from sensor in blocks of MAX_DATA_TRANSFER_LENGTH,
     *  with the last fetch collecting the residual partial block (if present).
     *  This is done to minimize the memory usage to store the received unorganized
     *  FIFO data from the IMU sensor */
    num_i2c_transfer = (v_fifo_user_length - (v_temp * MAX_DATA_TRANSFER_LENGTH))?
                             (v_temp + 1) : v_temp;
    while (num_i2c_transfer) 
    {
        /** read FIFO data*/
        v_fifo_length = (v_fifo_user_length > MAX_DATA_TRANSFER_LENGTH)?
                          (MAX_DATA_TRANSFER_LENGTH) : v_fifo_user_length;
        v_fifo_user_length = v_fifo_user_length - v_fifo_length;
        memset(p_fifo_data, 0, MAX_DATA_TRANSFER_LENGTH);
        com_rslt = imu_d_fifo_data(p_fifo_data, v_fifo_length);
        for (v_fifo_index = IMU_DRIVER_INIT_VALUE;v_fifo_index < v_fifo_length;)
        {
            v_frame_head = p_fifo_data[v_fifo_index] & IMU_DRIVER_FIFO_TAG_INTR_MASK;
            switch (v_frame_head)
            {
            /** Header frame of Accel */
            case FIFO_HEAD_A:
            {
                /**fifo data frame index + 1*/
                v_fifo_index += IMU_DRIVER_FIFO_INDEX_LENGTH;
                if ((v_fifo_index + IMU_DRIVER_FIFO_A_LENGTH) >  v_fifo_length)
                {
                    v_last_return_stat = FIFO_A_OVER_LEN;
                    break;
                }

                /** Accel raw x data */
               p_fifo_header_data->accel.p_imu_data[v_accel_data_index] =
               (int16)(((p_fifo_data[v_fifo_index +
               IMU_DRIVER_FIFO_X_MSB_DATA])
               << IMU_DRIVER_SHIFT_BIT_POSITION_BY_08_BITS)
               | (p_fifo_data[v_fifo_index +
               IMU_DRIVER_FIFO_X_LSB_DATA]));
               v_accel_data_index += IMU_DRIVER_FIFO_INDEX_LENGTH;
               /** Accel raw y data */
               p_fifo_header_data->accel.p_imu_data[v_accel_data_index] =
               (int16)(((p_fifo_data[v_fifo_index +
               IMU_DRIVER_FIFO_Y_MSB_DATA])
               << IMU_DRIVER_SHIFT_BIT_POSITION_BY_08_BITS)
               | (p_fifo_data[v_fifo_index +
               IMU_DRIVER_FIFO_Y_LSB_DATA]));
               v_accel_data_index += IMU_DRIVER_FIFO_INDEX_LENGTH;
               /** Accel raw z data */
               p_fifo_header_data->accel.p_imu_data[v_accel_data_index] =
               (int16)(((p_fifo_data[v_fifo_index +
               IMU_DRIVER_FIFO_Z_MSB_DATA])
               << IMU_DRIVER_SHIFT_BIT_POSITION_BY_08_BITS)
               | (p_fifo_data[v_fifo_index +
               IMU_DRIVER_FIFO_Z_LSB_DATA]));
               v_accel_data_index += IMU_DRIVER_FIFO_INDEX_LENGTH;
               /** check for Accel frame count*/
               p_fifo_header_data->accel.frame_count += IMU_DRIVER_FRAME_COUNT;
               v_fifo_index += IMU_DRIVER_FIFO_A_LENGTH;
               break;
            }
            /** Header frame of gyro */
            case FIFO_HEAD_G:
            {	/**fifo data frame index + 1*/
                v_fifo_index += IMU_DRIVER_FIFO_INDEX_LENGTH;
                if ((v_fifo_index + IMU_DRIVER_FIFO_G_LENGTH) >  v_fifo_length)
                {
                    v_last_return_stat = FIFO_G_OVER_LEN;
                    break;
                }
                /** Gyro raw x data */
                p_fifo_header_data->gyro.p_imu_data[v_gyro_data_index] =
                (int16)(((p_fifo_data[v_fifo_index +
                IMU_DRIVER_FIFO_X_MSB_DATA])
                << IMU_DRIVER_SHIFT_BIT_POSITION_BY_08_BITS)
                | (p_fifo_data[v_fifo_index +
                IMU_DRIVER_FIFO_X_LSB_DATA]));
                v_gyro_data_index += IMU_DRIVER_FIFO_INDEX_LENGTH;
                /** Gyro raw y data */
                p_fifo_header_data->gyro.p_imu_data[v_gyro_data_index] =
                (int16)(((p_fifo_data[v_fifo_index +
                IMU_DRIVER_FIFO_Y_MSB_DATA])
                << IMU_DRIVER_SHIFT_BIT_POSITION_BY_08_BITS)
                | (p_fifo_data[v_fifo_index +
                IMU_DRIVER_FIFO_Y_LSB_DATA]));
                v_gyro_data_index += IMU_DRIVER_FIFO_INDEX_LENGTH;
                /** Gyro raw z data */
                p_fifo_header_data->gyro.p_imu_data[v_gyro_data_index] =
                (int16)(((p_fifo_data[v_fifo_index +
                IMU_DRIVER_FIFO_Z_MSB_DATA])
                << IMU_DRIVER_SHIFT_BIT_POSITION_BY_08_BITS)
                | (p_fifo_data[v_fifo_index +
                IMU_DRIVER_FIFO_Z_LSB_DATA]));
                v_gyro_data_index += IMU_DRIVER_FIFO_INDEX_LENGTH;
                /** check for gyro frame count*/
                p_fifo_header_data->gyro.frame_count += IMU_DRIVER_FRAME_COUNT;
                /**fifo G data frame index + 6*/
                v_fifo_index += IMU_DRIVER_FIFO_G_LENGTH;
            break;
            }
            /** Header frame of gyro and Accel */
            case FIFO_HEAD_G_A:
                v_fifo_index += IMU_DRIVER_FIFO_INDEX_LENGTH;
                if ((v_fifo_index + IMU_DRIVER_FIFO_AG_LENGTH) > v_fifo_length)
                {
                    v_last_return_stat = FIFO_G_A_OVER_LEN;
                    break;
                }
                /** Raw gyro x */
                p_fifo_header_data->gyro.p_imu_data[v_gyro_data_index] =
                (int16)(((p_fifo_data[v_fifo_index +
                IMU_DRIVER_GA_FIFO_G_X_MSB])
                << IMU_DRIVER_SHIFT_BIT_POSITION_BY_08_BITS)
                |(p_fifo_data[v_fifo_index +
                IMU_DRIVER_GA_FIFO_G_X_LSB]));
                v_gyro_data_index += IMU_DRIVER_FIFO_INDEX_LENGTH;
                /** Raw gyro y */
                p_fifo_header_data->gyro.p_imu_data[v_gyro_data_index] =
                (int16)(((p_fifo_data[v_fifo_index +
                IMU_DRIVER_GA_FIFO_G_Y_MSB])
                << IMU_DRIVER_SHIFT_BIT_POSITION_BY_08_BITS)
                |(p_fifo_data[v_fifo_index +
                IMU_DRIVER_GA_FIFO_G_Y_LSB]));
                v_gyro_data_index += IMU_DRIVER_FIFO_INDEX_LENGTH;
                /** Raw gyro z */
                p_fifo_header_data->gyro.p_imu_data[v_gyro_data_index] =
                (int16)(((p_fifo_data[v_fifo_index +
                IMU_DRIVER_GA_FIFO_G_Z_MSB])
                << IMU_DRIVER_SHIFT_BIT_POSITION_BY_08_BITS)
                |(p_fifo_data[v_fifo_index +
                IMU_DRIVER_GA_FIFO_G_Z_LSB]));
                v_gyro_data_index += IMU_DRIVER_FIFO_INDEX_LENGTH;
                /** check for gyro frame count*/
                p_fifo_header_data->gyro.frame_count += IMU_DRIVER_FRAME_COUNT;
                /** Raw Accel x */
                p_fifo_header_data->accel.p_imu_data[v_accel_data_index] =
                (int16)(((p_fifo_data[v_fifo_index +
                IMU_DRIVER_GA_FIFO_A_X_MSB])
                << IMU_DRIVER_SHIFT_BIT_POSITION_BY_08_BITS)
                |(p_fifo_data[v_fifo_index +
                IMU_DRIVER_GA_FIFO_A_X_LSB]));
                v_accel_data_index += IMU_DRIVER_FIFO_INDEX_LENGTH;
                /** Raw Accel y */
                p_fifo_header_data->accel.p_imu_data[v_accel_data_index] =
                (int16)(((p_fifo_data[v_fifo_index +
                IMU_DRIVER_GA_FIFO_A_Y_MSB])
                << IMU_DRIVER_SHIFT_BIT_POSITION_BY_08_BITS)
                |(p_fifo_data[v_fifo_index +
                IMU_DRIVER_GA_FIFO_A_Y_LSB]));
                v_accel_data_index += IMU_DRIVER_FIFO_INDEX_LENGTH;
                /** Raw Accel z */
                p_fifo_header_data->accel.p_imu_data[v_accel_data_index] =
                (int16)(((p_fifo_data[v_fifo_index +
                IMU_DRIVER_GA_FIFO_A_Z_MSB])
                << IMU_DRIVER_SHIFT_BIT_POSITION_BY_08_BITS)
                |(p_fifo_data[v_fifo_index
                + IMU_DRIVER_GA_FIFO_A_Z_LSB]));
                v_accel_data_index += IMU_DRIVER_FIFO_INDEX_LENGTH;
                /** check for Accel frame count*/
                p_fifo_header_data->accel.frame_count += IMU_DRIVER_FRAME_COUNT;
                /** Index added to 12 for gyro and Accel*/
                v_fifo_index += IMU_DRIVER_FIFO_AG_LENGTH;
            break;
            /** Header frame of skip frame */
            case FIFO_HEAD_SKIP_FRAME:
                {
                /**fifo data frame index + 1*/
                    v_fifo_index = v_fifo_index +
                    IMU_DRIVER_FIFO_INDEX_LENGTH;
                    if (v_fifo_index
                    + IMU_DRIVER_FIFO_INDEX_LENGTH
                    > v_fifo_length) {
                        v_last_return_stat =
                        FIFO_SKIP_OVER_LEN;
                    break;
                    }
                    p_fifo_header_data->skip_frame += IMU_DRIVER_FIFO_INDEX_LENGTH;
                    v_fifo_index += IMU_DRIVER_FIFO_INDEX_LENGTH;
            break;
                }
            case FIFO_HEAD_INPUT_CONFIG:
                {
                /**fifo data frame index + 1*/
                    v_fifo_index = v_fifo_index +
                    IMU_DRIVER_FIFO_INDEX_LENGTH;
                    if (v_fifo_index
                    + IMU_DRIVER_FIFO_INDEX_LENGTH
                    > v_fifo_length) {
                        v_last_return_stat =
                        FIFO_INPUT_CONFIG_OVER_LEN;
                    break;
                    }
                    p_fifo_header_data->fifo_input_config_info
                    = p_fifo_data[v_fifo_index];
                    v_accel_data_index = IMU_DRIVER_INIT_VALUE;
                    p_fifo_header_data->accel.frame_count = 0;
            break;
                }
            /** Header frame of over read FIFO data */
            case FIFO_HEAD_OVER_READ_LSB:
                {
                /**fifo data frame index + 1*/
                v_fifo_index = v_fifo_index +
                IMU_DRIVER_FIFO_INDEX_LENGTH;

                if ((v_fifo_index + IMU_DRIVER_FIFO_INDEX_LENGTH)
                > (v_fifo_length))
                {
                    v_last_return_stat = FIFO_OVER_READ_RETURN;
                    break;
                }
                if (p_fifo_data[v_fifo_index] ==
                FIFO_HEAD_OVER_READ_MSB)
                {
                    /**fifo over read frame index + 1*/
                    v_fifo_index = v_fifo_index + IMU_DRIVER_FIFO_INDEX_LENGTH;
                    break;
                }
                else
                {
                    v_last_return_stat = FIFO_OVER_READ_RETURN;
                    break;
                }
                }
            default:
               /** v_last_return_stat = IMU_DRIVER_FIFO_INDEX_LENGTH;*/
                v_fifo_index += IMU_DRIVER_FIFO_INDEX_LENGTH;
            break;
            }

        if (v_last_return_stat != 0)
            break;
        }

        num_i2c_transfer--;
    }
    return com_rslt;
}
/*!
 *	@brief This API reads the Any motion interrupt which is mapped to
 *	interrupt1 and interrupt2 from the register 0x55 and 0x57
 *	@brief interrupt1 bit 2 in the register 0x55
 *	@brief interrupt2 bit 2 in the register 0x57
 *
 *
 *	@param v_channel: The value of any motion selection
 *   v_channel  |   interrupt
 *  ---------------|---------------
 *       0         | IMU_DRIVER_INTR1_MAP_ANY_MOTION
 *       1         | IMU_DRIVER_INTR2_MAP_ANY_MOTION
 *
 *	@param v_intr_any_motion : The value of any motion enable
 *	value    | interrupt enable
 * ----------|-------------------
 *  0x01     |  IMU_DRIVER_ENABLE
 *  0x00     |  IMU_DRIVER_DISABLE
 *
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_intr_any_motion(
uint8 v_channel, uint8 *v_intr_any_motion)
{
    /* variable used to return the status of communication result*/
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
    uint8 v_data = IMU_DRIVER_INIT_VALUE;
    /* check the p_imu structure for NULL pointer assignment*/
    if (p_imu == IMU_DRIVER_NULL) {
        return E_IMU_DRIVER_NULL_PTR;
        } else {
        switch (v_channel) {
        /* read the any motion interrupt */
        case IMU_DRIVER_INTR1_MAP_ANY_MOTION:
            com_rslt = p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->
            dev_addr, IMU_DRIVER_USER_INTR_MAP_0_INTR1_ANY_MOTION__REG,
            &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            *v_intr_any_motion = IMU_DRIVER_GET_BITSLICE(v_data,
            IMU_DRIVER_USER_INTR_MAP_0_INTR1_ANY_MOTION);
        break;
        case IMU_DRIVER_INTR2_MAP_ANY_MOTION:
            com_rslt = p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->
            dev_addr, IMU_DRIVER_USER_INTR_MAP_2_INTR2_ANY_MOTION__REG,
            &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            *v_intr_any_motion = IMU_DRIVER_GET_BITSLICE(v_data,
            IMU_DRIVER_USER_INTR_MAP_2_INTR2_ANY_MOTION);
        break;
        default:
            com_rslt = E_IMU_DRIVER_OUT_OF_RANGE;
        break;
        }
    }
    return com_rslt;
}
/*!
 *	@brief This API writes the Any motion interrupt
 *	to be mapped to interrupt1
 *	and interrupt2 from the register 0x55 and 0x57
 *	@brief interrupt1 bit 2 in the register 0x55
 *	@brief interrupt2 bit 2 in the register 0x57
 *
 *
 *	@param v_channel: The value of any motion selection
 *   v_channel  |   interrupt
 *  ---------------|---------------
 *       0         | IMU_DRIVER_INTR1_MAP_ANY_MOTION
 *       1         | IMU_DRIVER_INTR2_MAP_ANY_MOTION
 *
 *	@param v_intr_any_motion : The value of any motion enable
 *	value    | interrupt enable
 * ----------|-------------------
 *  0x01     |  IMU_DRIVER_ENABLE
 *  0x00     |  IMU_DRIVER_DISABLE
 *
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_intr_any_motion(
uint8 v_channel, uint8 v_intr_any_motion)
{
/* variable used to return the status of communication result*/
IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
uint8 v_data = IMU_DRIVER_INIT_VALUE;
/* check the p_imu structure for NULL pointer assignment*/
if (p_imu == IMU_DRIVER_NULL) {
    return E_IMU_DRIVER_NULL_PTR;
    } else {
    switch (v_channel) {
    /* write the any motion interrupt */
    case IMU_DRIVER_INTR1_MAP_ANY_MOTION:
        com_rslt = p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->
        dev_addr, IMU_DRIVER_USER_INTR_MAP_0_INTR1_ANY_MOTION__REG,
        &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == DRIVER_SUCCESS) {
            v_data = IMU_DRIVER_SET_BITSLICE(v_data,
            IMU_DRIVER_USER_INTR_MAP_0_INTR1_ANY_MOTION,
            v_intr_any_motion);
            com_rslt += p_imu->IMU_DRIVER_BUS_WRITE_FUNC(p_imu->
            dev_addr, IMU_DRIVER_USER_INTR_MAP_0_INTR1_ANY_MOTION__REG,
            &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
        }
    break;
    case IMU_DRIVER_INTR2_MAP_ANY_MOTION:
        com_rslt = p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->
        dev_addr, IMU_DRIVER_USER_INTR_MAP_2_INTR2_ANY_MOTION__REG,
        &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == DRIVER_SUCCESS) {
            v_data = IMU_DRIVER_SET_BITSLICE(v_data,
            IMU_DRIVER_USER_INTR_MAP_2_INTR2_ANY_MOTION,
            v_intr_any_motion);
            com_rslt += p_imu->IMU_DRIVER_BUS_WRITE_FUNC(p_imu->
            dev_addr, IMU_DRIVER_USER_INTR_MAP_2_INTR2_ANY_MOTION__REG,
            &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
        }
    break;
    default:
        com_rslt = E_IMU_DRIVER_OUT_OF_RANGE;
    break;
    }
    /*Accel and Gyro power mode check */
    if (imu_power_mode_status_g != IMU_DRIVER_NORMAL_MODE)
        /*interface idle time delay */
        p_imu->delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
}
return com_rslt;
}
/*!
 *	@brief This API reads the No motion interrupt
 *	which is  mapped to interrupt1
 *	and interrupt2 from the register 0x55 and 0x57
 *	@brief interrupt1 bit 3 in the register 0x55
 *	@brief interrupt2 bit 3 in the register 0x57
 *
 *
 *	@param v_channel: The value of no motion selection
 *   v_channel  |   interrupt
 *  ---------------|---------------
 *       0         | IMU_DRIVER_INTR1_MAP_NOMO
 *       1         | IMU_DRIVER_INTR2_MAP_NOMO
 *
 *	@param v_intr_nomotion : The value of no motion enable
 *	value    | interrupt enable
 * ----------|-------------------
 *  0x01     |  IMU_DRIVER_ENABLE
 *  0x00     |  IMU_DRIVER_DISABLE
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_intr_nomotion(
uint8 v_channel, uint8 *v_intr_nomotion)
{
    /* variable used to return the status of communication result*/
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
    uint8 v_data = IMU_DRIVER_INIT_VALUE;
    /* check the p_imu structure for NULL pointer assignment*/
    if (p_imu == IMU_DRIVER_NULL) {
        return E_IMU_DRIVER_NULL_PTR;
        } else {
        switch (v_channel) {
        /* read the no motion interrupt*/
        case IMU_DRIVER_INTR1_MAP_NOMO:
            com_rslt = p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->
            dev_addr, IMU_DRIVER_USER_INTR_MAP_0_INTR1_NOMOTION__REG,
            &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            *v_intr_nomotion = IMU_DRIVER_GET_BITSLICE(v_data,
            IMU_DRIVER_USER_INTR_MAP_0_INTR1_NOMOTION);
            break;
        case IMU_DRIVER_INTR2_MAP_NOMO:
            com_rslt = p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->
            dev_addr, IMU_DRIVER_USER_INTR_MAP_2_INTR2_NOMOTION__REG,
            &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            *v_intr_nomotion = IMU_DRIVER_GET_BITSLICE(v_data,
            IMU_DRIVER_USER_INTR_MAP_2_INTR2_NOMOTION);
            break;
        default:
            com_rslt = E_IMU_DRIVER_OUT_OF_RANGE;
            break;
        }
    }
    return com_rslt;
}
/*!
 *	@brief This API configures the No motion interrupt
 *	to be mapped to interrupt1
 *	and interrupt2 from the register 0x55 and 0x57
 *	@brief interrupt1 bit 3 in the register 0x55
 *	@brief interrupt2 bit 3 in the register 0x57
 *
 *
 *	@param v_channel: The value of no motion selection
 *   v_channel  |   interrupt
 *  ---------------|---------------
 *       0         | IMU_DRIVER_INTR1_MAP_NOMO
 *       1         | IMU_DRIVER_INTR2_MAP_NOMO
 *
 *	@param v_intr_nomotion : The value of no motion enable
 *	value    | interrupt enable
 * ----------|-------------------
 *  0x01     |  IMU_DRIVER_ENABLE
 *  0x00     |  IMU_DRIVER_DISABLE
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_intr_nomotion(
uint8 v_channel, uint8 v_intr_nomotion)
{
/* variable used to return the status of communication result*/
IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
uint8 v_data = IMU_DRIVER_INIT_VALUE;
/* check the p_imu structure for NULL pointer assignment*/
if (p_imu == IMU_DRIVER_NULL) {
    return E_IMU_DRIVER_NULL_PTR;
    } else {
    switch (v_channel) {
    /* write the no motion interrupt*/
    case IMU_DRIVER_INTR1_MAP_NOMO:
        com_rslt = p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->
        dev_addr, IMU_DRIVER_USER_INTR_MAP_0_INTR1_NOMOTION__REG,
        &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == DRIVER_SUCCESS) {
            v_data = IMU_DRIVER_SET_BITSLICE(v_data,
            IMU_DRIVER_USER_INTR_MAP_0_INTR1_NOMOTION,
            v_intr_nomotion);
            com_rslt += p_imu->IMU_DRIVER_BUS_WRITE_FUNC(p_imu->
            dev_addr, IMU_DRIVER_USER_INTR_MAP_0_INTR1_NOMOTION__REG,
            &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
        }
        break;
    case IMU_DRIVER_INTR2_MAP_NOMO:
        com_rslt = p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->
        dev_addr, IMU_DRIVER_USER_INTR_MAP_2_INTR2_NOMOTION__REG,
        &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == DRIVER_SUCCESS) {
            v_data = IMU_DRIVER_SET_BITSLICE(v_data,
            IMU_DRIVER_USER_INTR_MAP_2_INTR2_NOMOTION,
            v_intr_nomotion);
            com_rslt += p_imu->IMU_DRIVER_BUS_WRITE_FUNC(p_imu->
            dev_addr, IMU_DRIVER_USER_INTR_MAP_2_INTR2_NOMOTION__REG,
            &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
        }
        break;
    default:
        com_rslt = E_IMU_DRIVER_OUT_OF_RANGE;
        break;
    }
    /*Accel and Gyro power mode check */
    if (imu_power_mode_status_g != IMU_DRIVER_NORMAL_MODE)
        /*interface idle time delay */
        p_imu->delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
}
return com_rslt;
}
/*!
 *	@brief This API reads any motion duration
 *	from the register 0x5F bit 0 and 1
 *
 *  @param v_any_motion_durn : The value of any motion duration
 *
 *	@note Any motion duration can be calculated by "v_any_motion_durn + 1"
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_intr_any_motion_durn(
uint8 *v_any_motion_durn)
{
    /* variable used to return the status of communication result*/
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
    uint8 v_data = IMU_DRIVER_INIT_VALUE;
    /* check the p_imu structure for NULL pointer assignment*/
    if (p_imu == IMU_DRIVER_NULL) {
        return E_IMU_DRIVER_NULL_PTR;
        } else {
        /* read any motion duration*/
        com_rslt = p_imu->IMU_DRIVER_BUS_READ_FUNC
        (p_imu->dev_addr,
        IMU_DRIVER_USER_INTR_MOTION_0_INTR_ANY_MOTION_DURN__REG,
        &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
        *v_any_motion_durn = IMU_DRIVER_GET_BITSLICE
        (v_data,
        IMU_DRIVER_USER_INTR_MOTION_0_INTR_ANY_MOTION_DURN);
    }
    return com_rslt;
}
/*!
 *	@brief This API writes any motion duration
 *	from the register 0x5F bit 0 and 1
 *
 *  @param v_any_motion_durn : The value of any motion duration
 *
 *	@note Any motion duration can be calculated by "v_any_motion_durn + 1"
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_intr_any_motion_durn(
uint8 v_any_motion_durn)
{
    /* variable used to return the status of communication result*/
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
    uint8 v_data = IMU_DRIVER_INIT_VALUE;
    /* check the p_imu structure for NULL pointer assignment*/
    if (p_imu == IMU_DRIVER_NULL) {
        return E_IMU_DRIVER_NULL_PTR;
        } else {
        /* write any motion duration*/
        com_rslt = p_imu->IMU_DRIVER_BUS_READ_FUNC
        (p_imu->dev_addr,
        IMU_DRIVER_USER_INTR_MOTION_0_INTR_ANY_MOTION_DURN__REG,
        &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == DRIVER_SUCCESS) {
            v_data = IMU_DRIVER_SET_BITSLICE(v_data,
            IMU_DRIVER_USER_INTR_MOTION_0_INTR_ANY_MOTION_DURN,
            v_any_motion_durn);
            com_rslt += p_imu->IMU_DRIVER_BUS_WRITE_FUNC
            (p_imu->dev_addr,
            IMU_DRIVER_USER_INTR_MOTION_0_INTR_ANY_MOTION_DURN__REG,
            &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);

            /*Check for the power mode of Accel and
            gyro not in normal mode */
            if (imu_power_mode_status_g !=
            IMU_DRIVER_NORMAL_MODE)
                /*interface idle time delay */
                p_imu->delay_msec(
                IMU_DRIVER_GEN_READ_WRITE_DELAY);
        }
    }
    return com_rslt;
}
 /*!
 *	@brief This API reads Slow/no-motion
 *	interrupt trigger delay duration from the register 0x5F bit 2 to 7
 *
 *  @param v_slow_no_motion :The value of slow no motion duration
 *
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *	@note
 *	@note v_slow_no_motion(5:4)=0b00 ->
 *	[v_slow_no_motion(3:0) + 1] * 1.28s (1.28s-20.48s)
 *	@note v_slow_no_motion(5:4)=1 ->
 *	[v_slow_no_motion(3:0)+5] * 5.12s (25.6s-102.4s)
 *	@note v_slow_no_motion(5)='1' ->
 *	[(v_slow_no_motion:0)+11] * 10.24s (112.64s-430.08s);
 *
 */
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_intr_slow_no_motion_durn(
uint8 *v_slow_no_motion)
{
/* variable used to return the status of communication result*/
IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
uint8 v_data = IMU_DRIVER_INIT_VALUE;
/* check the p_imu structure for NULL pointer assignment*/
if (p_imu == IMU_DRIVER_NULL) {
    return E_IMU_DRIVER_NULL_PTR;
    } else {
        /* read slow no motion duration*/
        com_rslt = p_imu->IMU_DRIVER_BUS_READ_FUNC
        (p_imu->dev_addr,
        IMU_DRIVER_USER_INTR_MOTION_0_INTR_SLOW_NO_MOTION_DURN__REG,
        &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
        *v_slow_no_motion = IMU_DRIVER_GET_BITSLICE
        (v_data,
        IMU_DRIVER_USER_INTR_MOTION_0_INTR_SLOW_NO_MOTION_DURN);
    }
return com_rslt;
}
 /*!
 *	@brief This API writes Slow/no-motion
 *	interrupt trigger delay duration from the register 0x5F bit 2 to 7
 *
 *  @param v_slow_no_motion :The value of slow no motion duration
 *
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *	@note
 *	@note v_slow_no_motion(5:4)=0b00 ->
 *	[v_slow_no_motion(3:0) + 1] * 1.28s (1.28s-20.48s)
 *	@note v_slow_no_motion(5:4)=1 ->
 *	[v_slow_no_motion(3:0)+5] * 5.12s (25.6s-102.4s)
 *	@note v_slow_no_motion(5)='1' ->
 *	[(v_slow_no_motion:0)+11] * 10.24s (112.64s-430.08s);
 *
 */
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_intr_slow_no_motion_durn(
uint8 v_slow_no_motion)
{
/* variable used to return the status of communication result*/
IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
uint8 v_data = IMU_DRIVER_INIT_VALUE;
/* check the p_imu structure for NULL pointer assignment*/
if (p_imu == IMU_DRIVER_NULL) {
    return E_IMU_DRIVER_NULL_PTR;
    } else {
    /* write slow no motion duration*/
    com_rslt = p_imu->IMU_DRIVER_BUS_READ_FUNC
    (p_imu->dev_addr,
    IMU_DRIVER_USER_INTR_MOTION_0_INTR_SLOW_NO_MOTION_DURN__REG,
    &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
    if (com_rslt == DRIVER_SUCCESS) {
        v_data = IMU_DRIVER_SET_BITSLICE
        (v_data,
        IMU_DRIVER_USER_INTR_MOTION_0_INTR_SLOW_NO_MOTION_DURN,
        v_slow_no_motion);
        com_rslt += p_imu->IMU_DRIVER_BUS_WRITE_FUNC
        (p_imu->dev_addr,
        IMU_DRIVER_USER_INTR_MOTION_0_INTR_SLOW_NO_MOTION_DURN__REG,
        &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);

        /*Check for the power mode of Accel and
        gyro not in normal mode */
        if (imu_power_mode_status_g != IMU_DRIVER_NORMAL_MODE)
            /*interface idle time delay */
            p_imu->delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);

    }
}
return com_rslt;
}
/*!
 *	@brief This API is used to read threshold
 *	definition for the any-motion interrupt
 *	from the register 0x60 bit 0 to 7
 *
 *
 *  @param  v_any_motion_thres : The value of any motion threshold
 *
 *	@note any motion threshold changes according to Accel g range
 *	Accel g range can be set by the function "imu_set_accel_range"
 *   accel_range    | any motion threshold
 *  ----------------|---------------------
 *      2g          |  v_any_motion_thres*3.91 mg
 *      4g          |  v_any_motion_thres*7.81 mg
 *      8g          |  v_any_motion_thres*15.63 mg
 *      16g         |  v_any_motion_thres*31.25 mg
 *	@note when v_any_motion_thres = 0
 *   accel_range    | any motion threshold
 *  ----------------|---------------------
 *      2g          |  1.95 mg
 *      4g          |  3.91 mg
 *      8g          |  7.81 mg
 *      16g         |  15.63 mg
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_intr_any_motion_thres(
uint8 *v_any_motion_thres)
{
    /* variable used to return the status of communication result*/
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
    uint8 v_data = IMU_DRIVER_INIT_VALUE;
    /* check the p_imu structure for NULL pointer assignment*/
    if (p_imu == IMU_DRIVER_NULL) {
        return E_IMU_DRIVER_NULL_PTR;
        } else {
            /* read any motion threshold*/
            com_rslt = p_imu->IMU_DRIVER_BUS_READ_FUNC
            (p_imu->dev_addr,
            IMU_DRIVER_USER_INTR_MOTION_1_INTR_ANY_MOTION_THRES__REG,
            &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            *v_any_motion_thres =
            IMU_DRIVER_GET_BITSLICE(v_data,
            IMU_DRIVER_USER_INTR_MOTION_1_INTR_ANY_MOTION_THRES);
        }
    return com_rslt;
}
/*!
 *	@brief This API is used to write threshold
 *	definition for  any-motion interrupt
 *	from the register 0x60 bit 0 to 7
 *
 *
 *  @param  v_any_motion_thres : The value of any motion threshold
 *
 *	@note any motion threshold changes according to Accel g range
 *	Accel g range can be set by the function "imu_set_accel_range"
 *   accel_range    | any motion threshold
 *  ----------------|---------------------
 *      2g          |  v_any_motion_thres*3.91 mg
 *      4g          |  v_any_motion_thres*7.81 mg
 *      8g          |  v_any_motion_thres*15.63 mg
 *      16g         |  v_any_motion_thres*31.25 mg
 *	@note when v_any_motion_thres = 0
 *   accel_range    | any motion threshold
 *  ----------------|---------------------
 *      2g          |  1.95 mg
 *      4g          |  3.91 mg
 *      8g          |  7.81 mg
 *      16g         |  15.63 mg
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_intr_any_motion_thres(
uint8 v_any_motion_thres)
{
    /* variable used to return the status of communication result*/
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
    /* check the p_imu structure for NULL pointer assignment*/
    if (p_imu == IMU_DRIVER_NULL) {
        return E_IMU_DRIVER_NULL_PTR;
        } else {
        /* write any motion threshold*/
        com_rslt = p_imu->IMU_DRIVER_BUS_WRITE_FUNC
        (p_imu->dev_addr,
        IMU_DRIVER_USER_INTR_MOTION_1_INTR_ANY_MOTION_THRES__REG,
        &v_any_motion_thres, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);

        /*Check for the power mode of Accel and
        gyro not in normal mode */
        if (imu_power_mode_status_g != IMU_DRIVER_NORMAL_MODE)
            /*interface idle time delay */
            p_imu->delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
    }
    return com_rslt;
}
 /*!
 *	@brief This API is used to read threshold
 *	for the slow/no-motion interrupt
 *	from the register 0x61 bit 0 to 7
 *
 *
 *
 *
 *  @param v_slow_no_motion_thres : The value of slow no motion threshold
 *	@note slow no motion threshold changes according to Accel g range
 *	Accel g range can be set by the function "imu_set_accel_range"
 *   accel_range    | slow no motion threshold
 *  ----------------|---------------------
 *      2g          |  v_slow_no_motion_thres*3.91 mg
 *      4g          |  v_slow_no_motion_thres*7.81 mg
 *      8g          |  v_slow_no_motion_thres*15.63 mg
 *      16g         |  v_slow_no_motion_thres*31.25 mg
 *	@note when v_slow_no_motion_thres = 0
 *   accel_range    | slow no motion threshold
 *  ----------------|---------------------
 *      2g          |  1.95 mg
 *      4g          |  3.91 mg
 *      8g          |  7.81 mg
 *      16g         |  15.63 mg
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_intr_slow_no_motion_thres(
uint8 *v_slow_no_motion_thres)
{
IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt  = E_IMU_DRIVER_COMM_RES;
uint8 v_data = IMU_DRIVER_INIT_VALUE;
/* check the p_imu structure for NULL pointer assignment*/
if (p_imu == IMU_DRIVER_NULL) {
    return E_IMU_DRIVER_NULL_PTR;
    } else {
        /* read slow no motion threshold*/
        com_rslt = p_imu->IMU_DRIVER_BUS_READ_FUNC
        (p_imu->dev_addr,
        IMU_DRIVER_USER_INTR_MOTION_2_INTR_SLOW_NO_MOTION_THRES__REG,
        &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
        *v_slow_no_motion_thres =
        IMU_DRIVER_GET_BITSLICE(v_data,
        IMU_DRIVER_USER_INTR_MOTION_2_INTR_SLOW_NO_MOTION_THRES);
    }
return com_rslt;
}
 /*!
 *	@brief This API is used to write threshold
 *	for the slow/no-motion interrupt
 *	in the register 0x61 bit 0 to 7
 *
 *
 *
 *
 *  @param v_slow_no_motion_thres : The value of slow no motion threshold
 *	@note slow no motion threshold changes according to Accel g range
 *	Accel g range can be set by the function "imu_set_accel_range"
 *   accel_range    | slow no motion threshold
 *  ----------------|---------------------
 *      2g          |  v_slow_no_motion_thres*3.91 mg
 *      4g          |  v_slow_no_motion_thres*7.81 mg
 *      8g          |  v_slow_no_motion_thres*15.63 mg
 *      16g         |  v_slow_no_motion_thres*31.25 mg
 *	@note when v_slow_no_motion_thres = 0
 *   accel_range    | slow no motion threshold
 *  ----------------|---------------------
 *      2g          |  1.95 mg
 *      4g          |  3.91 mg
 *      8g          |  7.81 mg
 *      16g         |  15.63 mg
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_intr_slow_no_motion_thres(
uint8 v_slow_no_motion_thres)
{
IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt  = E_IMU_DRIVER_COMM_RES;
/* check the p_imu structure for NULL pointer assignment*/
if (p_imu == IMU_DRIVER_NULL) {
    return E_IMU_DRIVER_NULL_PTR;
    } else {
        /* write slow no motion threshold*/
        com_rslt = p_imu->IMU_DRIVER_BUS_WRITE_FUNC(
        p_imu->dev_addr,
        IMU_DRIVER_USER_INTR_MOTION_2_INTR_SLOW_NO_MOTION_THRES__REG,
        &v_slow_no_motion_thres, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);

        /*Check for the power mode of Accel and
        gyro not in normal mode */
        if (imu_power_mode_status_g != IMU_DRIVER_NORMAL_MODE)
            /*interface idle time delay */
            p_imu->delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
    }
return com_rslt;
}
 /*!
 *	@brief This API is used to read
 *	the slow/no-motion selection from the register 0x62 bit 0
 *
 *
 *
 *
 *  @param  v_intr_slow_no_motion_select :
 *	The value of slow/no-motion select
 *	value    | Behaviour
 * ----------|-------------------
 *  0x00     |  SLOW_MOTION
 *  0x01     |  NO_MOTION
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_intr_slow_no_motion_select(
uint8 *v_intr_slow_no_motion_select)
{
IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt  = E_IMU_DRIVER_COMM_RES;
uint8 v_data = IMU_DRIVER_INIT_VALUE;
/* check the p_imu structure for NULL pointer assignment*/
if (p_imu == IMU_DRIVER_NULL) {
    return E_IMU_DRIVER_NULL_PTR;
    } else {
        /* read slow no motion select*/
        com_rslt = p_imu->IMU_DRIVER_BUS_READ_FUNC(
        p_imu->dev_addr,
        IMU_DRIVER_USER_INTR_MOTION_3_INTR_SLOW_NO_MOTION_SELECT__REG,
        &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
        *v_intr_slow_no_motion_select =
        IMU_DRIVER_GET_BITSLICE(v_data,
        IMU_DRIVER_USER_INTR_MOTION_3_INTR_SLOW_NO_MOTION_SELECT);
    }
return com_rslt;
}
 /*!
 *	@brief This API is used to write
 *	the slow/no-motion selection from the register 0x62 bit 0
 *
 *
 *
 *
 *  @param  v_intr_slow_no_motion_select :
 *	The value of slow/no-motion select
 *	value    | Behaviour
 * ----------|-------------------
 *  0x00     |  SLOW_MOTION
 *  0x01     |  NO_MOTION
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_intr_slow_no_motion_select(
uint8 v_intr_slow_no_motion_select)
{
/* variable used to return the status of communication result*/
IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
uint8 v_data = IMU_DRIVER_INIT_VALUE;
/* check the p_imu structure for NULL pointer assignment*/
if (p_imu == IMU_DRIVER_NULL) {
    return E_IMU_DRIVER_NULL_PTR;
} else {
if (v_intr_slow_no_motion_select <= IMU_DRIVER_MAX_VALUE_NO_MOTION) {
    /* write slow no motion select*/
    com_rslt = p_imu->IMU_DRIVER_BUS_READ_FUNC
    (p_imu->dev_addr,
    IMU_DRIVER_USER_INTR_MOTION_3_INTR_SLOW_NO_MOTION_SELECT__REG,
    &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
    if (com_rslt == DRIVER_SUCCESS) {
        v_data = IMU_DRIVER_SET_BITSLICE(v_data,
        IMU_DRIVER_USER_INTR_MOTION_3_INTR_SLOW_NO_MOTION_SELECT,
        v_intr_slow_no_motion_select);
        com_rslt += p_imu->IMU_DRIVER_BUS_WRITE_FUNC
        (p_imu->dev_addr,
        IMU_DRIVER_USER_INTR_MOTION_3_INTR_SLOW_NO_MOTION_SELECT__REG,
        &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);

        /*Check for the power mode of Accel and
        gyro not in normal mode */
        if (imu_power_mode_status_g != IMU_DRIVER_NORMAL_MODE)
            /*interface idle time delay */
            p_imu->delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
    }
} else {
com_rslt = E_IMU_DRIVER_OUT_OF_RANGE;
}
}
return com_rslt;
}
/*!
 *	@brief  This API is used to set
 *	interrupt enable from the register 0x50 bit 0 to 7
 *
 *
 *
 *
 *	@param v_enable : Value which selects the interrupt
 *   v_enable   |   interrupt
 *  ---------------|---------------
 *       0         | IMU_DRIVER_ANY_MOTION_X_ENABLE
 *       1         | IMU_DRIVER_ANY_MOTION_Y_ENABLE
 *       2         | IMU_DRIVER_ANY_MOTION_Z_ENABLE
 *
 *	@param v_intr_enable_zero : The interrupt enable value
 *	value    | interrupt enable
 * ----------|-------------------
 *  0x01     |  IMU_DRIVER_ENABLE
 *  0x00     |  IMU_DRIVER_DISABLE
 *
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
 */
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_intr_enable_0(
uint8 v_enable, uint8 v_intr_enable_zero)
{
/* variable used to return the status of communication result*/
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
uint8 v_data = IMU_DRIVER_INIT_VALUE;
/* check the p_IMU_DRIVER structure for NULL pointer assignment*/
if (p_imu == IMU_DRIVER_NULL) {
    return E_IMU_DRIVER_NULL_PTR;
    } else {
    switch (v_enable) {
    case IMU_DRIVER_ANY_MOTION_X_ENABLE:
        /* write any motion x*/
        com_rslt = p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->
        dev_addr, IMU_DRIVER_USER_INTR_ENABLE_0_ANY_MOTION_X_ENABLE__REG,
        &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == DRIVER_SUCCESS) {
            v_data = IMU_DRIVER_SET_BITSLICE(v_data,
            IMU_DRIVER_USER_INTR_ENABLE_0_ANY_MOTION_X_ENABLE,
            v_intr_enable_zero);
            com_rslt +=
            p_imu->IMU_DRIVER_BUS_WRITE_FUNC(p_imu->
            dev_addr,
            IMU_DRIVER_USER_INTR_ENABLE_0_ANY_MOTION_X_ENABLE__REG,
            &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
        }
        break;
    case IMU_DRIVER_ANY_MOTION_Y_ENABLE:
        /* write any motion y*/
        com_rslt = p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->
        dev_addr, IMU_DRIVER_USER_INTR_ENABLE_0_ANY_MOTION_Y_ENABLE__REG,
        &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == DRIVER_SUCCESS) {
            v_data = IMU_DRIVER_SET_BITSLICE(v_data,
            IMU_DRIVER_USER_INTR_ENABLE_0_ANY_MOTION_Y_ENABLE,
            v_intr_enable_zero);
            com_rslt +=
            p_imu->IMU_DRIVER_BUS_WRITE_FUNC(p_imu->
            dev_addr,
            IMU_DRIVER_USER_INTR_ENABLE_0_ANY_MOTION_Y_ENABLE__REG,
            &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
        }
        break;
    case IMU_DRIVER_ANY_MOTION_Z_ENABLE:
        /* write any motion z*/
        com_rslt = p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->
        dev_addr, IMU_DRIVER_USER_INTR_ENABLE_0_ANY_MOTION_Z_ENABLE__REG,
        &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == DRIVER_SUCCESS) {
            v_data = IMU_DRIVER_SET_BITSLICE(v_data,
            IMU_DRIVER_USER_INTR_ENABLE_0_ANY_MOTION_Z_ENABLE,
            v_intr_enable_zero);
            com_rslt +=
            p_imu->IMU_DRIVER_BUS_WRITE_FUNC(p_imu->
            dev_addr,
            IMU_DRIVER_USER_INTR_ENABLE_0_ANY_MOTION_Z_ENABLE__REG,
            &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
        }
        break;
    default:
        com_rslt = E_IMU_DRIVER_OUT_OF_RANGE;
        break;
    }
    /*Accel and Gyro power mode check */
    if (imu_power_mode_status_g != IMU_DRIVER_NORMAL_MODE)
            /*interface idle time delay */
        p_imu->delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
}
return com_rslt;
}
/*!
 *	@brief  This API is used to set
 *	interrupt enable byte2 from the register bit 0x52 bit 0 to 3
 *	@brief It reads no motion x,y and z
 *
 *
 *
 *	@param v_enable: The value of interrupt enable
 *   v_enable   |   interrupt
 *  ---------------|---------------
 *       0         | IMU_DRIVER_NOMOTION_X_ENABLE
 *       1         | IMU_DRIVER_NOMOTION_Y_ENABLE
 *       2         | IMU_DRIVER_NOMOTION_Z_ENABLE
 *
 *	@param v_intr_enable_2 : The interrupt enable value
 *	value    | interrupt enable
 * ----------|-------------------
 *  0x01     |  IMU_DRIVER_ENABLE
 *  0x00     |  IMU_DRIVER_DISABLE
 *
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
 */
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_intr_enable_2(
uint8 v_enable, uint8 v_intr_enable_2)
{
/* variable used to return the status of communication result*/
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
uint8 v_data = IMU_DRIVER_INIT_VALUE;
/* check the p_imu structure for NULL pointer assignment*/
if (p_imu == IMU_DRIVER_NULL) {
    return E_IMU_DRIVER_NULL_PTR;
    } else {
    switch (v_enable) {
    case IMU_DRIVER_NOMOTION_X_ENABLE:
        /* write no motion x */
        com_rslt = p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->
        dev_addr,
        IMU_DRIVER_USER_INTR_ENABLE_2_NOMOTION_X_ENABLE__REG,
        &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == DRIVER_SUCCESS) {
            v_data = IMU_DRIVER_SET_BITSLICE(v_data,
            IMU_DRIVER_USER_INTR_ENABLE_2_NOMOTION_X_ENABLE,
            v_intr_enable_2);
            com_rslt +=
            p_imu->IMU_DRIVER_BUS_WRITE_FUNC(p_imu->
            dev_addr,
            IMU_DRIVER_USER_INTR_ENABLE_2_NOMOTION_X_ENABLE__REG,
            &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
        }
        break;
    case IMU_DRIVER_NOMOTION_Y_ENABLE:
        /* write no motion y */
        com_rslt = p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->
        dev_addr,
        IMU_DRIVER_USER_INTR_ENABLE_2_NOMOTION_Y_ENABLE__REG,
        &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == DRIVER_SUCCESS) {
            v_data = IMU_DRIVER_SET_BITSLICE(v_data,
            IMU_DRIVER_USER_INTR_ENABLE_2_NOMOTION_Y_ENABLE,
            v_intr_enable_2);
            com_rslt +=
            p_imu->IMU_DRIVER_BUS_WRITE_FUNC(p_imu->
            dev_addr,
            IMU_DRIVER_USER_INTR_ENABLE_2_NOMOTION_Y_ENABLE__REG,
            &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
        }
        break;
    case IMU_DRIVER_NOMOTION_Z_ENABLE:
        /* write no motion z */
        com_rslt = p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->
        dev_addr,
        IMU_DRIVER_USER_INTR_ENABLE_2_NOMOTION_Z_ENABLE__REG,
        &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
        if (com_rslt == DRIVER_SUCCESS) {
            v_data = IMU_DRIVER_SET_BITSLICE(v_data,
            IMU_DRIVER_USER_INTR_ENABLE_2_NOMOTION_Z_ENABLE,
            v_intr_enable_2);
            com_rslt +=
            p_imu->IMU_DRIVER_BUS_WRITE_FUNC(p_imu->
            dev_addr,
            IMU_DRIVER_USER_INTR_ENABLE_2_NOMOTION_Z_ENABLE__REG,
            &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
        }
        break;
    default:
        com_rslt = E_IMU_DRIVER_OUT_OF_RANGE;
        break;
    }
    /*Accel and Gyro power mode check*/
    if (imu_power_mode_status_g != IMU_DRIVER_NORMAL_MODE)
        /*interface idle time delay */
        p_imu->delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
}
return com_rslt;
}
/*!
*	@brief This API reads the any motion interrupt status
*	from the register 0x1C bit 2
*	flag is associated with a specific interrupt function.
*	It is set when the single tab interrupt triggers. The
*	setting of INT_LATCH controls the interrupt
*	signal and hence the
*	respective interrupt flag will be
*	permanently latched, temporarily latched
*	or not latched.
*
*  @param v_any_motion_intr : The status of any-motion interrupt
*
*	@return results of bus communication function
*	@retval 0 -> Success
*	@retval -1 -> Error
*
*
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_stat0_any_motion_intr(uint8 *v_any_motion_intr)
{
   /* variable used to return the status of communication result*/
   IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
   uint8 v_data = IMU_DRIVER_INIT_VALUE;
   /* check the p_imu structure for NULL pointer assignment*/
   if (p_imu == IMU_DRIVER_NULL) {
       return E_IMU_DRIVER_NULL_PTR;
       } else {
           com_rslt =
           p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->dev_addr,
           IMU_DRIVER_USER_INTR_STAT_0_ANY_MOTION__REG, &v_data,
           IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
           *v_any_motion_intr = IMU_DRIVER_GET_BITSLICE(v_data,
           IMU_DRIVER_USER_INTR_STAT_0_ANY_MOTION);
       }
   return com_rslt;
}
/*!
 *	@brief This API reads data ready no motion interrupt status
 *	from the register 0x1D bit 7
 *	flag is associated with a specific interrupt function.
 *	It is set when the no motion  interrupt triggers. The
 *	setting of INT_LATCH controls the interrupt signal and hence the
 *	respective interrupt flag will be permanently
 *	latched, temporarily latched
 *	or not latched.
 *
 *  @param v_nomotion_intr : The status of no motion interrupt
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_stat1_nomotion_intr(uint8 *v_nomotion_intr)
{
    /* variable used to return the status of communication result*/
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
    uint8 v_data = IMU_DRIVER_INIT_VALUE;
    /* check the p_imu structure for NULL pointer assignment*/
    if (p_imu == IMU_DRIVER_NULL) {
        return E_IMU_DRIVER_NULL_PTR;
        } else {
            /* read the no motion interrupt*/
            com_rslt =
            p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->dev_addr,
            IMU_DRIVER_USER_INTR_STAT_1_NOMOTION_INTR__REG,
            &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            *v_nomotion_intr =
            IMU_DRIVER_GET_BITSLICE(v_data,
            IMU_DRIVER_USER_INTR_STAT_1_NOMOTION_INTR);
        }
    return com_rslt;
}
/*!
 *	@brief This API is used to set the Output enable for interrupt1
 *	and interrupt2 pin from the register 0x53
 *	@brief interrupt1 - bit 3
 *	@brief interrupt2 - bit 7
 *
 *  @param v_channel: The value of output enable selection
 *   v_channel  |   level selection
 *  ---------------|---------------
 *       0         | IMU_DRIVER_INTR1_OUTPUT_ENABLE
 *       1         | IMU_DRIVER_INTR2_OUTPUT_ENABLE
 *
 *	@param v_output_enable :
 *	The value of output enable of interrupt enable
 *	value    | Behaviour
 * ----------|-------------------
 *  0x01     |  INTERRUPT OUTPUT ENABLED
 *  0x00     |  INTERRUPT OUTPUT DISABLED
 *
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_output_enable(
uint8 v_channel, uint8 v_output_enable)
{
    /* variable used to return the status of communication result*/
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
    uint8 v_data = IMU_DRIVER_INIT_VALUE;
    /* check the p_imu structure for NULL pointer assignment*/
    if (p_imu == IMU_DRIVER_NULL) {
        return E_IMU_DRIVER_NULL_PTR;
        } else {
        switch (v_channel) {
        case IMU_DRIVER_INTR1_OUTPUT_ENABLE:
            /* write the output enable of interrupt1*/
            com_rslt = p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->
            dev_addr, IMU_DRIVER_USER_INTR1_OUTPUT_ENABLE__REG,
            &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            if (com_rslt == DRIVER_SUCCESS) {
                v_data = IMU_DRIVER_SET_BITSLICE(v_data,
                IMU_DRIVER_USER_INTR1_OUTPUT_ENABLE,
                v_output_enable);
                com_rslt +=
                p_imu->IMU_DRIVER_BUS_WRITE_FUNC(p_imu->
                dev_addr, IMU_DRIVER_USER_INTR1_OUTPUT_ENABLE__REG,
                &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            }
        break;
        case IMU_DRIVER_INTR2_OUTPUT_ENABLE:
            /* write the output enable of interrupt2*/
            com_rslt = p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->
            dev_addr, IMU_DRIVER_USER_INTR2_OUTPUT_EN__REG,
            &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            if (com_rslt == DRIVER_SUCCESS) {
                v_data = IMU_DRIVER_SET_BITSLICE(v_data,
                IMU_DRIVER_USER_INTR2_OUTPUT_EN,
                v_output_enable);
                com_rslt +=
                p_imu->IMU_DRIVER_BUS_WRITE_FUNC(p_imu->
                dev_addr, IMU_DRIVER_USER_INTR2_OUTPUT_EN__REG,
                &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            }
        break;
        default:
            com_rslt = E_IMU_DRIVER_OUT_OF_RANGE;
        break;
        }
        /*Accel and Gyro power mode check*/
        if (imu_power_mode_status_g != IMU_DRIVER_NORMAL_MODE)
            /*interface idle time delay */
            p_imu->delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
    }
    return com_rslt;
}
/*!
 *	@brief  This API is used to set the configure level condition of
 *	interrupt1 and interrupt2 pin from the register 0x53
 *	@brief interrupt1 - bit 1
 *	@brief interrupt2 - bit 5
 *
 *  @param v_channel: The value of level condition selection
 *   v_channel  |   level selection
 *  ---------------|---------------
 *       0         | IMU_DRIVER_INTR1_LEVEL
 *       1         | IMU_DRIVER_INTR2_LEVEL
 *
 *	@param v_intr_level : The value of level of interrupt enable
 *	value    | Behaviour
 * ----------|-------------------
 *  0x01     |  IMU_DRIVER_LEVEL_HIGH
 *  0x00     |  IMU_DRIVER_LEVEL_LOW
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_intr_level(
uint8 v_channel, uint8 v_intr_level)
{
    /* variable used to return the status of communication result*/
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
    uint8 v_data = IMU_DRIVER_INIT_VALUE;
    /* check the p_IMU_DRIVER structure for NULL pointer assignment*/
    if (p_imu == IMU_DRIVER_NULL) {
        return E_IMU_DRIVER_NULL_PTR;
        } else {
        switch (v_channel) {
        case IMU_DRIVER_INTR1_LEVEL:
            /* write the interrupt1 level*/
            com_rslt = p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->
            dev_addr, IMU_DRIVER_USER_INTR1_LEVEL__REG,
            &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            if (com_rslt == DRIVER_SUCCESS) {
                v_data = IMU_DRIVER_SET_BITSLICE(v_data,
                IMU_DRIVER_USER_INTR1_LEVEL, v_intr_level);
                com_rslt +=
                p_imu->IMU_DRIVER_BUS_WRITE_FUNC(p_imu->
                dev_addr, IMU_DRIVER_USER_INTR1_LEVEL__REG,
                &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            }
            break;
        case IMU_DRIVER_INTR2_LEVEL:
            /* write the interrupt2 level*/
            com_rslt = p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->
            dev_addr, IMU_DRIVER_USER_INTR2_LEVEL__REG,
            &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            if (com_rslt == DRIVER_SUCCESS) {
                v_data = IMU_DRIVER_SET_BITSLICE(v_data,
                IMU_DRIVER_USER_INTR2_LEVEL, v_intr_level);
                com_rslt +=
                p_imu->IMU_DRIVER_BUS_WRITE_FUNC(p_imu->
                dev_addr, IMU_DRIVER_USER_INTR2_LEVEL__REG,
                &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            }
            break;
        default:
            com_rslt = E_IMU_DRIVER_OUT_OF_RANGE;
            break;
        }
        /*Accel and Gyro power mode check*/
        if (imu_power_mode_status_g != IMU_DRIVER_NORMAL_MODE)
            /*interface idle time delay */
            p_imu->delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
    }
    return com_rslt;
}
/*!
 *	@brief  This API configures trigger condition of interrupt1
 *	and interrupt2 pin from the register 0x53
 *	@brief interrupt1 - bit 0
 *	@brief interrupt2 - bit 4
 *
 *  @param v_channel: The value of edge trigger selection
 *   v_channel  |   Edge trigger
 *  ---------------|---------------
 *       0         | IMU_DRIVER_INTR1_EDGE_CTRL
 *       1         | IMU_DRIVER_INTR2_EDGE_CTRL
 *
 *	@param v_intr_edge_ctrl : The value of edge trigger enable
 *	value    | interrupt enable
 * ----------|-------------------
 *  0x01     |  IMU_DRIVER_EDGE
 *  0x00     |  IMU_DRIVER_LEVEL
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_intr_edge_ctrl(
uint8 v_channel, uint8 v_intr_edge_ctrl)
{
    /* variable used to return the status of communication result*/
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
    uint8 v_data = IMU_DRIVER_INIT_VALUE;
    /* check the p_imu structure for NULL pointer assignment*/
    if (p_imu == IMU_DRIVER_NULL) {
        return E_IMU_DRIVER_NULL_PTR;
        } else {
        switch (v_channel) {
        case IMU_DRIVER_INTR1_EDGE_CTRL:
            /* write the edge trigger interrupt1*/
            com_rslt = p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->
            dev_addr, IMU_DRIVER_USER_INTR1_EDGE_CTRL__REG,
            &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            if (com_rslt == DRIVER_SUCCESS) {
                v_data = IMU_DRIVER_SET_BITSLICE(v_data,
                IMU_DRIVER_USER_INTR1_EDGE_CTRL,
                v_intr_edge_ctrl);
                com_rslt +=
                p_imu->IMU_DRIVER_BUS_WRITE_FUNC(p_imu->
                dev_addr, IMU_DRIVER_USER_INTR1_EDGE_CTRL__REG,
                &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            }
            break;
        case IMU_DRIVER_INTR2_EDGE_CTRL:
            /* write the edge trigger interrupt2*/
            com_rslt = p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->
            dev_addr, IMU_DRIVER_USER_INTR2_EDGE_CTRL__REG,
            &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            if (com_rslt == DRIVER_SUCCESS) {
                v_data = IMU_DRIVER_SET_BITSLICE(v_data,
                IMU_DRIVER_USER_INTR2_EDGE_CTRL,
                v_intr_edge_ctrl);
                com_rslt +=
                p_imu->IMU_DRIVER_BUS_WRITE_FUNC(p_imu->
                dev_addr, IMU_DRIVER_USER_INTR2_EDGE_CTRL__REG,
                &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            }
            break;
        default:
            com_rslt = E_IMU_DRIVER_OUT_OF_RANGE;
            break;
        }
        /*Accel and Gyro power mode check*/
        if (imu_power_mode_status_g != IMU_DRIVER_NORMAL_MODE)
            /*interface idle time delay */
            p_imu->delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
    }
    return com_rslt;
}
/*!
 *	@brief  This API is used to set output enable of interrupt1
 *	and interrupt2 from the register 0x53
 *	@brief interrupt1 - bit 2
 *	@brief interrupt2 - bit 6
 *
 *
 *  @param v_channel: The value of output type enable selection
 *   v_channel  |   level selection
 *  ---------------|---------------
 *       0         | IMU_DRIVER_INTR1_OUTPUT_TYPE
 *       1         | IMU_DRIVER_INTR2_OUTPUT_TYPE
 *
 *	@param v_intr_output_type :
 *	The value of output type of interrupt enable
 *	value    | Behaviour
 * ----------|-------------------
 *  0x01     |  IMU_DRIVER_OPEN_DRAIN
 *  0x00     |  IMU_DRIVER_PUSH_PULL
 *
 *
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval -1 -> Error
 *
 *
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_intr_output_type(
uint8 v_channel, uint8 v_intr_output_type)
{
    /* variable used to return the status of communication result*/
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = E_IMU_DRIVER_COMM_RES;
    uint8 v_data = IMU_DRIVER_INIT_VALUE;
    /* check the p_imu structure for NULL pointer assignment*/
    if (p_imu == IMU_DRIVER_NULL) {
        return E_IMU_DRIVER_NULL_PTR;
        } else {
        switch (v_channel) {
        case IMU_DRIVER_INTR1_OUTPUT_TYPE:
            /* write the output type of interrupt1*/
            com_rslt = p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->
            dev_addr, IMU_DRIVER_USER_INTR1_OUTPUT_TYPE__REG,
            &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            if (com_rslt == DRIVER_SUCCESS) {
                v_data = IMU_DRIVER_SET_BITSLICE(v_data,
                IMU_DRIVER_USER_INTR1_OUTPUT_TYPE,
                v_intr_output_type);
                com_rslt +=
                p_imu->IMU_DRIVER_BUS_WRITE_FUNC(p_imu->
                dev_addr, IMU_DRIVER_USER_INTR1_OUTPUT_TYPE__REG,
                &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            }
            break;
        case IMU_DRIVER_INTR2_OUTPUT_TYPE:
            /* write the output type of interrupt2*/
            com_rslt = p_imu->IMU_DRIVER_BUS_READ_FUNC(p_imu->
            dev_addr, IMU_DRIVER_USER_INTR2_OUTPUT_TYPE__REG,
            &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            if (com_rslt == DRIVER_SUCCESS) {
                v_data = IMU_DRIVER_SET_BITSLICE(v_data,
                IMU_DRIVER_USER_INTR2_OUTPUT_TYPE,
                v_intr_output_type);
                com_rslt +=
                p_imu->IMU_DRIVER_BUS_WRITE_FUNC(p_imu->
                dev_addr, IMU_DRIVER_USER_INTR2_OUTPUT_TYPE__REG,
                &v_data, IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH);
            }
            break;
        default:
            com_rslt = E_IMU_DRIVER_OUT_OF_RANGE;
            break;
        }
        /*Accel and Gyro power mode check*/
        if (imu_power_mode_status_g != IMU_DRIVER_NORMAL_MODE)
            /*interface idle time delay */
            p_imu->delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
    }
    return com_rslt;
}
#endif

