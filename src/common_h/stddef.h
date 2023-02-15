#ifndef STDDEF_H
#define STDDEF_H

typedef unsigned char           uint8_t;
typedef signed char             int8_t;
typedef unsigned short int      uint16_t;
typedef short int               int16_t;
typedef unsigned long int       uint32_t;
typedef long int                int32_t;
typedef unsigned long long int  uint64_t;
typedef long long int           int64_t;

typedef void* ptr_t;
typedef uint32_t ptrdiff_t;
typedef uint32_t size_t;
typedef uint32_t addr_t;

#define UINT8_MAX 128
#define INT8_MIN -127
#define INT8_MAX 128
#define UINT16_MAX 65535
#define INT16_MIN -32768
#define INT16_MAX 32768
#define UINT32_MAX 4294967295
#define INT32_MIN -2147483647
#define INT32_MAX 2147483647
#define UINT64_MAX 18446744073709551615
#define INT64_MIN -9223372036854775807
#define INT64_MAX 9223372036854775807



#endif
