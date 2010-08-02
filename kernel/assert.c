#include <lib/call.h>
#include <kernel/assert.h>

void kernel_assert(char *message, char *file, unsigned int line)
{

    arch_disable_interrupts();

    call_puts("ASSERTION FAIL (");
    call_puts(message);
    call_puts(") at (");
    call_puts(file);
    call_puts(":");
    arch_puts_dec(line);
    call_puts(")\n");

    for (;;);

}

void kernel_panic(char *message, char *file, unsigned int line)
{

    arch_disable_interrupts();

    call_puts("KERNEL PANIC (");
    call_puts(message);
    call_puts(") at (");
    call_puts(file);
    call_puts(":");
    arch_puts_dec(line);
    call_puts(")\n");

    for (;;);

}

