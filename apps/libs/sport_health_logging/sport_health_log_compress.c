/**  Copyright (c) 2018 Qualcomm Technologies International, Ltd. */
/**     */
/** *
 * \file
 * Sport Health log compression functions
 * The file also includes decompression and test functions, but these are not used on the device so are not compiled by default
 */
#ifdef SPORT_HEALTH_LOG_COMPRESS

#include "sport_health_log_compress.h"

/* Mapping of compression format to number of bits used to encode differences in x,y,z.
   The data usually has greater differences in one or two directions than the third
*/
#define SH_LOG_MAX_FORMAT_CODE 6
static const uint8 sh_log_compress_bits_x[SH_LOG_MAX_FORMAT_CODE] = { 10,  10,  9, 11,  9,  9};
static const uint8 sh_log_compress_bits_y[SH_LOG_MAX_FORMAT_CODE] = { 10,   9, 10,  9, 11,  9 };
static const uint8 sh_log_compress_bits_z[SH_LOG_MAX_FORMAT_CODE] = {  9,  10, 10,  9,  9, 11 };
#ifndef XYZ_SIZE
#define XYZ_SIZE 3
#endif
#ifndef ABS
#define ABS(X) ((X)>=0?(X):(-(X)))
#endif

static uint8 sh_log_bits_to_format(sh_log_compress_imu_info* info);
static void sh_log_increment_bits_used(const imu_sensor_data_t * p_data, sh_log_compress_imu_info* info);


/* Given how many bits are used for each axis determine the format code */
uint8 sh_log_bits_to_format(sh_log_compress_imu_info* info)
{
	uint8 format;
	for (format = 0; format < SH_LOG_MAX_FORMAT_CODE; format++) {
		if (info->bits_x == sh_log_compress_bits_x[format] && 
            info->bits_y == sh_log_compress_bits_y[format] && 
            info->bits_z == sh_log_compress_bits_z[format]) {
			return format;
		}
	}
/* If we get here there is no match. Return an invalid code */
	return SH_LOG_MAX_FORMAT_CODE;
}

/* look over the provided sample data counting the number of which would fail compression due to each axis
   then increment that axis that causes the most fails by one bit thereby doubling its range
*/
void sh_log_increment_bits_used(const imu_sensor_data_t * p_data, sh_log_compress_imu_info* info) 
{
	uint8 n, count_x, count_y, count_z;
	int16 prev_x=0, prev_y=0, prev_z=0, x, y, z;
    uint16 xlimit = (1 << info->bits_x);
    uint16 ylimit = (1 << info->bits_y);
    uint16 zlimit = (1 << info->bits_z);
    int16* p_imu_data = *(p_data->accel.data.p_imu_data);
    if (p_data->accel.frame_count == 0) {
        return;
    }
	count_x = count_y = count_z = 0;
	prev_x = p_imu_data[0];
	prev_y = p_imu_data[1];
	prev_z = p_imu_data[2];
	for (n = 1; n < p_data->accel.frame_count; n++) {
		x = p_imu_data[n*XYZ_SIZE + 0];
		y = p_imu_data[n*XYZ_SIZE + 1];
		z = p_imu_data[n*XYZ_SIZE + 2];
		if (ABS(x - prev_x) > xlimit) {
            count_x++;
        }
		if (ABS(y - prev_y) > ylimit) {
            count_y++;
        }
		if (ABS(z - prev_z) > zlimit)  {
            count_z++;
        }
		prev_x = x;
		prev_y = y;
		prev_z = z;
	}
    /* Depending on which axis had the most overflows, increase the bit width used on that axis */
	if (count_x >= count_y && count_x >= count_z)
	{
		(info->bits_x)++;
	}
	else if (count_y >= count_x && count_y >= count_z)
	{
		(info->bits_y)++;
	}
	else
	{
		(info->bits_z)++;
	}
}

