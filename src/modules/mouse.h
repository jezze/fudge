struct mouse_interface
{

    struct resource resource;
    unsigned int buttons;

};

void mouse_notifymove(struct mouse_interface *interface, char relx, char rely);
void mouse_notifyposition(struct mouse_interface *interface, unsigned short x, unsigned short y);
void mouse_notifyscroll(struct mouse_interface *interface, char relz);
void mouse_notifypress(struct mouse_interface *interface, unsigned int button);
void mouse_notifyrelease(struct mouse_interface *interface, unsigned int button);
void mouse_registerinterface(struct mouse_interface *interface);
void mouse_unregisterinterface(struct mouse_interface *interface);
void mouse_initinterface(struct mouse_interface *interface);
