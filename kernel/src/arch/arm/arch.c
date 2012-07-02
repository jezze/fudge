#include <lib/memory.h>
#include <kernel/kernel.h>
#include <kernel/arch/arm/arch.h>
#include <kernel/arch/arm/cpu.h>

#define SERIAL_BASE 0x16000000
#define SERIAL_FLAG_REGISTER 0x18
#define SERIAL_BUFFER_FULL (1 << 5)
 
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

static struct arch_arm arm;

static void setup(struct kernel_arch *arch)
{

    puts("Fudge Console Text\n\n");

}

void arch_init()
{

    memory_clear(&arm, sizeof (struct arch_arm));

    kernel_arch_init(&arm.base, setup, 0, cpu_enable_interrupts, cpu_disable_interrupts, 0, 0, 0);

    kernel_init(&arm.base);

}

