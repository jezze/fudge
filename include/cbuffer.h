#ifndef CBUFFER_H
#define CBUFFER_H

typedef struct cbuffer
{

    char *buffer;
    uint32_t size;
    uint32_t head;
    uint32_t tail;

} cbuffer_t;

extern cbuffer_t cbuffer_create(char *buffer, uint32_t size);
extern void cbuffer_write(cbuffer_t *cbuffer, char c);
extern char cbuffer_read(cbuffer_t *cbuffer);
extern uint8_t cbuffer_full(cbuffer_t *cbuffer);

#endif

