#define MMAP_SIZE                       0x00004000
#define MMAP_TYPE_NORMAL                0
#define MMAP_TYPE_ZERO                  1
#define MMAP_TYPE_BINARY                2
#define MMAP_TYPE_MEM                   3
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
    unsigned int foffset;
    unsigned int fsize;
    unsigned int msize;

};

/* the header's other fields take the place of one entry */
#define MMAP_SLOTS                      (MMAP_SIZE / sizeof (struct mmap_entry) - 1)

struct mmap_header
{

    unsigned int nentries;
    unsigned int ntables;
    struct mmap_entry entries[MMAP_SLOTS];

};

struct mmap_entry *mmap_find(struct mmap_header *header, unsigned long vaddress);
void mmap_setbinary(struct mmap_entry *entry, unsigned int foffset, unsigned int fsize, unsigned int msize);
void mmap_initentry(struct mmap_entry *entry, unsigned int type, unsigned long paddress, unsigned long vaddress, unsigned int size, unsigned int flags);
struct mmap_entry *mmap_allocate(struct mmap_header *header, unsigned int type, unsigned long paddress, unsigned long vaddress, unsigned int size, unsigned int flags);
void mmap_remove(struct mmap_header *header, struct mmap_entry *entry);
void mmap_initheader(struct mmap_header *header);
