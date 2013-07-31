#define MMU_PAGESIZE                    4096
#define MMU_TABLES                      1024
#define MMU_PAGES                       1024

enum mmu_tflag
{

    MMU_TFLAG_PRESENT                   = 0x01,
    MMU_TFLAG_WRITEABLE                 = 0x02,
    MMU_TFLAG_USERMODE                  = 0x04,
    MMU_TFLAG_CACHEWRITE                = 0x08,
    MMU_TFLAG_CACHEDISABLE              = 0x10,
    MMU_TFLAG_ACCESSED                  = 0x20,
    MMU_TFLAG_LARGE                     = 0x40,
    MMU_TFLAG_IGNORED                   = 0x80

};

enum mmu_pflag
{

    MMU_PFLAG_PRESENT                   = 0x01,
    MMU_PFLAG_WRITEABLE                 = 0x02,
    MMU_PFLAG_USERMODE                  = 0x04,
    MMU_PFLAG_CACHEWRITE                = 0x08,
    MMU_PFLAG_CACHEDISABLE              = 0x10,
    MMU_PFLAG_ACCESSED                  = 0x20,
    MMU_PFLAG_DIRTY                     = 0x40,
    MMU_PFLAG_GLOBAL                    = 0x80

};

enum mmu_eflag
{

    MMU_EFLAG_PRESENT                   = 0x01,
    MMU_EFLAG_RW                        = 0x02,
    MMU_EFLAG_USER                      = 0x03,
    MMU_EFLAG_RESERVED                  = 0x04,
    MMU_EFLAG_FETCH                     = 0x05

};

struct mmu_table
{

    void *pages[MMU_PAGES];

} __attribute__((aligned(MMU_PAGESIZE)));

struct mmu_directory
{

    struct mmu_table *tables[MMU_TABLES];

} __attribute__((aligned(MMU_PAGESIZE)));

void mmu_map(struct mmu_directory *directory, struct mmu_table *table, unsigned int paddress, unsigned int vaddress, unsigned int size, unsigned int tflags, unsigned int pflags);
void mmu_enable();
void mmu_reload();
struct mmu_directory *mmu_get_directory();
void mmu_set_directory(struct mmu_directory *directory);
