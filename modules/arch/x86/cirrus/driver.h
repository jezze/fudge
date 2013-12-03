struct cirrus_driver
{

    struct base_driver base;
    struct base_video_interface ivideo;

};

void cirrus_init_driver(struct cirrus_driver *driver);
