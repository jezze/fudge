#define MMAP_VADDRESS                   0xC0000000
#define MMAP_SIZE                       0x00001000
#define MMAP_TYPE_NONE                  0
#define MMAP_TYPE_COPY                  1
#define MMAP_TYPE_ZERO                  2
#define MMAP_TYPE_FILE                  3

struct mmap_entry
{

    unsigned int type;
    unsigned int paddress;
    unsigned int vaddress;
    unsigned int size;
    unsigned int flags;
    unsigned int ioaddress;
    unsigned int iofsize;
    unsigned int iomsize;
    unsigned int ioflags;

};

struct mmap_header
{

    unsigned int entries;
    unsigned int ntables;

};

void mmap_initentry(struct mmap_entry *entry, unsigned int type, unsigned int paddress, unsigned int vaddress, unsigned int size, unsigned int flags, unsigned int ioaddress, unsigned int iofsize, unsigned int iomsize, unsigned int ioflags);
void mmap_initheader(struct mmap_header *header);
