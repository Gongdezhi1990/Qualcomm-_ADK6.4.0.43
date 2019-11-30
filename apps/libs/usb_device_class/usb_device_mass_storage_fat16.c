/*******************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.
 
*******************************************************************************/

#include "usb_device_mass_storage_fat16.h"

#include <panic.h>
#include <source.h>
#include <stream.h>
#include <string.h>
#include <print.h>
#include <usb.h>


#ifndef USB_DEVICE_CLASS_REMOVE_MASS_STORAGE


#ifdef DEBUG_PRINT_ENABLED
#define MS_DEBUG(x) PRINT(x)
#else
#define MS_DEBUG(x)
#endif

/*
    The FAT16 implementation is split into sectors as shown below:  
  
              | MBR | Hidden  |  Boot   | Reserved      |  FAT1    |    FAT2    |    Root        |    Data     | 
              |     | Sectors |  Record | Sectors       |          |            |    Directory   |    Area     |
        
sectors:         0     1 - 62      63       64 - 66      67 - 200     201 - 303     335 - 366      367 - 31555

*/


static const MasterBootRecordExeType mbr_exe = {
    {0x33,0xC0,0x8E,0xD0,0xBC,0x00,0x7C,0xFB,0x50,0x07,0x50,0x1F,0xFC,0xBE,0x1B,0x7C,0xBF,
     0x1B,0x06,0x50,0x57,0xB9,0xE5,0x01,0xF3,0xA4,0xCB,0xBE,0xBE,0x07,0xB1,0x04,0x38,0x2C,
     0x7C,0x09,0x75,0x15,0x83,0xC6,0x10,0xE2,0xF5,0xCD,0x18,0x8B,0x14,0x8B,0xEE,0x83,0xC6,
     0x10,0x49,0x74,0x16,0x38,0x2C,0x74,0xF6,0xBE,0x10,0x07,0x4E,0xAC,0x3C,0x00,0x74,0xFA,
     0xBB,0x07,0x00,0xB4,0x0E,0xCD,0x10,0xEB,0xF2,0x89,0x46,0x25,0x96,0x8A,0x46,0x04,0xB4,
     0x06,0x3C,0x0E,0x74,0x11,0xB4,0x0B,0x3C,0x0C,0x74,0x05,0x3A,0xC4,0x75,0x2B,0x40,0xC6,
     0x46,0x25,0x06,0x75,0x24,0xBB,0xAA,0x55,0x50,0xB4,0x41,0xCD,0x13,0x58,0x72,0x16,0x81,
     0xFB,0x55,0xAA,0x75,0x10,0xF6,0xC1,0x01,0x74,0x0B,0x8A,0xE0,0x88,0x56,0x24,0xC7,0x06,
     0xA1,0x06,0xEB,0x1E,0x88,0x66,0x04,0xBF,0x0A,0x00,0xB8,0x01,0x02,0x8B,0xDC,0x33,0xC9,
     0x83,0xFF,0x05,0x7F,0x03,0x8B,0x4E,0x25,0x03,0x4E,0x02,0xCD,0x13,0x72,0x29,0xBE,0x46,
     0x07,0x81,0x3E,0xFE,0x7D,0x55,0xAA,0x74,0x5A,0x83,0xEF,0x05,0x7F,0xDA,0x85,0xF6,0x75,
     0x83,0xBE,0x27,0x07,0xEB,0x8A,0x98,0x91,0x52,0x99,0x03,0x46,0x08,0x13,0x56,0x0A,0xE8,
     0x12,0x00,0x5A,0xEB,0xD5,0x4F,0x74,0xE4,0x33,0xC0,0xCD,0x13,0xEB,0xB8,0x00,0x00,0x00,
     0x00,0x00,0x00,0x56,0x33,0xF6,0x56,0x56,0x52,0x50,0x06,0x53,0x51,0xBE,0x10,0x00,0x56,
     0x8B,0xF4,0x50,0x52,0xB8,0x00,0x42,0x8A,0x56,0x24,0xCD,0x13,0x5A,0x58,0x8D,0x64,0x10,
     0x72,0x0A,0x40,0x75,0x01,0x42,0x80,0xC7,0x02,0xE2,0xF7,0xF8,0x5E,0xC3,0xEB,0x74,0x49,
     0x6E,0x76,0x61,0x6C,0x69,0x64,0x20,0x70,0x61,0x72,0x74,0x69,0x74,0x69,0x6F,0x6E,0x20,
     0x74,0x61,0x62,0x6C,0x65,0x00,0x45,0x72,0x72,0x6F,0x72,0x20,0x6C,0x6F,0x61,0x64,0x69,
     0x6E,0x67,0x20,0x6F,0x70,0x65,0x72,0x61,0x74,0x69,0x6E,0x67,0x20,0x73,0x79,0x73,0x74,
     0x65,0x6D,0x00,0x4D,0x69,0x73,0x73,0x69,0x6E,0x67,0x20,0x6F,0x70,0x65,0x72,0x61,0x74,
     0x69,0x6E,0x67,0x20,0x73,0x79,0x73,0x74,0x65,0x6D,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x8B,0xFC,0x1E,0x57,
     0x8B,0xF5,0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x37,0x39,
     0xBF,0x99,0x00,0x00}
};

