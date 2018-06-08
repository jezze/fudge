struct audio_interface
{

    struct resource resource;
    struct system_node root;
    struct system_node data;
    unsigned int id;

};

void audio_registerinterface(struct audio_interface *interface);
void audio_unregisterinterface(struct audio_interface *interface);
void audio_initinterface(struct audio_interface *interface, unsigned int id);
