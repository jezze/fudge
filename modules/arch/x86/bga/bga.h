struct bga_driver
{

    struct base_driver base;
    struct base_video ivideo;
    void *bank;
    void *lfb;

};

void bga_init_driver(struct bga_driver *driver);