/* MBR active partition details - note LSB is first */
static const MasterBootRecordPartitionType mbr_partition = {
    {0x80}, /* boot indicator */
    {0x01}, /* starting head */    
    {0x01,0x00}, /* starting sector cylinder */
    {0x06}, /* partition type */
    {0x05}, /* ending head */
    {0x7f,0x69}, /* ending sector cylinder */
    {BOOT_SECTOR & 0xff,(BOOT_SECTOR & 0xff00)>>8,(BOOT_SECTOR & 0xff0000)>>16,(BOOT_SECTOR & 0xff000000ul)>>24}, /* starting sector */
    {TOTAL_SECTORS & 0xff,(TOTAL_SECTORS & 0xff00)>>8,(TOTAL_SECTORS & 0xff0000)>>16,(TOTAL_SECTORS & 0xff000000ul)>>24} /* sectors in partition */
};    

static const ExeSignatureType exe_signature = {
    {0x55,0xaa}    
};
    
/* Boot Sector data - note LSB is first */
static const BootSectorType boot_sector = {
    {0xEB,0x3C,0x90}, /* jump instruction */
    {'M','S','D','O','S','5','.','0'}, /* OEM Name */
    {BYTES_PER_SECTOR & 0xff,(BYTES_PER_SECTOR & 0xff00)>>8}, /* bytes per sector (normally 512) */
    {SECTORS_PER_CLUSTER}, /* sectors per cluster */
    {RESERVED_SECTORS & 0xff,(RESERVED_SECTORS & 0xff00)>>8}, /* reserved sectors */
    {NO_FATS}, /* number FATs */
    {ROOT_DIR_ENTRIES & 0xff,(ROOT_DIR_ENTRIES & 0xff00)>>8}, /* max root directory entries */
    {TOTAL_SECTORS & 0xff,(TOTAL_SECTORS & 0xff00)>>8}, /* total sectors */
    {0xf8}, /* media description - fixed disk */
    {SECTORS_PER_FAT & 0xff,(SECTORS_PER_FAT & 0xff00)>>8}, /* sectors per FAT */
    {SECTORS_PER_TRACK & 0xff,(SECTORS_PER_TRACK & 0xff00)>>8}, /* sectors per track */
    {NUMBER_HEADS & 0xff,(NUMBER_HEADS & 0xff00)>>8}, /* number of heads */
    {BOOT_SECTOR & 0xff,(BOOT_SECTOR & 0xff00)>>8,(BOOT_SECTOR & 0xff0000)>>16,(BOOT_SECTOR & 0xff000000ul)>>24}, /* hidden sectors */
    {0x00,0x00,0x00,0x00}, /* total sectors for large values */
    {0x00}, /* physical drive number - hard disk */
    {0x00}, /* reserved */
    {0x29}, /* boot signature */
    {0x01,0x04,0xda,0x1f}, /* volume ID */
    {'N','O',' ','N','A','M','E',' ',' ',' ',' '}, /* volume label */ 
    {'F','A','T','1','6',' ',' ',' '} /* file system type */ 
};

