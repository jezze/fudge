struct base_clock_interface
{

    struct base_interface base;
    unsigned char (*get_seconds)(struct base_bus *bus, unsigned int id);
    unsigned char (*get_minutes)(struct base_bus *bus, unsigned int id);
    unsigned char (*get_hours)(struct base_bus *bus, unsigned int id);
    unsigned char (*get_weekday)(struct base_bus *bus, unsigned int id);
    unsigned char (*get_day)(struct base_bus *bus, unsigned int id);
    unsigned char (*get_month)(struct base_bus *bus, unsigned int id);
    unsigned short (*get_year)(struct base_bus *bus, unsigned int id);

};

struct base_clock_node
{

    struct system_group base;
    struct system_stream timestamp;
    struct system_stream date;
    struct system_stream time;
    struct base_device *device;
    struct base_clock_interface *interface;

};

void base_clock_register_interface(struct base_clock_interface *interface);
void base_clock_register_node(struct base_clock_node *node);
void base_clock_unregister_interface(struct base_clock_interface *interface);
void base_clock_unregister_node(struct base_clock_node *node);
void base_clock_init_interface(struct base_clock_interface *interface, unsigned char (*get_seconds)(struct base_bus *bus, unsigned int id), unsigned char (*get_minutes)(struct base_bus *bus, unsigned int id), unsigned char (*get_hours)(struct base_bus *bus, unsigned int id), unsigned char (*get_weekday)(struct base_bus *bus, unsigned int id), unsigned char (*get_day)(struct base_bus *bus, unsigned int id), unsigned char (*get_month)(struct base_bus *bus, unsigned int id), unsigned short (*get_year)(struct base_bus *bus, unsigned int id));
void base_clock_init_node(struct base_clock_node *node, struct base_device *device, struct base_clock_interface *interface);
void base_clock_setup();
