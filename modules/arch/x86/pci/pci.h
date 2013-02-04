#define PCI_ADDRESS                     0xCF8
#define PCI_DATA                        0xCFC

#define PCI_CLASS_UNKNOWN               0x00
#define PCI_CLASS_STORAGE               0x01
#define PCI_CLASS_NETWORK               0x02
#define PCI_CLASS_DISPLAY               0x03
#define PCI_CLASS_MEDIA                 0x04
#define PCI_CLASS_MEMORY                0x05
#define PCI_CLASS_BRIDGE                0x06
#define PCI_CLASS_SIMPLE                0x07
#define PCI_CLASS_BASE                  0x08
#define PCI_CLASS_INPUT                 0x09
#define PCI_CLASS_DOCK                  0x0A
#define PCI_CLASS_PROCESSOR             0x0B
#define PCI_CLASS_SERIAL                0x0C
#define PCI_CLASS_WIRELESS              0x0D
#define PCI_CLASS_IO                    0x0E
#define PCI_CLASS_SATELLITE             0x0F
#define PCI_CLASS_CRYPT                 0x10
#define PCI_CLASS_SIGNAL                0x11
#define PCI_CLASS_OTHER                 0xFF

#define PCI_CONFIG_VENDOR               0x00
#define PCI_CONFIG_DEVICE               0x02
#define PCI_CONFIG_REVISION             0x08
#define PCI_CONFIG_INTERFACE            0x09
#define PCI_CONFIG_SUBCLASS             0x0A
#define PCI_CONFIG_CLASS                0x0B
#define PCI_CONFIG_HEADER               0x0E
#define PCI_CONFIG_BAR0                 0x10
#define PCI_CONFIG_BAR1                 0x14
#define PCI_CONFIG_BAR2                 0x18
#define PCI_CONFIG_BAR3                 0x1C
#define PCI_CONFIG_BAR4                 0x20
#define PCI_CONFIG_BAR5                 0x24
#define PCI_CONFIG_IRQ_LINE             0x3C
#define PCI_CONFIG_IRQ_PIN              0x3C

#define PCI_BUS_TYPE                    0x0004
#define PCI_DEVICE_TYPE                 0x0004

struct pci_bus;

struct pci_device
{

    struct base_device base;
    struct pci_bus *bus;
    unsigned int num;
    unsigned int slot;
    unsigned int function;
    unsigned int address;

};

struct pci_bus
{

    struct base_bus base;
    struct pci_device devices[64];
    unsigned int devicesCount;

};

unsigned int pci_bus_ind(unsigned int address, unsigned short offset);
unsigned short pci_bus_inw(unsigned int address, unsigned short offset);
unsigned char pci_bus_inb(unsigned int address, unsigned short offset);
void pci_init_bus(struct pci_bus *bus);
void pci_init_device(struct pci_device *device, struct pci_bus *bus, unsigned int num, unsigned int slot, unsigned int function, unsigned int address);
