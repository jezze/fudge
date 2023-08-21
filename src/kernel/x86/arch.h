#define ARCH_GDTPHYSICAL                0x1000
#define ARCH_GDTDESCRIPTORS             256
#define ARCH_IDTPHYSICAL                0x2000
#define ARCH_IDTDESCRIPTORS             256
#define ARCH_TSSDESCRIPTORS             1
#define ARCH_KERNELCODEPHYSICAL         0x00100000
#define ARCH_KERNELMMUPHYSICAL          0x00800000
#define ARCH_KERNELSTACKPHYSICAL        0x00880000
#define ARCH_KERNELSTACKSIZE            0x00008000
#define ARCH_MAILBOXPHYSICAL            0x00900000
#define ARCH_MAILBOXSIZE                0x00001000
#define ARCH_TASKMMUPHYSICAL            0x00A00000
#define ARCH_TASKMMUSIZE                0x00004000
#define ARCH_TASKCODEPHYSICAL           0x04000000
#define ARCH_TASKCODESIZE               0x000F8000
#define ARCH_TASKSTACKSIZE              0x00008000
#define ARCH_TASKSTACKVIRTUAL           0x80000000
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

void arch_map(unsigned char index, unsigned int paddress, unsigned int vaddress, unsigned int size);
void arch_mapuncached(unsigned char index, unsigned int paddress, unsigned int vaddress, unsigned int size);
void arch_mapvideo(unsigned char index, unsigned int paddress, unsigned int vaddress, unsigned int size);
unsigned short arch_resume(struct cpu_general *general, struct cpu_interrupt *interrupt);
void arch_leave(void);
void arch_configuregdt(void);
void arch_configureidt(void);
void arch_configuretss(struct arch_tss *tss, unsigned int id, unsigned int sp);
void arch_setup1(void);
void arch_setup2(void);
