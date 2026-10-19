#define MMU_ROOTSIZE                    0x0200
#define MMU_TABLESIZE                   0x0200
#define MMU_PAGESIZE                    0x1000
#define MMU_PAGEMASK                    (MMU_PAGESIZE - 1)
#define MMU_TFLAG_PRESENT               0x0002
#define MMU_PFLAG_PRESENT               0x0001
#define MMU_PFLAG_WRITEPROTECT          0x0004
#define MMU_PFLAG_SUPERVISOR            0x0080

unsigned int mmu_pflags(unsigned int flags);
unsigned long mmu_getpointers(unsigned long root, unsigned long vaddress);
unsigned long mmu_getpages(unsigned long root, unsigned long vaddress);
unsigned long mmu_getpage(unsigned long root, unsigned long vaddress);
void mmu_setpointers(unsigned long root, unsigned long vaddress, unsigned long taddress);
void mmu_setpages(unsigned long root, unsigned long vaddress, unsigned long taddress);
void mmu_setpage(unsigned long root, unsigned long vaddress, unsigned long paddress, unsigned int flags);
