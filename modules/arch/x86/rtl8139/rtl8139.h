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
