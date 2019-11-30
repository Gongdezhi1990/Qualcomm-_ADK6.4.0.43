/*******************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.
 
*******************************************************************************/

#ifndef USB_DEVICE_CLASS_REMOVE_MASS_STORAGE

#ifndef _USB_DEVICE_MASS_STORAGE_FAT_16_H
#define _USB_DEVICE_MASS_STORAGE_FAT_16_H


#include <stdlib.h>

#include "usb_device_mass_storage.h"


/* General FAT defines */
#define BYTES_PER_SECTOR 512
#define SECTORS_PER_CLUSTER 4
#define NO_FATS 2
#define SECTORS_PER_FAT 134
#define SECTORS_PER_TRACK 63
#define RESERVED_SECTORS 4
#define ROOT_DIR_ENTRIES 512
#define ROOT_DIR_SIZE 32
#define NUMBER_HEADS 255
#define TOTAL_SECTORS 31556

/* Starting sectors for each area of data */
#define MBR_SECTOR 0 
#define BOOT_SECTOR 63 
#define FAT1_SECTOR (BOOT_SECTOR + RESERVED_SECTORS)
#define FAT2_SECTOR (FAT1_SECTOR + SECTORS_PER_FAT)
#define ROOT_SECTOR (FAT2_SECTOR + SECTORS_PER_FAT)
#define DATA_SECTOR (ROOT_SECTOR + (ROOT_DIR_ENTRIES * ROOT_DIR_SIZE / BYTES_PER_SECTOR))

/* cluster defines for File Allocation Table */
#define FAT_UNUSED_CLUSTER 0x0000
#define FAT_BAD_CLUSTER 0xfff7
#define FAT_LAST_CLUSTER 0xffff

/* directory attribute defines */
#define DIR_ATTRIBUTE_READ_ONLY 0x01
#define DIR_ATTRIBUTE_HIDDEN 0x02
#define DIR_ATTRIBUTE_SYSTEM 0x04
#define DIR_ATTRIBUTE_VOLUME_LABEL 0x08
#define DIR_ATTRIBUTE_SUB_DIR 0x10
#define DIR_ATTRIBUTE_ARCHIVE 0x20


/* First 446 bytes of Master Boot Record is executable code */     
typedef struct
{
    uint8 exe[446];
} MasterBootRecordExeType;

/* Partition details in Master Boot Record */
typedef struct
{
    uint8 bootIndicator[1];
    uint8 startingHead[1];
    uint8 startingSectorCyl[2];
    uint8 partitionType[1];
    uint8 endingHead[1];
    uint8 endingSectorCyl[2];
    uint8 startingSector[4];
    uint8 sectorsInPartition[4];
} MasterBootRecordPartitionType;

/* Executable signature */
typedef struct
{
    uint8 executableSignature[2];
} ExeSignatureType;

/* Boot record header information */
typedef struct
{
    uint8 jumpInstruction[3];
    uint8 oemName[8];
    uint8 bytesPerSector[2];
    uint8 sectorsPerCluster[1];
    uint8 noReservedSectors[2];
    uint8 noFats[1];
    uint8 maxRootDirEntries[2];
    uint8 totalSectorCountSmall[2];
    uint8 mediaDescriptor[1];
    uint8 sectorsPerFat[2];
    uint8 sectorsPerTrack[2];
    uint8 noHeads[2];
    uint8 hiddenSectors[4];
    uint8 totalSectorCountLarge[4];
    uint8 phyiscalDriveNumber[1];
    uint8 reserved1[1];
    uint8 extendedBootSignature[1];
    uint8 serialNumber[4];
    uint8 volumeLabel[11];
    uint8 fileSystemType[8];
} BootSectorType;

typedef struct
{
    uint8 exe[448];
} BootSectorExeType;

typedef struct
{
    uint8 filename[8];
    uint8 extension[3];
    uint8 attributes[1];
    uint8 ignore1[1];
    uint8 creationTime[3];
    uint8 creationDate[2];
    uint8 lastAccessDate[2];
    uint8 lastAccessTime[2];
    uint8 lastWriteTime[2];
    uint8 lastWriteDate[2];
    uint8 firstLogicalCluster[2];
    uint8 fileSizeBytes[4];    
} DirectoryType;


/*
    FUNCTION
        Fat16_Initialise
        
    DESCRIPTION
        Initialises the FAT16 implementation.
        
    PARAMS
        ms - the mass storage data pointer
         
*/
void Fat16_Initialise(MassStorageType *ms);


/*
    FUNCTION
        Fat16_Read
        
    DESCRIPTION
        Reads data from FAT file system. The data is read from the area starting at logical_address and ending at the address depending on transfer_length.
        All read data is written to the bulk endpoint to transfer data back to the host.        
    
    PARAMS
        ms - the mass storage data pointer 
        logical_address - the address to read data from
        transfer_length - the number of consecutive blocks to read data from       
        
*/    
void Fat16_Read(MassStorageType *ms, uint32 logical_address, uint32 transfer_length);


/*
    FUNCTION
        Fat16_GetBlockSize
        
    DESCRIPTION
        Retrieves the block size from the FAT file system. This will be the bytes per sector.
        
    RETURN VALUE
        Returns the block size of the file system.
*/    
uint32 Fat16_GetBlockSize(void);


/*
    FUNCTION
        Fat16_GetTotalBlocks
        
    DESCRIPTION
        Retrieves the total number of blocks from the FAT file system. This will be the total number of sectors.
        
    RETURN VALUE
        Returns the total number of blocks of the file system.
*/    
uint32 Fat16_GetTotalBlocks(void);


/*
    FUNCTION
        Fat16_ConfigureDataArea
        
    DESCRIPTION
        Configures what data exists in the FAT data area. Can pass in a file index and size (value_16 and value_32),
        or a pointer to some data and size (params and value_32).
        
    PARAMS
        ms - the mass storage data pointer
        value_16 - the file index to use for the data area
        value_32 - the size of the data passed in
        params - data to use for FAT data area
 
*/   
void Fat16_ConfigureDataArea(MassStorageType *ms, uint16 value_16, uint32 value_32, uint8 *params);


/*
    FUNCTION
        Fat16_ConfigureFat
        
    DESCRIPTION
        Configures what data exists in the FAT table. Can pass in a file index and size (value_16 and value_32),
        or a pointer to some data and size (params and value_32).
        
    PARAMS
        ms - the mass storage data pointer
        value_16 - the file index to use for the FAT table
        value_32 - the size of the data passed in
        params - data to use for the FAT table 
         
*/   
void Fat16_ConfigureFat(MassStorageType *ms, uint16 value_16, uint32 value_32, uint8 *params);


/*
    FUNCTION
        Fat16_ConfigureRootDir
        
    DESCRIPTION
        Configures what data exists in the FAT root directory. Can pass in a file index and size (value_16 and value_32),
        or a pointer to some data and size (params and value_32).
        
    PARAMS
        ms - the mass storage data pointer
        value_16 - the file index to use for the root directory
        value_32 - the size of the data passed in
        params - data to use for root directory
        
*/   
void Fat16_ConfigureRootDir(MassStorageType *ms, uint16 value_16, uint32 value_32, uint8 *params);


/*
    FUNCTION
        Fat16_WaitAvailable
        
    DESCRIPTION
        Wait until size bytes are available in sink
        
    PARAMS
        sink - the sink to check
        size - the size in bytes
*/   
void Fat16_WaitAvailable(Sink sink, uint16 size);


#endif /* _USB_DEVICE_MASS_STORAGE_FAT_16_H */

#endif /* !USB_DEVICE_CLASS_REMOVE_MASS_STORAGE */

