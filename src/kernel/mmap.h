struct mmap_header
{

    unsigned int offset;
    unsigned int entries;

};

struct mmap_entry
{

    unsigned int address;
    unsigned int fsize;
    unsigned int msize;
    unsigned int offset;
    unsigned int flags;
    unsigned int paddress;
    unsigned int vaddress;
    unsigned int vpaddress;
    unsigned int vpsize;

};

void mmap_initheader(struct mmap_header *header);
