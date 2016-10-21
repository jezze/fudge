struct con
{

    struct resource resource;
    struct system_node root;
    struct system_node ctrl;
    struct system_node data;
    struct ctrl_consettings settings;
    struct ethernet_interface *interface;
    struct list links;
    void (*configure)(unsigned int port);
    void (*open)(struct list_item *link);
    void (*close)(struct list_item *link);

};

void con_init(struct con *con, void (*configure)(unsigned int port), void (*open)(struct list_item *link), void (*close)(struct list_item *link));
void con_register(struct con *con);
void con_unregister(struct con *con);
