/* Copyright (c) 2017 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * PIO configuration
 *
 * \section pio_cfg_usage USAGE
 * This is the public header for the PIO configuration module. It should be
 * included in all modules that need to configure PIOs.
 */

/**
 * \defgroup pio_cfg pio_cfg
 * \ingroup core
 * \section pio_cfg_intro INTRODUCTION
 *
 * Besides direction, drive level, reading and interrups there are various
 * other configuration parameters that affect the PIOs. These controls are
 * usually in the Curator and we need to use the CCP interface for them.
 *
 * PIO_CFG_OP_ACQUIRE is used for the acquisition of PIO resources. The bank
 * and mask are used to identify PIOs and the value is unused.
 * PIO_CFG_OP_RELEASE is used for the release of PIO resources. The bank
 * and mask are used to identify PIOs and the value is unused.
 * PIO_CFG_OP_RESOURCE_QUERY is used for querying the PIO resource ownership.
 * The bank is used to identify PIOs and the mask and value are unused. The
 * response is a bit mask (1-free, 0-owned).
 * PIO_CFG_OP_SET_PIO_MUX is used for setting the PIO MUX. The bank and mask are
 * used to identify the PIO (only one allowed) and the value indicates the mux
 * value. Please set the PAD MUX to PIO_CFG_PAD_MUX_CORE_PIO otherwise this mux
 * setting is ignored.
 * PIO_CFG_OP_GET_PIO_MUX is used for interrogating the PIO MUX. The bank and
 * mask are used to identify the PIO (only one allowed) and the value is unused.
 * The response indicates the mux value.
 * PIO_CFG_OP_SET_PULL_ENABLE is used for enabling/disabling the pull resistors.
 * The bank and mask are used to identify PIOs and the value is a bit mask
 * (1-enable, 0-disable).
 * PIO_CFG_OP_GET_PULL_ENABLE is used to find out if the pull resistors are
 * enabled. The bank and mask are used to identify PIOs and the value is unused.
 * The response is a bit mask (1-enabled, 0-disabled).
 * PIO_CFG_OP_SET_PULL_DIRECTION is used to select the pull resistor direction.
 * The bank and mask are used to identify PIOs and the value is a bit mask
 * (1-up, 0-down).
 * PIO_CFG_OP_GET_PULL_DIRECTION is used to find out the pull resistor
 * direction. The bank and mask are used to identify PIOs and the value is
 * unused. The result is a bit mask (1-up, 0-down).
 * PIO_CFG_OP_SET_PULL_STRENGTH is used to select the pull resistor strength.
 * The bank and mask are used to identify PIOs and the value is a bit mask
 * (1-strong, 0-weak).
 * PIO_CFG_OP_GET_PULL_STRENGTH is used to find out the pull resistor
 * strength. The bank and mask are used to identify PIOs and the value is
 * unused. The result is a bit mask (1-strong, 0-weak).
 * PIO_CFG_OP_SET_DRIVE_STRENGTH is used to select the drive strength. The bank
 * and mask are used to identify the PIO (only one allowed) and the value
 * indicates the drive strength (0 to 3).
 * PIO_CFG_OP_GET_DRIVE_STRENGTH is used to find out the drive strength. The
 * bank and mask are used to identify the PIO (only one allowed) and the value
 * is unused. The result indicates the drive strength (0 to 3).
 * PIO_CFG_OP_SET_SLEW_ENABLE is used for enabling/disabling slew. The bank and
 * mask are used to identify PIOs and the value is a bit mask
 * (1-enable, 0-disable).
 * PIO_CFG_OP_GET_SLEW_ENABLE is used to find out if slew is enabled. The bank
 * and mask are used to identify PIOs and the value is unused. The response is
 * a bit mask (1-enabled, 0-disabled).
 * PIO_CFG_OP_SET_STICKY_ENABLE is used for enabling/disabling sticky pulls.
 * The bank and mask are used to identify PIOs and the value is a bit mask
 * (1-enable, 0-disable).
 * PIO_CFG_OP_GET_STICKY_ENABLE is used to find out if sticky pulls are enabled.
 * The bank and mask are used to identify PIOs and the value is unused. The
 * response is a bit mask (1-enabled, 0-disabled).
 * PIO_CFG_OP_SET_PAD_MUX is used to select the PAD MUX. The bank and mask are
 * used to identify the PIO (only one allowed) and the value indicates the
 * PAD MUX value.
 * PIO_CFG_OP_GET_PAD_MUX is used to find out the drive strength. The bank and
 * mask are used to identify the PIO (only one allowed) and the value is unused.
 * The result indicates the PAD MUX value.
 * PIO_CFG_OP_SET_XIO_MODE is used to select the XIO mode. The bank and mask
 * are used to identify PIOs and the value is a bit mask
 * (1-special mode, 0-normal). This can only be used for XIO pins.
 * PIO_CFG_OP_GET_XIO_MODE is used to find out if the special XIO mode is
 * enabled. The bank and mask are used to identify PIOs and the value is unused.
 * The response is a bit mask (1-special mode, 0-normal).
 */

#ifndef PIO_CFG_H
#define PIO_CFG_H


#include "hydra/hydra_types.h"

