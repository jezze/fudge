#define ARCH_FPUSTATESIZE               512
#define ARCH_GDT_BASE                   0x00001000
#define ARCH_GDT_DESCRIPTORS            (5 + 256 * 2)
#define ARCH_IDT_BASE                   0x00002000
#define ARCH_IDT_DESCRIPTORS            256
#define ARCH_TSS_BASE                   0x00003000
#define ARCH_TSS_DESCRIPTORS            256
#define ARCH_ALIGN(x, a)                (((x) + (a) - 1) & ~((a) - 1))
#define ARCH_KERNEL_CODEBASE            0x00100000
#define ARCH_KERNEL_CODESIZE            (KERNEL_CODESIZE + 0x200000)
#define ARCH_KERNEL_STACKBASE           (ARCH_KERNEL_CODEBASE + ARCH_KERNEL_CODESIZE)
#define ARCH_KERNEL_STACKSIZE           (KERNEL_STACKSIZE * POOL_CORES)
#define ARCH_FIRMWARE_BASE              ARCH_ALIGN(ARCH_KERNEL_STACKBASE + ARCH_KERNEL_STACKSIZE, 0x1000)
#define ARCH_FIRMWARE_SIZE              0x1000
#define ARCH_FIRMWARE_ACPI              (ARCH_FIRMWARE_BASE + 0x0000)
#define ARCH_FIRMWARE_FRAMEBUFFER       (ARCH_FIRMWARE_BASE + 0x0100)
#define ARCH_MMU_KERNELBASE             ARCH_ALIGN(ARCH_FIRMWARE_BASE + ARCH_FIRMWARE_SIZE, 0x1000)
#define ARCH_MMU_KERNELSIZE             0x200000
#define ARCH_MMU_LIMIT                  0x100000000UL
#define ARCH_FRAMEBUFFER_BASE           ARCH_ALIGN(ARCH_MMU_KERNELBASE + ARCH_MMU_KERNELSIZE, 0x1000)
#define ARCH_FRAMEBUFFER_SIZE           0x01E00000
#define ARCH_MEM_BASE                   ARCH_ALIGN(ARCH_FRAMEBUFFER_BASE + ARCH_FRAMEBUFFER_SIZE, 0x1000)
#define ARCH_MEM_SIZE                   (MEMSRV_PAGES * MEMSRV_PAGESIZE)
#define ARCH_FIXED                      (ARCH_MEM_BASE + ARCH_MEM_SIZE)
#define ARCH_USER_BASE                  0x08000000
#define ARCH_USER_SIZE                  0x00400000
#define ARCH_CHUNK_BASE                 (ARCH_USER_BASE + ARCH_USER_SIZE)
#define ARCH_MMAP_BASE                  ARCH_CHUNK_BASE
#define ARCH_MMAP_SIZE                  (MMAP_SIZE * POOL_TASKS)
#define ARCH_MMU_TASKBASE               (ARCH_MMAP_BASE + ARCH_MMAP_SIZE)
#define ARCH_MMU_TASKSIZE               POOL_TASKMMUSIZE
#define ARCH_MAILBOX_BASE               (ARCH_MMU_TASKBASE + ARCH_MMU_TASKSIZE * POOL_TASKS)
#define ARCH_MAILBOX_SIZE               (MESSAGE_CAPACITY * POOL_MAILBOXES)
#define ARCH_TASK_BUFFERBASE            (ARCH_MAILBOX_BASE + ARCH_MAILBOX_SIZE)
#define ARCH_TASK_BUFFERSIZE            (TASK_BUFFERSIZE * POOL_TASKS)
#define ARCH_TASK_CODEBASE              (ARCH_TASK_BUFFERBASE + ARCH_TASK_BUFFERSIZE)
#define ARCH_TASK_CODESIZE              (TASK_CODESIZE * POOL_TASKS)
#define ARCH_TASK_STACKBASE             (ARCH_TASK_CODEBASE + ARCH_TASK_CODESIZE)
#define ARCH_TASK_STACKSIZE             (TASK_STACKSIZE * POOL_TASKS)
#define ARCH_END                        (ARCH_TASK_STACKBASE + ARCH_TASK_STACKSIZE)
#define ARCH_KCODE                      0x01
#define ARCH_KDATA                      0x02
#define ARCH_UCODE                      0x03
#define ARCH_UDATA                      0x04
#define ARCH_TSS                        0x05


struct arch_framebuffer
{

    unsigned long address;
    unsigned int width;
    unsigned int height;
    unsigned int pitch;
    unsigned int bpp;

};

void arch_kmap(unsigned long paddress, unsigned long vaddress, unsigned int size, unsigned int flags);
unsigned short arch_resume(struct cpu_general *general, struct cpu_interrupt *interrupt);
void arch_leave(void);
void arch_setup1(unsigned int ramkb);
void arch_setup2(void);
void arch_runinit(unsigned long address);
