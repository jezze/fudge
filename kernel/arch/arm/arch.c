#include <kernel/kernel.h>
#include <kernel/arch/arm/arch.h>

static struct arch_arm arm;

#define SERIAL_BASE 0x16000000
#define SERIAL_FLAG_REGISTER 0x18
#define SERIAL_BUFFER_FULL (1 << 5)
 
void putc(char c)
{

    while (*(volatile unsigned long*)(SERIAL_BASE + SERIAL_FLAG_REGISTER) & (SERIAL_BUFFER_FULL));

    *(volatile unsigned long*)SERIAL_BASE = c;

}
                                                         
void puts(const char *s)
{

    while (*s)
        putc(*s++);

}

static void arch_setup(struct kernel_arch *arch)
{

    struct arch_arm *arm = (struct arch_arm *)arch;

    puts("Fudge Console Text\n\n");

}

void arch_init()
{

    arm.base.setup = arch_setup;
    arm.base.disable_interrupts = 0;
    arm.base.enable_interrupts = 0;
    arm.base.enter_usermode = 0;
    arm.base.set_stack = 0;
    arm.base.register_irq = 0;
    arm.base.unregister_irq = 0;
    arm.base.stack = 0;
    arm.base.initrdc = 0;
    arm.base.initrdv = 0;
    arm.base.reboot = 0;

    kernel_init(&arm.base);

}

