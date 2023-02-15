#ifndef ENTRY_C
#define ENTRY_C
#include "stddef.h"

typedef struct __attribute__((packed)) {
    uint64_t base_address;
    uint64_t length;
    uint32_t type;
    uint32_t acpi_bitfield;
} smap_entry_t;

typedef struct __attribute__((packed)) {
    uint16_t length;
    smap_entry_t entries[0];
} smap_t;

typedef struct __attribute__((packed)) {
    uint8_t status;
    uint8_t chs_first_dh;
    uint8_t chs_first_cl;
    uint8_t chs_first_ch;
    uint8_t type;
    uint8_t chs_last_dh;
    uint8_t chs_last_cl;
    uint8_t chs_last_ch;
    uint32_t lba_first;
    uint32_t size;
} partition_info_t;

typedef struct __attribute__((packed)) {
    smap_t            *smap;
    ptr_t              vbe;
    ptr_t              mode;
    partition_info_t  *partition;
} kernel_pass_t;

static void print(const char* str);
static void print_byte(uint8_t value);
static void print_word(uint16_t value);
static void print_dword(uint32_t value);
static void print_qword(uint64_t value);
static void clear_screen();

#endif
