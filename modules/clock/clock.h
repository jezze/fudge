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
    unsigned char (*getseconds)();
    unsigned char (*getminutes)();
    unsigned char (*gethours)();
    unsigned char (*getweekday)();
    unsigned char (*getday)();
    unsigned char (*getmonth)();
    unsigned short (*getyear)();
    struct clock_interfacenode node;

};

void clock_registerinterface(struct clock_interface *interface, unsigned int id);
void clock_unregisterinterface(struct clock_interface *interface);
void clock_initinterface(struct clock_interface *interface, struct base_driver *driver, unsigned char (*getseconds)(), unsigned char (*getminutes)(), unsigned char (*gethours)(), unsigned char (*getweekday)(), unsigned char (*getday)(), unsigned char (*getmonth)(), unsigned short (*getyear)());
