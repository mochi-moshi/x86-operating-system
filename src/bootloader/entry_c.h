#ifndef ENTRY_C
#define ENTRY_C
#include "stddef.h"

typedef struct __attribute__((packed)) {

} smap_entry_t;

typedef struct __attribute__((packed)) {
    uint16_t length;
    smap_entry_t entries[0];
} smap_t;

typedef struct __attribute__((packed)) {

} vbe_info_block_t;

typedef struct __attribute__((packed)) {

} mode_info_block_t;

typedef struct __attribute__((packed)) {

} partition_info_t;

typedef struct __attribute__((packed)) {
    smap_t            *smap;
    vbe_info_block_t  *vbe;
    mode_info_block_t *mode;
    partition_info_t  *partition;
} kernel_pass_t;

#endif
