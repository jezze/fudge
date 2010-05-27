#ifndef CBUFFER_H
#define CBUFFER_H

extern void cbuffer_write(char *buffer, uint32_t size, uint32_t head, uint32_t tail, char c);
extern char cbuffer_read(char *buffer, uint32_t size, uint32_t head);

#endif

