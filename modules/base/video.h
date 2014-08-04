struct base_video_interface
{

    struct base_interface base;
    void (*set_mode)(struct base_bus *bus, unsigned int id, unsigned int xres, unsigned int yres, unsigned int bpp);
    unsigned int (*read_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*read_colormap)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write_colormap)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer);

};

struct base_video_node
{

    struct system_group base;
    struct system_stream data;
    struct system_stream colormap;
    struct system_stream info;
    struct system_stream mode;
    struct base_device *device;
    struct base_video_interface *interface;

};

void base_video_register_interface(struct base_video_interface *interface);
void base_video_register_node(struct base_video_node *node);
void base_video_unregister_interface(struct base_video_interface *interface);
void base_video_unregister_node(struct base_video_node *node);
void base_video_init_interface(struct base_video_interface *interface, void (*set_mode)(struct base_bus *bus, unsigned int id, unsigned int xres, unsigned int yres, unsigned int bpp), unsigned int (*read_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*read_colormap)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_colormap)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer));
void base_video_init_node(struct base_video_node *node, struct base_device *device, struct base_video_interface *interface);
void base_video_setup();
