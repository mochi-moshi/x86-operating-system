#ifndef ASM_H
#define ASM_H
#include "stddef.h"

__attribute__((always_inline))
static inline void cli() { __asm__ __volatile__ ("cli"); }
__attribute__((always_inline))
static inline void hlt() { __asm__ __volatile__ ("hlt"); }
__attribute__((always_inline))
static inline uint8_t inb(uint16_t port) { 
    uint8_t ret;
    __asm__ __volatile__ ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
__attribute__((always_inline))
static inline uint16_t inw(uint16_t port) { 
    uint16_t ret;
    __asm__ __volatile__ ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
__attribute__((always_inline))
static inline void nop() { __asm__ __volatile__ ("nop"); }
__attribute__((always_inline))
static inline void outb(uint16_t port, uint8_t value)
    { __asm__ __volatile__ ("outb %0, %1" : : "Nd"(port), "a"(value)); }
__attribute__((always_inline))
static inline void outw(uint16_t port, uint16_t value)
    { __asm__ __volatile__ ("outw %0, %1" : : "Nd"(port), "a"(value)); }
__attribute__((always_inline))
static inline void sti() { __asm__ __volatile__ ("sti"); }

#endif
