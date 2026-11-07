struct touch_interface
{

    struct resource resource;

};

void touch_notifycontact(struct touch_interface *interface, unsigned int id, unsigned int tip, unsigned int x, unsigned int y, unsigned int xmax, unsigned int ymax, unsigned int count, unsigned int button, unsigned int time);
void touch_registerinterface(struct touch_interface *interface);
void touch_unregisterinterface(struct touch_interface *interface);
void touch_initinterface(struct touch_interface *interface);
