#define MMU_TABLESIZE                   0x1000
#define MMU_PAGESIZE                    0x1000
#define MMU_PAGEMASK                    (MMU_PAGESIZE - 1)
#define MMU_ADDRESSMASK                 0x000FFFFFFFFFF000UL
#define MMU_ENTRIES                     512
#define MMU_LEVELS                      4
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
#define MMU_EFLAG_PRESENT               0x0001
#define MMU_EFLAG_RW                    0x0002
#define MMU_EFLAG_USER                  0x0004
#define MMU_EFLAG_RESERVED              0x0008
#define MMU_EFLAG_INSTRUCTION           0x0010
#define MMU_EFLAG_PROTECTIONKEY         0x0020
#define MMU_EFLAG_SHADOWSTACK           0x0040
#define MMU_EFLAG_SGX                   0x8000

unsigned int mmu_tflags(unsigned int flags);
unsigned int mmu_pflags(unsigned int flags);
unsigned long mmu_gettable(unsigned long daddress, unsigned long vaddress, unsigned int level);
unsigned long mmu_getpage(unsigned long daddress, unsigned long vaddress);
void mmu_settable(unsigned long daddress, unsigned long vaddress, unsigned int level, unsigned long taddress, unsigned int flags);
void mmu_setpage(unsigned long daddress, unsigned long vaddress, unsigned long paddress, unsigned int flags);
