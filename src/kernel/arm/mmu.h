#define MMU_PDSIZE                      0x4000
#define MMU_PTSIZE                      0x0400
#define MMU_PAGESIZE                    0x1000
#define MMU_PAGEMASK                    (MMU_PAGESIZE - 1)
#define MMU_TABLEMASK                   (MMU_PTSIZE - 1)
#define MMU_TABLES                      4096
#define MMU_PAGES                       256
#define MMU_TFLAG_PRESENT               0x0001
#define MMU_TFLAG_ALWAYS1               0x0010
#define MMU_PFLAG_PRESENT               0x0002
#define MMU_PFLAG_KERNEL                0x0550
#define MMU_PFLAG_USERREAD              0x0AA0
#define MMU_PFLAG_USERWRITE             0x0FF0
#define MMU_FSR_STATUS                  0x000F
#define MMU_FSR_SECTION                 0x0005
#define MMU_FSR_PAGE                    0x0007

unsigned int mmu_tflags(unsigned int flags);
unsigned int mmu_pflags(unsigned int flags);
unsigned long mmu_gettable(unsigned long daddress, unsigned long vaddress);
unsigned long mmu_getpage(unsigned long daddress, unsigned long vaddress);
void mmu_settable(unsigned long daddress, unsigned long vaddress, unsigned long taddress, unsigned int flags);
void mmu_setpage(unsigned long daddress, unsigned long vaddress, unsigned long paddress, unsigned int flags);
void mmu_enable(void);
