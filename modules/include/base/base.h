#ifndef MODULES_BASE_H
#define MODULES_BASE_H

#define BASE_TYPE_BUS        1
#define BASE_TYPE_DEVICE     2
#define BASE_TYPE_DRIVER     3

#define BASE_MODULE_SLOTS 128

struct base_bus;
struct base_device;
struct base_driver;

struct base_module
{

    unsigned int type;
    char *name;

};

struct base_bus
{

    struct base_module module;
    unsigned int type;
    void (*scan)(struct base_bus *self);

};

struct base_device
{

    struct base_module module;
    unsigned int type;
    struct base_driver *driver;

};

struct base_driver
{

    struct base_module module;
    unsigned int type;
    void (*start)(struct base_driver *self);
    unsigned int (*check)(struct base_driver *self, struct base_device *device);
    void (*attach)(struct base_device *device);

};

void base_register_bus(struct base_bus *bus);
void base_register_device(struct base_device *device);
void base_register_driver(struct base_driver *driver);
void base_unregister_bus(struct base_bus *bus);
void base_unregister_device(struct base_device *device);
void base_unregister_driver(struct base_driver *driver);
void base_bus_init(struct base_bus *bus, unsigned int type, char *name, void (*scan)(struct base_bus *self));
void base_device_init(struct base_device *device, unsigned int type, char *name);
void base_driver_init(struct base_driver *driver, unsigned int type, char *name, void (*start)(struct base_driver *self), unsigned int (*check)(struct base_driver *self, struct base_device *device), void (*attach)(struct base_device *device));

#endif

