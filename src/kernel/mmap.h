#define MMAP_SIZE                       0x00001000
#define MMAP_TYPE_NORMAL                0
#define MMAP_TYPE_ZERO                  1
#define MMAP_TYPE_BINARY                2
#define MMAP_TYPE_MAILBOX               3
#define MMAP_FLAG_GLOBAL                0x01
#define MMAP_FLAG_USERMODE              0x02
#define MMAP_FLAG_WRITEABLE             0x04
#define MMAP_FLAG_WRITETHROUGH          0x08

struct mmap_entry
{

    unsigned int type;
    unsigned long paddress;
    unsigned long vaddress;
    unsigned int size;
    unsigned int flags;
    unsigned long ioaddress;
    unsigned int iofsize;
    unsigned int iomsize;
    unsigned int ioflags;
    unsigned int ichannel;

};

struct mmap_header
{

    unsigned int nentries;
    unsigned int ntables;
    struct mmap_entry entries[64];

};

struct mmap_entry *mmap_find(struct mmap_header *header, unsigned long vaddress);
void mmap_register(struct mmap_header *header, struct mmap_entry *entry);
void mmap_setbinary(struct mmap_entry *entry, unsigned long ioaddress, unsigned int iofsize, unsigned int iomsize, unsigned int ioflags); 
void mmap_setmailbox(struct mmap_entry *entry, unsigned int ichannel);
void mmap_initentry(struct mmap_entry *entry, unsigned int type, unsigned long paddress, unsigned long vaddress, unsigned int size, unsigned int flags);
void mmap_initheader(struct mmap_header *header);
