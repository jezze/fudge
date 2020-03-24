struct keyboard_interface
{

    struct resource resource;
    struct system_node root;
    struct system_node event;
    unsigned int id;

};

void keyboard_notifypress(struct keyboard_interface *interface, unsigned char scancode);
void keyboard_notifyrelease(struct keyboard_interface *interface, unsigned char scancode);
void keyboard_registerinterface(struct keyboard_interface *interface);
void keyboard_unregisterinterface(struct keyboard_interface *interface);
void keyboard_initinterface(struct keyboard_interface *interface, unsigned int id);