/** Bank where XIOs are */
#define PIO_CFG_XIO_BANK 0
/** Mask indicating XIOs within the XIO bank */
#define PIO_CFG_XIO_MASK 0U
/** Bank where LEDs are */
#define PIO_CFG_LED_BANK 2
/** Mask indicating LEDs within the LED bank */
#define PIO_CFG_LED_MASK 0xfcU

/**
 * Typedef for the PIO configuration status.
 */
typedef enum pio_cfg_status
{
    PIO_CFG_SUCCESS = 0,
    PIO_CFG_NO_BULK_QUERY,
    PIO_CFG_NON_XIOS_IN_MASK
} pio_cfg_status;

/**
 * Typedef for the PIO configuration operations.
 */
typedef enum pio_cfg_ops
{
    PIO_CFG_OP_ACQUIRE = 0,
    PIO_CFG_OP_RELEASE,
    PIO_CFG_OP_RESOURCE_QUERY,
    PIO_CFG_OP_SET_PIO_MUX,
    PIO_CFG_OP_GET_PIO_MUX,
    PIO_CFG_OP_SET_PULL_ENABLE,
    PIO_CFG_OP_GET_PULL_ENABLE,
    PIO_CFG_OP_SET_PULL_DIRECTION,
    PIO_CFG_OP_GET_PULL_DIRECTION,
    PIO_CFG_OP_SET_PULL_STRENGTH,
    PIO_CFG_OP_GET_PULL_STRENGTH,
    PIO_CFG_OP_SET_DRIVE_STRENGTH,
    PIO_CFG_OP_GET_DRIVE_STRENGTH,
    PIO_CFG_OP_SET_SLEW_ENABLE,
    PIO_CFG_OP_GET_SLEW_ENABLE,
    PIO_CFG_OP_SET_STICKY_ENABLE,
    PIO_CFG_OP_GET_STICKY_ENABLE,
    PIO_CFG_OP_SET_PAD_MUX,
    PIO_CFG_OP_GET_PAD_MUX,
    PIO_CFG_OP_SET_XIO_MODE_ENABLE,
    PIO_CFG_OP_GET_XIO_MODE_ENABLE,
    PIO_CFG_OP_NUM_CONFIG_KEY
} pio_cfg_ops;

/**
 * Type definition for the IDs accepted by the PAD mux. Please note that not
 * all of these are available for all PIOs. Also, for firmware controlled
 * PIOs \c PIO_CFG_PAD_MUX_CORE_PIO needs to be used.
 */

typedef enum
{
    PIO_CFG_PAD_MUX_CORE_PIO = 0,
    PIO_CFG_PAD_MUX_JANITOR_PIO,
    PIO_CFG_PAD_MUX_T_BRIDGE,
    PIO_CFG_PAD_MUX_DEBUG_SPI,
    PIO_CFG_PAD_MUX_SDIO_DEVICE,
    PIO_CFG_PAD_MUX_ULPI,
    PIO_CFG_PAD_MUX_AUDIO_PCM,
    PIO_CFG_PAD_MUX_AUDIO_PWM,
    PIO_CFG_PAD_MUX_MCLK,
    PIO_CFG_PAD_MUX_CLK32K,
    PIO_CFG_PAD_MUX_SDIO_HOST,
    PIO_CFG_PAD_MUX_SQIF,
    PIO_CFG_PAD_MUX_USB
} pio_cfg_pad_mux;

/**
 * Type definition for the IDs accepted by the PIO mux.
 */

typedef enum
{
    PIO_CFG_PIO_MUX_CURATOR = 0,
    PIO_CFG_PIO_MUX_HOST,
    PIO_CFG_PIO_MUX_BT,
    PIO_CFG_PIO_MUX_AUDIO,
    PIO_CFG_PIO_MUX_APPS,
    PIO_CFG_PIO_MUX_PIO_CTRL,
    PIO_CFG_PIO_MUX_LED_CTRL
} pio_cfg_pio_mux;

/**
 * Typedef for the pio_cfg_op callback. The op, bank and mask parameters are
 * the ones that were given to pio_cfg_op. The value parameter is used to
 * indicate either an error code or a bit mask of the failed PIOs.
 */
typedef void (*pio_cfg_cb)(pio_cfg_ops op,
                           uint16 bank,
                           uint32 mask,
                           uint32 value);

/**
 * Initiates a PIO configuration operation.
 * \param op Operation ID, see \c pio_cfg_ops.
 * \param bank Bank ID.
 * \param mask Bit mask indicating which PIOs to configure.
 * \param value Configuration value or bit mask indicating the configuration
 * value per PIO.
 * \param cb Callback to be called once the configuration is completed. Some
 * errors may prevent the callback from being called. Please note that in some
 * cases the callback may be called brefore this function returns.
 * \return PIO_CFG_SUCCESS if request was successful, PIO_CFG_NO_BULK_QUERY
 * indicates that the query requested cannot be made for more than one PIO
 * at a time, PIO_CFG_NON_XIOS_IN_MASK indicates an XIO only operation requested
 * for non-XIO PIOs. The callback will be called only if the PIO_CFG_SUCCESS
 * status is returned.
 */
extern pio_cfg_status pio_cfg_op(pio_cfg_ops op,
                                 uint16 bank,
                                 uint32 mask,
                                 uint32 value,
                                 pio_cfg_cb cb);


#endif /* PIO_CFG_H */
