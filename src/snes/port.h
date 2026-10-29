#pragma once

typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;
typedef unsigned long long uint64_t;
typedef signed long long int64_t;
typedef __SIZE_TYPE__ size_t;
typedef _Bool bool;

#define true                            1
#define false                           0
#define NULL                            ((void *)0)
#define offsetof(type, member)          __builtin_offsetof(type, member)

void *memcpy(void *out, const void *in, size_t count);
void *memset(void *out, int value, size_t count);
int strcmp(const char *in1, const char *in2);
size_t strlen(const char *in);
void *malloc(size_t size);
