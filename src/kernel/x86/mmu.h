#define MMU_PAGESIZE                    4096
#define MMU_TABLES                      1024
#define MMU_PAGES                       1024
#define MMU_TFLAG_PRESENT               0x0001
#define MMU_TFLAG_WRITEABLE             0x0002
#define MMU_TFLAG_USERMODE              0x0004
#define MMU_TFLAG_WRITETHROUGH          0x0008
#define MMU_TFLAG_CACHEDISABLE          0x0010
#define MMU_TFLAG_ACCESSED              0x0020
#define MMU_TFLAG_LARGEPAGE             0x0080
#define MMU_TFLAG_GLOBAL                0x0100
#define MMU_TFLAG_PAT                   0x1000
#define MMU_PFLAG_PRESENT               0x0001
#define MMU_PFLAG_WRITEABLE             0x0002
#define MMU_PFLAG_USERMODE              0x0004
#define MMU_PFLAG_WRITETHROUGH          0x0008
#define MMU_PFLAG_CACHEDISABLE          0x0010
#define MMU_PFLAG_ACCESSED              0x0020
#define MMU_PFLAG_DIRTY                 0x0040
#define MMU_PFLAG_PAT                   0x0080
#define MMU_PFLAG_GLOBAL                0x0100
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
void mmu_disable(void);