static const BootSectorExeType boot_exe = {
    {0x33,0xC9,0x8E,0xD1,0xBC,0xF0,0x7B,0x8E,0xD9,0xB8,0x00,0x20,0x8E,0xC0,0xFC,
     0xBD,0x00,0x7C,0x38,0x4E,0x24,0x7D,0x24,0x8B,0xC1,0x99,0xE8,0x3C,0x01,0x72,
     0x1C,0x83,0xEB,0x3A,0x66,0xA1,0x1C,0x7C,0x26,0x66,0x3B,0x07,0x26,0x8A,0x57,
     0xFC,0x75,0x06,0x80,0xCA,0x02,0x88,0x56,0x02,0x80,0xC3,0x10,0x73,0xEB,0x33,
     0xC9,0x8A,0x46,0x10,0x98,0xF7,0x66,0x16,0x03,0x46,0x1C,0x13,0x56,0x1E,0x03,
     0x46,0x0E,0x13,0xD1,0x8B,0x76,0x11,0x60,0x89,0x46,0xFC,0x89,0x56,0xFE,0xB8,
     0x20,0x00,0xF7,0xE6,0x8B,0x5E,0x0B,0x03,0xC3,0x48,0xF7,0xF3,0x01,0x46,0xFC,
     0x11,0x4E,0xFE,0x61,0xBF,0x00,0x00,0xE8,0xE6,0x00,0x72,0x39,0x26,0x38,0x2D,
     0x74,0x17,0x60,0xB1,0x0B,0xBE,0xA1,0x7D,0xF3,0xA6,0x61,0x74,0x32,0x4E,0x74,
     0x09,0x83,0xC7,0x20,0x3B,0xFB,0x72,0xE6,0xEB,0xDC,0xA0,0xFB,0x7D,0xB4,0x7D,
     0x8B,0xF0,0xAC,0x98,0x40,0x74,0x0C,0x48,0x74,0x13,0xB4,0x0E,0xBB,0x07,0x00,
     0xCD,0x10,0xEB,0xEF,0xA0,0xFD,0x7D,0xEB,0xE6,0xA0,0xFC,0x7D,0xEB,0xE1,0xCD,
     0x16,0xCD,0x19,0x26,0x8B,0x55,0x1A,0x52,0xB0,0x01,0xBB,0x00,0x00,0xE8,0x3B,
     0x00,0x72,0xE8,0x5B,0x8A,0x56,0x24,0xBE,0x0B,0x7C,0x8B,0xFC,0xC7,0x46,0xF0,
     0x3D,0x7D,0xC7,0x46,0xF4,0x29,0x7D,0x8C,0xD9,0x89,0x4E,0xF2,0x89,0x4E,0xF6,
     0xC6,0x06,0x96,0x7D,0xCB,0xEA,0x03,0x00,0x00,0x20,0x0F,0xB6,0xC8,0x66,0x8B,
     0x46,0xF8,0x66,0x03,0x46,0x1C,0x66,0x8B,0xD0,0x66,0xC1,0xEA,0x10,0xEB,0x5E,
     0x0F,0xB6,0xC8,0x4A,0x4A,0x8A,0x46,0x0D,0x32,0xE4,0xF7,0xE2,0x03,0x46,0xFC,
     0x13,0x56,0xFE,0xEB,0x4A,0x52,0x50,0x06,0x53,0x6A,0x01,0x6A,0x10,0x91,0x8B,
     0x46,0x18,0x96,0x92,0x33,0xD2,0xF7,0xF6,0x91,0xF7,0xF6,0x42,0x87,0xCA,0xF7,
     0x76,0x1A,0x8A,0xF2,0x8A,0xE8,0xC0,0xCC,0x02,0x0A,0xCC,0xB8,0x01,0x02,0x80,
     0x7E,0x02,0x0E,0x75,0x04,0xB4,0x42,0x8B,0xF4,0x8A,0x56,0x24,0xCD,0x13,0x61,
     0x61,0x72,0x0B,0x40,0x75,0x01,0x42,0x03,0x5E,0x0B,0x49,0x75,0x06,0xF8,0xC3,
     0x41,0xBB,0x00,0x00,0x60,0x66,0x6A,0x00,0xEB,0xB0,0x4E,0x54,0x4C,0x44,0x52,
     0x20,0x20,0x20,0x20,0x20,0x20,0x0D,0x0A,0x52,0x65,0x6D,0x6F,0x76,0x65,0x20,
     0x64,0x69,0x73,0x6B,0x73,0x20,0x6F,0x72,0x20,0x6F,0x74,0x68,0x65,0x72,0x20,
     0x6D,0x65,0x64,0x69,0x61,0x2E,0xFF,0x0D,0x0A,0x44,0x69,0x73,0x6B,0x20,0x65,
     0x72,0x72,0x6F,0x72,0xFF,0x0D,0x0A,0x50,0x72,0x65,0x73,0x73,0x20,0x61,0x6E,
     0x79,0x20,0x6B,0x65,0x79,0x20,0x74,0x6F,0x20,0x72,0x65,0x73,0x74,0x61,0x72,
     0x74,0x0D,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xAC,0xCB,0xD8}
};


