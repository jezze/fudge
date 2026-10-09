struct clock_interface
{

    struct resource resource;
    struct service service;
    unsigned int id;
    void (*oninfo)(struct event_clockinfo *clockinfo);

};

void clock_registerinterface(struct clock_interface *interface);
void clock_unregisterinterface(struct clock_interface *interface);
void clock_initinterface(struct clock_interface *interface, unsigned int id, void (*oninfo)(struct event_clockinfo *clockinfo));
