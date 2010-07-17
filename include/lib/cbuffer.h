#ifndef CBUFFER_H
#define CBUFFER_H

struct cbuffer
{

    char *buffer;
    unsigned int size;
    unsigned int head;
    unsigned int tail;

};

extern struct cbuffer cbuffer_create(char *buffer, unsigned int size);
extern void cbuffer_write(struct cbuffer *cbuffer, char c);
extern char cbuffer_read(struct cbuffer *cbuffer);

#endif

