struct base_video
{

    unsigned int bpp;
    unsigned int xres;
    unsigned int yres;
    void (*mode)(struct base_device *device);
    unsigned int (*read_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*read_colormap)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write_colormap)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer);

};

void base_register_video(struct base_video *interface, struct base_device *device);
void base_init_video(struct base_video *interface, void (*mode)(struct base_device *device), unsigned int (*read_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_data)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer), unsigned int (*read_colormap)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write_colormap)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer));
void base_setup_video();
