struct clock_interface
{

    struct resource resource;
    struct system_node root;
    struct system_node timestamp;
    struct system_node date;
    struct system_node time;
    unsigned int id;
    unsigned char (*getseconds)();
    unsigned char (*getminutes)();
    unsigned char (*gethours)();
    unsigned char (*getweekday)();
    unsigned char (*getday)();
    unsigned char (*getmonth)();
    unsigned short (*getyear)();

};

void clock_registerinterface(struct clock_interface *interface, unsigned int id);
void clock_unregisterinterface(struct clock_interface *interface);
void clock_initinterface(struct clock_interface *interface, unsigned char (*getseconds)(), unsigned char (*getminutes)(), unsigned char (*gethours)(), unsigned char (*getweekday)(), unsigned char (*getday)(), unsigned char (*getmonth)(), unsigned short (*getyear)());
