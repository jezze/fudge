struct video_interface
{

    struct resource resource;
    unsigned int id;
    unsigned int inode;
    unsigned int width;
    unsigned int height;
    unsigned int bpp;
    unsigned int (*onvideocmap)(unsigned int source, unsigned int count, void *data);
    unsigned int (*onvideoconf)(unsigned int source, unsigned int width, unsigned int height, unsigned int bpp);


};

void video_notifymode(struct video_interface *interface, unsigned int framebuffer, unsigned int w, unsigned int h, unsigned int bpp);
void video_registerinterface(struct video_interface *interface);
void video_unregisterinterface(struct video_interface *interface);
void video_initinterface(struct video_interface *interface, unsigned int id, unsigned int (*onvideocmap)(unsigned int source, unsigned int count, void *data), unsigned int (*onvideoconf)(unsigned int source, unsigned int width, unsigned int height, unsigned int bpp));
