struct video_interface
{

    struct resource resource;
    struct service service;
    unsigned int id;
    unsigned int width;
    unsigned int height;
    unsigned int bpp;
    unsigned long framebuffer;
    unsigned int (*setcmap)(unsigned int count, void *data);
    void (*setmode)(unsigned int width, unsigned int height, unsigned int bpp);
    void (*flush)(unsigned int x, unsigned int y, unsigned int width, unsigned int height);


};

struct video_interface *video_findinterface(unsigned int id);
void video_registerinterface(struct video_interface *interface);
void video_unregisterinterface(struct video_interface *interface);
void video_initinterface(struct video_interface *interface, unsigned int id, unsigned long framebuffer, unsigned int (*setcmap)(unsigned int count, void *data), void (*setmode)(unsigned int width, unsigned int height, unsigned int bpp));
