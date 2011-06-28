#ifndef KERNEL_MODULES_H
#define KERNEL_MODULES_H

#define MODULES_BUS_TYPE_IO     1
#define MODULES_BUS_TYPE_ATA    2
#define MODULES_BUS_TYPE_PCI    4
#define MODULES_BUS_TYPE_USB    8

#define MODULES_DEVICE_TYPE_IO       1
#define MODULES_DEVICE_TYPE_SERIAL   2
#define MODULES_DEVICE_TYPE_VGA      3
#define MODULES_DEVICE_TYPE_ATA      4
#define MODULES_DEVICE_TYPE_KEYBOARD 5

#define MODULES_DRIVER_TYPE_TIMER    1
#define MODULES_DRIVER_TYPE_KEYBOARD 2
#define MODULES_DRIVER_TYPE_VGA      3
#define MODULES_DRIVER_TYPE_TTY      4

struct vfs_node;

struct modules_module
{

    unsigned int type;

};

struct modules_device
{

    struct modules_module module;
    char name[32];
    struct vfs_node node; // REMOVE

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

struct modules_filesystem
{

    struct modules_module module;
    char name[32];
    struct vfs_node *root;
    struct vfs_node *(*lookup)(struct modules_filesystem *filesystem, unsigned int id);

};

extern void modules_register_bus(unsigned int type, struct modules_bus *bus);
extern void modules_register_device(unsigned int type, struct modules_device *device);
extern void modules_register_driver(unsigned int type, struct modules_driver *driver);
extern void modules_register_filesystem(struct modules_filesystem *filesystem);
extern void modules_init();

#endif

