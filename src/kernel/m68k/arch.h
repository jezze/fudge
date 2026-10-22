#define ARCH_RAMBASE                    0x00000000
#define ARCH_KERNEL_CODEBASE            (ARCH_RAMBASE + 0x00100000)
#define ARCH_KERNEL_CODESIZE            (KERNEL_CODESIZE)
#define ARCH_KERNEL_STACKBASE           (ARCH_KERNEL_CODEBASE + ARCH_KERNEL_CODESIZE)
#define ARCH_KERNEL_STACKSIZE           (KERNEL_STACKSIZE)
#define ARCH_MMAP_BASE                  (ARCH_RAMBASE + 0x00800000)
#define ARCH_MMAP_SIZE                  (MMAP_SIZE * POOL_TASKS)
#define ARCH_MMU_KERNELBASE             (ARCH_RAMBASE + 0x00A00000)
#define ARCH_MMU_KERNELSIZE             0x200000
#define ARCH_MMU_TASKBASE               (ARCH_RAMBASE + 0x00C00000)
#define ARCH_MMU_TASKSIZE               0x20000
#define ARCH_MAILBOX_BASE               (ARCH_RAMBASE + 0x01000000)
#define ARCH_MAILBOX_SIZE               (MESSAGE_CAPACITY * POOL_MAILBOXES)
#define ARCH_RAMDISK_BASE               (ARCH_RAMBASE + 0x01200000)
#define ARCH_RAMDISK_SIZE               0x00E00000
#define ARCH_TASK_BUFFERBASE            (ARCH_RAMBASE + 0x02000000)
#define ARCH_TASK_BUFFERSIZE            (TASK_BUFFERSIZE * POOL_TASKS)
#define ARCH_FRAMEBUFFER_BASE           (ARCH_RAMBASE + 0x02200000)
#define ARCH_FRAMEBUFFER_SIZE           0x01E00000
#define ARCH_TASK_CODEBASE              (ARCH_RAMBASE + 0x04000000)
#define ARCH_TASK_CODESIZE              (TASK_CODESIZE * POOL_TASKS)
#define ARCH_TASK_STACKBASE             (ARCH_TASK_CODEBASE + ARCH_TASK_CODESIZE)
#define ARCH_MEM_BASE                   (ARCH_RAMBASE + 0x0A000000)
#define ARCH_MEM_SIZE                   (MEMSRV_PAGES * MEMSRV_PAGESIZE)
#define ARCH_TTY_BASE                   0xFF008000
#define ARCH_INTERRUPTS                 8

void arch_kmap(unsigned int paddress, unsigned int vaddress, unsigned int size, unsigned int flags);
void arch_setinterrupt(unsigned int level, void (*routine)(void));
void arch_leave(void);
void arch_setup1(void);
void arch_setup2(void);
void arch_runinit(unsigned int address);
