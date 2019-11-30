/* Copyright (c) 2017 Qualcomm Technologies International, Ltd. */
/*    */

#ifndef BITSERIAL_IF_H
#define BITSERIAL_IF_H

/** Bitserial handle. */
typedef uint8 bitserial_handle;
/** Special value of Bitserial handle indicating that BitserialOpen() failed. */
#define BITSERIAL_HANDLE_ERROR ((bitserial_handle)0xff)

/** Bitserial transfer handle. */
typedef uint16 bitserial_transfer_handle;
/** Special value to be passed into BitserialTransfer, BitserialWrite,
 * BitserialRead for bitserial_transfer_handle *transfer_handle_ptr when
 * Bitserial message on completion is not requested. */
#define BITSERIAL_NO_MSG ((bitserial_transfer_handle *)NULL)

/** Bitserial instance/block index to be passed into BitserialOpen(). */
typedef enum bitserial_block_index
{
    /** Bitserial instance 0. */
    BITSERIAL_BLOCK_0,
    /** Bitserial instance 1. */
    BITSERIAL_BLOCK_1
} bitserial_block_index;

/** Flags for SPI mode. */
typedef enum bitserial_spi_flags
{
    /** SPI MODE 0 (CPOL=0, CPHA=0): clock not inverted, rising edge sample. */
    BITSERIAL_SPI_MODE_0 = (0 << 0),
    /** SPI MODE 1 (CPOL=0, CPHA=1): clock not inverted, falling edge sample. */
    BITSERIAL_SPI_MODE_1 = (1 << 0),
    /** SPI MODE 2 (CPOL=1, CPHA=0): clock inverted, rising edge sample. */
    BITSERIAL_SPI_MODE_2 = (2 << 0),
    /** SPI MODE 3 (CPOL=1, CPHA=1): clock inverted, falling edge sample. */
    BITSERIAL_SPI_MODE_3 = (3 << 0),

    /** Enable LS-bit first transfers. */
    BITSERIAL_SPI_BIT_REVERSE = (1 << 2),
    /** Invert the DIN pin. */
    BITSERIAL_SPI_DATA_IN_INVERT = (1 << 3),
    /** Invert the DOUT pin. */
    BITSERIAL_SPI_DATA_OUT_INVERT = (1 << 4),
    /** Invert nCS pin to use active high chip select. */
    BITSERIAL_SPI_CS_ACTIVE_HIGH = (1 << 5),

    /** Data out line after a transaction is set to "0". */
    BITSERIAL_SPI_DOUT_IDLE_0 = (0 << 6),
    /** Data out line after a transaction is set to "1". */
    BITSERIAL_SPI_DOUT_IDLE_1 = (1 << 6),
    /** Data out line after a transaction maintains state from last bit
     * of data byte sent. */
    BITSERIAL_SPI_DOUT_IDLE_LAST = (2 << 6),

    /** Number of bytes per word is 1. */
    BITSERIAL_SPI_WORD_BYTES_1 = (0 << 8),
    /** Number of bytes per word is 2. */
    BITSERIAL_SPI_WORD_BYTES_2 = (1 << 8),
    /** Number of bytes per word is 3. */
    BITSERIAL_SPI_WORD_BYTES_3 = (2 << 8),
    /** Number of bytes per word is 4. Note: in this mode start and stop
     * bits can't be used, therefore BITSERIAL_FLAG_START_X /
     * BITSERIAL_FLAG_STOP_X flags are ignored. */
    BITSERIAL_SPI_WORD_BYTES_4 = (3 << 8),

    /** Enable reverse transfers of bytes in a word. */
    BITSERIAL_SPI_BYTE_SWAP = (1 << 10),

    /** Use custom value of clock sample offset in "clock_sample_offset". */
    BITSERIAL_SPI_CUSTOM_CLOCK_OFFSET = (1 << 11)
} bitserial_spi_flags;

/** Mask to get SPI mode bits from bitserial_spi_flags */
#define BITSERIAL_SPI_MODE_MASK (3 << 0)
/** Macro to get SPI mode value from bitserial_spi_flags */
#define BITSERIAL_SPI_MODE_GET(flags) ((flags) & BITSERIAL_SPI_MODE_MASK)

/** Mask to get DOUT bits from bitserial_spi_flags */
#define BITSERIAL_SPI_DOUT_IDLE_MASK (3 << 6)
/** Macro to get DOUT idle value from bitserial_spi_flags */
#define BITSERIAL_SPI_DOUT_IDLE_GET(flags) \
                                       ((flags) &  BITSERIAL_SPI_DOUT_IDLE_MASK)

/** Mask to get word bytes bits from bitserial_spi_flags */
#define BITSERIAL_SPI_WORD_BYTES_MASK (3 << 8)
/** Macro to get word bytes value from bitserial_spi_flags */
#define BITSERIAL_SPI_WORD_BYTES_GET(flags) \
                                       ((flags) & BITSERIAL_SPI_WORD_BYTES_MASK)

