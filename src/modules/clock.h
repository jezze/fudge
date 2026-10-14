struct clock_interface
{

    struct resource resource;
    struct service service;
    unsigned int id;
    unsigned int (*getseconds)(void);
    unsigned int (*getminutes)(void);
    unsigned int (*gethours)(void);
    unsigned int (*getweekday)(void);
    unsigned int (*getday)(void);
    unsigned int (*getmonth)(void);
    unsigned int (*getyear)(void);

};

void clock_registerinterface(struct clock_interface *interface);
void clock_unregisterinterface(struct clock_interface *interface);
void clock_initinterface(struct clock_interface *interface, unsigned int id, unsigned int (*getseconds)(void), unsigned int (*getminutes)(void), unsigned int (*gethours)(void), unsigned int (*getweekday)(void), unsigned int (*getday)(void), unsigned int (*getmonth)(void), unsigned int (*getyear)(void));
