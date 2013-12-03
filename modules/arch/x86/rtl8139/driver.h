struct rtl8139_driver
{

    struct base_driver base;
    struct base_network_interface inetwork;
    unsigned short io;
    unsigned int mmio;
    unsigned char rx[0x2600];
    unsigned char tx0[0x800];
    unsigned char tx1[0x800];
    unsigned char tx2[0x800];
    unsigned char tx3[0x800];
    unsigned short rxp;
    unsigned short txp;

};

void rtl8139_init_driver(struct rtl8139_driver *driver);
