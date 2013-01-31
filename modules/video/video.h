struct video_interface
{

    struct base_driver *driver;
    unsigned int bpp;
    unsigned int xres;
    unsigned int yres;
    void (*enable)(struct video_interface *self);
    unsigned int (*read)(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer);

};

struct video_interface_node
{

    struct system_group root;
    struct system_integer bpp;
    struct system_integer xres;
    struct system_integer yres;
    struct video_interface *interface;

};

struct video_protocol
{

    char *name;

};

struct video_protocol_node
{

    struct system_group root;
    struct video_protocol *protocol;

};

void video_register_interface(struct video_interface *interface);
void video_register_protocol(struct video_protocol *protocol);
void video_unregister_interface(struct video_interface *interface);
void video_unregister_protocol(struct video_protocol *protocol);
void video_init_interface(struct video_interface *interface, struct base_driver *driver, void (*enable)(struct video_interface *self), unsigned int (*read)(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer));
void video_init_protocol(struct video_protocol *protocol, char *name);
