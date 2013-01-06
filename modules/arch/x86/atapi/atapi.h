#define ATAPI_DRIVER_TYPE               0x6325

struct atapi_driver
{

    struct base_driver base;
    struct block_interface interface;

};

void atapi_init_driver(struct atapi_driver *driver);
