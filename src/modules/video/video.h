struct video_interface
{

    struct resource resource;
    struct system_node root;
    struct system_node ctrl;
    struct system_node data;
    struct system_node colormap;
    struct system_node event;
    unsigned int id;
    unsigned int width;
    unsigned int height;
    unsigned int bpp;

};

void video_notifymode(struct video_interface *interface, void *framebuffer, unsigned int w, unsigned int h, unsigned int bpp);
void video_registerinterface(struct video_interface *interface);
void video_unregisterinterface(struct video_interface *interface);
void video_initinterface(struct video_interface *interface, unsigned int id);
