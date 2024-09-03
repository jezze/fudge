struct clock_interface
{

    struct resource resource;
    unsigned int id;
    unsigned int ichannel;
    unsigned int (*info)(unsigned int source);

};

void clock_registerinterface(struct clock_interface *interface);
void clock_unregisterinterface(struct clock_interface *interface);
void clock_initinterface(struct clock_interface *interface, unsigned int id, unsigned int ichannel, unsigned int (*info)(unsigned int source));