/* given the imu data determine the payload (serialized header information to be sent over BLE)
    and also populate the info structure that contains information needed by the data compression 
    part of the algorithm.
    payload[0] contains the packet id which is SH_LOG_COMPRESSED_IMU_BASE_TYPE
	payload[1] contains the packet length
	info contains the number of packets that will be needed in total (num_packets) so it is 
	known after calling this function how many times to call the data compression function
    Returns size of payload (or 0 if error)
*/
uint8 SportHealthLoggingCompressImuHeader(const imu_sensor_data_t * p_data, sh_log_compress_imu_info* info, uint8* payload) 
{
	uint8 format_bits, bytes_packed, num_header_bytes;
	uint8 compress, num_compressed;
	uint8 n_bytes_bit_field, current_byte, current_bit;
	int16 prev_x=0, prev_y=0, prev_z=0, x, y, z, total_bytes, n;
    int16* p_imu_data = *(p_data->accel.data.p_imu_data);

	/* It is the first packet then pack the header 
	   initialize the info structure, apart from the total number of packets which is not yet known
    */
	info->num_samples_packed = 0;
	info->n_bytes_remaining = 0;
	info->num_packets = 0;
	info->packet_number = 0;
	info->bits_x = info->bits_y = info->bits_z = 9;

	/* set the number of bits to be used for each dimension - call the function twice as up to 11 bits may be needed */
	sh_log_increment_bits_used(p_data, info);
	sh_log_increment_bits_used(p_data, info);
    /* Now work out the compression format */
	format_bits = sh_log_bits_to_format(info);
    uint16 xlimit = (1 << info->bits_x);
    uint16 ylimit = (1 << info->bits_y);
    uint16 zlimit = (1 << info->bits_z);

	/* do not compress if out of representable range */
	if (p_data->accel.frame_count > SH_LOG_COMPRESS_MAX_SAMPLES || p_data->accel.range > 32 || p_data->accel.range == 0 || format_bits == SH_LOG_MAX_FORMAT_CODE) {
		return 0;
	}

	/* populate the metadata packet payload
       If range is equal to 32 then we represent this as zero (since zero is not a valid range value)
    */
	payload[0] = SH_LOG_COMPRESSED_IMU_BASE_TYPE; /* Packet number zero */
	payload[1] = GATT_LOGGING_PAYLOAD_SIZE_BYTES;
	payload[2] = (uint8)((p_data->accel.last_sample_time        & 0xFF));
	payload[3] = (uint8)((p_data->accel.last_sample_time >> 8)  & 0xFF);
	payload[4] = (uint8)((p_data->accel.last_sample_time >> 16) & 0xFF);
	payload[5] = (uint8)((p_data->accel.last_sample_time >> 24) & 0xFF);
    payload[6] = p_data->accel.sampling_interval;                          /* Sampling interval in ms */
	payload[7] = (format_bits << 5) + ((uint8)p_data->accel.range & 0x1F); /* lowest 5 bits, acc range in g, top 3 bits = format for the bit field */
	payload[8] = (uint8)p_data->accel.frame_count;                         /*  number of data samples */ 
	num_header_bytes = 9;

	/* work out the required number of bytes for the sample bitfield */
	n_bytes_bit_field = (SH_LOG_COMPRESS_MAX_SAMPLES >> 3);
	if ((n_bytes_bit_field << 3) < SH_LOG_COMPRESS_MAX_SAMPLES) 
        n_bytes_bit_field++;
	/* check if needed length exists: if it does not exit with a failure */
	if ((GATT_LOGGING_TOTAL_PACKET_SIZE - num_header_bytes)  < n_bytes_bit_field) 
        return 0;
	/* initialize the bitfield to zero */
	for (bytes_packed = num_header_bytes; bytes_packed < (num_header_bytes + n_bytes_bit_field); bytes_packed++) {
		payload[bytes_packed] = 0;
	}
	payload[1] = bytes_packed;
	/* now populate the bit field and count the total number of packets  */
	prev_x = p_imu_data[0];
	prev_y = p_imu_data[1];
	prev_z = p_imu_data[2];
	num_compressed = 0;
	for (n = 1; n < p_data->accel.frame_count; n++) {
		x = p_imu_data[n*XYZ_SIZE + 0];
		y = p_imu_data[n*XYZ_SIZE + 1];
		z = p_imu_data[n*XYZ_SIZE + 2];
		compress = (ABS(x - prev_x) < xlimit) &&
                   (ABS(y - prev_y) < ylimit) &&
                   (ABS(z - prev_z) < zlimit);
		prev_x = x;
		prev_y = y;
		prev_z = z;
		if (compress) {
            num_compressed++;
        }
		current_byte = (n >> 3);
		current_bit = 7 - (n - (current_byte << 3));
        /* Set a bit for each compressed set of samples */
		payload[num_header_bytes + current_byte] |= (compress << current_bit);
	}
    /* Each compressed set of samples fits in 4 bytes, and each uncompressed set requires 6 */
	total_bytes = num_compressed * 4 + (p_data->accel.frame_count - num_compressed) * 6;
    /* Count the numbers of compressed and uncompressed packets in this set of samples */
	info->num_packets = total_bytes / GATT_LOGGING_PAYLOAD_SIZE_BYTES;
	if (info->num_packets*GATT_LOGGING_PAYLOAD_SIZE_BYTES < total_bytes) {
        info->num_packets++;
    }
	info->num_packets_uncompressed = (p_data->accel.frame_count * 6) / GATT_LOGGING_PAYLOAD_SIZE_BYTES;
	if (info->num_packets_uncompressed*GATT_LOGGING_PAYLOAD_SIZE_BYTES < (p_data->accel.frame_count * 6)) {
        info->num_packets_uncompressed++;
    }
	return bytes_packed;
}

