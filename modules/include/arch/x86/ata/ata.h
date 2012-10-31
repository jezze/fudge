#define ATA_BUS1_DATA1                  0x0168
#define ATA_BUS0_DATA1                  0x0170
#define ATA_BUS1_DATA0                  0x01E8
#define ATA_BUS0_DATA0                  0x01F0
#define ATA_BUS1_COMMAND1               0x0366
#define ATA_BUS0_COMMAND1               0x0376
#define ATA_BUS1_COMMAND0               0x03E6
#define ATA_BUS0_COMMAND0               0x03F6

#define ATA_COMMAND_ATAPI_EJECT         0x1B
#define ATA_COMMAND_PIO28_READ          0x20
#define ATA_COMMAND_PIO48_READ          0x24
#define ATA_COMMAND_DMA48_READ          0x25
#define ATA_COMMAND_PIO28_WRITE         0x30
#define ATA_COMMAND_PIO48_WRITE         0x34
#define ATA_COMMAND_DMA48_WRITE         0x35
#define ATA_COMMAND_ATAPI               0xA0
#define ATA_COMMAND_ID_ATAPI            0xA1
#define ATA_COMMAND_ATAPI_READ          0xA8
#define ATA_COMMAND_DMA28_READ          0xC8
#define ATA_COMMAND_DMA28_WRITE         0xCA
#define ATA_COMMAND_ID_ATA              0xEC

#define ATA_DATA_FEATURE                0x01
#define ATA_DATA_COUNT0                 0x02
#define ATA_DATA_LBA0                   0x03
#define ATA_DATA_LBA1                   0x04
#define ATA_DATA_LBA2                   0x05
#define ATA_DATA_SELECT                 0x06
#define ATA_DATA_COMMAND                0x07
#define ATA_DATA_COUNT1                 0x08
#define ATA_DATA_LBA3                   0x09
#define ATA_DATA_LBA4                   0x0A
#define ATA_DATA_LBA5                   0x0B

#define ATA_ID_TYPE                     0x00
#define ATA_ID_SERIAL                   0x0A
#define ATA_ID_MODEL                    0x1B
#define ATA_ID_CAP                      0x31
#define ATA_ID_VALID                    0x35
#define ATA_ID_LBA28MAX                 0x3C
#define ATA_ID_SUPPORT                  0x53
#define ATA_ID_LBA48MAX                 0x64

#define ATA_STATUS_ERROR                (1 << 0)
#define ATA_STATUS_DRQ                  (1 << 3)
#define ATA_STATUS_SRV                  (1 << 4)
#define ATA_STATUS_DF                   (1 << 5)
#define ATA_STATUS_RDY                  (1 << 6)
#define ATA_STATUS_BUSY                 (1 << 7)

#define ATA_DEVICE_TYPE_UNKNOWN         0x00
#define ATA_DEVICE_TYPE_ATA             0x01
#define ATA_DEVICE_TYPE_ATAPI           0x02
#define ATA_DEVICE_TYPE_SATA            0x03
#define ATA_DEVICE_TYPE_SATAPI          0x04

#define ATA_IRQ_PRIMARY                 0x0E
#define ATA_IRQ_SECONDARY               0x0F

#define ATA_BUS_TYPE                    0x0002
#define ATA_DEVICE_TYPE                 0x0004
#define ATA_DRIVER_TYPE                 0x6324

struct ata_bus;

struct ata_partition
{

    unsigned char boot;
    unsigned char headStart;
    unsigned char sectorStart;
    unsigned char cylinderStart;
    unsigned char systemId;
    unsigned char headEnd;
    unsigned char sectorEnd;
    unsigned char cylinderEnd;
    unsigned int sectorLba;
    unsigned int sectorTotal;

} __attribute__((packed));

struct ata_device
{

    struct base_device base;
    struct ata_bus *bus;
    struct pci_device *pciDevice;
    unsigned int irq;
    unsigned int type;
    unsigned int slave;
    unsigned char model[41];
    void (*configure_ata)(struct ata_device *self);
    void (*configure_atapi)(struct ata_device *self);
    unsigned int lba28Max;
    unsigned int (*read_lba28)(struct ata_device *self, unsigned int sector, unsigned int count, void *buffer);
    void (*read_lba28_async)(struct ata_device *self, unsigned int sector, unsigned int count, void *buffer);
    unsigned int (*write_lba28)(struct ata_device *self, unsigned int sector, unsigned int count, void *buffer);
    void (*write_lba28_async)(struct ata_device *self, unsigned int sector, unsigned int count, void *buffer);
    unsigned int lba48MaxLow;
    unsigned int lba48MaxHigh;
    unsigned int (*read_lba48)(struct ata_device *self, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer);
    void (*read_lba48_async)(struct ata_device *self, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer);
    unsigned int (*write_lba48)(struct ata_device *self, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer);
    void (*write_lba48_async)(struct ata_device *self, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer);

};

struct ata_bus
{

    struct base_bus base;
    unsigned short control;
    unsigned short data;
    struct ata_device devices[8];
    unsigned int devicesCount;
    void (*sleep)(struct ata_bus *self);
    void (*wait)(struct ata_bus *self);
    void (*select)(struct ata_bus *self, unsigned char operation, unsigned int slave);
    void (*set_lba)(struct ata_bus *self, unsigned char count, unsigned char lba0, unsigned char lba1, unsigned char lba2);
    void (*set_lba2)(struct ata_bus *self, unsigned char count, unsigned char lba3, unsigned char lba4, unsigned char lba5);
    void (*set_command)(struct ata_bus *self, unsigned char command);
    unsigned int (*read_block)(struct ata_bus *self, unsigned int count, void *buffer);
    unsigned int (*read_blocks)(struct ata_bus *self, unsigned int count, void *buffer);
    unsigned int (*write_block)(struct ata_bus *self, unsigned int count, void *buffer);
    unsigned int (*write_blocks)(struct ata_bus *self, unsigned int count, void *buffer);
    void (*add_device)(struct ata_bus *self, unsigned int slave, unsigned int type);

};

struct ata_driver
{

    struct base_driver base;
    struct block_interface interface;

};

void ata_init_device(struct ata_device *device, struct ata_bus *bus, unsigned int irq, unsigned int slave, unsigned int type);
void ata_init_bus(struct ata_bus *bus, unsigned int control, unsigned int data);
void ata_init_driver(struct ata_driver *driver);
