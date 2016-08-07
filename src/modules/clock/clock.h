struct clock_interface
{

    struct resource resource;
    struct system_node root;
    struct system_node ctrl;
    unsigned int id;
    struct ctrl_clocksettings settings;
    unsigned char (*getseconds)(void);
    unsigned char (*getminutes)(void);
    unsigned char (*gethours)(void);
    unsigned char (*getweekday)(void);
    unsigned char (*getday)(void);
    unsigned char (*getmonth)(void);
    unsigned short (*getyear)(void);

};

void clock_registerinterface(struct clock_interface *interface, unsigned int id);
void clock_unregisterinterface(struct clock_interface *interface);
void clock_initinterface(struct clock_interface *interface, unsigned char (*getseconds)(void), unsigned char (*getminutes)(void), unsigned char (*gethours)(void), unsigned char (*getweekday)(void), unsigned char (*getday)(void), unsigned char (*getmonth)(void), unsigned short (*getyear)(void));
