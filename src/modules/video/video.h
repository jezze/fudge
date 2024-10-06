struct video_interface
{

    struct resource resource;
    struct node node;
    unsigned int id;
    unsigned int width;
    unsigned int height;
    unsigned int bpp;
    unsigned int (*onvideocmap)(struct node *source, unsigned int count, void *data);
    unsigned int (*onvideoconf)(struct node *source, unsigned int width, unsigned int height, unsigned int bpp);


};

void video_notifymode(struct video_interface *interface, void *framebuffer, unsigned int w, unsigned int h, unsigned int bpp);
void video_registerinterface(struct video_interface *interface);
void video_unregisterinterface(struct video_interface *interface);
void video_initinterface(struct video_interface *interface, unsigned int id, unsigned int (*onvideocmap)(struct node *source, unsigned int count, void *data), unsigned int (*onvideoconf)(struct node *source, unsigned int width, unsigned int height, unsigned int bpp));
