#define PCI_BUS_TYPE                    0x0004
#define PCI_DEVICE_TYPE                 0x0004

enum pci_class
{

    PCI_CLASS_LEGACY                    = 0x00,
    PCI_CLASS_STORAGE                   = 0x01,
    PCI_CLASS_NETWORK                   = 0x02,
    PCI_CLASS_DISPLAY                   = 0x03,
    PCI_CLASS_MEDIA                     = 0x04,
    PCI_CLASS_MEMORY                    = 0x05,
    PCI_CLASS_BRIDGE                    = 0x06,
    PCI_CLASS_SIMPLE                    = 0x07,
    PCI_CLASS_PERIPHERAL                = 0x08,
    PCI_CLASS_INPUT                     = 0x09,
    PCI_CLASS_DOCK                      = 0x0A,
    PCI_CLASS_PROCESSOR                 = 0x0B,
    PCI_CLASS_SERIAL                    = 0x0C,
    PCI_CLASS_WIRELESS                  = 0x0D,
    PCI_CLASS_IO                        = 0x0E,
    PCI_CLASS_SATELLITE                 = 0x0F,
    PCI_CLASS_CRYPT                     = 0x10,
    PCI_CLASS_SIGNAL                    = 0x11,
    PCI_CLASS_OTHER                     = 0xFF

};

enum pci_class_legacy
{

    PCI_CLASS_LEGACY_NONVGA             = 0x00,
    PCI_CLASS_LEGACY_VGA                = 0x01

};

enum pci_class_storage
{

    PCI_CLASS_STORAGE_SCSI              = 0x00,
    PCI_CLASS_STORAGE_IDE               = 0x01,
    PCI_CLASS_STORAGE_FLOPPY            = 0x02,
    PCI_CLASS_STORAGE_IPI               = 0x03,
    PCI_CLASS_STORAGE_RAID              = 0x04,
    PCI_CLASS_STORAGE_ATA               = 0x05,
    PCI_CLASS_STORAGE_SATA              = 0x06,
    PCI_CLASS_STORAGE_SAS               = 0x07,
    PCI_CLASS_STORAGE_OTHER             = 0x80

};

enum pci_class_network
{

    PCI_CLASS_NETWORK_ETHERNET          = 0x00,
    PCI_CLASS_NETWORK_TOKENRING         = 0x01,
    PCI_CLASS_NETWORK_FDDI              = 0x02,
    PCI_CLASS_NETWORK_ATM               = 0x03,
    PCI_CLASS_NETWORK_ISDN              = 0x04,
    PCI_CLASS_NETWORK_WORLDFIP          = 0x05,
    PCI_CLASS_NETWORK_PICMG             = 0x06,
    PCI_CLASS_NETWORK_OTHER             = 0x80

};

enum pci_class_display
{

    PCI_CLASS_DISPLAY_VGA               = 0x00,
    PCI_CLASS_DISPLAY_XGA               = 0x01,
    PCI_CLASS_DISPLAY_3D                = 0x02,
    PCI_CLASS_DISPLAY_OTHER             = 0x80

};

enum pci_class_media
{

    PCI_CLASS_MEDIA_VIDEO               = 0x00,
    PCI_CLASS_MEDIA_AUDIO               = 0x01,
    PCI_CLASS_MEDIA_TELE                = 0x02,
    PCI_CLASS_MEDIA_OTHER               = 0x80

};

enum pci_class_memory
{

    PCI_CLASS_MEMORY_RAM                = 0x00,
    PCI_CLASS_MEMORY_FLASH              = 0x01,
    PCI_CLASS_MEMORY_OTHER              = 0x80

};

enum pci_class_bridge
{

    PCI_CLASS_BRIDGE_HOST               = 0x00,
    PCI_CLASS_BRIDGE_ISA                = 0x01,
    PCI_CLASS_BRIDGE_EISA               = 0x02,
    PCI_CLASS_BRIDGE_MCA                = 0x03,
    PCI_CLASS_BRIDGE_PCI                = 0x04,
    PCI_CLASS_BRIDGE_PCMCIA             = 0x05,
    PCI_CLASS_BRIDGE_NUBUS              = 0x06,
    PCI_CLASS_BRIDGE_CARDBUS            = 0x07,
    PCI_CLASS_BRIDGE_RACEWAY            = 0x08,
    PCI_CLASS_BRIDGE_PCISEMI            = 0x09,
    PCI_CLASS_BRIDGE_INFINIBRAND        = 0x0A,
    PCI_CLASS_BRIDGE_OTHER              = 0x80

};

