/**  Copyright (c) 2018 Qualcomm Technologies International, Ltd. */
/**     */
/** *
 * \file
 * Sport Health log decompression and test functions
 * These are not used on the device so are not compiled by default
 */
#include "sport_health_log_decompress.h"

/* Mapping of compression format to number of bits used to encode differences in x,y,z.
   The data usually has greater differences in one or two directions than the third
*/
#define SH_LOG_MAX_FORMAT_CODE 6
static const uint8 sh_log_decompress_bits_x[SH_LOG_MAX_FORMAT_CODE] = { 10,  10,  9, 11,  9,  9};
static const uint8 sh_log_decompress_bits_y[SH_LOG_MAX_FORMAT_CODE] = { 10,   9, 10,  9, 11,  9 };
static const uint8 sh_log_decompress_bits_z[SH_LOG_MAX_FORMAT_CODE] = {  9,  10, 10,  9,  9, 11 };
#ifndef XYZ_SIZE
#define XYZ_SIZE 3
#endif
#ifndef ABS
#define ABS(X) ((X)>=0?(X):(-(X)))
#endif

static void format_to_bits(uint8 format, sh_log_decompress_imu_info * info); 

/* Convert a compression format to number of bits used for diff in each axis */
void format_to_bits(uint8 format, sh_log_decompress_imu_info * info) 
{
	if (format >= SH_LOG_MAX_FORMAT_CODE) { 
        format = 0;
    }
	info->bits_x = sh_log_decompress_bits_x[format];
	info->bits_y = sh_log_decompress_bits_y[format];
	info->bits_z = sh_log_decompress_bits_z[format];	
}

/* given the a header payload (i.e.SH_LOG_COMPRESSED_IMU_BASE_TYPE) deserialize it putting the results into p_data and also info
   info contains num_packets which gives the number of data packets expected 
   Return number of decompressed packets
*/
uint8 SportHealthLoggingDecompressImuHeader(imu_sensor_data_t * p_data, sh_log_decompress_imu_info * info, const uint8 * payload)
{
	int num_bytes, length_bitfield_bytes, num_compressed, total_bytes, n;
	uint8 current_byte, current_bit, num_header_bytes, compress;

	/* extract the stored meta data from the header payload */
	if (payload[0] != SH_LOG_COMPRESSED_IMU_BASE_TYPE) {
		return 0;
	}
	num_bytes = payload[1];
	p_data->accel.last_sample_time = (payload[5] << 24) + (payload[4] << 16) + (payload[3] << 8) + payload[2];
	p_data->accel.sampling_interval = payload[6];
	p_data->accel.range = payload[7] & 0x1F;
	if (p_data->accel.range == 0) p_data->accel.range = 32; 
	p_data->accel.frame_count = payload[8];
	num_header_bytes = 9;
	length_bitfield_bytes = 0;
	for (n = num_header_bytes; n < num_bytes; n++) {
		info->bitfield[n - num_header_bytes] = payload[n];
		length_bitfield_bytes++;
	}

	/* figure out the compression format */
	format_to_bits(payload[7] >> 5, info);
	/* count the expected packets based on the bitfield */
	num_compressed = 0;
	for (n = 0; n < p_data->accel.frame_count; n++) {
		current_byte = (n >> 3);
		current_bit = 7 - (n - (current_byte << 3));
		compress = payload[num_header_bytes + current_byte] & (1 << current_bit);
		if (compress > 0) { 
            num_compressed++;
        }
	}
	total_bytes = num_compressed * 4 + (p_data->accel.frame_count - num_compressed) * 6;
	info->num_packets = total_bytes / GATT_LOGGING_PAYLOAD_SIZE_BYTES;
	if (info->num_packets*GATT_LOGGING_PAYLOAD_SIZE_BYTES < total_bytes) {
        info->num_packets++;
    }
	info->last_packet_received = 0;
	info->unzip_byte_buffer_len = 0;
	info->uzip_complete = 0;
	return info->num_packets;
}

