#ifndef KERNEL_MODULES_H
#define KERNEL_MODULES_H

#define MODULES_TYPE_BUS    1
#define MODULES_TYPE_DEVICE 2
#define MODULES_TYPE_DRIVER 3

#define MODULES_BUS_MAX    8
#define MODULES_DEVICE_MAX 64
#define MODULES_DRIVER_MAX 64

struct modules_module
{

    unsigned int type;

};

struct modules_device
{

    struct modules_module module;
    unsigned int type;

};

struct modules_driver
{

    struct modules_module module;
    unsigned int type;
    struct modules_device *device;

};

struct modules_bus
{

    struct modules_module module;
    unsigned int type;

};

extern struct modules_bus *modules_get_bus(unsigned int type);
extern struct modules_device *modules_get_device(unsigned int type);
extern struct modules_driver *modules_get_driver(unsigned int type);
extern void modules_register_bus(struct modules_bus *bus);
extern void modules_register_device(struct modules_device *device);
extern void modules_register_driver(struct modules_driver *driver);
extern void modules_module_init(struct modules_module *module, unsigned int type);
extern void modules_bus_init(struct modules_bus *bus, unsigned int type);
extern void modules_device_init(struct modules_device *device, unsigned int type);
extern void modules_driver_init(struct modules_driver *driver, unsigned int type);
extern void modules_init();

#endif

