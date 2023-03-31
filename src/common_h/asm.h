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
    __asm__ __volatile__ ("inb %0, %1" : "=a"(ret) : "Nd"(port));
    return ret;
}
__attribute__((always_inline))
static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ __volatile__ ("inw %0, %1" : "=a"(ret) : "Nd"(port));
    return ret;
}
__attribute__((always_inline))
static inline uint32_t inl(uint16_t port) {
    uint32_t ret;
    __asm__ __volatile__ ("ind %0, %1" : "=a"(ret) : "Nd"(port));
    return ret;
}
__attribute__((always_inline))
static inline void insb(uint16_t port, uint8_t *dst, size_t count) {
    __asm__ __volatile__ ("cld; rep; insb" : "+D"(dst), "+c"(count) : "Nd"(port));
}
__attribute__((always_inline))
static inline void insw(uint16_t port, uint16_t *dst, size_t count) {
    __asm__ __volatile__ ("cld; rep; insw" : "+D"(dst), "+c"(count) : "Nd"(port));
}
__attribute__((always_inline))
static inline void insl(uint16_t port, uint32_t *dst, size_t count) {
    __asm__ __volatile__ ("cld; rep; insd" : "+D"(dst), "+c"(count) : "Nd"(port));
}
__attribute__((always_inline))
static inline void io_wait() {
    __asm__ __volatile__ ("outb %0, %1" : : "Nd"(0x80), "al"(0));
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
static inline void outl(uint16_t port, uint32_t value)
    { __asm__ __volatile__ ("outd %0, %1" : : "Nd"(port), "a"(value)); }
__attribute__((always_inline))
static inline void outsb(uint16_t port, uint8_t *src, size_t count) {
    __asm__ __volatile__ ("cld; rep; outsb" : "+S"(src), "+c"(count) : "Nd"(port));
}
__attribute__((always_inline))
static inline void outsw(uint16_t port, uint16_t *src, size_t count) {
    __asm__ __volatile__ ("cld; rep; outsw" : "+S"(src), "+c"(count) : "Nd"(port));
}
__attribute__((always_inline))
static inline void outsl(uint16_t port, uint32_t *src, size_t count) {
    __asm__ __volatile__ ("cld; rep; outsd" : "+S"(src), "+c"(count) : "Nd"(port));
}
__attribute__((always_inline))
static inline void sti() { __asm__ __volatile__ ("sti"); }

#endif