/* This is the first few entries of the FAT, first sector.
   This states that one file exists at the start of the data area. */
static const uint8 default_fat_entries[] = {
    0x00, 0x00, /* cluster 0 not used */
    0x00, 0x00, /* cluster 1 not used */
    FAT_LAST_CLUSTER & 0xff, (FAT_LAST_CLUSTER & 0xff00)>>8 /* cluster 2 is last cluster for file */
};

static const uint8 default_file_data[] = {
    'C','S','R',' ','m','a','s','s',' ','s','t','o','r','a','g','e',' ','e','x','a','m','p','l','e',' ','t','e','x','t',' ','f','i','l','e','.'
};
#define SIZE_FAT_FILE sizeof(default_file_data)

/* root directory file entry -  - note LSB is first */
static const DirectoryType default_root_file = {
    {'R','E','A','D',' ','M','E',' '}, /* filename */
    {'T','X','T'}, /* extension */
    {DIR_ATTRIBUTE_READ_ONLY}, /* attributes */
    {0x00}, /* ignore */
    {0x00,0x00,0x0c}, /* creation time */
    {0x90,0x3c}, /* creation date */
    {0x90,0x3c}, /* last access date */
    {0x00,0x00}, /* last access time must be zeros */
    {0x00,0x0c}, /* last write time */
    {0x90,0x3c}, /* last write date */
    {0x02,0x00}, /* first logical cluster */
    {SIZE_FAT_FILE & 0xff,(SIZE_FAT_FILE & 0xff00)>>8,(SIZE_FAT_FILE & 0xff0000)>>16,(SIZE_FAT_FILE & 0xff000000ul)>>24} /* file size */
};


static uint8 *claimSink(Sink sink, uint16 size)
{
    uint8 *dest = SinkMap(sink);
    uint16 claim_result = SinkClaim(sink, size);
    if (claim_result == 0xffff)
    {
        return 0;
    }

    return (dest + claim_result);
}


