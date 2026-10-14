struct audio_interface
{

    struct resource resource;
    struct service service;
    unsigned int id;
    unsigned int rate;
    unsigned int channels;
    unsigned int bits;
    unsigned int (*setformat)(unsigned int rate, unsigned int channels, unsigned int bits);
    unsigned int (*write)(void *buffer, unsigned int count);

};

void audio_registerinterface(struct audio_interface *interface);
void audio_unregisterinterface(struct audio_interface *interface);
void audio_initinterface(struct audio_interface *interface, unsigned int id, unsigned int (*setformat)(unsigned int rate, unsigned int channels, unsigned int bits), unsigned int (*write)(void *buffer, unsigned int count));
