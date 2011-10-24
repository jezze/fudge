#include <kernel/error.h>
#include <kernel/kernel.h>
#include <kernel/log.h>

void error_assert(unsigned int condition, char *message, char *file, unsigned int line)
{

    if (condition)
        return;

    kernel_disable_interrupts();
    log_write("ASSERTION FAIL (%s) at (%s:%d)\n", message, file, line);

    for (;;);

}

void error_panic(char *message, char *file, unsigned int line)
{

    kernel_disable_interrupts();
    log_write("KERNEL PANIC (%s) at (%s:%d)\n", message, file, line);

    for (;;);

}

