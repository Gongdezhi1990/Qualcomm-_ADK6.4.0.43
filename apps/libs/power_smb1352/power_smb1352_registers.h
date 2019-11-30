/*******************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    power_smb1352_registers.h

DESCRIPTION
    Defines all the registers and their corresponding values
*/

#ifndef POWER_SMB1352_REGISTERS_H_
#define POWER_SMB1352_REGISTERS_H_

#define BIT(x) (1 << x)

/* Config registers. */
#define SMB1352_BASE_CONFIG_REGISTER                                (0x00)
#define SMB1352_TOP_CONFIG_REGISTER                                 (0x14)
#define SMB1352_NUMBER_OF_CONFIGURATION_REGISTERS                   (SMB1352_TOP_CONFIG_REGISTER - SMB1352_BASE_CONFIG_REGISTER + 1)

#define SMB1352_CURRENT_CONFIG_REG_ADDR                             (0x00)
#define SMB1352_ICL_3A_CONFIG_MASK                                  (BIT(3) | BIT(2) | BIT(0))

#define SMB1352_AICL_CONFIG_REG_ADDR                                (0x02)
#define SMB1352_AICL_CONFIG_MASK                                    BIT(4)

#define SMB1352_PIN_AND_ENABLE_CONTROL_REG_ADDR                     (0x06)
#define SMB1352_CHARGER_EN_REG_ACTIVE_HIGH_CONFIG_MASK              (BIT(6) | BIT(5))

#define SMB1352_MIN_SYS_VOLTAGE_REG_ADDR                            (0x07)
#define SMB1352_MIN_SYS_VOLTAGE_CONFIG_MASK                         (BIT(7) | BIT(6))

#define SMB1352_STATUS_INTERRUPT_REGISTER_REG_ADDR                  (0x0D)
#define SMB1352_BMD_INT_CONFIG_MASK                                 BIT(1)
#define SMB1352_POK_INT_CONFIG_MASK                                 BIT(5)

#define SMB1352_QC3_CONFIG_REG_ADDR                                 (0x11)
#define SMB1352_QC3_AUTO_INC_CONFIG_MASK                            BIT(1)
#define SMB1352_QC3_ALG_AUTH_CONFIG_MASK                            BIT(2)

#define SMB1352_BATTERY_MISSING_CONFIG_REG_ADDR                     (0x12)
#define SMB1352_BATT_MISSING_THERM_CONFIG_MASK                      BIT(0)

#define SMB1352_HVDCP_CONFIG_REG_ADDR                               (0x12)
#define SMB1352_HVDCP_CONFIG_MASK                                   BIT(5)

#define SMB1352_QC3_AUTH_REG_ADDR                                   (0x13)
#define SMB1352_QC3_AUTH_INT_CONFIG_MASK                            BIT(0)


/* Summit registers. */
#define SMB1352_BASE_SUMMIT_REGISTER                                (0x15)
#define SMB1352_TOP_SUMMIT_REGISTER                                 (0x2E)
#define SMB1352_NUMBER_OF_SUMMIT_REGISTERS                          (SMB1352_TOP_SUMMIT_REGISTER - SMB1352_BASE_SUMMIT_REGISTER + 1)

/* Command registers. */
#define SMB1352_I2C_COMMAND_REG_ADDR                                (0x30)
#define SMB1352_ENABLE_VOLATILE_WRITES_COMMAND_MASK                 BIT(6)

#define SMB1352_CHG_COMMAND_REG_ADDR                                (0x32)
#define SMB1352_ENABLE_CHARGING_COMMAND_MASK                        BIT(1)


/* Status registers */
#define SMB1352_BASE_STATUS_REGISTER                                (0x36)
#define SMB1352_TOP_STATUS_REGISTER                                 (0x47)
#define SMB1352_NUMBER_OF_STATUS_REGISTERS                          (SMB1352_TOP_STATUS_REGISTER - SMB1352_BASE_STATUS_REGISTER + 1)

/* Status registers with status bits only */
#define SMB1352_CHARGING_STATUS_REG_ADDR                            (0x3A)
#define SMB1352_CHARGING_STATUS_STATUS_MASK                         (BIT(2) | BIT(1))

