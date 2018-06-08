struct clock_interface
{

    struct resource resource;
    struct system_node root;
    struct system_node ctrl;
    unsigned int id;

};

void clock_registerinterface(struct clock_interface *interface);
void clock_unregisterinterface(struct clock_interface *interface);
void clock_initinterface(struct clock_interface *interface, unsigned int id);
