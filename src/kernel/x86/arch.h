#define ARCH_GDT_BASE                   0x00001000
#define ARCH_GDT_DESCRIPTORS            256
#define ARCH_IDT_BASE                   0x00002000
#define ARCH_IDT_DESCRIPTORS            256
#define ARCH_SMP_BASE16                 0x00008000
#define ARCH_SMP_BASE32                 0x00008200
#define ARCH_TSS_DESCRIPTORS            1
#define ARCH_KERNEL_CODEBASE            0x00100000
#define ARCH_KERNEL_CODESIZE            (KERNEL_CODESIZE)
#define ARCH_KERNEL_STACKBASE           (ARCH_KERNEL_CODEBASE + ARCH_KERNEL_CODESIZE)
#define ARCH_KERNEL_STACKSIZE           (KERNEL_STACKSIZE * POOL_CORES)
#define ARCH_MMAP_BASE                  0x00800000
#define ARCH_MMAP_SIZE                  (MMAP_SIZE * POOL_TASKS)
#define ARCH_MMU_KERNELBASE             0x00A00000
#define ARCH_MMU_KERNELSIZE             0x200000
#define ARCH_MMU_TASKBASE               0x00C00000
#define ARCH_MMU_TASKSIZE               0x8000
#define ARCH_MAILBOX_BASE               0x01000000
#define ARCH_MAILBOX_SIZE               (MESSAGE_CAPACITY * POOL_MAILBOXES)
#define ARCH_MEM_BASE                   0x02000000
#define ARCH_TASK_CODEBASE              0x04000000
#define ARCH_TASK_CODESIZE              (TASK_CODESIZE * POOL_TASKS)
#define ARCH_TASK_STACKBASE             (ARCH_TASK_CODEBASE + ARCH_TASK_CODESIZE)
#define ARCH_TASK_STACKSIZE             (TASK_STACKSIZE + POOL_TASKS)
#define ARCH_KCODE                      0x01
#define ARCH_KDATA                      0x02
#define ARCH_UCODE                      0x03
#define ARCH_UDATA                      0x04
#define ARCH_TSS                        0x05

void arch_kmap(unsigned int paddress, unsigned int vaddress, unsigned int size, unsigned int flags);
unsigned short arch_resume(struct cpu_general *general, struct cpu_interrupt *interrupt);
void arch_leave(void);
void arch_configuregdt(struct gdt_pointer *gdt, struct gdt_descriptor *descriptors, unsigned int count);
void arch_configureidt(struct idt_pointer *idt, struct idt_descriptor *descriptors, unsigned int count, unsigned short selector);
void arch_configuretss(struct tss_pointer *tss, struct tss_descriptor *descriptors, unsigned int count, unsigned int id, unsigned short selector1, unsigned short selector2);
void arch_setup1(void);
void arch_setup2(unsigned int address);
