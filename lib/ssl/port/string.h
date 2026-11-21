#ifndef SSL_PORT_STRING_H
#define SSL_PORT_STRING_H

#include <stddef.h>

void *memcpy(void *out, const void *in, size_t count);
void *memmove(void *out, const void *in, size_t count);
void *memset(void *out, int value, size_t count);
int memcmp(const void *in1, const void *in2, size_t count);
size_t strlen(const char *in);

#endif
