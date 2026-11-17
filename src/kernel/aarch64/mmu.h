#define MMU_TABLESIZE                   0x1000
#define MMU_PAGESIZE                    0x1000
#define MMU_PAGEMASK                    (MMU_PAGESIZE - 1)
#define MMU_ENTRIES                     512
#define MMU_LEVELS                      3
#define MMU_FLAGMASK                    0x0FFF
#define MMU_ADDRESSMASK                 0x0000FFFFFFFFF000UL
#define MMU_TFLAG_PRESENT               0x0001
#define MMU_TFLAG_TABLE                 0x0002
#define MMU_PFLAG_PRESENT               0x0001
#define MMU_PFLAG_PAGE                  0x0002
#define MMU_PFLAG_DEVICE                0x0000
#define MMU_PFLAG_NORMAL                0x0004
#define MMU_PFLAG_USERMODE              0x0040
#define MMU_PFLAG_READONLY              0x0080
#define MMU_PFLAG_SHARED                0x0300
#define MMU_PFLAG_ACCESSED              0x0400
#define MMU_PFLAG_NOTGLOBAL             0x0800

unsigned int mmu_tflags(unsigned int flags);
unsigned int mmu_pflags(unsigned int flags);
unsigned long mmu_gettable(unsigned long daddress, unsigned long vaddress, unsigned int level);
unsigned long mmu_getpage(unsigned long daddress, unsigned long vaddress);
void mmu_settable(unsigned long daddress, unsigned long vaddress, unsigned int level, unsigned long taddress, unsigned int flags);
void mmu_setpage(unsigned long daddress, unsigned long vaddress, unsigned long paddress, unsigned int flags);
