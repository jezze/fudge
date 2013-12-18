struct base_bus;
struct base_device;
struct base_driver;

enum base_type
{

    BASE_TYPE_BUS                       = 0x01,
    BASE_TYPE_DEVICE                    = 0x02,
    BASE_TYPE_DRIVER                    = 0x03

};

struct base_module
{

    struct list_item item;
    enum base_type type;
    const char *name;

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
    unsigned int irq;
    struct base_bus *bus;
    struct base_driver *driver;

};

struct base_driver
{

    struct base_module module;
    unsigned int (*check)(struct base_device *device);
    void (*attach)(struct base_device *device);

};

void base_register_bus(struct base_bus *bus);
void base_register_device(struct base_device *device);
void base_register_driver(struct base_driver *driver);
void base_unregister_bus(struct base_bus *bus);
void base_unregister_device(struct base_device *device);
void base_unregister_driver(struct base_driver *driver);
void base_init_bus(struct base_bus *bus, unsigned int type, const char *name, void (*scan)(struct base_bus *self));
void base_init_device(struct base_device *device, unsigned int type, unsigned int irq, const char *name, struct base_bus *bus);
void base_init_driver(struct base_driver *driver, const char *name, unsigned int (*check)(struct base_device *device), void (*attach)(struct base_device *device));
