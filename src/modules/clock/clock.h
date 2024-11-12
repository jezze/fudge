struct clock_interface
{

    struct resource resource;
    unsigned int id;
    unsigned int inode;
    unsigned int (*oninfo)(unsigned int source);

};

void clock_registerinterface(struct clock_interface *interface);
void clock_unregisterinterface(struct clock_interface *interface);
void clock_initinterface(struct clock_interface *interface, unsigned int id, unsigned int (*oninfo)(unsigned int source));
