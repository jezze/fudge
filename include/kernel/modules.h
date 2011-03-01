#ifndef KERNEL_MODULES_H
#define KERNEL_MODULES_H

#define MODULES_TYPE_BASE              0
#define MODULES_TYPE_BINARY            1
#define MODULES_TYPE_STREAM_BUFFERED   2
#define MODULES_TYPE_STREAM_UNBUFFERED 3

struct modules_module
{

    unsigned int type;

};

struct modules_module_binary
{

    struct modules_module base;
    unsigned int (*check)(void *address);

};

struct modules_module_stream_buffered
{

    struct modules_module base;
    unsigned int (*read)(char *buffer, unsigned int count, unsigned int offset);
    unsigned int (*write)(char *buffer, unsigned int count, unsigned int offset);

};

struct modules_module_stream_unbuffered
{

    struct modules_module base;
    unsigned int (*read)(char *buffer);
    unsigned int (*write)(char *buffer);

};

extern void modules_register_module(struct modules_module *module);
extern void modules_init();

#endif

