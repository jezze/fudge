struct video_interface
{

    struct resource resource;
    struct service service;
    unsigned int id;
    unsigned int width;
    unsigned int height;
    unsigned int bpp;
    void (*oninfo)(struct event_videoinfo *videoinfo);
    unsigned int (*setcmap)(unsigned int count, void *data);
    void (*setmode)(unsigned int width, unsigned int height, unsigned int bpp);


};

struct video_interface *video_findinterface(unsigned int id);
void video_registerinterface(struct video_interface *interface);
void video_unregisterinterface(struct video_interface *interface);
void video_initinterface(struct video_interface *interface, unsigned int id, void (*oninfo)(struct event_videoinfo *videoinfo), unsigned int (*setcmap)(unsigned int count, void *data), void (*setmode)(unsigned int width, unsigned int height, unsigned int bpp));
