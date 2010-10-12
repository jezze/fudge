#include <lib/stdout.h>
#include <kernel/assert.h>

void kernel_assert(char *message, char *file, unsigned int line)
{

    arch_disable_interrupts();

    stdout_write("ASSERTION FAIL (");
    stdout_write(message);
    stdout_write(") at (");
    stdout_write(file);
    stdout_write(":");
    stdout_write_dec(line);
    stdout_write(")\n");

    for (;;);

}

void kernel_panic(char *message, char *file, unsigned int line)
{

    arch_disable_interrupts();

    stdout_write("KERNEL PANIC (");
    stdout_write(message);
    stdout_write(") at (");
    stdout_write(file);
    stdout_write(":");
    stdout_write_dec(line);
    stdout_write(")\n");

    for (;;);

}

