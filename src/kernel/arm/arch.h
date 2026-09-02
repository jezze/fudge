#define ARCH_KERNEL_STACKBASE           0x00880000
#define ARCH_KERNEL_STACKSIZE           0x00008000
#define ARCH_MAILBOX_BASE               0x00900000

void arch_syscall(void *stack);
void arch_irq(void *stack);
void arch_fiq(void);
void arch_leave(void);
void arch_setup1(void);
void arch_setup2(void);
void arch_setup(void);
