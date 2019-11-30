/*!
\copyright  Copyright (c) 2017 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       adxl362.c
\brief      Support for the adxl362 accelerometer
*/

#ifdef INCLUDE_ACCELEROMETER
#ifdef HAVE_ADXL362
#include <bitserial_api.h>
#include <panic.h>
#include <pio.h>
#include <hydra_macros.h>
#include <input_event_manager.h>

#include "../av_headset.h"
#include "../av_headset_log.h"
#include "../av_headset_accelerometer.h"
#include "adxl362.h"

/*! \brief Returns the PIOs bank number.
    \param pio The pio.
*/
#define PIO2BANK(pio) ((uint16)((pio) / 32))
/*! \brief Returns the PIO bit position mask within a bank.
    \param pio The pio.
*/
#define PIO2MASK(pio) (1UL << ((pio) % 32))

/*! \brief Read a register from the accelerometer */
static bool adxl362ReadRegister(bitserial_handle handle, uint8 reg,  uint8 *value)
{
    bitserial_result result;
    uint8 command[2] = {REG_READ_CMD, reg};

    /* Write the command and register to read */
    result = BitserialWrite(handle, BITSERIAL_NO_MSG,
                            command, ARRAY_DIM(command), BITSERIAL_FLAG_NO_STOP);
    if (result == BITSERIAL_RESULT_SUCCESS)
    {
        /* Read the register value */
        result = BitserialRead(handle, BITSERIAL_NO_MSG,
                               value, 1, BITSERIAL_FLAG_BLOCK);
    }
    return (result == BITSERIAL_RESULT_SUCCESS);
}

/*! \brief Write a single accelerometer register */
static bool adxl362WriteRegister8(bitserial_handle handle, uint8 reg, uint8 value)
{
    uint8 command[] = {REG_WRITE_CMD, reg, value};
    return (BitserialWrite(handle, BITSERIAL_NO_MSG,
                           command, ARRAY_DIM(command),
                           BITSERIAL_FLAG_BLOCK) == BITSERIAL_RESULT_SUCCESS);
}

/*! \brief Write a pair of accelerometer registers.
    The least significant octet of value is written first */
static bool adxl362WriteRegister16(bitserial_handle handle, uint8 reg, uint16 value)
{
    uint8 command[] = {REG_WRITE_CMD, reg, value & 0xff, value >> 8};
    return (BitserialWrite(handle, BITSERIAL_NO_MSG,
                           command, ARRAY_DIM(command),
                           BITSERIAL_FLAG_BLOCK) == BITSERIAL_RESULT_SUCCESS);
}

/*! Get the client message based on the PIO state and mask.
    The sensor signals detected motion by setting the masked PIO high. */
static MessageId getMessage(uint32 pio_state, uint32 pio_mask)
{
    bool in_motion = (0 != (pio_state & pio_mask));
    return in_motion ? ACCELEROMETER_MESSAGE_IN_MOTION : ACCELEROMETER_MESSAGE_NOT_IN_MOTION;
}

/*! \brief Handle the accelerometer interrupt */
static void adxl362InterruptHandler(Task task, MessageId id, Message msg)
{
    accelTaskData *accel = (accelTaskData *)task;
    UNUSED(accel);
    switch(id)
    {
        case MESSAGE_PIO_CHANGED:
        {
            const MessagePioChanged *mpc = (const MessagePioChanged *)msg;
            uint32 state = mpc->state16to31 << 16 | mpc->state;
            uint16 bank = PIO2BANK(accel->config->pios.interrupt);
            uint32 mask = PIO2MASK(accel->config->pios.interrupt);
            if (mpc->bank == bank)
            {
                appTaskListMessageSendId(accel->clients, getMessage(state, mask));
            }
        }
        break;
        default:
        break;
    }
}

