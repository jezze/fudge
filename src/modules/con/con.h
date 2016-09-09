struct con
{

    struct system_node root;
    struct system_node ctrl;
    struct system_node data;
    struct ctrl_consettings settings;
    struct ethernet_interface *interface;
    struct udp_hook hook;

};

void con_init(struct con *con);
void con_register(struct con *con);
void con_unregister(struct con *con);
