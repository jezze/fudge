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

void base_clock_connect_interface(struct base_interface *interface, struct base_bus *bus, unsigned int id);
void base_clock_register_interface(struct base_clock_interface *interface);
void base_clock_unregister_interface(struct base_clock_interface *interface);
void base_clock_init_interface(struct base_clock_interface *interface, unsigned char (*get_seconds)(struct base_bus *bus, unsigned int id), unsigned char (*get_minutes)(struct base_bus *bus, unsigned int id), unsigned char (*get_hours)(struct base_bus *bus, unsigned int id), unsigned char (*get_weekday)(struct base_bus *bus, unsigned int id), unsigned char (*get_day)(struct base_bus *bus, unsigned int id), unsigned char (*get_month)(struct base_bus *bus, unsigned int id), unsigned short (*get_year)(struct base_bus *bus, unsigned int id));
void base_clock_setup();
