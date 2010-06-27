#ifndef CBUFFER_H
#define CBUFFER_H

typedef struct cbuffer
{

    char *buffer;
    unsigned int size;
    unsigned int head;
    unsigned int tail;

} cbuffer_t;

extern cbuffer_t cbuffer_create(char *buffer, unsigned int size);
extern void cbuffer_write(cbuffer_t *cbuffer, char c);
extern char cbuffer_read(cbuffer_t *cbuffer);

#endif

