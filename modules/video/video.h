struct video_interface
{

    struct base_driver *driver;
    unsigned int bpp;
    unsigned int xres;
    unsigned int yres;
    void (*enable)(struct video_interface *self);
    unsigned int (*read_data)(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write_data)(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer);

};

struct video_protocol
{

    char *name;

};

void video_register_interface(struct video_interface *interface);
void video_register_protocol(struct video_protocol *protocol);
void video_unregister_interface(struct video_interface *interface);
void video_unregister_protocol(struct video_protocol *protocol);
void video_init_interface(struct video_interface *interface, struct base_driver *driver, void (*enable)(struct video_interface *self), unsigned int (*read_data)(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer));
void video_init_protocol(struct video_protocol *protocol, char *name);
