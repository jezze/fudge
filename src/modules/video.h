struct video_interface
{

    struct resource resource;
    unsigned int width;
    unsigned int height;
    unsigned int bpp;
    unsigned int connected;
    unsigned int preferredwidth;
    unsigned int preferredheight;
    unsigned long framebuffer;
    unsigned int (*setcmap)(unsigned int count, void *data);
    void (*setmode)(unsigned int width, unsigned int height, unsigned int bpp);
    void (*flush)(unsigned int x, unsigned int y, unsigned int width, unsigned int height);
    unsigned int (*checkmode)(unsigned int width, unsigned int height, unsigned int bpp);


};

void video_registerinterface(struct video_interface *interface);
void video_unregisterinterface(struct video_interface *interface);
void video_initinterface(struct video_interface *interface, unsigned long framebuffer, unsigned int (*setcmap)(unsigned int count, void *data), void (*setmode)(unsigned int width, unsigned int height, unsigned int bpp));
