#define AHCI_BUS                        0x0005
#define AHCI_TYPE_H2D                   0x27
#define AHCI_TYPE_D2H                   0x34
#define AHCI_TYPE_DMAACT                0x39
#define AHCI_TYPE_DMASETUP              0x41
#define AHCI_TYPE_DATA                  0x46
#define AHCI_TYPE_BIST                  0x58
#define AHCI_TYPE_PIOSETUP              0x5F
#define AHCI_TYPE_DEVBITS               0xA1
#define AHCI_SIG_ATA                    0x00000101
#define AHCI_SIG_ATAPI                  0xEB140101
#define AHCI_SIG_SEMB                   0xC33C0101
#define AHCI_SIG_PM                     0x96690101

struct ahci_h2d
{

    unsigned char type;
    unsigned char port;
    unsigned char command;
    unsigned char featurel;
    unsigned char lba0;
    unsigned char lba1;
    unsigned char lba2;
    unsigned char device;
    unsigned char lba3;
    unsigned char lba4;
    unsigned char lba5;
    unsigned char featureh;
    unsigned char countl;
    unsigned char counth;
    unsigned char icc;
    unsigned char control;
    unsigned char rsv1[4];

};

struct ahci_d2h
{

    unsigned char type;
    unsigned char port;
    unsigned char status;
    unsigned char error;
    unsigned char lba0;
    unsigned char lba1;
    unsigned char lba2;
    unsigned char device;
    unsigned char lba3;
    unsigned char lba4;
    unsigned char lba5;
    unsigned char rsv2;
    unsigned char countl;
    unsigned char counth;
    unsigned char rsv3[2];
    unsigned char rsv4[4];

};

struct ahci_data
{

    unsigned char type;
    unsigned char port;
    unsigned char rsv1[2];
    unsigned char data[4];

};

struct ahci_piosetup
{

    unsigned char type;
    unsigned char port;
    unsigned char status;
    unsigned char error;
    unsigned char lba0;
    unsigned char lba1;
    unsigned char lba2;
    unsigned char device;
    unsigned char lba3;
    unsigned char lba4;
    unsigned char lba5;
    unsigned char rsv2;
    unsigned char countl;
    unsigned char counth;
    unsigned char rsv3;
    unsigned char estatus;
    unsigned char tc[2];
    unsigned char rsv4[2];

};

struct ahci_dmasetup
{

    unsigned char type;
    unsigned char port;
    unsigned char rsved[2];
    unsigned char DMAbufferID[8];
    unsigned char rsvd[4];
    unsigned char DMAbufOffset[4];
    unsigned char TransferCount[4];
    unsigned char resvd[4];

};
