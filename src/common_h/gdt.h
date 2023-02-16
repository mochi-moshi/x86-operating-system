#ifndef GDT_H
#define GDT_h
#include "stddef.h"

#define GDT_PRESENT    0b10000000
#define GDT_DPL_0      0b00000000
#define GDT_DPL_1      0b00100000
#define GDT_DPL_2      0b01000000
#define GDT_DPL_3      0b01100000
#define GDT_SYSTEM     0b00000000
#define GDT_DATA       0b00010000
#define GDT_CODE       0b00011000
#define GDT_GROW_UP    0b00000000
#define GDT_GROW_DOWN  0b00000100
#define GDT_DPL_ONLY   0b00000000
#define GDT_DPL_MAX    0b00000100
#define GDT_CODE_READ  0b00000010
#define GDT_DATA_WRITE 0b00000010

#define GDT_BYTE_GRAN  0b0000
#define GDT_PAGE_GRAN  0b1000
#define GDT_16BIT      0b0000
#define GDT_32BIT      0b0100

#define GDT_16BIT_TSS_AVL  0x1
#define GDT_LDT            0x2
#define GDT_16BIT_TSS_BSY  0x3
#define GDT_32BIT_TSS_AVL  0x9
#define GDT_32BIT_TSS_BSY  0xB

typedef union {
    struct __attribute__((packed)) {
        uint16_t limit_low;
        uint32_t base_low: 24;
        uint8_t access;
        uint8_t limit_high: 4;
        uint8_t flags: 4;
        uint8_t base_high;
    };
    uint64_t _raw;
} GDT_entry_t;

typedef GDT_entry_t* GDT_t;

typedef struct __attribute__((packed)) {
    uint16_t size;
    GDT_t entries;
} GDTr_t;

static inline void gdt_set_limit(GDT_entry_t* entry, uint32_t limit) {
    if(limit < 0xFFFFF) {
        entry->limit_low = limit & 0xFFFF;
        entry->limit_high = (limit >> 16) & 0xF;
    } else {
        // TODO: ERROR handling
    }
}
static inline void gdt_set_base(GDT_entry_t* entry, uint32_t base) {
    entry->base_low = base & 0xFFFFFF;
    entry->base_high = base >> 24;
}
static inline void gdt_set_flags(GDT_entry_t* entry, uint8_t flags) {
    if(flags < 0xF) {
        entry->flags = flags & 0xF;
    } else {
        // TODO: ERROR handling
    }
}

void gdt_set_entry(GDT_entry_t *entry, uint32_t limit, uint32_t base, uint8_t access, uint8_t flags) {
    gdt_set_limit(entry, limit);
    gdt_set_base(entry, base);
    gdt_set_flags(entry, flags);
    entry->access = access;
}

static inline void gdt_load(GDT_t gdt, uint16_t count) {
    GDTr_t GDTr = {
        .size = count * sizeof(GDT_entry_t),
        .entries = &gdt[0]
    };
    __asm__ __volatile__ ("lgdt %0" : : "m"(GDTr));
}

#endif
