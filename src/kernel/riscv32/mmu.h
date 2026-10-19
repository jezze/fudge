#define MMU_TABLESIZE                   0x1000
#define MMU_PAGESIZE                    0x1000
#define MMU_PAGEMASK                    (MMU_PAGESIZE - 1)
#define MMU_ENTRIES                     1024
#define MMU_LEVELS                      2
#define MMU_FLAGMASK                    0x03FF
#define MMU_TFLAG_PRESENT               0x0001
#define MMU_PFLAG_PRESENT               0x0001
#define MMU_PFLAG_READ                  0x0002
#define MMU_PFLAG_WRITE                 0x0004
#define MMU_PFLAG_EXECUTE               0x0008
#define MMU_PFLAG_USERMODE              0x0010
#define MMU_PFLAG_GLOBAL                0x0020
#define MMU_PFLAG_ACCESSED              0x0040
#define MMU_PFLAG_DIRTY                 0x0080
#define MMU_SATP_SV32                   (1UL << 31)
#define MMU_SATP_PPNMASK                0x003FFFFF

unsigned int mmu_tflags(unsigned int flags);
unsigned int mmu_pflags(unsigned int flags);
unsigned long mmu_gettable(unsigned long daddress, unsigned long vaddress, unsigned int level);
unsigned long mmu_getpage(unsigned long daddress, unsigned long vaddress);
void mmu_settable(unsigned long daddress, unsigned long vaddress, unsigned int level, unsigned long taddress, unsigned int flags);
void mmu_setpage(unsigned long daddress, unsigned long vaddress, unsigned long paddress, unsigned int flags);
