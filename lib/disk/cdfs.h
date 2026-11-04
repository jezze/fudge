#define CDFS_SECTORSIZE                 2048
#define CDFS_DESCRIPTORSECTOR           16
#define CDFS_DESCRIPTORLIMIT            32
#define CDFS_ENTRYSIZE                  33
#define CDFS_TYPE_PRIMARY               1
#define CDFS_TYPE_SUPPLEMENTARY         2
#define CDFS_TYPE_TERMINATOR            255
#define CDFS_FLAG_DIRECTORY             0x02
#define CDFS_FLAG_ASSOCIATED            0x04

struct cdfs_entry
{

    unsigned char length;
    unsigned char extlength;
    unsigned char extent[8];
    unsigned char size[8];
    unsigned char date[7];
    unsigned char flags;
    unsigned char unitsize;
    unsigned char gapsize;
    unsigned char volume[4];
    unsigned char namelength;

};

struct cdfs_descriptor
{

    unsigned char type;
    unsigned char identifier[5];
    unsigned char version;
    unsigned char flags;
    unsigned char system[32];
    unsigned char volume[32];
    unsigned char unused0[8];
    unsigned char sectorcount[8];
    unsigned char escape[32];
    unsigned char setsize[4];
    unsigned char sequence[4];
    unsigned char blocksize[4];
    unsigned char pathtable[24];
    unsigned char root[34];

};

unsigned int cdfs_read16(void *data);
unsigned int cdfs_read32(void *data);
unsigned int cdfs_validate(struct cdfs_descriptor *descriptor);
