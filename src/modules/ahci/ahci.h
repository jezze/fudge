#define AHCI_GHC                        0x04
#define AHCI_GHC_AE                     0x80000000
#define AHCI_GHC_IE                     0x00000002
#define AHCI_IS                         0x08
#define AHCI_PI                         0x0C
#define AHCI_PORT                       0x100
#define AHCI_PORTSIZE                   0x80
#define AHCI_PORTS                      32
#define AHCI_PCLB                       0x00
#define AHCI_PCLBU                      0x04
#define AHCI_PFB                        0x08
#define AHCI_PFBU                       0x0C
#define AHCI_PIS                        0x10
#define AHCI_PIS_DHRS                   0x00000001
#define AHCI_PIS_TFES                   0x40000000
#define AHCI_PIE                        0x14
#define AHCI_PCMD                       0x18
#define AHCI_PCMD_ST                    0x0001
#define AHCI_PCMD_SUD                   0x0002
#define AHCI_PCMD_POD                   0x0004
#define AHCI_PCMD_FRE                   0x0010
#define AHCI_PCMD_FR                    0x4000
#define AHCI_PCMD_CR                    0x8000
#define AHCI_PTFD                       0x20
#define AHCI_PTFD_ERR                   0x01
#define AHCI_PTFD_DRQ                   0x08
#define AHCI_PTFD_BSY                   0x80
#define AHCI_PSIG                       0x24
#define AHCI_PSSTS                      0x28
#define AHCI_PSERR                      0x30
#define AHCI_PCI                        0x38
#define AHCI_HEADER_WRITE               0x0040
#define AHCI_FIS_COMMAND                0x80
#define AHCI_DEVICE_LBA                 0x40
#define AHCI_ATA_READDMAEXT             0x25
#define AHCI_ATA_WRITEDMAEXT            0x35
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

struct ahci_header
{

    unsigned short flags;
    unsigned short prdtl;
    unsigned int prdbc;
    unsigned int ctba;
    unsigned int ctbau;
    unsigned int rsv[4];

};

struct ahci_prd
{

    unsigned int dba;
    unsigned int dbau;
    unsigned int rsv;
    unsigned int dbc;

};

struct ahci_table
{

    unsigned char cfis[64];
    unsigned char acmd[16];
    unsigned char rsv[48];
    struct ahci_prd prd;

};
