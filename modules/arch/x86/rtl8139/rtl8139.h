#define RTL8139_IDR0                    0x00
#define RTL8139_IDR1                    0x01
#define RTL8139_IDR2                    0x02
#define RTL8139_IDR3                    0x03
#define RTL8139_IDR4                    0x04
#define RTL8139_IDR5                    0x05
#define RTL8139_MAR0                    0x08
#define RTL8139_MAR1                    0x09
#define RTL8139_MAR2                    0x0A
#define RTL8139_MAR3                    0x0B
#define RTL8139_MAR4                    0x0C
#define RTL8139_MAR5                    0x0D
#define RTL8139_MAR6                    0x0E
#define RTL8139_MAR7                    0x0F
#define RTL8139_TSD0                    0x10
#define RTL8139_TSD1                    0x14
#define RTL8139_TSD2                    0x18
#define RTL8139_TSD3                    0x1C
#define RTL8139_TSAD0                   0x20
#define RTL8139_TSAD1                   0x24
#define RTL8139_TSAD2                   0x28
#define RTL8139_TSAD3                   0x2C
#define RTL8139_RBSTART                 0x30
#define RTL8139_ERBCR                   0x34
#define RTL8139_ERSR                    0x36
#define RTL8139_CR                      0x37
#define RTL8139_CAPR                    0x38
#define RTL8139_CBR                     0x3A
#define RTL8139_IMR                     0x3C
#define RTL8139_ISR                     0x3E
#define RTL8139_TCR                     0x40
#define RTL8139_RCR                     0x44
#define RTL8139_TCTR                    0x48
#define RTL8139_MPC                     0x4C
#define RTL8139_9346CR                  0x50
#define RTL8139_CONFIG0                 0x51
#define RTL8139_CONFIG1                 0x52
#define RTL8139_TIMERINT                0x54
#define RTL8139_MSR                     0x58
#define RTL8139_CONFIG3                 0x59
#define RTL8139_CONFIG4                 0x5A
#define RTL8139_MULINT                  0x5C
#define RTL8139_RERID                   0x5E
#define RTL8139_TSAD                    0x60
#define RTL8139_BMCR                    0x62
#define RTL8139_BMSR                    0x64
#define RTL8139_ANAR                    0x66
#define RTL8139_ANLPAR                  0x68
#define RTL8139_ANER                    0x6A
#define RTL8139_DIS                     0x6C
#define RTL8139_FCSC                    0x6E
#define RTL8139_NWAYTR                  0x70
#define RTL8139_REC                     0x72
#define RTL8139_CSCR                    0x74
#define RTL8139_PHY1PARM                0x78
#define RTL8139_TWPARM                  0x7C
#define RTL8139_PHY2PARM                0x80
#define RTL8139_CRC0                    0x84
#define RTL8139_CRC1                    0x85
#define RTL8139_CRC2                    0x86
#define RTL8139_CRC3                    0x87
#define RTL8139_CRC4                    0x88
#define RTL8139_CRC5                    0x89
#define RTL8139_CRC6                    0x8A
#define RTL8139_CRC7                    0x8B
#define RTL8139_WAKEUP0                 0x8C
#define RTL8139_WAKEUP1                 0x94
#define RTL8139_WAKEUP2                 0x9C
#define RTL8139_WAKEUP3                 0xA4
#define RTL8139_WAKEUP4                 0xAC
#define RTL8139_WAKEUP5                 0xB4
#define RTL8139_WAKEUP6                 0xBC
#define RTL8139_WAKEUP7                 0xC4
#define RTL8139_LSBCRC0                 0xCC
#define RTL8139_LSBCRC1                 0xCD
#define RTL8139_LSBCRC2                 0xCE
#define RTL8139_LSBCRC3                 0xCF
#define RTL8139_LSBCRC4                 0xD0
#define RTL8139_LSBCRC5                 0xD1
#define RTL8139_LSBCRC6                 0xD2
#define RTL8139_LSBCRC7                 0xD3
#define RTL8139_CONFIG5                 0xD8

#define RTL8139_ISR_ROK                 (1 << 0)
#define RTL8139_ISR_TOK                 (1 << 2)

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
    struct net_interface interface;
    unsigned short io;
    char rx[0x2600] __attribute__((aligned(4)));
    char tx0[0x800] __attribute__((aligned(4)));
    char tx1[0x800] __attribute__((aligned(4)));
    char tx2[0x800] __attribute__((aligned(4)));
    char tx3[0x800] __attribute__((aligned(4)));
    unsigned int txp;

};

void rtl8139_init_driver(struct rtl8139_driver *driver);
