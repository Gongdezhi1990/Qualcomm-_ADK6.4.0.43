/**  Copyright (c) 2018 Qualcomm Technologies International, Ltd. */
/**     */
/** *
 * \file
 * Sport Health log decompression functions.
 * These are not used on the device so are not compiled in to the application; they are for use by the decompressing device
 */
#ifndef SPORT_HEALTH_LOG_DECOMPRESS_H
#define SPORT_HEALTH_LOG_DECOMPRESS_H
#include "sport_health_logging.h"
#include "sport_health_log_compress.h"

/* On decompress we expect there to be plenty of memory available so we can use a long buffer */
typedef struct {
	uint8 num_packets;                  // total number of packets we will serialize excluding the meta data packet
	uint8 bits_x;                       // max bits for dx to be compressible where dx = abs(x-xprev)
	uint8 bits_y;                       // max bits for dy to be compressible where dy = abs(y-yprev)
	uint8 bits_z;                       // max bits for dz to be compressible where dx = abs(z-zprev)
	uint8 bitfield[(SH_LOG_COMPRESS_MAX_SAMPLES + 8) / 8];  // number of current packet (0 is the meta then 1...num_packets)
	uint8 last_packet_received;         // # of the last unzipped packet
	uint8 unzip_byte_buffer[2048];      // storage for cat'ing together payloads
	uint16 unzip_byte_buffer_len;       // when we are in the byte buffer
	uint8 uzip_complete;
} sh_log_decompress_imu_info;

/* de-compression side. Rteurnt he number of bytes decompressed (or 0 if error or complete) */
uint8 SportHealthLoggingDecompressImuHeader(imu_sensor_data_t * p_data, sh_log_decompress_imu_info * info, const uint8 * payload);
uint8 SportHealthLoggingDecompressImuData  (imu_sensor_data_t * p_data, sh_log_decompress_imu_info * info, const uint8 * payload);

#ifdef SH_LOG_COMPRESS_TEST
/* compress, uncompress and check that the results match */
extern int8 sh_log_test_compression(imu_sensor_data_t * p_data);
#endif

#endif /* SPORT_HEALTH_LOG_DECOMPRESS_H */

