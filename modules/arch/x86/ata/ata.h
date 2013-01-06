#define ATA_DRIVER_TYPE                 0x6324

struct ata_driver
{

    struct base_driver base;
    struct block_interface interface;

};

void ata_init_driver(struct ata_driver *driver);
