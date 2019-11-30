/**  Copyright (c) 2018 Qualcomm Technologies International, Ltd. */
/**     */
/** *
 * \file
 * Sport Health log compression functions
 */
#ifndef SPORT_HEALTH_LOG_COMPRESS_H
#define SPORT_HEALTH_LOG_COMPRESS_H

/* the results buffer for acc only is 512 bytes so @ 6 bytes per sample we will never exceed 85 samples */
#define SH_LOG_COMPRESS_MAX_SAMPLES 85                    
/* The low 6 bits of the log type is used to store the packet number (an uncompressed set of samples at 100Hz requires 34 packets) */
#define SH_LOG_COMPRESSED_IMU_BASE_TYPE 0x40

#ifdef SPORT_HEALTH_LOG_COMPRESS
#include "sport_health_logging.h"
typedef struct {
	uint8 num_packets;              // total number of packets we will serialize excluding the meta data packet
	uint8 num_packets_uncompressed; // for information only: the number of packets we would have used without compression
	uint8 packet_number;            // number of current packet (0 is the meta then 1...num_packets)
	uint8 bits_x;                   // max bits for dx to be compressible where dx = abs(x-xprev)
	uint8 bits_y;                   // max bits for dy to be compressible where dy = abs(y-yprev)
	uint8 bits_z;                   // max bits for dz to be compressible where dx = abs(z-zprev)
	uint8 num_samples_packed;       // number of samples that have been serialized
	uint8 n_bytes_remaining;        // how many bytes of the current sample remaining to be serialized
	uint8 sample[6];                // current sample to be serialized
} sh_log_compress_imu_info;

/* Compress meta-data, updating the compression info and filling in the payload. Returns size of payload data or 0 if there is an error */
uint8 SportHealthLoggingCompressImuHeader(const imu_sensor_data_t * p_data, sh_log_compress_imu_info * info, uint8 * payload);
/* Compress data, updating the compression info and filling in the payload. Returns size of payload data or 0 if there is an error or compression is done */
uint8 SportHealthLoggingCompressImuData(const imu_sensor_data_t * p_data, sh_log_compress_imu_info * info, uint8 * payload);
#endif
#endif /* SPORT_HEALTH_LOG_COMPRESS_H */

