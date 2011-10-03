#include <kernel/kernel.h>
#include <kernel/log.h>
#include <kernel/error.h>

void error_assert(unsigned int condition, char *message, char *file, unsigned int line)
{

    if (condition)
        return;

    kernel_disable_interrupts();

    log_string("ASSERTION FAIL (");
    log_string(message);
    log_string(") at (");
    log_string(file);
    log_string(":");
    log_num(line, 10);
    log_string(")\n");

    for (;;);

}

void error_panic(char *message, char *file, unsigned int line)
{

    kernel_disable_interrupts();

    log_string("KERNEL PANIC (");
    log_string(message);
    log_string(") at (");
    log_string(file);
    log_string(":");
    log_num(line, 10);
    log_string(")\n");


    for (;;);

}

