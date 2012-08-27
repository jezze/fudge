#include <memory.h>
#include <kernel.h>
#include <arch/arm/arch.h>
#include <arch/arm/cpu.h>

#define SERIAL_BASE 0x16000000
#define SERIAL_FLAG_REGISTER 0x18
#define SERIAL_BUFFER_FULL (1 << 5)

static struct arch_interface interface;

static void putc(char c)
{

    while (*(volatile unsigned long*)(SERIAL_BASE + SERIAL_FLAG_REGISTER) & (SERIAL_BUFFER_FULL));

    *(volatile unsigned long*)SERIAL_BASE = c;

}

static void puts(const char *s)
{

    while (*s)
        putc(*s++);

}

static void setup(struct kernel_arch *arch)
{

    puts("Fudge Console Text\n\n");

}

void arch_interface_init(struct arch_interface *interface)
{

    memory_clear(interface, sizeof (struct arch_interface));

    kernel_arch_init(&interface->base, setup, 0, 0, 0);

}

void arch_setup()
{

    arch_interface_init(&interface);

    interface.base.start(&interface.base);

}

