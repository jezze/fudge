struct con
{

    struct resource resource;
    struct system_node root;
    struct system_node ctrl;
    struct system_node data;

};

void con_init(struct con *con);
void con_register(struct con *con);
void con_unregister(struct con *con);
