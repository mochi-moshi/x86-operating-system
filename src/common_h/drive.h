#ifndef DRIVE_H
#define DRIVE_H
#include "stddef.h"

#define DRIVE_DATA_PORT          0x1F0
#define DRIVE_ERROR_PORT         0x1F1
#define DRIVE_FEATURES_PORT      0x1F1
#define DRIVE_SECTOR_COUNT_PORT  0x1F2
#define DRIVE_SECTOR_NUMBER_PORT 0x1F3
#define DRIVE_CYLINDER_LOW_PORT  0x1F4
#define DRIVE_CYLINDER_HIGH_PORT 0x1F5
#define DRIVE_REGISTER_PORT      0x1F6
#define DRIVE_STATUS_PORT        0x1F7
#define DRIVE_COMMAND_PORT       0x1F7
#define DRIVE_CONTROL_PORT       0x3F6

#define DRIVE_READ_SECTORS_WITH_RETRY  0x20
#define DRIVE_READ_SECTORS             0x21
#define DRIVE_READ_LONG_WITH_RETRY     0x22
#define DRIVE_READ_LONG                0x23
#define DRIVE_READ_SECTORS_EXT         0x24
#define DRIVE_WRITE_SECTORS_WITH_RETRY 0x30
#define DRIVE_WRITE_SECTORS            0x31
#define DRIVE_WRITE_LONG_WITH_RETRY    0x32
#define DRIVE_WRITE_LONG               0x33
#define DRIVE_WRITE_SECTORS_EXT        0x34

typedef struct {
    char error : 1;
    char index : 1;
    char corrected : 1;
    char drive_request : 1;
    char service : 1;
    char drive_fault : 1;
    char ready : 1;
    char busy : 1;
} drive_status_t;

typedef struct {
    char addr_mark_not_found : 1;
    char track_zero_not_found : 1;
    char aborted_command : 1;
    char media_change_request : 1;
    char id_not_found : 1;
    char media_changed : 1;
    char uncorrectable_data : 1;
    char bad_block : 1;
} drive_error_t;

typedef struct {
    char head_block_num : 4;
    char drive_number : 1;
    char _resverved0 : 1;
    char lba_addressing : 1;
    char _resverved1 : 1;
} drive_info_t;

typedef struct {
    uint8_t driveno;
    uint32_t lba_first;
    uint32_t size;
} drive_partition_info_t;

static drive_partition_info_t __drive_current_partition;

static inline drive_status_t drive_get_status() {
    drive_status_t status = { inb(DRIVE_STATUS_PORT) };
    return status;
}

static inline drive_error_t drive_get_error() {
    drive_error_t error = { inb(DRIVE_ERROR_PORT) };
    return error;
}

static inline drive_info_t drive_get_info() {
    drive_info_t info = { inb(DRIVE_REGISTER_PORT) };
    return info;
}

static void drive_init(uint8_t driveno, uint32_t lba_first, uint32_t size) {
    __drive_current_partition.driveno = driveno;
    __drive_current_partition.lba_first = lba_first;
    __drive_current_partition.size = size;
}

static void drive_read_from(ptr_t location, uint64_t lba48, uint16_t number_of_sectors) {
    while(drive_get_status().busy);
    if(lba48 + number_of_sectors > __drive_current_partition.size)
        number_of_sectors -= (lba48 - __drive_current_partition.size + number_of_sectors);
    lba48 += __drive_current_partition.lba_first;
    outb(DRIVE_REGISTER_PORT, 0x40); // 48b LBA, master 
    outb(DRIVE_SECTOR_COUNT_PORT, number_of_sectors >> 8);
    outb(DRIVE_SECTOR_NUMBER_PORT, (lba48 >> 24)  & 0xFF);
    outb(DRIVE_CYLINDER_LOW_PORT,  (lba48 >> 32)  & 0xFF);
    outb(DRIVE_CYLINDER_HIGH_PORT, (lba48 >> 40)  & 0xFF);
    outb(DRIVE_SECTOR_COUNT_PORT, number_of_sectors & 0xFF);
    outb(DRIVE_SECTOR_NUMBER_PORT, (lba48 >>  0)  & 0xFF);
    outb(DRIVE_CYLINDER_LOW_PORT,  (lba48 >>  8)  & 0xFF);
    outb(DRIVE_CYLINDER_HIGH_PORT, (lba48 >> 16)  & 0xFF);
    outb(DRIVE_COMMAND_PORT, DRIVE_READ_SECTORS_EXT);

    for(uint8_t i=0; i<number_of_sectors; i++) {
        drive_status_t status;
        do {
            status = drive_get_status();
        } while(status.busy || !status.drive_request); // && (counter < 100000)

        insw(DRIVE_DATA_PORT, location + i*512, 256);
    }
}

static void drive_write_to(ptr_t location, uint64_t lba48, uint16_t number_of_sectors) {
    while(drive_get_status().busy);
    if(lba48 + number_of_sectors > __drive_current_partition.size)
        number_of_sectors -= (lba48 - __drive_current_partition.size + number_of_sectors);
    lba48 += __drive_current_partition.lba_first;
    outb(DRIVE_REGISTER_PORT, 0x40); // 48b LBA, master 
    outb(DRIVE_SECTOR_COUNT_PORT, number_of_sectors >> 8);
    outb(DRIVE_SECTOR_NUMBER_PORT, (lba48 >> 24)  & 0xFF);
    outb(DRIVE_CYLINDER_LOW_PORT,  (lba48 >> 32)  & 0xFF);
    outb(DRIVE_CYLINDER_HIGH_PORT, (lba48 >> 40)  & 0xFF);
    outb(DRIVE_SECTOR_COUNT_PORT, number_of_sectors & 0xFF);
    outb(DRIVE_SECTOR_NUMBER_PORT, (lba48 >>  0)  & 0xFF);
    outb(DRIVE_CYLINDER_LOW_PORT,  (lba48 >>  8)  & 0xFF);
    outb(DRIVE_CYLINDER_HIGH_PORT, (lba48 >> 16)  & 0xFF);
    outb(DRIVE_COMMAND_PORT, DRIVE_WRITE_SECTORS_EXT);

    for(uint8_t i=0; i<number_of_sectors; i++) {
        drive_status_t status;
        do {
            status = drive_get_status();
        } while(status.busy || !status.drive_request);

        outsl(DRIVE_DATA_PORT, location + i*512, 128);
    }
}


#endif
