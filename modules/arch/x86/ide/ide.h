#define IDE_BUS_TYPE                    0x0002
#define IDE_DEVICE_TYPE                 0x0002
#define IDE_DEVICE_TYPE_UNKNOWN         0x00
#define IDE_DEVICE_TYPE_ATA             0x01
#define IDE_DEVICE_TYPE_ATAPI           0x02
#define IDE_DEVICE_TYPE_SATA            0x03
#define IDE_DEVICE_TYPE_SATAPI          0x04

enum ide_control
{

    IDE_CONTROL_ATAPIEJECT              = 0x1B,
    IDE_CONTROL_PIO28READ               = 0x20,
    IDE_CONTROL_PIO48READ               = 0x24,
    IDE_CONTROL_DMA48READ               = 0x25,
    IDE_CONTROL_PIO28WRITE              = 0x30,
    IDE_CONTROL_PIO48WRITE              = 0x34,
    IDE_CONTROL_DMA48WRITE              = 0x35,
    IDE_CONTROL_ATAPI                   = 0xA0,
    IDE_CONTROL_IDATAPI                 = 0xA1,
    IDE_CONTROL_ATAPIREAD               = 0xA8,
    IDE_CONTROL_DMA28READ               = 0xC8,
    IDE_CONTROL_DMA28WRITE              = 0xCA,
    IDE_CONTROL_IDATA                   = 0xEC

};

struct ide_partition
{

    unsigned char boot;
    unsigned char headbase;
    unsigned char sectorbase;
    unsigned char cylinderbase;
    unsigned char systemid;
    unsigned char headlimit;
    unsigned char sectorlimit;
    unsigned char cylinderlimit;
    unsigned char sectorlba[4];
    unsigned char sectortotal[4];

};

struct ide_device
{

    struct base_device base;
    struct pci_device *pciDevice;
    unsigned int type;
    unsigned int slave;
    unsigned char model[41];
    unsigned int lba28Max;
    unsigned int lba48MaxLow;
    unsigned int lba48MaxHigh;
    void (*configure_ata)(struct ide_device *self);
    void (*configure_atapi)(struct ide_device *self);

};

struct ide_bus
{

    struct base_bus base;
    unsigned short control;
    unsigned short data;
    struct {struct ide_device item[8]; unsigned int count;} devices;

};

unsigned int ide_device_read_lba28(struct ide_device *device, unsigned int sector, unsigned int count, void *buffer);
void ide_device_read_lba28_async(struct ide_device *device, unsigned int sector, unsigned int count, void *buffer);
unsigned int ide_device_write_lba28(struct ide_device *device, unsigned int sector, unsigned int count, void *buffer);
void ide_device_write_lba28_async(struct ide_device *device, unsigned int sector, unsigned int count, void *buffer);
unsigned int ide_device_read_lba48(struct ide_device *device, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer);
void ide_device_read_lba48_async(struct ide_device *device, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer);
unsigned int ide_device_write_lba48(struct ide_device *device, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer);
void ide_device_write_lba48_async(struct ide_device *device, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer);
void ide_bus_sleep(struct ide_bus *bus);
void ide_bus_select(struct ide_bus *bus, unsigned char operation, unsigned int slave);
void ide_bus_set_lba(struct ide_bus *bus, unsigned char count, unsigned char lba0, unsigned char lba1, unsigned char lba2);
void ide_bus_set_lba2(struct ide_bus *bus, unsigned char count, unsigned char lba3, unsigned char lba4, unsigned char lba5);
void ide_bus_set_command(struct ide_bus *bus, unsigned char command);
unsigned int ide_bus_read_block(struct ide_bus *bus, unsigned int count, void *buffer);
unsigned int ide_bus_read_blocks(struct ide_bus *bus, unsigned int count, void *buffer);
unsigned int ide_bus_write_block(struct ide_bus *bus, unsigned int count, void *buffer);
unsigned int ide_bus_write_blocks(struct ide_bus *bus, unsigned int count, void *buffer);
void ide_init_device(struct ide_device *device, struct ide_bus *bus, unsigned int irq, unsigned int slave, unsigned int type);
void ide_init_bus(struct ide_bus *bus, unsigned short control, unsigned short data);
