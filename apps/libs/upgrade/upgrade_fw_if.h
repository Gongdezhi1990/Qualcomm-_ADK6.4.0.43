/****************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    upgrade_fw_if.h

DESCRIPTION
    Interface to functions which (largely) interact with the firmware.

*/
#ifndef UPGRADE_FW_IF_H_
#define UPGRADE_FW_IF_H_

#include <csrtypes.h>
#include <sink.h>

#include "imageupgrade.h"

#include "upgrade_msg_host.h"

/*!
    @brief Enumeration of the types of partition data which can be handled.
*/
typedef enum
{
    /*! VM Executable Partition */
    UPGRADE_FW_IF_PARTITION_TYPE_EXE = 0x0,
    /*! DFU File Partition */
    UPGRADE_FW_IF_PARTITION_TYPE_DFU = 0x1,
    /*! PSFS Configuration Data */
    UPGRADE_FW_IF_PARTITION_TYPE_CONFIG = 0x2,
    /*! Standard Data on a read only (RO) partition */
    UPGRADE_FW_IF_PARTITION_TYPE_DATA = 0x3,
    /*! Standard Data on a raw serial (RS) partition */
    UPGRADE_FW_IF_PARTITION_TYPE_DATA_RAW_SERIAL = 0x4,

    UPGRADE_FW_IF_PARTITION_TYPE_NUM
} UpgradeFWIFPartitionType;

/*!
    @brief Enumeration of the status which can be returned from
           UpgradeFWIFValidateExecutablePartition.
*/
typedef enum
{
    UPGRADE_FW_IF_PARTITION_VALIDATION_TRIGGERED,   /*!< Partition was executable, validation started */
    UPGRADE_FW_IF_PARTITION_VALIDATION_IN_PROGRESS, /*!< Validation of another partition still in progress */
    UPGRADE_FW_IF_PARTITION_VALIDATION_SKIP         /*!< Not an executable partition */
} UpgradeFWIFPartitionValidationStatus;

/*!
    @brief Enumeration of the status which can be returned from
           UpgradeFWIFValidateApplication.
*/
typedef enum
{
    UPGRADE_FW_IF_APPLICATION_VALIDATION_RUNNING,   /*!< Application partition validation in progress */
    UPGRADE_FW_IF_APPLICATION_VALIDATION_PASS,      /*!< Application partition validation passed */
    UPGRADE_FW_IF_APPLICATION_VALIDATION_SKIP       /*!< Application partition validation not required */
} UpgradeFWIFApplicationValidationStatus;

/*!
    @brief An opaque handle to an writeable external flash partition.
*/
typedef Sink UpgradeFWIFPartitionHdl;

/*!
    @brief Checks if the audio dfu bit is set in the partition map.

    @return bool TRUE if the bit audio dfu is set.
*/
bool UpgradeFWIFAudioDFUExists(void);

/*!
    @brief Initialise the context for the Upgrade FW IF.
*/
void UpgradeFWIFInit(void);

/*!
    @brief Get the identifier for the header of an upgrade file.

    @return String containing the header ID.
*/
const char *UpgradeFWIFGetHeaderID(void);

/*!
    @brief Get the identifier for a partition header within an upgrade file.

    @return String containing the partition header ID.
*/
const char *UpgradeFWIFGetPartitionID(void);

/*!
    @brief Get the identifier for the footer of an upgrade file.

    @return String containing the footer ID.
*/
const char *UpgradeFWIFGetFooterID(void);

/*!
    @brief Get the identifier for the current device variant.

    @return String containing the device variant.
*/
const char *UpgradeFWIFGetDeviceVariant(void);

/*!
    @brief Get the current (running) app version.

    @todo This shouldn't be implemented in upgrade library, and is now little used.

    @return The running app version.
*/
uint16 UpgradeFWIFGetAppVersion(void);

/*!
    @brief Find the physical partition corresponding to a logical partition in a bank.

    @param logicPartition Logical partition number, from the upgrade file partition section header.

    @return uint16 Identifier for the physical partition corresponding to the logical 
                partition. Note that the ID is the full ID from FSTAB, including the 
                flash device ID. Use UPGRADE_PARTITION_PHYSICAL_PARTITION() to get the
                specific ID.
*/
uint16 UpgradeFWIFGetPhysPartition(uint16 logicPartition);

/*!
    @brief Find the number of physical partitions on the serial flash.

    @return uint16 Number of partitions.
*/
uint16 UpgradeFWIFGetPhysPartitionNum(void);

/*!
    @brief Find the size of a physical partition in bytes.

    @param physPartition Number of the physical partition for which the size will be returned.

    @return uint32 Size of partition physPartition in bytes.
*/
uint32 UpgradeFWIFGetPhysPartitionSize(uint16 physPartition);

/*!
    @brief Determine if the partition type is a valid type.

    @param type Partition type.
    @param physPartition Physical partition this partition is intended for.

    @return bool TRUE if type is valid, FALSE if type is invalid.
*/
bool UpgradeFWIFValidPartitionType(UpgradeFWIFPartitionType type, uint16 physPartition);

/*!
    @brief Open a physical partition in external flash for writing.

    @param logic logic
    @param physPartition Index of the physical partition to open.
    @param firstWord First word of partition data.

    @return Valid UpgradeFWIFPartitionHdl if ok, zero otherwise.
*/
UpgradeFWIFPartitionHdl UpgradeFWIFPartitionOpen(uint16 logic,uint16 physPartition, uint16 firstWord);