/* given  a data payload (i.e. SH_LOG_COMPRESSED_IMU_BASE_TYPE) deserialize it putting the results into p_data 
   requires that data packets are given to it in the right order.  It will assemble a big array of 
   bytes and then unzip it once the last packet has been received since memory is not such an issue off device
   Return number of decompressed samples
*/
uint8 SportHealthLoggingDecompressImuData  (imu_sensor_data_t * p_data, sh_log_decompress_imu_info * info, const uint8 * payload)
{
	int packet_number, expected_packet_number, buffer_byte;
	int current_byte_bitfield, current_bit_bitfield, sign_ext;
	uint8 compress;
	int16 delta;
	uint32 wk, dx, dy, dz, n;
    int16* p_imu_data = *(p_data->accel.data.p_imu_data);

	/* check the packets are going up as expected, otherwise return a failure */
	packet_number = payload[0] - SH_LOG_COMPRESSED_IMU_BASE_TYPE;
	expected_packet_number = info->last_packet_received + 1;
	if (packet_number != expected_packet_number) {
        return 0;
    }
	info->last_packet_received = expected_packet_number;

	/* append data into an array */
	if (payload[1] > GATT_LOGGING_PAYLOAD_SIZE_BYTES) {
        return 0;
    }

	for (n = 2; n < payload[1]; n++) {
		info->unzip_byte_buffer[info->unzip_byte_buffer_len] = payload[n];
		info->unzip_byte_buffer_len++;
	}

	/* only actually do the populating on the last packet */
	if (packet_number < info->num_packets) {
        return packet_number;
    }

	/* when we reach here we have the data from all the packets in a byte array */
	buffer_byte = 0;
	for (n = 0; n < p_data->accel.frame_count; n++) {
		current_byte_bitfield = (n >> 3);
		current_bit_bitfield = 7 - (n - (current_byte_bitfield << 3));
		compress = info->bitfield[current_byte_bitfield] & (1 << current_bit_bitfield);
		if (n == 0 && (compress > 0)) {
            return 0; /* first sample cannot be compressed */
        }
		if (compress > 0) {
			wk = info->unzip_byte_buffer[buffer_byte + 0] + (info->unzip_byte_buffer[buffer_byte + 1] << 8) +
				(info->unzip_byte_buffer[buffer_byte + 2] << 16) + (info->unzip_byte_buffer[buffer_byte + 3] << 24);
			dx = (wk & ((1 << (info->bits_x + 1)) - 1));
			sign_ext = 32 - (info->bits_x + 1);
			delta = (int16)(((int32)(dx << sign_ext)) >> sign_ext);
			p_imu_data[n*XYZ_SIZE + 0] = p_imu_data[(n - 1)*XYZ_SIZE + 0] + delta;

			wk = wk >> (info->bits_x + 1);
			dy = wk & ((1 << (info->bits_y + 1)) - 1);
			sign_ext = 32 - (info->bits_y + 1);
			delta = (int16)(((int32)(dy << sign_ext)) >> sign_ext);
			p_imu_data[n*XYZ_SIZE + 1] = p_imu_data[(n - 1)*XYZ_SIZE + 1] + delta;

			wk = wk >> (info->bits_y + 1);
			dz = wk & ((1 << (info->bits_z + 1)) - 1);
			sign_ext = 32 - (info->bits_z + 1);
			delta = (int16)(((int32)(dz << sign_ext)) >> sign_ext);
			p_imu_data[n*XYZ_SIZE + 2] = p_imu_data[(n - 1)*XYZ_SIZE + 2] + delta;

			buffer_byte += 4;
		}
		else {
			p_imu_data[n*XYZ_SIZE + 0] = info->unzip_byte_buffer[buffer_byte + 0] + (info->unzip_byte_buffer[buffer_byte + 1] << 8);
			p_imu_data[n*XYZ_SIZE + 1] = info->unzip_byte_buffer[buffer_byte + 2] + (info->unzip_byte_buffer[buffer_byte + 3] << 8);
			p_imu_data[n*XYZ_SIZE + 2] = info->unzip_byte_buffer[buffer_byte + 4] + (info->unzip_byte_buffer[buffer_byte + 5] << 8);
			buffer_byte += 6;
		}
	}
	info->uzip_complete = 1;
	return packet_number;
}

