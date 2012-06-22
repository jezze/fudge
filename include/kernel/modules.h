#ifndef KERNEL_MODULES_H
#define KERNEL_MODULES_H

#define MODULES_TYPE_BUS        1
#define MODULES_TYPE_DEVICE     2
#define MODULES_TYPE_DRIVER     3
#define MODULES_TYPE_FILESYSTEM 4

#define MODULES_MODULE_SLOTS 128

struct modules_bus;
struct modules_device;
struct modules_driver;
struct modules_filesystem;

union modules_module
{

    struct modules_base
    {

        unsigned int type;
        char *name;

    } base;

    struct modules_bus
    {

        struct modules_base base;
        unsigned int type;
        void (*scan)(struct modules_bus *self);

    } bus;

    struct modules_device
    {

        struct modules_base base;
        unsigned int type;
        struct modules_driver *driver;

    } device;

    struct modules_driver
    {

        struct modules_base base;
        unsigned int type;
        void (*start)(struct modules_driver *self);
        unsigned int (*check)(struct modules_driver *self, struct modules_device *device);
        void (*attach)(struct modules_device *device);

    } driver;

    struct modules_filesystem
    {

        struct modules_base base;
        unsigned int type;
        char *path;
        void (*open)(struct modules_filesystem *self, unsigned int id);
        void (*close)(struct modules_filesystem *self, unsigned int id);
        unsigned int (*read)(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer);
        unsigned int (*write)(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer);
        unsigned int (*find)(struct modules_filesystem *self, char *path);
        unsigned int (*get_physical)(struct modules_filesystem *self, unsigned int id);

    } filesystem;

};

struct modules_filesystem *modules_get_filesystem(char *path);
void modules_register_bus(struct modules_bus *bus);
void modules_register_device(struct modules_device *device);
void modules_register_driver(struct modules_driver *driver);
void modules_register_filesystem(struct modules_filesystem *filesystem);
void modules_unregister_bus(struct modules_bus *bus);
void modules_unregister_device(struct modules_device *device);
void modules_unregister_driver(struct modules_driver *driver);
void modules_unregister_filesystem(struct modules_filesystem *filesystem);
void modules_bus_init(struct modules_bus *bus, unsigned int type, char *name, void (*scan)(struct modules_bus *self));
void modules_device_init(struct modules_device *device, unsigned int type, char *name);
void modules_driver_init(struct modules_driver *driver, unsigned int type, char *name, void (*start)(struct modules_driver *self), unsigned int (*check)(struct modules_driver *self, struct modules_device *device), void (*attach)(struct modules_device *device));
void modules_filesystem_init(struct modules_filesystem *filesystem, unsigned int type, char *name, void (*open)(struct modules_filesystem *self, unsigned int id), void (*close)(struct modules_filesystem *self, unsigned int id), unsigned int (*read)(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*find_node)(struct modules_filesystem *self, char *path), unsigned int (*get_physical)(struct modules_filesystem *self, unsigned int id));
void modules_init();

#endif