static uint32 read_sectors(FileInfoType *file_info, uint32 logical_address, uint32 transfer_length, uint32 area_start_sector)
{
    uint32 start_sector;
    uint32 end_sector;
    uint32 file_end_sector = file_info->end_sector;
    uint16 i = 0;
    Sink sink = StreamUsbEndPointSink(end_point_bulk_out);
    
    /* correct end sector for FAT2, as it's otherwise treated as FAT1 */
    if (area_start_sector == FAT2_SECTOR)
        file_end_sector += SECTORS_PER_FAT;
    
    /* find the start sector and end sector for this type of data */
    start_sector = logical_address - area_start_sector;
    end_sector = start_sector + transfer_length - 1;
    if (end_sector > (file_end_sector - area_start_sector))
        end_sector = file_end_sector - area_start_sector;

    MS_DEBUG(("FAT16: start %ld end %ld fileend %ld log %ld areastart %ld\n",start_sector,end_sector,file_info->end_sector,logical_address,area_start_sector));
    
    /* check to see if the file read should begin at the start of the file */
    if ((file_info->src == 0) || (start_sector < file_info->current_start_sector))
    {
        if (file_info->params)
            file_info->src = StreamRegionSource(file_info->params, file_info->size);
        else
            file_info->src = StreamFileSource(file_info->index);
        file_info->current_start_sector = 0;
        
        MS_DEBUG(("FAT16: new file\n"));
    }
    
    /* seek through the file until the correct sector is reached */
    while ((start_sector > file_info->current_start_sector) && (file_info->current_start_sector < end_sector))
    {
        SourceDrop(file_info->src, BYTES_PER_SECTOR);
        file_info->current_start_sector++;   
        MS_DEBUG(("FAT16: src drop %ld\n",file_info->current_start_sector));
    }    
    
    /* send the data in the sectors from start_sector to end_sector */
    while (i <= (end_sector - start_sector))
    {
        uint8 *buffer = 0;
        uint16 sink_slack = 0;
        uint16 source_size;
        uint16 blocks_in_sink;
        uint16 blocks_in_source;
        uint16 blocks_to_read;
        uint32 bytes_to_read;
        uint32 remaining_bytes;
        uint16 bytes_to_copy = 0;          
        
        /* wait for free space in Sink */
        Fat16_WaitAvailable(sink, BYTES_PER_SECTOR);
        
        sink_slack = SinkSlack(sink);
        source_size = SourceSize(file_info->src);
        blocks_in_sink = sink_slack / BYTES_PER_SECTOR;
        
        /* find the maximum sectors that can be sent */
        if ((source_size % BYTES_PER_SECTOR) == 0)
            blocks_in_source = source_size / BYTES_PER_SECTOR;
        else
            blocks_in_source = source_size / BYTES_PER_SECTOR + 1;
        blocks_to_read = blocks_in_sink > blocks_in_source ? blocks_in_source : blocks_in_sink;
        if (blocks_to_read > (end_sector - i + 1))
            blocks_to_read = end_sector - i + 1;
        bytes_to_read = blocks_to_read * BYTES_PER_SECTOR;
        remaining_bytes = file_info->size - (file_info->current_start_sector * BYTES_PER_SECTOR);
        
        MS_DEBUG(("FAT16: info sink_slack:%d source_size:%d blocks_to_read:%d\n",sink_slack,source_size,blocks_to_read));
        
        if (blocks_to_read == 0)
            break;
        
        if (remaining_bytes < bytes_to_read)
            bytes_to_copy = remaining_bytes;
        else
            bytes_to_copy = bytes_to_read;
                             
        if ((buffer = claimSink(sink, bytes_to_read)) != 0)
        {            
            const uint8 *data_ptr = SourceMap(file_info->src);

            if (bytes_to_copy < bytes_to_read)
                memset(buffer + bytes_to_copy, 0, bytes_to_read - bytes_to_copy); 
            memmove(buffer, data_ptr, bytes_to_copy);            
            SinkConfigure(sink, VM_SINK_USB_TRANSFER_LENGTH, bytes_to_read);
            SinkFlush(sink, bytes_to_read);
            SourceDrop(file_info->src, bytes_to_copy);
            file_info->current_start_sector += blocks_to_read;
            i += blocks_to_read;
            MS_DEBUG(("FAT16: send bytes %d pos %ld i %d\n",bytes_to_copy,file_info->current_start_sector,i));
        }   
        else
        {
            break;
        }
    }
    
    /* return the next logical address to process */
    return logical_address + end_sector - start_sector + 1;
}
    

void Fat16_Initialise(MassStorageType *ms)
{
    if (ms->file_info[FILE_INFO_DATA].size == 0)
    {
        ms->file_info[FILE_INFO_DATA].size = SIZE_FAT_FILE;
        ms->file_info[FILE_INFO_DATA].params = (uint8 *)default_file_data; 
        ms->file_info[FILE_INFO_DATA].index = 0;
    }
    
    if (ms->file_info[FILE_INFO_ROOT_DIR].size == 0)
    {
        ms->file_info[FILE_INFO_ROOT_DIR].size = sizeof(default_root_file);
        ms->file_info[FILE_INFO_ROOT_DIR].params = (uint8 *)&default_root_file.filename[0];
        ms->file_info[FILE_INFO_ROOT_DIR].index = 0;
    }
    
    if (ms->file_info[FILE_INFO_FAT].size == 0)
    {
        ms->file_info[FILE_INFO_FAT].size = sizeof(default_fat_entries);
        ms->file_info[FILE_INFO_FAT].params = (uint8 *)default_fat_entries; 
        ms->file_info[FILE_INFO_FAT].index = 0;
    }
    ms->info_read = FALSE;
}


