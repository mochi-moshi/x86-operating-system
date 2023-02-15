#include "entry_c.h"
#include "asm.h"
#define _GLOBAL_OFFSET_TABLE_

static void print(const char* str);
static void print_dword(uint32_t value);
static void clear_screen();

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
const char hta[] = "0123456789ABCDEF";
static void print_dword(uint32_t value) {
    char *holder = "0x00000000 ";
    for(uint8_t i = 0; i < 8; i++)
        *(holder+9-i) = hta[(value >> (4*i)) & 0xF];
    print(holder);
}
