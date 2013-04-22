enum pci_class
{

    PCI_CLASS_UNKNOWN                   = 0x00,
    PCI_CLASS_STORAGE                   = 0x01,
    PCI_CLASS_NETWORK                   = 0x02,
    PCI_CLASS_DISPLAY                   = 0x03,
    PCI_CLASS_MEDIA                     = 0x04,
    PCI_CLASS_MEMORY                    = 0x05,
    PCI_CLASS_BRIDGE                    = 0x06,
    PCI_CLASS_SIMPLE                    = 0x07,
    PCI_CLASS_BASE                      = 0x08,
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

enum pci_config
{

    PCI_CONFIG_VENDOR                   = 0x00,
    PCI_CONFIG_DEVICE                   = 0x02,
    PCI_CONFIG_REVISION                 = 0x08,
    PCI_CONFIG_INTERFACE                = 0x09,
    PCI_CONFIG_SUBCLASS                 = 0x0A,
    PCI_CONFIG_CLASS                    = 0x0B,
    PCI_CONFIG_HEADER                   = 0x0E,
    PCI_CONFIG_BAR0                     = 0x10,
    PCI_CONFIG_BAR1                     = 0x14,
    PCI_CONFIG_BAR2                     = 0x18,
    PCI_CONFIG_BAR3                     = 0x1C,
    PCI_CONFIG_BAR4                     = 0x20,
    PCI_CONFIG_BAR5                     = 0x24,
    PCI_CONFIG_IRQ_LINE                 = 0x3C,
    PCI_CONFIG_IRQ_PIN                  = 0x3C

};

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

};

struct pci_bus
{

    struct base_bus base;
    unsigned short control;
    unsigned short data;
    struct {struct pci_device item[64]; unsigned int count;} devices;

};

unsigned int pci_calculate_address(unsigned int num, unsigned int slot, unsigned int function);
unsigned int pci_bus_ind(struct pci_bus *bus, unsigned int address, unsigned short offset);
unsigned short pci_bus_inw(struct pci_bus *bus, unsigned int address, unsigned short offset);
unsigned char pci_bus_inb(struct pci_bus *bus, unsigned int address, unsigned short offset);
unsigned int pci_device_ind(struct pci_device *device, unsigned short offset);
unsigned short pci_device_inw(struct pci_device *device, unsigned short offset);
unsigned char pci_device_inb(struct pci_device *device, unsigned short offset);
void pci_init_bus(struct pci_bus *bus);
void pci_init_device(struct pci_device *device, struct pci_bus *bus, unsigned int num, unsigned int slot, unsigned int function);
