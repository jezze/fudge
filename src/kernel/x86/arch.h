#define ARCH_GDTADDRESS                 0x1000
#define ARCH_GDTDESCRIPTORS             256
#define ARCH_IDTADDRESS                 0x2000
#define ARCH_IDTDESCRIPTORS             256
#define ARCH_TSSDESCRIPTORS             1
#define ARCH_KERNELCODEADDRESS          0x00100000
#define ARCH_KERNELSTACKADDRESS         0x007F8000
#define ARCH_KERNELSTACKSIZE            0x00008000
#define ARCH_MMUKERNELADDRESS           0x00800000
#define ARCH_MMUTASKADDRESS             0x00C00000
#define ARCH_MMUTASKSIZE                0x00008000
#define ARCH_MAILBOXADDRESS             0x01000000
#define ARCH_MAILBOXSIZE                0x00002000
#define ARCH_TASKCODEADDRESS            0x02000000
#define ARCH_KCODE                      0x01
#define ARCH_KDATA                      0x02
#define ARCH_UCODE                      0x03
#define ARCH_UDATA                      0x04
#define ARCH_TSS                        0x05

struct arch_gdt
{

    struct gdt_pointer pointer;
    struct gdt_descriptor descriptors[ARCH_GDTDESCRIPTORS];

};

struct arch_idt
{

    struct idt_pointer pointer;
    struct idt_descriptor descriptors[ARCH_IDTDESCRIPTORS];

};

struct arch_tss
{

    struct tss_pointer pointer;
    struct tss_descriptor descriptors[ARCH_TSSDESCRIPTORS];

};

void arch_map(unsigned int paddress, unsigned int vaddress, unsigned int size);
void arch_mapuncached(unsigned int paddress, unsigned int vaddress, unsigned int size);
void arch_mapvideo(unsigned int paddress, unsigned int vaddress, unsigned int size);
unsigned short arch_resume(struct cpu_general *general, struct cpu_interrupt *interrupt);
void arch_leave(void);
void arch_configuregdt(void);
void arch_configureidt(void);
void arch_configuretss(struct arch_tss *tss, unsigned int id, unsigned int sp);
void arch_setup1(void);
void arch_setup2(unsigned int address);
