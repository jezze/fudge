struct atapi_driver
{

    struct base_driver base;
    struct block_interface iblock;

};

void atapi_init_driver(struct atapi_driver *driver);