/*! \brief Enable the accelerometer */
static bitserial_handle adxl362Enable(const accelerometerConfig *config)
{
    bitserial_config bsconfig;
    uint16 bank;
    uint32 mask;

    DEBUG_LOG("adxl362Enable");

    if (config->pios.on != ADXL362_ON_PIO_UNUSED)
    {
        /* Enable PIO to power the accelerometer as an output */
        bank = PIO2BANK(config->pios.on);
        mask = PIO2MASK(config->pios.on);
        PanicNotZero(PioSetMapPins32Bank(bank, mask, mask));
        PanicNotZero(PioSetDir32Bank(bank, mask, mask));
        PanicNotZero(PioSet32Bank(bank, mask, mask));
    }

    /* Setup the PIOs for Bitserial SPI use */
    bank = PIO2BANK(config->pios.spi_cs);
    mask = PIO2MASK(config->pios.spi_cs);
    PanicNotZero(PioSetMapPins32Bank(bank, mask, 0));
    bank = PIO2BANK(config->pios.spi_clk);
    mask = PIO2MASK(config->pios.spi_clk);
    PanicNotZero(PioSetMapPins32Bank(bank, mask, 0));
    bank = PIO2BANK(config->pios.spi_miso);
    mask = PIO2MASK(config->pios.spi_miso);
    PanicNotZero(PioSetMapPins32Bank(bank, mask, 0));
    bank = PIO2BANK(config->pios.spi_mosi);
    mask = PIO2MASK(config->pios.spi_mosi);
    PanicNotZero(PioSetMapPins32Bank(bank, mask, 0));

    /* Setup the PIOs for Bitserial SPI use*/
    PanicFalse(PioSetFunction(config->pios.spi_cs, BITSERIAL_0_SEL_OUT));
    PanicFalse(PioSetFunction(config->pios.spi_clk, BITSERIAL_0_CLOCK_OUT));
    PanicFalse(PioSetFunction(config->pios.spi_miso, BITSERIAL_0_DATA_IN));
    PanicFalse(PioSetFunction(config->pios.spi_mosi, BITSERIAL_0_DATA_OUT));

    /* Setup Interrupt as input with weak pull up */
    bank = PIO2BANK(config->pios.interrupt);
    mask = PIO2MASK(config->pios.interrupt);
    PanicNotZero(PioSetMapPins32Bank(bank, mask, mask));
    PanicNotZero(PioSetDir32Bank(bank, mask, 0));
    PanicNotZero(PioSet32Bank(bank, mask, mask));

    /* Configure Bitserial to work with adxl362 accelerometer */
    memset(&bsconfig, 0, sizeof(bsconfig));
    bsconfig.mode = BITSERIAL_MODE_SPI_MASTER;
    bsconfig.clock_frequency_khz = config->spi_clock_khz;
    bsconfig.u.spi_cfg.sel_enabled = TRUE;
    bsconfig.u.spi_cfg.clock_sample_offset = 0;
    bsconfig.u.spi_cfg.interword_spacing = 0;
    bsconfig.u.spi_cfg.select_time_offset = 0;
    bsconfig.u.spi_cfg.flags = BITSERIAL_SPI_MODE_0;
    return BitserialOpen((bitserial_block_index)BITSERIAL_BLOCK_0, &bsconfig);
}

/*! \brief Disable the accelerometer */
static void adxl362Disable(bitserial_handle handle, const accelerometerConfig *config)
{
    uint16 bank;
    uint32 mask;
    DEBUG_LOG("adxl362Disable");

    /* Disable interrupt and set weak pull down */
    bank = PIO2BANK(config->pios.interrupt);
    mask = PIO2MASK(config->pios.interrupt);
    PanicNotZero(PioSet32Bank(bank, mask, 0));

    /* Release bitserial instance */
    BitserialClose(handle);
    handle = BITSERIAL_HANDLE_ERROR;

    if (config->pios.on != ADXL362_ON_PIO_UNUSED)
    {
        /* Power off the Accelerometer*/
        bank = PIO2BANK(config->pios.on);
        mask = PIO2MASK(config->pios.on);
        PanicNotZero(PioSet32Bank(bank, mask, 0));
    }
}

