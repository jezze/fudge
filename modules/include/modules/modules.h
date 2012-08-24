#ifndef MODULES_MODULES_H
#define MODULES_MODULES_H

#define MODULES_TYPE_BUS        1
#define MODULES_TYPE_DEVICE     2
#define MODULES_TYPE_DRIVER     3

#define MODULES_MODULE_SLOTS 128

struct modules_bus;
struct modules_device;
struct modules_driver;

struct modules_base
{

    unsigned int type;
    char *name;

};

struct modules_bus
{

    struct modules_base base;
    unsigned int type;
    void (*scan)(struct modules_bus *self);

};

struct modules_device
{

    struct modules_base base;
    unsigned int type;
    struct modules_driver *driver;

};

struct modules_driver
{

    struct modules_base base;
    unsigned int type;
    void (*start)(struct modules_driver *self);
    unsigned int (*check)(struct modules_driver *self, struct modules_device *device);
    void (*attach)(struct modules_device *device);

};

void modules_register_bus(struct modules_bus *bus);
void modules_register_device(struct modules_device *device);
void modules_register_driver(struct modules_driver *driver);
void modules_unregister_bus(struct modules_bus *bus);
void modules_unregister_device(struct modules_device *device);
void modules_unregister_driver(struct modules_driver *driver);
void modules_bus_init(struct modules_bus *bus, unsigned int type, char *name, void (*scan)(struct modules_bus *self));
void modules_device_init(struct modules_device *device, unsigned int type, char *name);
void modules_driver_init(struct modules_driver *driver, unsigned int type, char *name, void (*start)(struct modules_driver *self), unsigned int (*check)(struct modules_driver *self, struct modules_device *device), void (*attach)(struct modules_device *device));

#endif

