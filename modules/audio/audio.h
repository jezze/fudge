struct audio_interface
{

    struct resource resource;
    struct system_node root;
    struct system_node data;
    unsigned int id;
    unsigned int (*rdata)(unsigned int offset, unsigned int size, unsigned int count, void *buffer);
    unsigned int (*wdata)(unsigned int offset, unsigned int size, unsigned int count, void *buffer);

};

void audio_registerinterface(struct audio_interface *interface, unsigned int id);
void audio_unregisterinterface(struct audio_interface *interface);
void audio_initinterface(struct audio_interface *interface, unsigned int (*rdata)(unsigned int offset, unsigned int size, unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int offset, unsigned int size, unsigned int count, void *buffer));
