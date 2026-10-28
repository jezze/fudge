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

#define NULL                            ((void *)0)

void *memcpy(void *out, const void *in, size_t count);
void *memset(void *out, int value, size_t count);
int strncmp(const char *in1, const char *in2, size_t count);
char *strstr(const char *in, const char *find);
void *malloc(size_t size);
void *calloc(size_t count, size_t size);
