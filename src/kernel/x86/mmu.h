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
#define MMU_EFLAG_PRESENT               (1 << 0)
#define MMU_EFLAG_RW                    (1 << 1)
#define MMU_EFLAG_USER                  (1 << 2)
#define MMU_EFLAG_RESERVED              (1 << 3)
#define MMU_EFLAG_INSTRUCTION           (1 << 4)
#define MMU_EFLAG_PROTECTIONKEY         (1 << 5)
#define MMU_EFLAG_SHADOWSTACK           (1 << 6)
#define MMU_EFLAG_SGX                   (1 << 15)

struct mmu_table
{

    unsigned int pages[MMU_PAGES];

};

struct mmu_directory
{

    unsigned int tables[MMU_TABLES];

};

struct mmu_directory *mmu_getdirectory(void);
unsigned int mmu_gettablevalue(struct mmu_directory *directory, unsigned int vaddress);
unsigned int mmu_gettableaddress(struct mmu_directory *directory, unsigned int vaddress);
unsigned int mmu_gettableflags(struct mmu_directory *directory, unsigned int vaddress);
unsigned int mmu_getpagevalue(struct mmu_directory *directory, unsigned int vaddress);
unsigned int mmu_getpageaddress(struct mmu_directory *directory, unsigned int vaddress);
unsigned int mmu_getpageflags(struct mmu_directory *directory, unsigned int vaddress);
void mmu_setdirectory(struct mmu_directory *directory);
void mmu_settablevalue(struct mmu_directory *directory, unsigned int vaddress, unsigned int value);
void mmu_settableaddress(struct mmu_directory *directory, unsigned int vaddress, unsigned int address);
void mmu_settableflags(struct mmu_directory *directory, unsigned int vaddress, unsigned int flags);
void mmu_setpagevalue(struct mmu_directory *directory, unsigned int vaddress, unsigned int value);
void mmu_setpageaddress(struct mmu_directory *directory, unsigned int vaddress, unsigned int address);
void mmu_setpageflags(struct mmu_directory *directory, unsigned int vaddress, unsigned int flags);
void mmu_enable(void);
void mmu_disable(void);
