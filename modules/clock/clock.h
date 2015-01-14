struct clock_interfacenode
{

    struct system_node base;
    struct system_node timestamp;
    struct system_node date;
    struct system_node time;
    struct clock_interface *interface;

};

struct clock_interface
{

    struct base_interface base;
    struct clock_interfacenode node;
    unsigned char (*getseconds)();
    unsigned char (*getminutes)();
    unsigned char (*gethours)();
    unsigned char (*getweekday)();
    unsigned char (*getday)();
    unsigned char (*getmonth)();
    unsigned short (*getyear)();

};

void clock_registerinterface(struct clock_interface *interface);
void clock_unregisterinterface(struct clock_interface *interface);
void clock_initinterface(struct clock_interface *interface, struct base_driver *driver, struct base_bus *bus, unsigned int id, unsigned char (*getseconds)(), unsigned char (*getminutes)(), unsigned char (*gethours)(), unsigned char (*getweekday)(), unsigned char (*getday)(), unsigned char (*getmonth)(), unsigned short (*getyear)());
