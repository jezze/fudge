#ifndef LIB_CBUFFER_H
#define LIB_CBUFFER_H

struct cbuffer
{

    char *buffer;
    unsigned int size;
    unsigned int head;
    unsigned int tail;

};

extern void cbuffer_init(struct cbuffer *cbuffer, char *buffer, unsigned int size);
extern void cbuffer_write(struct cbuffer *cbuffer, char c);
extern char cbuffer_read(struct cbuffer *cbuffer);

#endif

