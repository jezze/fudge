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

void arch_init()
{

    puts("Fudge Console Text\n\n");

    for (;;);

    kernel_init(&arm.base);

}