/*!
    @brief Write data to an open external flash partition.
           Each byte of the data is copied to the partition
           in a byte by byte copy operation.

    @param handle Handle to a writeable partition.
    @param data Pointer to the data buffer to write.
    @param len Number of bytes (not words) to write.

    @return The number of bytes written, or 0 if there was an error.
*/
uint16 UpgradeFWIFPartitionWrite(UpgradeFWIFPartitionHdl handle, uint8 *data, uint16 len);

/*!
    @brief Close a handle to an external flash partition.

    @param handle Handle to a writeable partition.

    @return UPGRADE_HOST_SUCCESS if close successful, 
            UPGRADE_HOST_ERROR_PARTITION_CLOSE_FAILED_PS_SPACE if PS space is critical
            UPGRADE_HOST_ERROR_PARTITION_CLOSE_FAILED in (unlikely) error scenario
            
*/
UpgradeHostErrorCode UpgradeFWIFPartitionClose(UpgradeFWIFPartitionHdl handle);

/*!
    @brief Initialise the context for validating the MD5 checksum of some data.
           Only one context is supported at a time.
*/
void UpgradeFWIFValidateInit(void);

/*!
    @brief Update the validation context with the next set of data. The data
           is expected to be "unpacked", i.e. one (8-bit) byte of  data
           per (16-bit) word. e.g. in memory, { 0x00AE, 0x002D, ... }.

    @param buffer Pointer to the next set of data.
    @param len length of the data.

    @return TRUE if a validation context is updated successfully, FALSE otherwise.
*/
bool UpgradeFWIFValidateUpdate(uint8 *buffer, uint16 len);

/*!
    @brief Start verify the accumulated data in the validation context against
           the given signature. The signature is a sequence of 128 bytes
           packed into 64 16-bit words.

    @param vctx P0 Hash context.

    @return Status code.
*/
UpgradeHostErrorCode UpgradeFWIFValidateStart(hash_context_t *vctx);

/*!
    @brief Finish verify the accumulated data in the validation context against
           the given signature. The signature is a sequence of 128 bytes
           packed into 64 16-bit words.

    @param vctx P0 Hash context.
    @param signature Signature sequence.

    @return TRUE if a validation was successful, FALSE otherwise.
*/
bool UpgradeFWIFValidateFinish(hash_context_t *vctx, uint8 *signature);

/*!
    @brief Add the data from an existing partition to the validation context.

    This function is used when resuming an upgrade to add partition data that
    has already been downloaded and written to flash to the validation context.

    This ensures that the calculated validation hash is the same as if the
    upgrade had not been resumed at this point.

    It can add data from partitions that are either complete or partially
    complete. The only restriction is that the partition is in external flash.

    @param [in] partition Physical partition number.
    @param [in] offset Number of octets to skip from the beginning of the partition data.
    @param [out] read Total number of octets added to the validation context.
    
    @return TRUE if the partition was successfully opened and read; FALSE otherwise.
*/
bool UpgradeFWIFValidateAddPartitionData(uint16 partition, uint32 offset, uint32 *read);

/*!
    @brief Initiate validation of a partition which may be executable.

    @return UpgradeFWIFPartitionValidationStatus status of validation.
*/
UpgradeFWIFPartitionValidationStatus UpgradeFWIFValidateExecutablePartition(uint16 physPartition);

/*!
*/
UpgradeFWIFApplicationValidationStatus UpgradeFWIFValidateApplication(void);


/* Need to add a function to do a warm reboot?
    It will need to be told to do a zarkov or DFU reboot */

#ifdef MAYBE_USEFUL_LATER
/*!
    @brief UpgradeFWIFIsPartitionCompleted()

    @param physPartition Physical partition number to be checked.

    @return TRUE if partition is not completed (doesn't have first word written).
*/
bool UpgradeFWIFIsPartitionCompleted(uint16 physPartition);
#endif

/*!
    @brief UpgradeFWIFPartitionGetOffset()

    @param handle Handle to the partition offset to be taken from.

    @return Size in bytes of offset to skip already written data.
*/
uint32 UpgradeFWIFPartitionGetOffset(UpgradeFWIFPartitionHdl handle);

/*!
    @brief UpgradeFWIFGetSinkPosition()
Get the sink position of the partition sink stream.

    @param sink sink stream.

    @return the position of the sink stream.
*/
uint32 UpgradeFWIFGetSinkPosition(Sink sink);

#ifdef MAYBE_USEFUL_LATER
/*!
    @brief UpgradeFWIFGetPartitionIsInProgress()

    @return TRUE if there is partition which should be open for resume.
*/
bool UpgradeFWIFGetPartitionIsInProgress(void);

/*!
    @brief UpgradeFWIFClearPartitionIsInProgress()

    Makes UpgradeFWIFGetPartitionIsInProgress() return FALSE .
*/
void UpgradeFWIFClearPartitionIsInProgress(void);

/*!
    @brief UpgradeFWIFSetPartitionInProgressNum()

    @return Number of a physical partition to be resumed.
*/
uint16 UpgradeFWIFSetPartitionInProgressNum(void);

/*!
    @brief UpgradeFWIFGetPartitionInProgressNum()

    @return Number of a physical partition to be resumed.
*/
uint16 UpgradeFWIFGetPartitionInProgressNum(void);
#endif


#endif /* UPGRADE_FW_IF_H_ */
