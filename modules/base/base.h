#define BASE_RESOURCE_BUS               8021
#define BASE_RESOURCE_DRIVER            8022
#define BASE_RESOURCE_INTERFACE         8023
#define BASE_RESOURCE_DEVICE            8024

struct base_bus
{

    struct resource resource;
    unsigned int type;
    const char *name;
    void (*setup)(struct base_bus *self);
    unsigned int (*device_next)(struct base_bus *self, unsigned int id);
    unsigned short (*device_irq)(struct base_bus *self, unsigned int id);

};

struct base_driver
{

    struct resource resource;
    const char *name;
    unsigned int (*check)(struct base_bus *bus, unsigned int id);
    void (*attach)(struct base_bus *bus, unsigned int id);
    void (*detach)(struct base_bus *bus, unsigned int id);

};

enum base_interface_type
{

    BASE_INTERFACE_TYPE_BLOCK           = 0x01,
    BASE_INTERFACE_TYPE_CLOCK           = 0x02,
    BASE_INTERFACE_TYPE_KEYBOARD        = 0x03,
    BASE_INTERFACE_TYPE_MOUSE           = 0x04,
    BASE_INTERFACE_TYPE_NETWORK         = 0x05,
    BASE_INTERFACE_TYPE_TERMINAL        = 0x06,
    BASE_INTERFACE_TYPE_TIMER           = 0x07,
    BASE_INTERFACE_TYPE_VIDEO           = 0x08

};

struct base_interface
{

    struct resource resource;
    unsigned int type;

};

struct base_device
{

    struct resource resource;
    struct base_interface *interface;
    struct base_bus *bus;
    unsigned int id;

};

void base_register_bus(struct base_bus *bus);
void base_register_driver(struct base_driver *driver);
void base_register_interface(struct base_interface *interface);
void base_unregister_bus(struct base_bus *bus);
void base_unregister_driver(struct base_driver *driver);
void base_unregister_interface(struct base_interface *interface);
void base_init_bus(struct base_bus *bus, unsigned int type, const char *name, void (*setup)(struct base_bus *self), unsigned int (*device_next)(struct base_bus *self, unsigned int id), unsigned short (*device_irq)(struct base_bus *self, unsigned int id));
void base_init_driver(struct base_driver *driver, const char *name, unsigned int (*check)(struct base_bus *bus, unsigned int id), void (*attach)(struct base_bus *bus, unsigned int id), void (*detach)(struct base_bus *bus, unsigned int id));
void base_init_interface(struct base_interface *interface, unsigned int type);
void base_init_device(struct base_device *device, struct base_interface *interface, struct base_bus *bus, unsigned int id);
