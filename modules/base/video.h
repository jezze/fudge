struct base_video_interface
{

    struct base_interface base;
    void (*setmode)(struct base_bus *bus, unsigned int id, unsigned int xres, unsigned int yres, unsigned int bpp);
    unsigned int (*rdata)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*wdata)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*rcolormap)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*wcolormap)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer);

};

struct base_video_node
{

    struct system_node base;
    struct system_node data;
    struct system_node colormap;
    struct system_node info;
    struct system_node mode;
    struct base_device *device;
    struct base_video_interface *interface;

};

void base_video_registerinterface(struct base_video_interface *interface);
void base_video_registernode(struct base_video_node *node);
void base_video_unregisterinterface(struct base_video_interface *interface);
void base_video_unregisternode(struct base_video_node *node);
void base_video_initinterface(struct base_video_interface *interface, void (*setmode)(struct base_bus *bus, unsigned int id, unsigned int xres, unsigned int yres, unsigned int bpp), unsigned int (*rdata)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*wdata)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*rcolormap)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*wcolormap)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer));
void base_video_initnode(struct base_video_node *node, struct base_device *device, struct base_video_interface *interface);
void base_video_setup();
