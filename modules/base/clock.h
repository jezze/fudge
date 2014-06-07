struct base_clock_interface
{

    struct base_interface base;
    unsigned char (*get_seconds)(struct base_device *device);
    unsigned char (*get_minutes)(struct base_device *device);
    unsigned char (*get_hours)(struct base_device *device);
    unsigned char (*get_weekday)(struct base_device *device);
    unsigned char (*get_day)(struct base_device *device);
    unsigned char (*get_month)(struct base_device *device);
    unsigned short (*get_year)(struct base_device *device);

};

void base_clock_register_interface(struct base_clock_interface *interface, struct base_device *device);
void base_clock_unregister_interface(struct base_clock_interface *interface);
void base_clock_init_interface(struct base_clock_interface *interface, unsigned char (*get_seconds)(struct base_device *device), unsigned char (*get_minutes)(struct base_device *device), unsigned char (*get_hours)(struct base_device *device), unsigned char (*get_weekday)(struct base_device *device), unsigned char (*get_day)(struct base_device *device), unsigned char (*get_month)(struct base_device *device), unsigned short (*get_year)(struct base_device *device));
void base_clock_setup();
