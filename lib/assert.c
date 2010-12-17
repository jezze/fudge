#include <lib/assert.h>
#include <lib/file.h>
#include <lib/session.h>

void kernel_assert(char *message, char *file, unsigned int line)
{

    arch_interrupts_disable();

    file_write_string(session_get_out(), "ASSERTION FAIL (");
    file_write_string(session_get_out(), message);
    file_write_string(session_get_out(), ") at (");
    file_write_string(session_get_out(), file);
    file_write_string(session_get_out(), ":");
    file_write_dec(session_get_out(), line);
    file_write_string(session_get_out(), ")\n");

    for (;;);

}

void kernel_panic(char *message, char *file, unsigned int line)
{

    arch_interrupts_disable();

    file_write_string(session_get_out(), "KERNEL PANIC (");
    file_write_string(session_get_out(), message);
    file_write_string(session_get_out(), ") at (");
    file_write_string(session_get_out(), file);
    file_write_string(session_get_out(), ":");
    file_write_dec(session_get_out(), line);
    file_write_string(session_get_out(), ")\n");

    for (;;);

}

