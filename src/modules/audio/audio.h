struct audio_interface
{

    struct resource resource;
    unsigned int id;
    unsigned int ichannel;

};

void audio_registerinterface(struct audio_interface *interface);
void audio_unregisterinterface(struct audio_interface *interface);
void audio_initinterface(struct audio_interface *interface, unsigned int id, unsigned int ichannel);