bool appAccelerometerClientRegister(Task task)
{
    accelTaskData *accel = appGetAccelerometer();

    if (NULL == accel->clients)
    {
        uint8 device_id, part_id, revision;
        adxl362_activity_control_register_t activity_control = {0};
        adxl362_power_control_register_t power_control = {0};
        adxl362_interrupt_control_register_t interrupt_control = {0};

        accel->clients = appTaskListInit();
        accel->config = appConfigAccelerometer();

        accel->handle = adxl362Enable(accel->config);
        PanicFalse(accel->handle != BITSERIAL_HANDLE_ERROR);

        PanicFalse(adxl362ReadRegister(accel->handle, ADXL362_DEVID_AD_REG, &device_id));
        PanicFalse(adxl362ReadRegister(accel->handle, ADXL362_PARTID_REG, &part_id));
        PanicFalse(adxl362ReadRegister(accel->handle, ADXL362_REVID_REG, &revision));
        DEBUG_LOGF("appAccelerometerInit %x %x %x", device_id, part_id, revision);

        /* Startup the accelerometer as an autonomous motion switch with INT1
        signalling the active (high) inactive (low) state */
        PanicFalse(adxl362WriteRegister16(accel->handle, ADXL362_THRESH_ACTL_REG,
                                          accel->config->activity_threshold));
        PanicFalse(adxl362WriteRegister16(accel->handle, ADXL362_THRESH_INACTL_REG,
                                          accel->config->inactivity_threshold));
        PanicFalse(adxl362WriteRegister16(accel->handle, ADXL362_TIME_INACTL_REG,
                                          accel->config->inactivity_timer));
        /* Motion detection in loop mode and enabled referenced activity and
        inactivity detection */
        activity_control.bits.link_loop = 3; /* loop */
        activity_control.bits.act_en = 1;
        activity_control.bits.inact_en = 1;
        activity_control.bits.act_ref = 1;
        activity_control.bits.inact_ref = 1;
        PanicFalse(adxl362WriteRegister8(accel->handle, ADXL362_ACT_INACT_CTL_REG, activity_control.reg));
        /* Map awake status to INT1 */
        interrupt_control.bits.awake = 1;
        PanicFalse(adxl362WriteRegister8(accel->handle, ADXL362_INTMAP1_REG, interrupt_control.reg));

        /* Register for interrupt events */
        accel->task.handler = adxl362InterruptHandler;
        InputEventManagerRegisterTask(&accel->task, accel->config->pios.interrupt);

        /*! Start measurement in wakeup mode */
        power_control.bits.wakeup = 1;
        power_control.bits.measure = 2;
        PanicFalse(adxl362WriteRegister8(accel->handle, ADXL362_POWER_CTL_REG, power_control.reg));
    }

    /* Send a state message to the registering client: read the interrupt PIO state */
    uint16 bank = PIO2BANK(accel->config->pios.interrupt);
    uint32 mask = PIO2MASK(accel->config->pios.interrupt);
    MessageSend(task, getMessage(PioGet32Bank(bank), mask), NULL);

    return appTaskListAddTask(accel->clients, task);
}

void appAccelerometerClientUnregister(Task task)
{
    accelTaskData *accel = appGetAccelerometer();

    appTaskListRemoveTask(accel->clients, task);
    if (0 == appTaskListSize(accel->clients))
    {
        appTaskListDestroy(accel->clients);
        accel->clients = NULL;

        PanicFalse(accel->handle != BITSERIAL_HANDLE_ERROR);

        /* Unregister for interrupt events */
        InputEventManagerUnregisterTask(&accel->task, accel->config->pios.interrupt);

        PanicFalse(adxl362WriteRegister8(accel->handle, ADXL362_SOFT_RESET_REG, 0x52));
        adxl362Disable(accel->handle, accel->config);
        accel->handle = BITSERIAL_HANDLE_ERROR;
    }
}

bool appAccelerometerGetDormantConfigureKeyValue(dormant_config_key *key, uint32* value)
{
    accelTaskData *accel = appGetAccelerometer();
    uint8 interrupt = accel->config->pios.interrupt;

    if (appConfigPioIsLed(interrupt))
    {
        *key = LED_WAKE_MASK;
        *value = 1 << appConfigPioLedNumber(interrupt);
    }
    else if (appConfigPioCanWakeFromDormant(interrupt))
    {
        *key = PIO_WAKE_MASK;
        *value = 1 << interrupt;
    }
    else
    {
        DEBUG_LOGF("The accelerometer interrupt PIO (%d) cannot wake the chip from dormant", interrupt);
        return FALSE;
    }
    return TRUE;
}

#endif /* HAVE_ADXL362 */
#endif /* INCLUDE_ACCELEROMETER */
