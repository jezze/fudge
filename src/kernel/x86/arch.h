#define ARCH_GDT_BASE                   0x00001000
#define ARCH_GDT_DESCRIPTORS            256
#define ARCH_IDT_BASE                   0x00002000
#define ARCH_IDT_DESCRIPTORS            256
#define ARCH_SMP_BASE16                 0x00008000
#define ARCH_SMP_BASE32                 0x00008200
#define ARCH_TSS_DESCRIPTORS            1
#define ARCH_KERNEL_CODEBASE            0x00100000
#define ARCH_KERNEL_STACKBASE           0x00600000
#define ARCH_KERNEL_STACKSIZE           0x00002000
#define ARCH_MMAP_BASE                  0x00800000
#define ARCH_MMU_KERNELBASE             0x00A00000
#define ARCH_MMU_KERNELSIZE             0x00200000
#define ARCH_MMU_TASKBASE               0x00C00000
#define ARCH_MMU_TASKSIZE               0x00008000
#define ARCH_MAILBOX_BASE               0x01000000
#define ARCH_TASK_CODEBASE              0x02000000
#define ARCH_KCODE                      0x01
#define ARCH_KDATA                      0x02
#define ARCH_UCODE                      0x03
#define ARCH_UDATA                      0x04
#define ARCH_TSS                        0x05

struct arch_gdt
{

    struct gdt_pointer pointer;
    struct gdt_descriptor descriptors[ARCH_GDT_DESCRIPTORS];

};

struct arch_idt
{

    struct idt_pointer pointer;
    struct idt_descriptor descriptors[ARCH_IDT_DESCRIPTORS];

};

struct arch_tss
{

    struct tss_pointer pointer;
    struct tss_descriptor descriptors[ARCH_TSS_DESCRIPTORS];

};

void arch_kmap(unsigned int paddress, unsigned int vaddress, unsigned int size, unsigned int flags);
unsigned short arch_resume(struct cpu_general *general, struct cpu_interrupt *interrupt);
void arch_leave(void);
void arch_configuregdt(void);
void arch_configureidt(void);
void arch_configuretss(struct arch_tss *tss, unsigned int id, unsigned int sp);
void arch_setup1(void);
void arch_setup2(unsigned int address);
