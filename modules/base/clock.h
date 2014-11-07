struct base_clock_interface
{

    struct base_interface base;
    unsigned char (*getseconds)();
    unsigned char (*getminutes)();
    unsigned char (*gethours)();
    unsigned char (*getweekday)();
    unsigned char (*getday)();
    unsigned char (*getmonth)();
    unsigned short (*getyear)();

};

struct base_clock_interfacenode
{

    struct system_node base;
    struct system_node timestamp;
    struct system_node date;
    struct system_node time;
    struct base_clock_interface *interface;

};

void base_clock_registerinterface(struct base_clock_interface *interface);
void base_clock_registerinterfacenode(struct base_clock_interfacenode *node);
void base_clock_unregisterinterface(struct base_clock_interface *interface);
void base_clock_unregisterinterfacenode(struct base_clock_interfacenode *node);
void base_clock_initinterface(struct base_clock_interface *interface, struct base_bus *bus, unsigned int id, unsigned char (*getseconds)(), unsigned char (*getminutes)(), unsigned char (*gethours)(), unsigned char (*getweekday)(), unsigned char (*getday)(), unsigned char (*getmonth)(), unsigned short (*getyear)());
void base_clock_initinterfacenode(struct base_clock_interfacenode *node, struct base_clock_interface *interface);
void base_clock_setup();
