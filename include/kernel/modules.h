#ifndef KERNEL_MODULES_H
#define KERNEL_MODULES_H

#define MODULES_TYPE_BASE     0
#define MODULES_TYPE_TIMER    1
#define MODULES_TYPE_KEYBOARD 2
#define MODULES_TYPE_DISPLAY  3

struct file_node;

struct modules_module
{

    unsigned int type;

};

struct modules_binary_module
{

    struct modules_module base;
    unsigned int (*check)(struct modules_binary_module *module, void *address);

};

struct modules_io_device
{

    struct modules_module base;
    unsigned int (*read)(char *buffer, unsigned int count, unsigned int offset);
    unsigned int (*write)(char *buffer, unsigned int count, unsigned int offset);
    struct file_node node;

};

struct modules_serial_device
{

    struct modules_module base;
    unsigned int port;
    unsigned int (*read)(struct modules_serial_device *device, char *buffer, unsigned int count);
    unsigned int (*write)(struct modules_serial_device *device, char *buffer, unsigned int count);
    struct file_node node;

};

struct modules_tty_device
{

    struct modules_module base;
    unsigned short cursorOffset;
    unsigned char cursorColor;
    unsigned int (*read)(char *buffer, unsigned int count, unsigned int offset);
    unsigned int (*write)(char *buffer, unsigned int count, unsigned int offset);
    void (*set_color)(unsigned char fg, unsigned char bg);
    struct file_node node;

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
    struct file_node nodeFramebuffer;
    struct file_node nodeCursorColor;
    struct file_node nodeCursorOffset;

};

extern void modules_register(unsigned int type, struct modules_module *module);
extern struct modules_module *modules_find(unsigned int type);

extern unsigned int modules_binary_module_check(struct modules_binary_module *module, void *address);
extern struct modules_io_device *modules_get_io_device();
extern struct modules_kbd_device *modules_get_kbd_device();
extern struct modules_serial_device *modules_get_serial_device();
extern struct modules_tty_device *modules_get_tty_device();
extern struct modules_vga_device *modules_get_vga_device();
extern unsigned int modules_io_device_read(struct modules_io_device *device, char *buffer, unsigned int count, unsigned int offset);
extern unsigned int modules_io_device_write(struct modules_io_device *device, char *buffer, unsigned int count, unsigned int offset);
extern void modules_init();
extern unsigned int modules_kbd_device_read(struct modules_kbd_device *device, char *buffer);
extern unsigned int modules_kbd_device_write(struct modules_kbd_device *device, char *buffer);
extern unsigned int modules_serial_device_read(struct modules_serial_device *device, char *buffer, unsigned int count);
extern unsigned int modules_serial_device_write(struct modules_serial_device *device, char *buffer, unsigned int count);
extern void modules_set_io_device(struct modules_io_device *device);
extern void modules_set_kbd_device(struct modules_kbd_device *device);
extern void modules_set_serial_device(struct modules_serial_device *device);
extern void modules_set_tty_device(struct modules_tty_device *device);
extern void modules_set_vga_device(struct modules_vga_device *device);
extern unsigned int modules_tty_device_read(struct modules_tty_device *device, char *buffer, unsigned int count, unsigned int offset);
extern unsigned int modules_tty_device_write(struct modules_tty_device *device, char *buffer, unsigned int count, unsigned int offset);
extern void modules_tty_device_set_color(struct modules_tty_device *device, unsigned char fg, unsigned char bg);
extern unsigned int modules_vga_device_read_framebuffer(struct modules_vga_device *device, char *buffer, unsigned int count, unsigned int offset);
extern unsigned int modules_vga_device_write_framebuffer(struct modules_vga_device *device, char *buffer, unsigned int count, unsigned int offset);
extern void modules_vga_device_set_cursor_color(struct modules_vga_device *device, unsigned char fg, unsigned char bg);
extern void modules_vga_device_set_cursor_offset(struct modules_vga_device *device, unsigned short offset);

#endif