void Fat16_Read(MassStorageType *ms, uint32 logical_address, uint32 transfer_length)
{       
    uint32 end_address = logical_address + transfer_length - 1;
    Sink sink_bulk_out = StreamUsbEndPointSink(end_point_bulk_out);
    
    MS_DEBUG(("FAT16: Read log addr: %ld end addr: %ld\n", logical_address, end_address));
    
    if (!ms->info_read)
    {
        /* information read once from read-only file system */
        uint32 fat_number_sectors;
        uint32 root_number_sectors;
        uint32 data_number_sectors;   
    
        /* FAT size calculations */
        fat_number_sectors = ms->file_info[FILE_INFO_FAT].size / BYTES_PER_SECTOR + 1;       
        if ((ms->file_info[FILE_INFO_FAT].size % BYTES_PER_SECTOR) == 0)
        {            
            fat_number_sectors--;
        }
        ms->file_info[FILE_INFO_FAT].end_sector = FAT1_SECTOR + fat_number_sectors - 1;
    
        /* Root dir size calculations */
        root_number_sectors = ms->file_info[FILE_INFO_ROOT_DIR].size / BYTES_PER_SECTOR + 1;        
        if ((ms->file_info[FILE_INFO_ROOT_DIR].size % BYTES_PER_SECTOR) == 0)
        {            
            root_number_sectors--;
        }
        ms->file_info[FILE_INFO_ROOT_DIR].end_sector = ROOT_SECTOR + root_number_sectors - 1;
    
        /* Data area size calculations */
        data_number_sectors = ms->file_info[FILE_INFO_DATA].size / BYTES_PER_SECTOR + 1;       
        if ((ms->file_info[FILE_INFO_DATA].size % BYTES_PER_SECTOR) == 0)
        {            
            data_number_sectors--;
        }
        ms->file_info[FILE_INFO_DATA].end_sector = DATA_SECTOR + data_number_sectors - 1;
    
        /* don't read this information again to speed things up */
        ms->info_read = TRUE;
    }
    
    while (logical_address <= end_address)
    {
        MS_DEBUG(("FAT16: log addr: %ld\n", logical_address));
        if (logical_address == MBR_SECTOR) /* Master Boot Record */
        {            
            uint8 *buffer = 0;
            
            /* wait for free space in Sink */
            Fat16_WaitAvailable(sink_bulk_out, BYTES_PER_SECTOR);
            
            if ((buffer = claimSink(sink_bulk_out, BYTES_PER_SECTOR)) != 0)
            {
                uint16 offset = 0;
                uint16 size_data = sizeof(MasterBootRecordExeType);
                memmove(buffer, &mbr_exe, size_data);
                offset += size_data;            
                size_data = sizeof(MasterBootRecordPartitionType);
                memmove(buffer + offset, &mbr_partition, size_data);
                offset += size_data;
                memset(buffer + offset, 0, size_data * 3);
                size_data = sizeof(ExeSignatureType);            
                memmove(buffer + BYTES_PER_SECTOR - size_data, &exe_signature, size_data);                
                SinkConfigure(sink_bulk_out, VM_SINK_USB_TRANSFER_LENGTH, BYTES_PER_SECTOR);
                SinkFlush(sink_bulk_out, BYTES_PER_SECTOR);
                MS_DEBUG(("FAT16: MBR returned data\n"));                
            }
            logical_address++;
        }
        else if (logical_address == BOOT_SECTOR) /* Boot Sector */
        {       
            uint8 *buffer = 0;  
            
            /* wait for free space in Sink */
            Fat16_WaitAvailable(sink_bulk_out, BYTES_PER_SECTOR);
                          
            if ((buffer = claimSink(sink_bulk_out, BYTES_PER_SECTOR)) != 0)
            {
                uint16 offset = 0;
                uint16 size_data = sizeof(BootSectorType);
                memmove(buffer, &boot_sector, size_data);
                offset += size_data;
                size_data = sizeof(BootSectorExeType);
                memmove(buffer + offset, &boot_exe, size_data);
                offset += size_data;
                size_data = sizeof(ExeSignatureType);
                memmove(buffer + offset, &exe_signature, size_data);
                SinkConfigure(sink_bulk_out, VM_SINK_USB_TRANSFER_LENGTH, BYTES_PER_SECTOR);
                SinkFlush(sink_bulk_out, BYTES_PER_SECTOR);
                MS_DEBUG(("FAT16: BOOT returned data\n"));
            }
            logical_address++;
        }
        else if ((logical_address >= FAT1_SECTOR) && (logical_address <= ms->file_info[FILE_INFO_FAT].end_sector)) /* FAT 1 */
        {
            MS_DEBUG(("FAT16: FAT1 sector\n"));
            logical_address = read_sectors(&ms->file_info[FILE_INFO_FAT], logical_address, end_address - logical_address + 1, FAT1_SECTOR);
        }
        else if ((logical_address >= FAT2_SECTOR) && (logical_address <= (ms->file_info[FILE_INFO_FAT].end_sector + SECTORS_PER_FAT))) /* FAT 2 */
        {
            MS_DEBUG(("FAT16: FAT2 sector\n"));
            logical_address = read_sectors(&ms->file_info[FILE_INFO_FAT], logical_address, end_address - logical_address + 1, FAT2_SECTOR);
        }
        else if ((logical_address >= ROOT_SECTOR) && (logical_address <= ms->file_info[FILE_INFO_ROOT_DIR].end_sector)) /* Root Directory */
        {
            MS_DEBUG(("FAT16: root sector\n"));
            logical_address = read_sectors(&ms->file_info[FILE_INFO_ROOT_DIR], logical_address, end_address - logical_address + 1, ROOT_SECTOR);
        }
        else if ((logical_address >= DATA_SECTOR) && (logical_address <= ms->file_info[FILE_INFO_DATA].end_sector)) /* Data Area */
        {
            MS_DEBUG(("FAT16: data sector\n"));
            logical_address = read_sectors(&ms->file_info[FILE_INFO_DATA], logical_address, end_address - logical_address + 1, DATA_SECTOR);
        }
        else /* sector with no data */
        {
            uint8 *buffer = 0;
            
            /* wait for free space in Sink */
            Fat16_WaitAvailable(sink_bulk_out, BYTES_PER_SECTOR);
                
            if ((buffer = claimSink(sink_bulk_out, BYTES_PER_SECTOR)) != 0)
            {
                memset(buffer, 0, BYTES_PER_SECTOR);                
                SinkConfigure(sink_bulk_out, VM_SINK_USB_TRANSFER_LENGTH, BYTES_PER_SECTOR);
                SinkFlush(sink_bulk_out, BYTES_PER_SECTOR);
                MS_DEBUG(("FAT16: empty sector\n"));
            }
            logical_address++;
        }
    }
}


