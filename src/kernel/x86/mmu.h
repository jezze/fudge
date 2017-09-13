#define MMU_PAGESIZE                    4096
#define MMU_TABLES                      1024
#define MMU_PAGES                       1024
#define MMU_TFLAG_PRESENT               0x01
#define MMU_TFLAG_WRITEABLE             0x02
#define MMU_TFLAG_USERMODE              0x04
#define MMU_TFLAG_CACHEWRITE            0x08
#define MMU_TFLAG_CACHEDISABLE          0x10
#define MMU_TFLAG_ACCESSED              0x20
#define MMU_TFLAG_LARGE                 0x40
#define MMU_TFLAG_IGNORED               0x80
#define MMU_PFLAG_PRESENT               0x01
#define MMU_PFLAG_WRITEABLE             0x02
#define MMU_PFLAG_USERMODE              0x04
#define MMU_PFLAG_CACHEWRITE            0x08
#define MMU_PFLAG_CACHEDISABLE          0x10
#define MMU_PFLAG_ACCESSED              0x20
#define MMU_PFLAG_DIRTY                 0x40
#define MMU_PFLAG_GLOBAL                0x80
#define MMU_EFLAG_PRESENT               0x01
#define MMU_EFLAG_RW                    0x02
#define MMU_EFLAG_USER                  0x03
#define MMU_EFLAG_RESERVED              0x04
#define MMU_EFLAG_FETCH                 0x05

struct mmu_table
{

    void *pages[MMU_PAGES];

};

struct mmu_directory
{

    struct mmu_table *tables[MMU_TABLES];

};

void mmu_map(struct mmu_directory *directory, struct mmu_table *table, unsigned int paddress, unsigned int vaddress, unsigned int size, unsigned int tflags, unsigned int pflags);
void mmu_setdirectory(struct mmu_directory *directory);
void mmu_enable(void);
