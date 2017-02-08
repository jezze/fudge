struct mouse_interface
{

    struct resource resource;
    struct system_node root;
    struct system_node data;
    struct list datalinks;
    unsigned int id;

};

void mouse_notifymove(char relx, char rely);
void mouse_notifypress(unsigned int button);
void mouse_notifyrelease(unsigned int button);
void mouse_registerinterface(struct mouse_interface *interface, unsigned int id);
void mouse_unregisterinterface(struct mouse_interface *interface);
void mouse_initinterface(struct mouse_interface *interface);
