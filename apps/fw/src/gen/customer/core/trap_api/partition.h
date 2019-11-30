#ifndef __PARTITION_H__
#define __PARTITION_H__
#include <app/partition/partition_if.h>
#include <sink_.h>
#include <source_.h>



#if TRAPSET_PARTITION

/**
 *  \brief Mount a partition to the union file system
 *   
 *  \param device device to which to mount, see \#partition_filesystem_devices 
 *  \param partition number of partition to mount 
 *  \param priority mount at higher or lower priority to already mounted file systems, see
 *  \#partition_filesystem_priority
 *  \return TRUE if partition found and mounted successfully, otherwise FALSE
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_partition
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
bool PartitionMountFilesystem(partition_filesystem_devices device, uint16 partition, partition_filesystem_priority priority);

/**
 *  \brief Get information about a partition
 *   
 *  \param device device to query, see \#partition_filesystem_devices 
 *  \param partition number of partition to query 
 *  \param key specifies information requested, see \#partition_info_key 
 *  \param value returned value as specified by \#key
 *  \return TRUE if partition found and queried successfully, otherwise FALSE
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_partition
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
bool PartitionGetInfo(partition_filesystem_devices device, uint16 partition, partition_info_key key, uint32 * value);

/**
 *  \brief Set a message digest for a stream writing to flash partition. Message digest
 *  data will be copied and stored by the firmware. When a flash partition is
 *  written to, the first word is not immediately written. Instead it is saved in
 *  RAM until the stream is closed. At this point the flash partition is read back
 *  and verified against the stored message digest. If the verification is
 *  successful, the first word is written to flash. This protects against partly
 *  or incorrectly written partitions. If this trap is not called against a
 *  partition write sink, the verification will not be performed, but the first
 *  word will not be written until the stream is closed, protecting against partly
 *  written partitions.
 *   
 *  \param sink sink that is writing to the partition 
 *  \param md_type the type of message digest, see \#partition_message_digest_type 
 *  \param data pointer to message digest 
 *  \param len length of message digest
 *  \return TRUE if sink is valid, data is correct length and message digest set
 *  successfully, otherwise FALSE
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_partition
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
bool PartitionSetMessageDigest(Sink sink, partition_message_digest_type md_type, uint16 * data, uint16 len);

/**
 *  \brief This API will return the source with the contents of the specified raw serial
 *  partition 
 *   
 *  \param device device to query, see \#partition_filesystem_devices 
 *  \param partition number of partition to read raw data
 *  \return The source associated with the raw partition stream.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_partition
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
Source PartitionGetRawSerialSource(uint16 device, uint16 partition);

/**
 *  \brief Get the sink position of the partition sink stream.
 *   
 *   This VM trap is used to get the sink position of the specified sink partition
 *  in 
 *   octets. Firmware returns a sink position from which, each word has a value of 
 *   0xFFFF till the end of the partition. Firmware assumes that the partition 
 *   contents before the sink position (except first word) are successfully
 *  written.
 *  \param sink sink stream.
 *  \return sink position of the specified sink partition.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_partition
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
uint32 PartitionSinkGetPosition(Sink sink);
#endif /* TRAPSET_PARTITION */
#endif
