#ifndef WE26N_TYPE_H
#define WE26N_TYPE_H

#include <stddef.h>
#include <time.h>

typedef char w26n_char;
typedef const char w26n_const_char;
typedef unsigned char w26n_byte;

typedef signed int w26n_bool;

typedef unsigned char w26n_uint8;
typedef signed char w26n_int8;
typedef unsigned short int w26n_uint16;
typedef signed short int w26n_int16;
typedef unsigned int w26n_uint32;
typedef signed int w26n_int32;
typedef unsigned long int w26n_ulong;
typedef signed long int w26n_long;

typedef size_t w26n_size;
typedef time_t w26n_time;

#define BLST_TRUE (1)
#define BLST_FALSE (0)

#endif
