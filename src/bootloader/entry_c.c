#include "entry_c.h"
#include "asm.h"
#define _GLOBAL_OFFSET_TABLE_

static void print(const char* str);
static void print_dword(uint32_t value);
__attribute__((noreturn))
__attribute__((section (".text.first")))
void kernel_entry() {
    kernel_pass_t* kernel_pass;
    __asm__ __volatile__ ("mov %0, %%eax" : "=r"(kernel_pass));
    print("TEST\n");
    print_dword((uint32_t) kernel_pass);
    print_dword((uint32_t) kernel_pass->smap);
    print_dword((uint32_t) kernel_pass->vbe);
    print_dword((uint32_t) kernel_pass->mode);
    print_dword((uint32_t) kernel_pass->partition);
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
            if(offset % WIDTH)
                cursor += WIDTH - offset - 2;
            else
                cursor += WIDTH - 2;
        } else { 
            *cursor = *str;
        }
    }
}
const char hta[] = "0123456789ABCDEF";
static void print_dword(uint32_t value) {
    char *holder = "0x00000000 ";
    for(uint8_t i = 0; i < 8; i++)
        *(holder+9-i) = hta[(value >> (4*i)) & 0xF];
    print(holder);
}
