#ifndef KERNEL_MODULES_H
#define KERNEL_MODULES_H

#define MODULES_TYPE_BASE       0
#define MODULES_TYPE_TIMER      1
#define MODULES_TYPE_KEYBOARD   2
#define MODULES_TYPE_VGA        3
#define MODULES_TYPE_IO         4
#define MODULES_TYPE_SERIAL     5
#define MODULES_TYPE_TTY        6
#define MODULES_TYPE_FILESYSTEM 7

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

extern void modules_register(unsigned int type, struct modules_module *module);
extern struct modules_module *modules_find(unsigned int type);

extern unsigned int modules_binary_module_check(struct modules_binary_module *module, void *address);
extern struct modules_tty_device *modules_get_tty_device();
extern struct modules_vga_device *modules_get_vga_device();
extern void modules_init();
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

