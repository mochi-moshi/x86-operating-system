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
#define DRIVE_FLUSH                    0xE7
#define DRIVE_IDENTIFY                 0xEC


#define DRIVE_SECTOR_SIZE 512

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

static inline void drive_flush() {
    outb(DRIVE_COMMAND_PORT, DRIVE_FLUSH);
    while(drive_get_status().busy);
}

static uint16_t* drive_identify(void) {
    static uint16_t identify[256];
    memset(identify, 0, 512);
    outb(DRIVE_REGISTER_PORT, 0xA0);
    outb(DRIVE_SECTOR_COUNT_PORT, 0);
    outb(DRIVE_SECTOR_NUMBER_PORT, 0);
    outb(DRIVE_CYLINDER_LOW_PORT, 0);
    outb(DRIVE_CYLINDER_HIGH_PORT, 0);
    outb(DRIVE_COMMAND_PORT, DRIVE_IDENTIFY);
    io_wait();
    drive_status_t status = drive_get_status();
    if(*(uint8_t*)&status == 0) return 0;
    while(drive_get_status().busy);
    uint8_t lbamid = inb(DRIVE_CYLINDER_LOW_PORT);
    uint8_t lbahi  = inb(DRIVE_CYLINDER_HIGH_PORT);
    if(!(lbamid || lbahi)) {
        do {
            status = drive_get_status();
        } while(!status.drive_request || !status.error);
    }
    status = drive_get_status();
    if(!status.error) {
        insw(DRIVE_DATA_PORT, identify, 256);
    }
    return identify;
}

static uint32_t drive_init(uint8_t driveno, uint32_t lba_first, uint32_t size) {
    __drive_current_partition.driveno = driveno;
    __drive_current_partition.lba_first = lba_first;
    __drive_current_partition.size = size; 
    uint32_t identifiers = 0;
    identifiers |= inb(DRIVE_SECTOR_COUNT_PORT)  << 24;
    identifiers |= inb(DRIVE_SECTOR_NUMBER_PORT) << 16;
    identifiers |= inb(DRIVE_CYLINDER_LOW_PORT)  <<  8;
    identifiers |= inb(DRIVE_CYLINDER_HIGH_PORT) <<  0;
    return identifiers;
}

static void drive_read_sectors_to(ptr_t location, uint32_t lba28, uint8_t number_of_sectors) {
    while(drive_get_status().busy);
    if(lba28 + number_of_sectors > __drive_current_partition.size)
        number_of_sectors -= (lba28 - __drive_current_partition.size + number_of_sectors);
    lba28 += __drive_current_partition.lba_first;
    outb(DRIVE_REGISTER_PORT, 0xE0 || ((lba28 >> 24) & 0xF)); // 28b LBA, master 
    outb(DRIVE_SECTOR_COUNT_PORT, number_of_sectors);
    outb(DRIVE_SECTOR_NUMBER_PORT, (lba28 >>  0)  & 0xFF);
    outb(DRIVE_CYLINDER_LOW_PORT,  (lba28 >>  8)  & 0xFF);
    outb(DRIVE_CYLINDER_HIGH_PORT, (lba28 >> 16)  & 0xFF);
    outb(DRIVE_COMMAND_PORT, DRIVE_READ_SECTORS_WITH_RETRY);

    for(uint8_t i=0; i<number_of_sectors; i++) {
        drive_status_t status;
        do {
            status = drive_get_status();
        } while(status.busy || !status.drive_request); // && (counter < 100000)

        insw(DRIVE_DATA_PORT, location + i*512, 256);
    }
}

static void drive_read_to(ptr_t location, uint32_t lba28, size_t size) {
    static uint8_t buffer[DRIVE_SECTOR_SIZE];
    uint16_t sectors;
    uint8_t over;
    for(over = 0; over < (size / DRIVE_SECTOR_SIZE) / UINT8_MAX; over++) {
        drive_read_sectors_to(location+over*DRIVE_SECTOR_SIZE*UINT8_MAX, lba28+over*UINT8_MAX, 0);
    }
    sectors = (size / DRIVE_SECTOR_SIZE) % UINT8_MAX;
    size_t trailing = size % DRIVE_SECTOR_SIZE;
    if(sectors)
        drive_read_sectors_to(location+over*DRIVE_SECTOR_SIZE*UINT8_MAX, lba28+over*UINT8_MAX, sectors);
    drive_read_sectors_to(buffer, lba28+over*UINT8_MAX+sectors, 1);
    memcpy(location+over*DRIVE_SECTOR_SIZE*UINT8_MAX+sectors*DRIVE_SECTOR_SIZE, buffer, trailing);
}


static void drive_write_sectors_from(ptr_t location, uint32_t lba28, uint8_t number_of_sectors) {
    while(drive_get_status().busy);
    if(lba28 + number_of_sectors > __drive_current_partition.size)
        number_of_sectors -= (lba28 - __drive_current_partition.size + number_of_sectors);
    lba28 += __drive_current_partition.lba_first;
    outb(DRIVE_REGISTER_PORT, 0xE0 || ((lba28 >> 24) & 0xFF)); // 48b LBA, master 
    outb(DRIVE_SECTOR_COUNT_PORT, number_of_sectors);
    outb(DRIVE_SECTOR_NUMBER_PORT, (lba28 >>  0)  & 0xFF);
    outb(DRIVE_CYLINDER_LOW_PORT,  (lba28 >>  8)  & 0xFF);
    outb(DRIVE_CYLINDER_HIGH_PORT, (lba28 >> 16)  & 0xFF);
    outb(DRIVE_COMMAND_PORT, DRIVE_WRITE_SECTORS_WITH_RETRY);

    for(uint8_t i=0; i<number_of_sectors; i++) {
        drive_status_t status;
        do {
            status = drive_get_status();
        } while(status.busy || !status.drive_request);

        outsl(DRIVE_DATA_PORT, location + i*512, 128);
    }
}


#endif
