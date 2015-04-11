struct sound_interface
{

    struct resource resource;
    struct system_node root;
    struct system_node data;
    unsigned int id;
    unsigned int (*rdata)(unsigned int offset, unsigned int size, unsigned int count, void *buffer);
    unsigned int (*wdata)(unsigned int offset, unsigned int size, unsigned int count, void *buffer);

};

void sound_registerinterface(struct sound_interface *interface, unsigned int id);
void sound_unregisterinterface(struct sound_interface *interface);
void sound_initinterface(struct sound_interface *interface, unsigned int (*rdata)(unsigned int offset, unsigned int size, unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int offset, unsigned int size, unsigned int count, void *buffer));