enum pci_class_simple
{

    PCI_CLASS_SIMPLE_SERIAL             = 0x00,
    PCI_CLASS_SIMPLE_PARALLEL           = 0x01,
    PCI_CLASS_SIMPLE_MULTIPORT          = 0x02,
    PCI_CLASS_SIMPLE_MODEM              = 0x03,
    PCI_CLASS_SIMPLE_GPIB               = 0x04,
    PCI_CLASS_SIMPLE_SMART              = 0x05,
    PCI_CLASS_SIMPLE_OTHER              = 0x80

};

enum pci_class_peripheral
{

    PCI_CLASS_PERIPHERAL_PIC            = 0x00,
    PCI_CLASS_PERIPHERAL_DMA            = 0x01,
    PCI_CLASS_PERIPHERAL_TIMER          = 0x02,
    PCI_CLASS_PERIPHERAL_RTC            = 0x03,
    PCI_CLASS_PERIPHERAL_PCI            = 0x04,
    PCI_CLASS_PERIPHERAL_OTHER          = 0x80

};

enum pci_class_input
{

    PCI_CLASS_INPUT_KEYBOARD            = 0x00,
    PCI_CLASS_INPUT_DIGITIZER           = 0x01,
    PCI_CLASS_INPUT_MOUSE               = 0x02,
    PCI_CLASS_INPUT_SCANNER             = 0x03,
    PCI_CLASS_INPUT_CONTROLLER          = 0x04,
    PCI_CLASS_INPUT_OTHER               = 0x80

};

enum pci_class_dock
{

    PCI_CLASS_DOCK_GENERIC              = 0x01,
    PCI_CLASS_DOCK_OTHER                = 0x80

};

enum pci_class_processor
{

    PCI_CLASS_PROCESSOR_386             = 0x00,
    PCI_CLASS_PROCESSOR_486             = 0x01,
    PCI_CLASS_PROCESSOR_PENTIUM         = 0x02,
    PCI_CLASS_PROCESSOR_ALPHA           = 0x10,
    PCI_CLASS_PROCESSOR_POWERPC         = 0x20,
    PCI_CLASS_PROCESSOR_MIPS            = 0x30,
    PCI_CLASS_PROCESSOR_COPROCESSOR     = 0x40

};

enum pci_class_serial
{

    PCI_CLASS_SERIAL_FIREWIRE           = 0x00,
    PCI_CLASS_SERIAL_ACCESS             = 0x01,
    PCI_CLASS_SERIAL_SSA                = 0x02,
    PCI_CLASS_SERIAL_USB                = 0x03,
    PCI_CLASS_SERIAL_FIBRE              = 0x04,
    PCI_CLASS_SERIAL_SMBUS              = 0x05,
    PCI_CLASS_SERIAL_INFINIBAND         = 0x06,
    PCI_CLASS_SERIAL_IPMI               = 0x07,
    PCI_CLASS_SERIAL_SERCOS             = 0x08,
    PCI_CLASS_SERIAL_CANBUS             = 0x09

};

enum pci_class_wireless
{

    PCI_CLASS_WIRELESS_IRDA             = 0x00,
    PCI_CLASS_WIRELESS_IR               = 0x01,
    PCI_CLASS_WIRELESS_RF               = 0x10,
    PCI_CLASS_WIRELESS_BLUETOOTH        = 0x11,
    PCI_CLASS_WIRELESS_BROADBAND        = 0x12,
    PCI_CLASS_WIRELESS_80211A           = 0x20,
    PCI_CLASS_WIRELESS_80211B           = 0x21,
    PCI_CLASS_WIRELESS_OTHER            = 0x80

};

enum pci_class_io
{

    PCI_CLASS_IO_GENERIC                = 0x00

};

enum pci_class_satellite
{

    PCI_CLASS_SATELLITE_TV              = 0x01,
    PCI_CLASS_SATELLITE_AUDIO           = 0x02,
    PCI_CLASS_SATELLITE_VOICE           = 0x03,
    PCI_CLASS_SATELLITE_DATA            = 0x04

};

