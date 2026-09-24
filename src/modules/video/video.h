struct video_interface
{

    struct resource resource;
    struct service service;
    unsigned int id;
    unsigned int width;
    unsigned int height;
    unsigned int bpp;
    void (*oninfo)(struct event_videoinfo *videoinfo);
    unsigned int (*onvideocmap)(unsigned int count, void *data);
    void (*onvideoconf)(unsigned int width, unsigned int height, unsigned int bpp);


};

void video_registerinterface(struct video_interface *interface);
void video_unregisterinterface(struct video_interface *interface);
void video_initinterface(struct video_interface *interface, unsigned int id, void (*oninfo)(struct event_videoinfo *videoinfo), unsigned int (*onvideocmap)(unsigned int count, void *data), void (*onvideoconf)(unsigned int width, unsigned int height, unsigned int bpp));
