struct bga_driver
{

    struct base_driver base;
    struct video_interface ivideo;
    void *bank;
    void *lfb;

};

void bga_init_driver(struct bga_driver *driver);
