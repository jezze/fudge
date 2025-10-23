#define MMAP_VADDRESS                   0xC0000000
#define MMAP_SIZE                       0x00001000

struct mmap_entry
{

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
void mmap_initentry(struct mmap_entry *entry, unsigned int address, unsigned int fsize, unsigned int msize, unsigned int flags, unsigned int vaddress);
void mmap_initheader(struct mmap_header *header);
