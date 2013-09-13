struct video_interface
{

    struct system_group base;
    unsigned int bpp;
    unsigned int xres;
    unsigned int yres;
    void (*mode)(struct base_device *device);
    unsigned int (*read_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*read_colormap)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write_colormap)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer);

};

void video_register_device(struct video_interface *interface, struct base_device *device);
void video_init_interface(struct video_interface *interface, void (*mode)(struct base_device *device), unsigned int (*read_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer), unsigned int (*read_colormap)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_colormap)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer));
