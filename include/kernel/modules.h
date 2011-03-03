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

struct modules_io_device
{

    struct modules_module base;
    unsigned int (*read)(char *buffer, unsigned int count, unsigned int offset);
    unsigned int (*write)(char *buffer, unsigned int count, unsigned int offset);

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

struct modules_tty_device
{

    struct modules_module base;
    unsigned short cursorOffset;
    unsigned char cursorColor;
    unsigned int (*read)(char *buffer, unsigned int count, unsigned int offset);
    unsigned int (*write)(char *buffer, unsigned int count, unsigned int offset);
    void (*set_color)(unsigned char fg, unsigned char bg);

};

struct modules_kbd_device
{

    struct modules_module base;
    char buffer[256];
    unsigned int bufferSize;
    unsigned int bufferHead;
    unsigned int bufferTail;
    unsigned char toggleAlt;
    unsigned char toggleCtrl;
    unsigned char toggleShift;
    unsigned int (*read)(char *buffer);
    unsigned int (*write)(char *buffer);

};

extern void modules_register_module(struct modules_module *module);
extern struct modules_vga_device *modules_get_vga_device();
extern void modules_set_vga_device(struct modules_vga_device *device);
extern void modules_init();

#endif