/* serialize and compress the next packet into payload based on the provided imu data and the info 
   payload[0] contains the packet id which is SH_LOG_COMPRESSED_IMU_BASE_TYPE + the packet counter (first data packet is 1, header is 0)
   payload[1] contains the packet length in bytes
*/
uint8 SportHealthLoggingCompressImuData(const imu_sensor_data_t * p_data, sh_log_compress_imu_info* info, uint8* payload) {
	uint8 bytes_packed;
	uint8 compress;
	int16 prev_x=0, prev_y=0, prev_z=0, x, y, z, dx, dy, dz, n;
	uint32 wk;
    /* Limits are for the unsigned differences */
    uint16 xlimit = (1 << info->bits_x);
    uint16 ylimit = (1 << info->bits_y);
    uint16 zlimit = (1 << info->bits_z);
    /* Corresponding masks for each compression limit - we shift left an extra bit to account for the sign bit */
    uint16 xmask = (xlimit << 1) - 1;
    uint16 ymask = (ylimit << 1) - 1;
    uint16 zmask = (zlimit << 1) - 1;
    int16* p_imu_data = *(p_data->accel.data.p_imu_data);
	/* do not compress if out of representable range */
	if (p_data->accel.frame_count > SH_LOG_COMPRESS_MAX_SAMPLES) {
		return 0;
	}
	/* compress samples into byte stream */
	info->packet_number++;                                    
	if (info->packet_number > info->num_packets) { /* Stop if we've done all the data */
        return 0;
    }
	payload[0] = SH_LOG_COMPRESSED_IMU_BASE_TYPE + info->packet_number;
	payload[1] = GATT_LOGGING_PAYLOAD_SIZE_BYTES;  /* populate temporarily with the max until we know the exact number */
	bytes_packed = GATT_LOGGING_HEADER_SIZE;                                                
    /* if some of the previous sample is outstanding complete it */
	if (info->n_bytes_remaining > 0) {
		for (n = (info->n_bytes_remaining - 1); (n >= 0) && (bytes_packed < GATT_LOGGING_TOTAL_PACKET_SIZE); n--) {
			payload[bytes_packed] = info->sample[n];
			info->n_bytes_remaining--;
			bytes_packed++;
		}
		info->num_samples_packed++;
	}
	/* main loop over samples */
	while (bytes_packed < GATT_LOGGING_TOTAL_PACKET_SIZE && info->num_samples_packed < p_data->accel.frame_count)
	{
		/* advance to next sample */
		x = p_imu_data[info->num_samples_packed*XYZ_SIZE + 0];
		y = p_imu_data[info->num_samples_packed*XYZ_SIZE + 1];
		z = p_imu_data[info->num_samples_packed*XYZ_SIZE + 2];
		/* determine whether to compress sample by comparing it with the previous one */
		compress = 0;
		if (info->num_samples_packed > 0) {
			prev_x = p_imu_data[(info->num_samples_packed - 1)*XYZ_SIZE + 0];
			prev_y = p_imu_data[(info->num_samples_packed - 1)*XYZ_SIZE + 1];
			prev_z = p_imu_data[(info->num_samples_packed - 1)*XYZ_SIZE + 2];
			compress = (ABS(x - prev_x) < xlimit) &&
                       (ABS(y - prev_y) < ylimit) &&
                       (ABS(z - prev_z) < zlimit);
		}
		/* Note that when we put the data into the byte array we will do so in the opposite order to the sample array
		   decrementing the number of bits remaining as we do so.  Hence for example for compressed data sample[3] will
		   be the first byte added to the byte array
        */
		if (compress) {  /* use 4 bytes for compressible sample */
						 /* example 1: positive numbers bit_x = 10, bit_y = 10, bit_z = 9 (add one bit for sign)
						   dx = 176; //   000 1011 0000  
						   dy = 602; //   010 0101 1010
						   dz = 401; //    01 1001 0001
						   result should be  [0110 0100 01][01 0010 1101 0][000 1011 0000] = 1683148976
                         example 2: negative numbers
                           dx = -176; //   111 0101 0000  
						   dy = -602; //   101 1010 0110
						   dz = -401; //    10 0110 1111
						   result should be = [1001 1011 11][10 1101 0011 0][111 0101 0000] = 2616014672
                         */
            /* We've already checked the numbers will fit, so we can safely mask off the bits we need */
			dx = (x - prev_x) & xmask;
			dy = (y - prev_y) & ymask;
			dz = (z - prev_z) & zmask;
            /* bits_x is the number of bits for the unsigned diff. We need an extra bit for the sign, likewise for y */
			wk = (((uint32)dz) << (info->bits_x + info->bits_y + 2)) + (((uint32)dy) << (info->bits_x + 1)) + ((uint32)dx);
			info->sample[3] = (uint8)(wk & 0xFF); /* This will go in the payload buffer first */
			info->sample[2] = (uint8)((wk >> 8) & 0xFF);
            info->sample[1] = (uint8)((wk >> 16) & 0xFF);
			info->sample[0] = (uint8)((wk >> 24) & 0xFF);
			info->n_bytes_remaining = 4;
		}
		else {                                   /* use 6 bytes for full sample */
			info->sample[5] = (uint8)(x & 0xFF); /* this will go into the payload buffer first */
			info->sample[4] = (uint8)(x >> 8);
			info->sample[3] = (uint8)(y & 0xFF);
			info->sample[2] = (uint8)(y >> 8);
			info->sample[1] = (uint8)(z & 0xFF);
			info->sample[0] = (uint8)(z >> 8);
			info->n_bytes_remaining = 6;
		}
		/* copy result into the buffer, little-endian */
		for (n = (info->n_bytes_remaining - 1); (n >= 0) && (bytes_packed < GATT_LOGGING_TOTAL_PACKET_SIZE); n--) {
			payload[bytes_packed] = info->sample[n];
			info->n_bytes_remaining--;
			bytes_packed++;
		}
		if (info->n_bytes_remaining == 0) {
            info->num_samples_packed++;
        }
	}
    /* Finally fill in the packet size */
	payload[1] = bytes_packed;
	return bytes_packed;
}

#endif
