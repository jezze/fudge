#define MMAP_VADDRESS                   0xC0000000
#define MMAP_SIZE                       0x00001000
#define MMAP_TYPE_COPY                  1
#define MMAP_TYPE_ZERO                  2
#define MMAP_TYPE_FILE                  3

struct mmap_entry
{

    unsigned int type;
    unsigned int address;
    unsigned int fsize;
    unsigned int msize;
    unsigned int flags;
    unsigned int vaddress;
    unsigned int paddress;
    unsigned int vpaddress;
    unsigned int vpsize;

};

struct mmap_header
{

    unsigned int offset;
    unsigned int entries;

};

void mmap_setmapping(struct mmap_entry *entry, unsigned int paddress, unsigned int pagesize, unsigned int pagemask);
void mmap_initentry(struct mmap_entry *entry, unsigned int type, unsigned int address, unsigned int fsize, unsigned int msize, unsigned int flags, unsigned int vaddress);
void mmap_initheader(struct mmap_header *header);
