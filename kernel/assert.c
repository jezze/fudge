#include <lib/write.h>
#include <kernel/assert.h>

void kernel_assert(char *message, char *file, unsigned int line)
{

    arch_disable_interrupts();

    write_string("ASSERTION FAIL (");
    write_string(message);
    write_string(") at (");
    write_string(file);
    write_string(":");
    write_string_dec(line);
    write_string(")\n");

    for (;;);

}

void kernel_panic(char *message, char *file, unsigned int line)
{

    arch_disable_interrupts();

    write_string("KERNEL PANIC (");
    write_string(message);
    write_string(") at (");
    write_string(file);
    write_string(":");
    write_string_dec(line);
    write_string(")\n");

    for (;;);

}