uint32 Fat16_GetBlockSize(void)
{
    return BYTES_PER_SECTOR;
}


uint32 Fat16_GetTotalBlocks(void)
{
    return TOTAL_SECTORS;    
}


void Fat16_ConfigureDataArea(MassStorageType *ms, uint16 value_16, uint32 value_32, uint8 *params)
{
    ms->file_info[FILE_INFO_DATA].size = value_32;
    if (params)
    {
        ms->file_info[FILE_INFO_DATA].params = params;
        ms->file_info[FILE_INFO_DATA].index = 0;
    }
    else
    {
        ms->file_info[FILE_INFO_DATA].index = value_16;  
        ms->file_info[FILE_INFO_DATA].params = 0;
    }
}


void Fat16_ConfigureFat(MassStorageType *ms, uint16 value_16, uint32 value_32, uint8 *params)
{
    ms->file_info[FILE_INFO_FAT].size = value_32;
    if (params)
    {
        ms->file_info[FILE_INFO_FAT].params = params;
        ms->file_info[FILE_INFO_FAT].index = 0;
    }
    else
    {
        ms->file_info[FILE_INFO_FAT].index = value_16;  
        ms->file_info[FILE_INFO_FAT].params = 0;
    }
}


void Fat16_ConfigureRootDir(MassStorageType *ms, uint16 value_16, uint32 value_32, uint8 *params)
{
    ms->file_info[FILE_INFO_ROOT_DIR].size = value_32;
    if (params)
    {
        ms->file_info[FILE_INFO_ROOT_DIR].params = params;
        ms->file_info[FILE_INFO_ROOT_DIR].index = 0;
    }
    else
    {
        ms->file_info[FILE_INFO_ROOT_DIR].index = value_16;
        ms->file_info[FILE_INFO_ROOT_DIR].params = 0;
    }
}


void Fat16_WaitAvailable(Sink sink, uint16 size)
{
    /* While not enough space and USB still attached, wait */
    while((SinkSlack(sink) < size) && (UsbDeviceState() == usb_device_state_configured));
}

#endif /* USB_DEVICE_CLASS_REMOVE_MASS_STORAGE */

