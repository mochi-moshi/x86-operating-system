#ifndef ASM_H
#define ASM_H
#include "stddef.h"

__attribute__((always_inline))
static inline void cli() { __asm__ __volatile__ ("cli"); }
__attribute__((always_inline))
static inline void hlt() { __asm__ __volatile__ ("hlt"); }
__attribute__((always_inline))
static inline void sti() { __asm__ __volatile__ ("sti"); }
__attribute__((always_inline))
static inline void nop() { __asm__ __volatile__ ("nop"); }

#endif
