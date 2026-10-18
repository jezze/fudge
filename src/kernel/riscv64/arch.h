#define ARCH_KERNEL_CODEBASE            0x80200000
#define ARCH_KERNEL_CODESIZE            (KERNEL_CODESIZE)
#define ARCH_KERNEL_STACKBASE           (ARCH_KERNEL_CODEBASE + ARCH_KERNEL_CODESIZE)
#define ARCH_KERNEL_STACKSIZE           (KERNEL_STACKSIZE)
#define ARCH_MMAP_BASE                  0x80900000
#define ARCH_MMAP_SIZE                  (MMAP_SIZE * POOL_TASKS)
#define ARCH_MMU_KERNELBASE             0x80A00000
#define ARCH_MMU_KERNELSIZE             0x200000
#define ARCH_MMU_TASKBASE               0x80C00000
#define ARCH_MMU_TASKSIZE               0x20000
#define ARCH_MMU_LIMIT                  0x100000000UL
#define ARCH_MAILBOX_BASE               0x81000000
#define ARCH_MAILBOX_SIZE               (MESSAGE_CAPACITY * POOL_MAILBOXES)
#define ARCH_RAMDISK_BASE               0x81200000
#define ARCH_RAMDISK_SIZE               0x00E00000
#define ARCH_TASK_BUFFERBASE            0x82000000
#define ARCH_TASK_BUFFERSIZE            (TASK_BUFFERSIZE * POOL_TASKS)
#define ARCH_FRAMEBUFFER_BASE           0x82200000
#define ARCH_FRAMEBUFFER_SIZE           0x01E00000
#define ARCH_TASK_CODEBASE              0x84000000
#define ARCH_TASK_CODESIZE              (TASK_CODESIZE * POOL_TASKS)
#define ARCH_TASK_STACKBASE             (ARCH_TASK_CODEBASE + ARCH_TASK_CODESIZE)
#define ARCH_MEM_BASE                   0x8A000000
#define ARCH_MEM_SIZE                   (MEMSRV_PAGES * MEMSRV_PAGESIZE)
#define ARCH_UART_BASE                  0x10000000
#define ARCH_INTERRUPTS                 16

void arch_kmap(unsigned long paddress, unsigned long vaddress, unsigned int size, unsigned int flags);
void arch_setinterrupt(unsigned int index, void (*routine)(void));
void arch_resume(struct cpu_general *general, struct cpu_interrupt *interrupt);
void arch_leave(void);
void arch_setup1(void);
void arch_setup2(void);
void arch_runinit(unsigned long address);