#define SMB1352_APSD_DETECTION_RESULT_STATUS_REG_ADDR               (0x3B)
#define SMB1352_CDP_STATUS_MASK                                     BIT(7)
#define SMB1352_DCP_STATUS_MASK                                     BIT(2)
#define SMB1352_SDP_STATUS_MASK                                     BIT(4)

/* Status registers with IRQ and status bits */
#define SMB1352_BATTERY_COLD_SOFT_LIMIT_REG_ADDR                    (0x40)
#define SMB1352_BATTERY_COLD_SOFT_LIMIT_STATUS_MASK                 BIT(0)
#define SMB1352_BATTERY_COLD_SOFT_LIMIT_INTERRUPT_MASK              BIT(1)

#define SMB1352_BATTERY_HOT_SOFT_LIMIT_REG_ADDR                     (0x40)
#define SMB1352_BATTERY_HOT_SOFT_LIMIT_STATUS_MASK                  BIT(2)
#define SMB1352_BATTERY_HOT_SOFT_LIMIT_INTERRUPT_MASK               BIT(3)

#define SMB1352_BATTERY_COLD_HARD_LIMIT_REG_ADDR                    (0x40)
#define SMB1352_BATTERY_COLD_HARD_LIMIT_STATUS_MASK                 BIT(4)
#define SMB1352_BATTERY_COLD_HARD_LIMIT_INTERRUPT_MASK              BIT(5)

#define SMB1352_BATTERY_HOT_HARD_LIMIT_REG_ADDR                     (0x40)
#define SMB1352_BATTERY_HOT_HARD_LIMIT_STATUS_MASK                  BIT(6)
#define SMB1352_BATTERY_HOT_HARD_LIMIT_INTERRUPT_MASK               BIT(7)

#define SMB1352_INTERNAL_TEMPERATURE_LIMIT_REACHED_REG_ADDR         (0x41)
#define SMB1352_INTERNAL_TEMPERATURE_LIMIT_REACHED_STATUS_MASK      BIT(0)
#define SMB1352_INTERNAL_TEMPERATURE_LIMIT_REACHED_INTERRUPT_MASK   BIT(1)

#define SMB1352_BATTERY_LOW_REG_ADDR                                (0x41)
#define SMB1352_BATTERY_LOW_STATUS_MASK                             BIT(2)
#define SMB1352_BATTERY_LOW_INTERRUPT_MASK                          BIT(3)

#define SMB1352_BATTERY_MISSING_STATUS_REG_ADDR                     (0x41)
#define SMB1352_BATTERY_MISSING_STATUS_MASK                         BIT(4)
#define SMB1352_BATTERY_MISSING_INTERRUPT_MASK                      BIT(5)

#define SMB1352_BATTERY_THERM_PIN_MISSING_REG_ADDR                  (0x41)
#define SMB1352_BATTERY_THERM_PIN_MISSING_STATUS_MASK               BIT(4)
#define SMB1352_BATTERY_THERM_PIN_MISSING_INTERRUPT_MASK            BIT(5)

#define SMB1352_BATTERY_TERMINAL_REMOVED_REG_ADDR                   (0x41)
#define SMB1352_BATTERY_TERMINAL_REMOVED_STATUS_MASK                BIT(6)
#define SMB1352_BATTERY_TERMINAL_REMOVED_INTERRUPT_MASK             BIT(7)

#define SMB1352_CHARGING_COMPLETE_REG_ADDR                          (0x42)
#define SMB1352_CHARGING_COMPLETE_STATUS_MASK                       BIT(0)
#define SMB1352_CHARGING_COMPLETE_INTERRUPT_MASK                    BIT(1)

#define SMB1352_TAPER_CHARGING_REG_ADDR                             (0x42)
#define SMB1352_TAPER_CHARGING_STATUS_MASK                          BIT(2)
#define SMB1352_TAPER_CHARGING_INTERRUPT_MASK                       BIT(3)

#define SMB1352_AUTOMATIC_RECHARGE_REG_ADDR                         (0x42)
#define SMB1352_AUTOMATIC_RECHARGE_STATUS_MASK                      BIT(4)
#define SMB1352_AUTOMATIC_RECHARGE_INTERRUPT_MASK                   BIT(5)

