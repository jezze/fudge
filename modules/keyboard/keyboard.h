struct keyboard_interface
{

    struct system_interface base;
    struct system_node data;

};

void keyboard_notify(struct keyboard_interface *interface, unsigned int count, void *buffer);
void keyboard_registerinterface(struct keyboard_interface *interface, unsigned int id);
void keyboard_unregisterinterface(struct keyboard_interface *interface);
void keyboard_initinterface(struct keyboard_interface *interface, struct base_driver *driver);
