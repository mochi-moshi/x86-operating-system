#ifndef IDT_H
#define IDT_H

#define IDT_TASK       0x5
#define IDT_16BIT_INT  0x6
#define IDT_16BIT_TRAP 0x7
#define IDT_32BIT_INT  0xE
#define IDT_32BIT_TRAP 0xF

#define IDT_DPL_0   0b0000
#define IDT_DPL_1   0b0010
#define IDT_DPL_2   0b0100
#define IDT_DPL_3   0b0110
#define IDT_PRESENT 0b1000

typedef union {
    struct __attribute__((packed)) {
        uint16_t address_low;
        uint16_t segment_selector;
        uint8_t _reserved;
        uint8_t type: 4;
        uint8_t flags: 4;
        uint16_t address_high;
    };
    uint64_t _raw;
} IDT_entry_t;

typedef struct __attribute__((packed)) {
    uint16_t size;
    IDT_entry_t *entries;
} IDTr_t;

typedef IDT_entry_t IDT_t[256];

static inline void idt_set_address(IDT_entry_t *entry, ptr_t address) {
    entry->address_low = (uint32_t) address & 0xFFFF;
    entry->address_high = ((uint32_t) address >> 16) & 0xFFFF;
}

static inline void idt_set_type(IDT_entry_t *entry, uint8_t type) {
    if(type < 0xF && type > 0x5 && ((type & 0b0110) || type == 0x5)) {
        entry->flags = type & 0xF;
    } else {
        // TODO: ERROR handling
    }
}

static inline void idt_set_flags(IDT_entry_t *entry, uint8_t flags) {
    if(flags < 0xE && !(flags & 1)) {
        entry->flags = flags & 0xE;
    } else {
        // TODO: ERROR handling
    }
}

static inline void idt_set_entry(IDT_entry_t *entry, ptr_t address, uint16_t segment, uint8_t type, uint8_t flags) {
    idt_set_address(entry, address);
    entry->segment_selector = segment;
    idt_set_type(entry, type);
    idt_set_flags(entry, flags);
}

static inline void idt_load(IDT_t idt) {
    IDTr_t IDTr = {
        .size = sizeof(IDT_t),
        .entries = &idt[0]
    };
    __asm__ __volatile__ ("lidt %0" : : "m"(IDTr));
}

#endif
