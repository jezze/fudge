#ifndef KERNEL_MODULES_H
#define KERNEL_MODULES_H

#define MODULES_BUS_TYPE_ATA 1
#define MODULES_BUS_TYPE_SERIAL 2 
#define MODULES_BUS_TYPE_PCI 3
#define MODULES_BUS_TYPE_USB 4
#define MODULES_BUS_TYPE_VGA 4

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
    char name[32];
    struct file_node node; // REMOVE

};

struct modules_driver
{

    struct modules_module module;
    char name[32];
    struct modules_device *devices[32];

};

struct modules_bus
{

    struct modules_module module;
    char name[32];
    struct modules_device *devices[32];
    struct modules_driver *drivers[32];

};

extern void modules_register(unsigned int type, struct modules_module *module); // REMOVE
extern void modules_register_bus(unsigned int type, struct modules_bus *bus);
extern void modules_register_device(unsigned int type, struct modules_device *device);
extern void modules_register_driver(unsigned int type, struct modules_driver *driver);
extern struct modules_module *modules_find(unsigned int type);
extern void modules_init();

#endif

