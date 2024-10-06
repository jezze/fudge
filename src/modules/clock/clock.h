struct clock_interface
{

    struct resource resource;
    struct node node;
    unsigned int id;
    unsigned int (*oninfo)(struct node *source);

};

void clock_registerinterface(struct clock_interface *interface);
void clock_unregisterinterface(struct clock_interface *interface);
void clock_initinterface(struct clock_interface *interface, unsigned int id, unsigned int (*oninfo)(struct node *source));
