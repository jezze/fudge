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

struct modules_device
{

    struct modules_module module;
    struct file_node node;

};

extern void modules_register(unsigned int type, struct modules_module *module);
extern struct modules_module *modules_find(unsigned int type);
extern void modules_init();

#endif

