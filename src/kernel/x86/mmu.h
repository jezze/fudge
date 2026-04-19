#define MMU_PDSIZE                      0x1000
#define MMU_PTSIZE                      0x1000
#define MMU_PAGESIZE                    0x1000
#define MMU_PAGEMASK                    (MMU_PAGESIZE - 1)
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
#define MMU_EFLAG_PRESENT               0x0001
#define MMU_EFLAG_RW                    0x0002
#define MMU_EFLAG_USER                  0x0004
#define MMU_EFLAG_RESERVED              0x0008
#define MMU_EFLAG_INSTRUCTION           0x0010
#define MMU_EFLAG_PROTECTIONKEY         0x0020
#define MMU_EFLAG_SHADOWSTACK           0x0040
#define MMU_EFLAG_SGX                   0x8000

unsigned int mmu_getdirectory(void);
unsigned int mmu_gettable(unsigned int daddress, unsigned int vaddress);
unsigned int mmu_getpage(unsigned int daddress, unsigned int vaddress);
void mmu_setdirectory(unsigned int daddress);
void mmu_settable(unsigned int daddress, unsigned int vaddress, unsigned int taddress, unsigned int flags);
void mmu_setpage(unsigned int daddress, unsigned int vaddress, unsigned int paddress, unsigned int flags);
void mmu_enable(void);
void mmu_disable(void);
