struct mouse_interface
{

    struct resource resource;
    struct system_node root;
    struct system_node data;
    unsigned int id;

};

void mouse_notify(struct mouse_interface *interface, void *buffer, unsigned int count);
void mouse_registerinterface(struct mouse_interface *interface, unsigned int id);
void mouse_unregisterinterface(struct mouse_interface *interface);
void mouse_initinterface(struct mouse_interface *interface);