#define SMB1352_PRE_TO_FAST_CHARGE_THRESHOLD_REG_ADDR               (0x42)
#define SMB1352_PRE_TO_FAST_CHARGE_THRESHOLD_STATUS_MASK            BIT(6)
#define SMB1352_PRE_TO_FAST_CHARGE_THRESHOLD_INTERRUPT_MASK         BIT(7)

#define SMB1352_PRE_CHARGE_TIMEOUT_REG_ADDR                         (0x43)
#define SMB1352_PRE_CHARGE_TIMEOUT_STATUS_MASK                      BIT(0)
#define SMB1352_PRE_CHARGE_TIMEOUT_INTERRUPT_MASK                   BIT(1)

#define SMB1352_COMPLETE_CHARGE_TIMEOUT_REG_ADDR                    (0x43)
#define SMB1352_COMPLETE_CHARGE_TIMEOUT_STATUS_MASK                 BIT(2)
#define SMB1352_COMPLETE_CHARGE_TIMEOUT_INTERRUPT_MASK              BIT(3)

#define SMB1352_CHARGE_ERROR_REG_ADDR                               (0x43)
#define SMB1352_CHARGE_ERROR_STATUS_MASK                            BIT(4)
#define SMB1352_CHARGE_ERROR_INTERRUPT_MASK                         BIT(5)

#define SMB1352_BATTERY_OV_REG_ADDR                                 (0x43)
#define SMB1352_BATTERY_OV_STATUS_MASK                              BIT(6)
#define SMB1352_BATTERY_OV_INTERRUPT_MASK                           BIT(7)

#define SMB1352_POWER_OK_REG_ADDR                                   (0x44)
#define SMB1352_POWER_OK_STATUS_MASK                                BIT(0)
#define SMB1352_POWER_OK_INTERRUPT_MASK                             BIT(1)

#define SMB1352_AFVC_ACTIVE_REG_ADDR                                (0x44)
#define SMB1352_AFVC_ACTIVE_STATUS_MASK                             BIT(2)
#define SMB1352_AFVC_ACTIVE_INTERRUPT_MASK                          BIT(3)

#define SMB1352_INPUT_UVLO_REG_ADDR                                 (0x44)
#define SMB1352_INPUT_UVLO_STATUS_MASK                              BIT(4)
#define SMB1352_INPUT_UVLO_INTERRUPT_MASK                           BIT(5)

#define SMB1352_INPUT_OVLO_REG_ADDR                                 (0x44)
#define SMB1352_INPUT_OVLO_STATUS_MASK                              BIT(6)
#define SMB1352_INPUT_OVLO_INTERRUPT_MASK                           BIT(7)

#define SMB1352_CHARGER_INHIBIT_REG_ADDR                            (0x46)
#define SMB1352_CHARGER_INHIBIT_STATUS_MASK                         BIT(0)
#define SMB1352_CHARGER_INHIBIT_INTERRUPT_MASK                      BIT(1)

#define SMB1352_AICL_FAILED_REG_ADDR                                (0x46)
#define SMB1352_AICL_FAILED_STATUS_MASK                             BIT(2)
#define SMB1352_AICL_FAILED_INTERRUPT_MASK                          BIT(3)

#define SMB1352_AICL_DONE_REG_ADDR                                  (0x46)
#define SMB1352_AICL_DONE_STATUS_MASK                               BIT(4)
#define SMB1352_AICL_DONE_INTERRUPT_MASK                            BIT(5)

#define SMB1352_APSD_COMPLETE_REG_ADDR                              (0x46)
#define SMB1352_APSD_COMPLETE_STATUS_MASK                           BIT(6)
#define SMB1352_APSD_COMPLETE_INTERRUPT_MASK                        BIT(7)

#define SMB1352_QC3_AUTH_ALOGORITHM_DONE_REG_ADDR                   (0x47)
#define SMB1352_QC3_AUTH_ALOGORITHM_DONE_STATUS_MASK                BIT(2)
#define SMB1352_QC3_AUTH_ALOGORITHM_DONE_INTERRUPT_MASK             BIT(3)

#endif /* POWER_SMB1352_REGISTERS_H_ */
