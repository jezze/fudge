#define ARCH_RAMBASE                    0x40000000
#define ARCH_KERNEL_CODEBASE            0x40200000
#define ARCH_KERNEL_CODESIZE            (KERNEL_CODESIZE)
#define ARCH_KERNEL_STACKBASE           (ARCH_KERNEL_CODEBASE + ARCH_KERNEL_CODESIZE)
#define ARCH_KERNEL_STACKSIZE           (KERNEL_STACKSIZE)
#define ARCH_MMAP_BASE                  0x40900000
#define ARCH_MMAP_SIZE                  (MMAP_SIZE * POOL_TASKS)
#define ARCH_MMU_KERNELBASE             0x40A00000
#define ARCH_MMU_KERNELSIZE             0x200000
#define ARCH_MMU_TASKBASE               0x40C00000
#define ARCH_MMU_TASKSIZE               0x20000
#define ARCH_MMU_LIMIT                  0x100000000UL
#define ARCH_RAMDISK_BASE               0x41200000
#define ARCH_RAMDISK_SIZE               0x00E00000
#define ARCH_FRAMEBUFFER_BASE           0x42200000
#define ARCH_FRAMEBUFFER_SIZE           0x01E00000
#define ARCH_MEM_BASE                   0x44000000
#define ARCH_MEM_SIZE                   (MEMSRV_PAGES * MEMSRV_PAGESIZE)
#define ARCH_TASK_CODEBASE              0x48000000
#define ARCH_TASK_CODESIZE              (TASK_CODESIZE * POOL_TASKS)
#define ARCH_TASK_STACKBASE             (ARCH_TASK_CODEBASE + ARCH_TASK_CODESIZE)
#define ARCH_PAGEPOOL_BASE              (ARCH_TASK_STACKBASE + TASK_STACKSIZE * POOL_TASKS)
#define ARCH_PAGEPOOL_SIZE              (PAGE_COUNT * PAGE_SIZE)
#define ARCH_UART_BASE                  0x09000000
#define ARCH_INTERRUPTS                 2
#define ARCH_FPUSTATESIZE               528

void arch_kmap(unsigned long paddress, unsigned long vaddress, unsigned int size, unsigned int flags);
void arch_setinterrupt(unsigned int index, void (*routine)(void));
void arch_resume(struct cpu_general *general, struct cpu_interrupt *interrupt);
void arch_leave(void);
void arch_setup1(void);
void arch_setup2(void);
void arch_runinit(unsigned long address);
