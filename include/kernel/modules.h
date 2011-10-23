#ifndef KERNEL_MODULES_H
#define KERNEL_MODULES_H

#define MODULES_TYPE_BUS    1
#define MODULES_TYPE_DEVICE 2
#define MODULES_TYPE_DRIVER 3

struct vfs_node;

struct modules_module
{

    unsigned int type;

};

struct modules_device
{

    struct modules_module module;
    unsigned int type;
    char name[32];
    struct vfs_node node;

};

struct modules_driver
{

    struct modules_module module;
    unsigned int type;
    char name[32];
    struct modules_device *device;
    struct vfs_node node;

};

struct modules_bus
{

    struct modules_module module;
    unsigned int type;
    char name[32];
    struct vfs_node node;

};

extern struct modules_bus *modules_get_bus(unsigned int type);
extern struct modules_device *modules_get_device(unsigned int type);
extern struct modules_driver *modules_get_driver(unsigned int type);
extern void modules_register_bus(struct modules_bus *bus);
extern void modules_register_device(struct modules_device *device);
extern void modules_register_driver(struct modules_driver *driver);
extern void modules_init();

#endif

