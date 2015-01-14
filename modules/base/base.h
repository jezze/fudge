struct base_bus
{

    struct resource resource;
    unsigned int type;
    const char *name;
    void (*setup)(struct base_bus *self);
    unsigned int (*next)(struct base_bus *self, unsigned int id);
    unsigned short (*irq)(struct base_bus *self, unsigned int id);

};

struct base_driver
{

    struct resource resource;
    const char *name;
    unsigned int (*match)(struct base_bus *bus, unsigned int id);
    void (*attach)(struct base_bus *bus, unsigned int id);
    void (*detach)(struct base_bus *bus, unsigned int id);

};

struct base_interface
{

    struct resource resource;
    struct base_driver *driver;
    struct base_bus *bus;
    unsigned int id;

};

void base_registerbus(struct base_bus *bus);
void base_registerdriver(struct base_driver *driver);
void base_registerinterface(struct base_interface *interface, struct base_bus *bus, unsigned int id);
void base_unregisterbus(struct base_bus *bus);
void base_unregisterdriver(struct base_driver *driver);
void base_unregisterinterface(struct base_interface *interface);
void base_initbus(struct base_bus *bus, unsigned int type, const char *name, void (*setup)(struct base_bus *self), unsigned int (*next)(struct base_bus *self, unsigned int id), unsigned short (*irq)(struct base_bus *self, unsigned int id));
void base_initdriver(struct base_driver *driver, const char *name, unsigned int (*match)(struct base_bus *bus, unsigned int id), void (*attach)(struct base_bus *bus, unsigned int id), void (*detach)(struct base_bus *bus, unsigned int id));
void base_initinterface(struct base_interface *interface, struct base_driver *driver);
