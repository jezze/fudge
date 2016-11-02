struct con
{

    struct resource resource;
    struct system_node root;
    struct system_node ctrl;
    struct system_node data;
    struct ctrl_consettings settings;
    struct list links;
    void (*open)();
    void (*close)();

};

void con_init(struct con *con, void (*open)(), void (*close)());
void con_register(struct con *con);
void con_unregister(struct con *con);
