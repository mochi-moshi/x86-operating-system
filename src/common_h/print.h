#ifndef PRINT_H
#define PRINT_H
#include "stddef.h"

static char * const __SCREEN = (char*)0xB8000;
static const ptrdiff_t __WIDTH = 80*2;
static const ptrdiff_t __HEIGHT = 25;
static const ptrdiff_t __SIZE = __WIDTH*__HEIGHT;
static char * __cursor = __SCREEN;
static const char __hta[] = "0123456789ABCDEF";
static char *__holder = "0x0000000000000000";
static void scroll(int amt);
__attribute__((used))
static void putch(const char c) {
  if(c == 0xA || c == 0xD) {
    ptrdiff_t offset = __cursor-__SCREEN;
    __cursor += __WIDTH - (offset % __WIDTH);
  } else {
    *__cursor = c;
    __cursor += 2;
  }
  if(__cursor-__SCREEN > __SIZE*2) {
    scroll(1);
    __cursor = __SCREEN+__WIDTH*2*(__HEIGHT-1);
  }
}
__attribute__((used))
static void print(const char* str) {
    for(;*str != 0 && __cursor - __SCREEN < __SIZE; str++)
      putch(*str);
}
__attribute__((used))
static void print_byte(uint8_t value) {
    __holder[2] = __hta[(value >> 4) & 0xF];
    __holder[3] = __hta[(value >> 8) & 0xF];
    __holder[4] = 0;
    print(__holder);
}
__attribute__((used))
static void print_word(uint16_t value) {
    for(uint8_t i = 0; i < 4; i++)
        *(__holder+5-i) = __hta[(value >> (4*i)) & 0xF];
    __holder[6] = 0;
    print(__holder);
}
__attribute__((used))
static void print_dword(uint32_t value) {
    for(uint8_t i = 0; i < 8; i++)
        *(__holder+9-i) = __hta[(value >> (4*i)) & 0xF];
    __holder[10] = 0;
    print(__holder);
}
__attribute__((used))
static void print_qword(uint64_t value) {
    for(uint8_t i = 0; i < 16; i++)
        *(__holder+17-i) = __hta[(value >> (4*i)) & 0xF];
    __holder[18] = 0;
    print(__holder);
}

__attribute__((used))
static void clear_screen() {
    for(__cursor = __SCREEN; __cursor - __SCREEN < __SIZE; __cursor += 2)
        *__cursor = ' ';
    __cursor = __SCREEN;
}
__attribute((used))
static void scroll(int amt) {
  if(amt >= __HEIGHT || -amt >= __HEIGHT) {
    clear_screen();
  } else if(amt < 0) {
    amt = -amt;
    for(size_t j = __HEIGHT-1; j > __HEIGHT-amt-1; j--)
      for(size_t i = 0; i < __WIDTH*2; i++) {
        __SCREEN[i+(j+amt-1)*__WIDTH*2] = __SCREEN[i+(j-1)*__WIDTH*2];
        __SCREEN[i+(j-1)*__WIDTH*2] = ' ';
      }
  } else {
    for(size_t j = (size_t)amt; j < __HEIGHT; j++)
      for(size_t i = 0; i < __WIDTH*2; i++) {
        __SCREEN[i+(j-amt)*__WIDTH*2] = __SCREEN[i+j*__WIDTH*2];
        __SCREEN[i+j*__WIDTH*2] = ' ';
      }
  }
}
#endif
