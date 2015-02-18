struct mouse_interface
{

    struct system_interface base;
    struct system_node data;

};

void mouse_notify(struct mouse_interface *interface, unsigned int count, void *buffer);
void mouse_registerinterface(struct mouse_interface *interface, unsigned int id);
void mouse_unregisterinterface(struct mouse_interface *interface);
void mouse_initinterface(struct mouse_interface *interface, struct base_driver *driver);
