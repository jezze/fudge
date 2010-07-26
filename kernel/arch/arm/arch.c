#include <kernel/arch/arm/arch.h>

#define SERIAL_BASE 0x16000000
#define SERIAL_FLAG_REGISTER 0x18
#define SERIAL_BUFFER_FULL (1 << 5)

struct vfs_node *initrd_init(unsigned int location)
{

    location++;
    return 0;

}

void mboot_init()
{

}

void shell_init()
{

}

void arch_putc(char c)
{

    while (*(volatile unsigned long*)(SERIAL_BASE + SERIAL_FLAG_REGISTER) & (SERIAL_BUFFER_FULL));

    *(volatile unsigned long*)SERIAL_BASE = c;

}

void arch_puts(char *s)
{

    while (*s) arch_putc(*s++);

}

void arch_puts_dec(unsigned int n)
{

    n++;

}

void arch_enable_interrupts()
{

}

void arch_disable_interrupts()
{

}

void arch_init()
{

    arch_puts("Hello World!\n");

}

void arch_init_devices()
{

}

void arch_init_interrupts()
{

}

void arch_init_syscalls()
{

}

