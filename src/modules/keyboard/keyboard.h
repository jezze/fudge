struct keyboard_interface
{

    struct resource resource;
    unsigned int id;
    unsigned int inode;

};

void keyboard_notifypress(struct keyboard_interface *interface, unsigned char scancode);
void keyboard_notifyrelease(struct keyboard_interface *interface, unsigned char scancode);
void keyboard_registerinterface(struct keyboard_interface *interface);
void keyboard_unregisterinterface(struct keyboard_interface *interface);
void keyboard_initinterface(struct keyboard_interface *interface, unsigned int id);
