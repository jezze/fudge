#include <kernel/arch/i386/screen.h>
#include <kernel/assert.h>

void kernel_panic(char *message, char *file, unsigned int line)
{

    arch_disable_interrupts();

    screen_puts("KERNEL PANIC (");
    screen_puts(message);
    screen_puts(") at (");
    screen_puts(file);
    screen_puts(":");
    screen_puts_dec(line);
    screen_puts(")\n");

    for (;;);

}

void kernel_assert(char *message, char *file, unsigned int line)
{

    arch_disable_interrupts();

    screen_puts("ASSERTION FAIL (");
    screen_puts(message);
    screen_puts(") at (");
    screen_puts(file);
    screen_puts(":");
    screen_puts_dec(line);
    screen_puts(")\n");

    for (;;);

}

