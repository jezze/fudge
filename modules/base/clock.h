struct base_clock_interface
{

    struct base_interface base;
    unsigned char (*getseconds)(struct base_bus *bus, unsigned int id);
    unsigned char (*getminutes)(struct base_bus *bus, unsigned int id);
    unsigned char (*gethours)(struct base_bus *bus, unsigned int id);
    unsigned char (*getweekday)(struct base_bus *bus, unsigned int id);
    unsigned char (*getday)(struct base_bus *bus, unsigned int id);
    unsigned char (*getmonth)(struct base_bus *bus, unsigned int id);
    unsigned short (*getyear)(struct base_bus *bus, unsigned int id);

};

struct base_clock_node
{

    struct system_node base;
    struct system_node timestamp;
    struct system_node date;
    struct system_node time;
    struct base_device *device;
    struct base_clock_interface *interface;

};

void base_clock_registerinterface(struct base_clock_interface *interface);
void base_clock_registernode(struct base_clock_node *node);
void base_clock_unregisterinterface(struct base_clock_interface *interface);
void base_clock_unregisternode(struct base_clock_node *node);
void base_clock_initinterface(struct base_clock_interface *interface, unsigned char (*getseconds)(struct base_bus *bus, unsigned int id), unsigned char (*getminutes)(struct base_bus *bus, unsigned int id), unsigned char (*gethours)(struct base_bus *bus, unsigned int id), unsigned char (*getweekday)(struct base_bus *bus, unsigned int id), unsigned char (*getday)(struct base_bus *bus, unsigned int id), unsigned char (*getmonth)(struct base_bus *bus, unsigned int id), unsigned short (*getyear)(struct base_bus *bus, unsigned int id));
void base_clock_initnode(struct base_clock_node *node, struct base_device *device, struct base_clock_interface *interface);
void base_clock_setup();
