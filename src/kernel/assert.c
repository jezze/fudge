#include <lib/types.h>
#include <kernel/screen.h>
#include <kernel/kernel.h>
#include <kernel/assert.h>

void kernel_panic(char *message, char *file, uint32_t line)
{

    cli();

    screen_puts(&screen, "KERNEL PANIC (");
    screen_puts(&screen, message);
    screen_puts(&screen, ") at (");
    screen_puts(&screen, file);
    screen_puts(&screen, ":");
    screen_puts_dec(&screen, line);
    screen_puts(&screen, ")\n");

    for (;;);

}

void kernel_assert(char *message, char *file, uint32_t line)
{

    cli();

    screen_puts(&screen, "ASSERTION FAIL (");
    screen_puts(&screen, message);
    screen_puts(&screen, ") at (");
    screen_puts(&screen, file);
    screen_puts(&screen, ":");
    screen_puts_dec(&screen, line);
    screen_puts(&screen, ")\n");

    for (;;);

}

