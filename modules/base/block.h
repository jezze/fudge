struct base_block
{

    struct system_group base;
    unsigned int (*read)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer);

};

void base_register_block(struct base_block *interface, struct base_device *device);
void base_init_block(struct base_block *interface, unsigned int (*read)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct base_device *device, unsigned int offset, unsigned int count, void *buffer));
void base_setup_block();
