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

struct modules_serial_device
{

    struct modules_module base;
    unsigned int port;
    unsigned int (*read)(struct modules_serial_device *device, char *buffer, unsigned int count);
    unsigned int (*write)(struct modules_serial_device *device, char *buffer, unsigned int count);

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

struct modules_vga_device
{

    struct modules_module base;
    unsigned char cursorColor;
    unsigned short cursorOffset;
    unsigned int (*read_framebuffer)(char *buffer, unsigned int count, unsigned int offset);
    unsigned int (*write_framebuffer)(char *buffer, unsigned int count, unsigned int offset);
    void (*set_cursor_color)(unsigned char fg, unsigned char bg);
    void (*set_cursor_offset)(unsigned short offset);

};

extern void modules_register_module(struct modules_module *module);
extern struct modules_vga_device *modules_get_vga_device();
extern void modules_set_vga_device(struct modules_vga_device *device);
extern void modules_init();

#endif