/** Configuration for SPI mode. */
typedef struct
{
    /** If BITSERIAL_SPI_CUSTOM_CLOCK_OFFSET is set in "flags", then
     * "clock_sample_offset" provides custom value of clock sample offset:
     * T(offset) = T(half_serial_clock) * clock_sample_offset / 0xffff.
     * T(half_serial_clock) is 1/2 serial clock period and is the
     * maximum offset supported.
     * If BITSERIAL_SPI_CUSTOM_CLOCK_OFFSET is not set in "flags", then
     * "clock_sample_offset" is ignored and default offset, equal to
     * 1/4 serial clock period, is used */
    uint16 clock_sample_offset;
    /** Spacing inserted between every "word" number of bytes, in number of
     * serial clocks periods. */
    uint16 interword_spacing;
    /** Time in serial clock cycles between
     * a) nCS being asserted and the 1st bit;
     * b) the last bit and nCS being deasserted. */
    uint32 select_time_offset;
    /** SPI mode flags. */
    bitserial_spi_flags flags;
    /** SPI chip select enabled. */
    bool sel_enabled;
} bitserial_spi_config;

/** Flags for I2C mode. */
typedef enum bitserial_i2c_flags
{
    /** When NAK is received, proceed with the transfer. */
    BITSERIAL_I2C_ACT_ON_NAK_CONTINUE = (0 << 0),
    /** When NAK is received, generate STOP. */
    BITSERIAL_I2C_ACT_ON_NAK_STOP = (1 << 0),
    /** When NAK is received, generate RESTART. */
    BITSERIAL_I2C_ACT_ON_NAK_RESTART = (2 << 0)
} bitserial_i2c_flags;

/** Macro to get I2C_ACT_ON_NAK value from bitserial_i2c_flags */
#define BITSERIAL_I2C_ACT_ON_NAK_GET(flags) ((flags >> 0) & 3)


/** Configuration for I2C mode. */
typedef struct
{
    /** I2C device address */
    uint16 i2c_address;
    /** I2C mode flags */
    bitserial_i2c_flags flags;
} bitserial_i2c_config;

/** Bitserial mode. */
typedef enum BitserialMode
{
    /** SPI master mode. */
    BITSERIAL_MODE_SPI_MASTER,
    /** I2C master mode. */
    BITSERIAL_MODE_I2C_MASTER
} bitserial_mode;

/** Bitserial configuration. */
typedef struct bitserial_config
{
    /** Bitserial mode */
    bitserial_mode mode;
    /** Serial clock frequency in kHz. */
    uint16 clock_frequency_khz;
    /** When transfer has not finished in time (based on transfer length
     * and clock_frequency_khz) wait for "timeout_ms" and then terminate
     * with BITSERIAL_RESULT_TIMEOUT.
     * If set to "0" and the transfer did not finish after 1000 ms,
     * firmware assumes it is stuck and panics. */
    uint16 timeout_ms;
    union
    {
        /** Configuration for SPI mode. */
        bitserial_spi_config spi_cfg;
        /** Configuration for I2C mode. */
        bitserial_i2c_config i2c_cfg;
    } u;
} bitserial_config;

/** Bitserial parameters that can be changed using BitserialChangeParam. */
typedef enum bitserial_changeable_params
{
    /** Changes the address of the I2C device for future Bitserial transfers. */
    BITSERIAL_PARAMS_I2C_DEVICE_ADDRESS
} bitserial_changeable_params;

/** Result of Bitserial transfer. */
typedef enum bitserial_result
{
    /** Transfer was successful. */
    BITSERIAL_RESULT_SUCCESS,
    /** Can't queue transfer as there is not enough space in internal buffer. */
    BITSERIAL_RESULT_NO_SPACE,
    /** Error in transfer parameters. */
    BITSERIAL_RESULT_INVAL,
    /** Error on internal memory bus. */
    BITSERIAL_RESULT_MMU,
    /** I2C arbitration error. */
    BITSERIAL_RESULT_I2C_ARBITRATION,
    /** I2C NAK error. */
    BITSERIAL_RESULT_I2C_NACK,
    /** Transfer did not finish in time, this is returned after waiting for
     * the expected transfer time and bitserial_config::timeout_ms after that.
     * BitserialClose() / BitserialOpen() sequence is required to recover. */
    BITSERIAL_RESULT_TIMEOUT
} bitserial_result;

/** Bitserial transfer flags for BitserialRead() and BitserialWrite() traps. */
typedef enum bitserial_transfer_flags
{
    /** Don't generate STOP (I2C) / deassert (nCS) after the transfer. */
    BITSERIAL_FLAG_NO_STOP = (1 << 0),
    /** Make this Bitserial transfer call blocking until transfer
     * has finished. */
    BITSERIAL_FLAG_BLOCK =   (1 << 1),

    /** For SPI transfers: enable start bit and make it "0". */
    BITSERIAL_SPI_FLAG_START_0 = (1 << 2),
    /** For SPI transfers: enable start bit and make it "1". */
    BITSERIAL_SPI_FLAG_START_1 = (1 << 3),
    /** For SPI transfers: enable stop bit and make it "0". */
    BITSERIAL_SPI_FLAG_STOP_0 =  (1 << 4),
    /** For SPI transfers: enable stop bit and make it "1". */
    BITSERIAL_SPI_FLAG_STOP_1 =  (1 << 5)
} bitserial_transfer_flags;

/** Maximum number of bytes that can be transfered in single transfer,
 * either to or from device. */
#define BITSERIAL_MAX_TRANSFER_LENGTH 256

#endif /* BITSERIAL_IF_H */
