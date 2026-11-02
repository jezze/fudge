struct clock_interface
{

    struct resource resource;
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
void clock_initinterface(struct clock_interface *interface, unsigned int (*getseconds)(void), unsigned int (*getminutes)(void), unsigned int (*gethours)(void), unsigned int (*getweekday)(void), unsigned int (*getday)(void), unsigned int (*getmonth)(void), unsigned int (*getyear)(void));
