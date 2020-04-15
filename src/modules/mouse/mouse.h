struct mouse_interface
{

    struct resource resource;
    struct system_node root;
    struct system_node event;
    unsigned int id;

};

void mouse_notifymove(struct mouse_interface *interface, char relx, char rely);
void mouse_notifyscroll(struct mouse_interface *interface, char relz);
void mouse_notifypress(struct mouse_interface *interface, unsigned int button);
void mouse_notifyrelease(struct mouse_interface *interface, unsigned int button);
void mouse_registerinterface(struct mouse_interface *interface);
void mouse_unregisterinterface(struct mouse_interface *interface);
void mouse_initinterface(struct mouse_interface *interface, unsigned int id);
