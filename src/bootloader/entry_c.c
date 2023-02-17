#include "entry_c.h"
#include "asm.h"

static GDT_entry_t GDT[3];
static IDT_t IDT;
static memory_region_t *Regions[8];

__attribute__((noreturn))
__attribute__((section (".text.first")))
void kernel_entry() {
    kernel_pass_t* kernel_pass;
    __asm__ __volatile__ ("mov %0, %%eax" : "=r"(kernel_pass));
    
    clear_screen();
//    print("Partition:\n    lba: ");
//    print_dword(kernel_pass->partition->lba_first);
//    print("\n    size: ");
//    print_dword(kernel_pass->partition->size);
    
    smap_t* smap = kernel_pass->smap;
    ptr_t current_start = 0;
    size_t current_size = 0;
    uint8_t current_region = 0;
    for(uint16_t i = 0; i < smap->length; i++) {
        smap_entry_t entry = smap->entries[i];
        if(entry.type == 1) {
            if(current_size == 0) current_start = (ptr_t)entry.base_address;
            current_size += (size_t)entry.length;
        } else if(current_size) {
            Regions[current_region++] = pmm_create_region(current_start, current_start+current_size);
            current_size = 0;
        }
//        print("\nBase Address: ");
//        print_qword(entry.base_address);
//        print("\nLength: ");
//        print_qword(entry.length);
//        print("\nType: ");
        switch(entry.type) {
            case 1:
//                print("Usable");
                break;
            case 2:
//                print("Reserved");
                break;
            case 3:
//                print("ACPI reclaimable");
                break;
            case 4:
//                print("ACPI NVS");
                break;
            case 5:
                break;
            default:
//                print_dword(entry.type);
                break;
        }
    }

    // TODO: SETUP GDT and IDT
    memset(GDT, 0, sizeof(GDT));
    memset(IDT, 0, sizeof(IDT));

    gdt_set_entry(&GDT[1], 0xFFFFF, 0, 0x9A, 0xC);
    gdt_set_entry(&GDT[2], 0xFFFFF, 0, 0x92, 0xC);
    gdt_load(GDT, 3);

    idt_set_entry(&IDT[0], default_int, 0x8, IDT_32BIT_INT, IDT_PRESENT);
    for(uint8_t i = 1; i > 0; ++i)
        idt_set_entry(&IDT[i], default_int, 0x8, IDT_32BIT_INT, IDT_PRESENT);

    // TODO: SETUP PIC
    pic_init(IDT, 0x20, 0x28);
    idt_load(IDT);
    sti();
    // TODO: SETUP Virtual Memory
    for(uint8_t i = 0; Regions[i]; i++) {
        memory_region_t *region = Regions[i];
        print("Start Address:    ");
        print_dword((uint32_t)region->start_address);
        print("\nEnd Address:      ");
        print_dword((uint32_t)region->end_address);
        print("\nDescriptor:       ");
        print_dword((uint32_t)region);
        print("\nBlocks:           ");
        print_dword((uint32_t)region->blocks);
        print("\nBlocks End:       ");
        print_dword((uint32_t)region->blocks + region->blocks_desc.number_of_bytes);
        print("\nNumber of Blocks: ");
        print_dword(region->blocks_desc.number_of_blocks);
        print("\nNumber of Bytes:  ");
        print_dword(region->blocks_desc.number_of_bytes);
        print("\n\n");
    }
    // TODO: Load Kernel Modules into Upper Memory

    for(;;) {
        hlt();
    }
}

__attribute__((interrupt))
void default_int(ptr_t stack) { }

static char * const SCREEN = (char*)0xB8000;
static const ptrdiff_t WIDTH = 80*2;
static const ptrdiff_t HEIGHT = 25;
static const ptrdiff_t SIZE = WIDTH*HEIGHT;
static char * cursor = SCREEN;
static const char hta[] = "0123456789ABCDEF";
static char *holder = "0x0000000000000000";
__attribute__((used))
static void print(const char* str) {
    for(;*str != 0 && cursor - SCREEN < SIZE; str++, cursor+=2) {
        if(*str == 0xA || *str == 0xD) {
            ptrdiff_t offset = cursor-SCREEN;
            cursor += WIDTH - (offset % WIDTH) - 2;
        } else { 
            *cursor = *str;
        }
    }
}
__attribute__((used))
static void clear_screen() {
    for(cursor = SCREEN; cursor - SCREEN < SIZE; cursor += 2)
        *cursor = ' ';
    cursor = SCREEN;
}
__attribute__((used))
static void print_byte(uint8_t value) {
    holder[2] = hta[(value >> 4) & 0xF];
    holder[3] = hta[(value >> 8) & 0xF];
    holder[4] = 0;
    print(holder);
}
__attribute__((used))
static void print_word(uint16_t value) {
    for(uint8_t i = 0; i < 4; i++)
        *(holder+5-i) = hta[(value >> (4*i)) & 0xF];
    holder[6] = 0;
    print(holder);
}
__attribute__((used))
static void print_dword(uint32_t value) {
    for(uint8_t i = 0; i < 8; i++)
        *(holder+9-i) = hta[(value >> (4*i)) & 0xF];
    holder[10] = 0;
    print(holder);
}
__attribute__((used))
static void print_qword(uint64_t value) {
    for(uint8_t i = 0; i < 16; i++)
        *(holder+17-i) = hta[(value >> (4*i)) & 0xF];
    holder[18] = 0;
    print(holder);
}
