#ifndef STDLIB_H
#define STDLIB_H
#include "stddef.h"

static inline void memset(ptr_t dst, uint8_t value, ptrdiff_t size) {
    for(ptrdiff_t __i = 0; __i < size; __i++) ((uint8_t*)dst)[__i] = value;
}

static inline void memcpy(ptr_t dst, ptr_t src, ptrdiff_t size) {
    for(ptrdiff_t __i = 0; __i < size; __i++) ((uint8_t*)dst)[__i] = ((uint8_t*)src)[__i];
}

#endif
