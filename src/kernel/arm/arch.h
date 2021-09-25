#define ARCH_KERNELSTACKPHYSICAL        0x003F8000
#define ARCH_KERNELSTACKSIZE            0x00008000
#define ARCH_MAILBOXPHYSICAL            0x00400000
#define ARCH_MAILBOXSIZE                0x00001000
#define ARCH_TASKSTACKSIZE              0x00008000
#define ARCH_TASKSTACKVIRTUAL           0x80000000

void arch_undefined(void);
void arch_reset(void);
void arch_syscall(void);
void arch_irq(void);
void arch_fiq(void);
void arch_setup(void);
