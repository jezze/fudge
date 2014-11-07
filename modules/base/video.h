struct base_video_interface
{

    struct base_interface base;
    struct ctrl_videoctrl ctrl;
    void (*setmode)(unsigned int xres, unsigned int yres, unsigned int bpp);
    unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*rcolormap)(unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*wcolormap)(unsigned int offset, unsigned int count, void *buffer);

};

struct base_video_interfacenode
{

    struct system_node base;
    struct system_node ctrl;
    struct system_node data;
    struct system_node colormap;
    struct base_video_interface *interface;

};

void base_video_registerinterface(struct base_video_interface *interface);
void base_video_registerinterfacenode(struct base_video_interfacenode *node);
void base_video_unregisterinterface(struct base_video_interface *interface);
void base_video_unregisterinterfacenode(struct base_video_interfacenode *node);
void base_video_initinterface(struct base_video_interface *interface, struct base_bus *bus, unsigned int id, void (*setmode)(unsigned int xres, unsigned int yres, unsigned int bpp), unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*rcolormap)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wcolormap)(unsigned int offset, unsigned int count, void *buffer));
void base_video_initinterfacenode(struct base_video_interfacenode *node, struct base_video_interface *interface);
void base_video_setup();
