struct video_interfacenode
{

    struct system_node base;
    struct system_node ctrl;
    struct system_node data;
    struct system_node colormap;
    struct video_interface *interface;

};

struct video_interface
{

    struct base_interface base;
    struct ctrl_videosettings settings;
    void (*setmode)(unsigned int xres, unsigned int yres, unsigned int bpp);
    unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*rcolormap)(unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*wcolormap)(unsigned int offset, unsigned int count, void *buffer);
    struct video_interfacenode node;

};

void video_registerinterface(struct video_interface *interface, struct base_bus *bus, unsigned int id);
void video_unregisterinterface(struct video_interface *interface);
void video_initinterface(struct video_interface *interface, struct base_driver *driver, void (*setmode)(unsigned int xres, unsigned int yres, unsigned int bpp), unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*rcolormap)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wcolormap)(unsigned int offset, unsigned int count, void *buffer));
