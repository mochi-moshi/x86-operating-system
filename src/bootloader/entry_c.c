#include "entry_c.h"
#include "asm.h"
#define _GLOBAL_OFFSET_TABLE_


__attribute__((noreturn))
__attribute__((section (".text.first")))
void kernel_entry() {
    kernel_pass_t* kernel_pass;
    __asm__ __volatile__ ("mov %0, %%eax" : "=r"(kernel_pass));
    
    clear_screen();
    print("Partition:\n    lba: ");
    print_dword(kernel_pass->partition->lba_first);
    print("\n    size: ");
    print_dword(kernel_pass->partition->size);
    
    smap_t* smap = kernel_pass->smap;
    for(uint16_t i = 0; i < smap->length; i++) {
        smap_entry_t entry = smap->entries[i];
        print("\nBase Address: ");
        print_qword(entry.base_address);
        print("\nLength: ");
        print_qword(entry.length);
        print("\nType: ");
        switch(entry.type) {
            case 1:
                print("Usable");
                break;
            case 2:
                print("Reserved");
                break;
            case 3:
                print("ACPI reclaimable");
                break;
            case 4:
                print("ACPI NVS");
                break;
            case 5:
                break;
            default:
                print_dword(entry.type);
                break;
        }
    }

    // TODO: SETUP GDT and IDT
    // TODO: SETUP PIC
    // TODO: SETUP Virtual Memory
    // TODO: Load Kernel Modules into Upper Memory

    cli();
    for(;;) {
        hlt();
    }
}

static char * const SCREEN = (char*)0xB8000;
static const ptrdiff_t WIDTH = 80*2;
static const ptrdiff_t HEIGHT = 25;
static const ptrdiff_t SIZE = WIDTH*HEIGHT;
static char * cursor = SCREEN;
static const char hta[] = "0123456789ABCDEF";
static char *holder = "0x0000000000000000";
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
static void clear_screen() {
    for(cursor = SCREEN; cursor - SCREEN < SIZE; cursor += 2)
        *cursor = ' ';
    cursor = SCREEN;
}
static void print_byte(uint8_t value) {
    holder[2] = hta[(value >> 4) & 0xF];
    holder[3] = hta[(value >> 8) & 0xF];
    holder[4] = 0;
    print(holder);
}
static void print_word(uint16_t value) {
    for(uint8_t i = 0; i < 4; i++)
        *(holder+5-i) = hta[(value >> (4*i)) & 0xF];
    holder[6] = 0;
    print(holder);
}
static void print_dword(uint32_t value) {
    for(uint8_t i = 0; i < 8; i++)
        *(holder+9-i) = hta[(value >> (4*i)) & 0xF];
    holder[10] = 0;
    print(holder);
}
static void print_qword(uint64_t value) {
    for(uint8_t i = 0; i < 16; i++)
        *(holder+17-i) = hta[(value >> (4*i)) & 0xF];
    holder[18] = 0;
    print(holder);
}
