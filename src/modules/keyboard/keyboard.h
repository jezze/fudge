struct keyboard_interface
{

    struct resource resource;
    struct system_node root;
    struct system_node data;
    struct list datalinks;
    unsigned int id;

};

void keyboard_notify(struct keyboard_interface *interface, void *buffer, unsigned int count);
void keyboard_registerinterface(struct keyboard_interface *interface, unsigned int id);
void keyboard_unregisterinterface(struct keyboard_interface *interface);
void keyboard_initinterface(struct keyboard_interface *interface);
