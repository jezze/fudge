#include <lib/file.h>
#include <kernel/assert.h>

void kernel_assert(char *message, char *file, unsigned int line)
{

    arch_disable_interrupts();

    file_write("ASSERTION FAIL (");
    file_write(message);
    file_write(") at (");
    file_write(file);
    file_write(":");
    file_write_dec(line);
    file_write(")\n");

    for (;;);

}

void kernel_panic(char *message, char *file, unsigned int line)
{

    arch_disable_interrupts();

    file_write("KERNEL PANIC (");
    file_write(message);
    file_write(") at (");
    file_write(file);
    file_write(":");
    file_write_dec(line);
    file_write(")\n");

    for (;;);

}