#ifdef SH_LOG_COMPRESS_TEST
/* This is for use on a desktop machine to validate the code */
#include "math.h"
#include <stdio.h>

static int8 sh_log_compare_data(const imu_sensor_data_t * p_data, const imu_sensor_data_t * p_data_ref);

/* Compare two blocks of data to make sure they are identical */
/* returns 1 if the two structues match and otherwise returns 0 */
int8 sh_log_compare_data(const imu_sensor_data_t * p_data, const imu_sensor_data_t * p_data_ref) 
{
	int8 chk, chkx, chky, chkz;
    int n;
	chk = 1;
	chk &= (p_data->accel.last_sample_time == p_data_ref->last_sample_time);
	chk &= (p_data->accel.frame_count == p_data_ref->frame_count);
	chk &= (p_data->accel.range == p_data_ref->range);
	chk &= (p_data->accel.sampling_interval == p_data_ref->sampling_interval);
	if (chk == 0) return chk;
	for (n = 0; n < p_data->accel.frame_count; n++) {
		chkx = (p_data->accel.data.p_imu_data[n*XYZ_SIZE + 0] == p_data_ref->accel.data.p_imu_data[n*XYZ_SIZE + 0]);
		chky = (p_data->accel.data.p_imu_data[n*XYZ_SIZE + 1] == p_data_ref->accel.data.p_imu_data[n*XYZ_SIZE + 1]);
		chkz = (p_data->accel.data.p_imu_data[n*XYZ_SIZE + 2] == p_data_ref->accel.data.p_imu_data[n*XYZ_SIZE + 2]);
		chk = chk & chkx & chky & chkz;
	}
	return chk;
}

int8 sh_log_test_compression(imu_sensor_data_t * p_data) {
	/* example usage of the comprssion and uncompression
	   will compress the given data into packets and then decompress it into a new data structure
	   and validate that the new and original match
    */
	static uint8 payload[GATT_LOGGING_TOTAL_PACKET_SIZE];   /* storage for 1 packets worth of data */
	uint8 imu_err;                       /* result from compression/uncompression algorithms */
	uint8 ok;                                        /* final validation result */
	static sh_log_compress_info compress_info;              /* info about how far through the compression we are */
	static sh_log_decompress_info decompress_info;          /* info about decompression obtained from header */
	static imu_sensor_data_t decompressed_imu_data;         /* storage for the final unzipped data */
	static int16 decompress_sample_buffer[MAX_SAMPLES * 6]; /* create storage for unzipped result */
    int n;
	decompressed_imu_data.accel.data.p_imu_data = decompress_sample_buffer;  /* attach the sample buffer to the struct */

	imu_err = imu_compress_header(p_data, &compress_info, payload);
	imu_err = imu_uncompress_header(&decompressed_imu_data, &decompress_info, payload);

	for (n = 0; n < zip_info.num_packets; n++) {
		imu_err = imu_compress_data(p_data, &compress_info, payload);

		// payload would be sent over the air.  We assume that we receive it and call imu_uncompress_data
		// Note we also know on the receiving side how many packets to expect (uzip_info). 
		imu_err = imu_uncompress_data(&decompressed_imu_data, &decompress_info, payload);
	}

	ok = validate_data_transfer(&decompressed_imu_data, p_data);

	if (ok) {
		// add one to include the header packet in the calculation
	//	printf("encode/decode successful: compression acheieved = %.1f%%[%d->%d]\n",
	//		100.0 - (((float)100 * (zip_info.num_packets+1)) / (zip_info.num_packets_uncompressed+1)),
	//		(zip_info.num_packets_uncompressed+1), (zip_info.num_packets+1));
	}
	else {
		printf("encode/decode: FAILED: timestamp = %d\n", p_data->accel.last_sample_time);
	}

	return ok;
}

#endif
