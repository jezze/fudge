#define RTL8139_HEADER_ROK              (1 << 0)
#define RTL8139_HEADER_FAE              (1 << 1)
#define RTL8139_HEADER_CRC              (1 << 2)
#define RTL8139_HEADER_LONG             (1 << 3)
#define RTL8139_HEADER_RUNT             (1 << 4)
#define RTL8139_HEADER_ISE              (1 << 5)
#define RTL8139_HEADER_BAR              (1 << 13)
#define RTL8139_HEADER_PAM              (1 << 14)
#define RTL8139_HEADER_MAR              (1 << 15)

#define RTL8139_PCI_VENDOR              0x10EC
#define RTL8139_PCI_DEVICE              0x8139

struct rtl8139_header
{

    unsigned short flags;
    unsigned short length;

};

struct rtl8139_driver
{

    struct base_driver base;
    struct net_interface inet;
    unsigned short io;
    unsigned int mmio;
    char rx[0x2600] __attribute__((aligned(4)));
    char tx0[0x800] __attribute__((aligned(4)));
    char tx1[0x800] __attribute__((aligned(4)));
    char tx2[0x800] __attribute__((aligned(4)));
    char tx3[0x800] __attribute__((aligned(4)));
    unsigned short rxp;
    unsigned short txp;

};

void rtl8139_init_driver(struct rtl8139_driver *driver);