enum pci_class_crypt
{

    PCI_CLASS_CRYPT_NETWORK             = 0x00,
    PCI_CLASS_CRYPT_ENTERTAINMENT       = 0x01,
    PCI_CLASS_CRYPT_OTHER               = 0x80

};

enum pci_class_signal
{

    PCI_CLASS_SIGNAL_DPIO               = 0x00,
    PCI_CLASS_SIGNAL_PERF               = 0x01,
    PCI_CLASS_SIGNAL_SYNC               = 0x10,
    PCI_CLASS_SIGNAL_MANAGEMENT         = 0x20,
    PCI_CLASS_SIGNAL_OTHER              = 0x80

};

enum pci_config
{

    PCI_CONFIG_VENDOR                   = 0x00,
    PCI_CONFIG_DEVICE                   = 0x02,
    PCI_CONFIG_COMMAND                  = 0x04,
    PCI_CONFIG_STATUS                   = 0x06,
    PCI_CONFIG_REVISION                 = 0x08,
    PCI_CONFIG_INTERFACE                = 0x09,
    PCI_CONFIG_SUBCLASS                 = 0x0A,
    PCI_CONFIG_CLASS                    = 0x0B,
    PCI_CONFIG_CACHE                    = 0x0C,
    PCI_CONFIG_TIMER                    = 0x0D,
    PCI_CONFIG_HEADER                   = 0x0E,
    PCI_CONFIG_BIST                     = 0x0F,
    PCI_CONFIG_BAR0                     = 0x10,
    PCI_CONFIG_BAR1                     = 0x14,
    PCI_CONFIG_BAR2                     = 0x18,
    PCI_CONFIG_BAR3                     = 0x1C,
    PCI_CONFIG_BAR4                     = 0x20,
    PCI_CONFIG_BAR5                     = 0x24,
    PCI_CONFIG_CARDBUS                  = 0x28,
    PCI_CONFIG_SUBVENDOR                = 0x2C,
    PCI_CONFIG_SUBSYSTEM                = 0x2E,
    PCI_CONFIG_EXPANSION                = 0x30,
    PCI_CONFIG_CAPABILITIES             = 0x34,
    PCI_CONFIG_LINE                     = 0x3C,
    PCI_CONFIG_PIN                      = 0x3D,
    PCI_CONFIG_GRANT                    = 0x3E,
    PCI_CONFIG_LATENCY                  = 0x3F

};

struct pci_device
{

    struct base_device base;
    unsigned int num;
    unsigned int slot;
    unsigned int function;

};

struct pci_bus
{

    struct base_bus base;
    unsigned short control;
    unsigned short data;
    struct {struct pci_device item[64]; unsigned int count;} devices;

};

unsigned int pci_bus_ind(struct pci_bus *bus, unsigned int num, unsigned int slot, unsigned int function, unsigned short offset);
unsigned short pci_bus_inw(struct pci_bus *bus, unsigned int num, unsigned int slot, unsigned int function, unsigned short offset);
unsigned char pci_bus_inb(struct pci_bus *bus, unsigned int num, unsigned int slot, unsigned int function, unsigned short offset);
void pci_bus_outd(struct pci_bus *bus, unsigned int num, unsigned int slot, unsigned int function, unsigned short offset, unsigned int value);
void pci_bus_outw(struct pci_bus *bus, unsigned int num, unsigned int slot, unsigned int function, unsigned short offset, unsigned short value);
void pci_bus_outb(struct pci_bus *bus, unsigned int num, unsigned int slot, unsigned int function, unsigned short offset, unsigned char value);
unsigned int pci_device_ind(struct pci_device *device, unsigned short offset);
unsigned short pci_device_inw(struct pci_device *device, unsigned short offset);
unsigned char pci_device_inb(struct pci_device *device, unsigned short offset);
void pci_device_outd(struct pci_device *device, unsigned short offset, unsigned int value);
void pci_device_outw(struct pci_device *device, unsigned short offset, unsigned short value);
void pci_device_outb(struct pci_device *device, unsigned short offset, unsigned char value);
void pci_init_bus(struct pci_bus *bus, unsigned short control, unsigned short data);
void pci_init_device(struct pci_device *device, struct pci_bus *bus, unsigned int irq, unsigned int num, unsigned int slot, unsigned int function);
