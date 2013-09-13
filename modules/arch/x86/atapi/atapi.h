struct atapi_driver
{

    struct base_driver base;
    struct base_block iblock;

};

void atapi_init_driver(struct atapi_driver *driver);
