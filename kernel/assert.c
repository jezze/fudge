#include <kernel/assert.h>

void kernel_assert(char *message, char *file, unsigned int line)
{

    arch_disable_interrupts();

    arch_puts("ASSERTION FAIL (");
    arch_puts(message);
    arch_puts(") at (");
    arch_puts(file);
    arch_puts(":");
    arch_puts_dec(line);
    arch_puts(")\n");

    for (;;);

}

void kernel_panic(char *message, char *file, unsigned int line)
{

    arch_disable_interrupts();

    arch_puts("KERNEL PANIC (");
    arch_puts(message);
    arch_puts(") at (");
    arch_puts(file);
    arch_puts(":");
    arch_puts_dec(line);
    arch_puts(")\n");

    for (;;);

}

