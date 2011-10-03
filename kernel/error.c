#include <kernel/kernel.h>
#include <kernel/log.h>
#include <kernel/error.h>

void error_assert(unsigned int condition, char *message, char *file, unsigned int line)
{

    if (condition)
        return;

    kernel_disable_interrupts();

    void *args[] = {message, file, &line};
    log_message(LOG_TYPE_ERROR, "ASSERTION FAIL (%s) at (%s:%d)", args);

    for (;;);

}

void error_panic(char *message, char *file, unsigned int line)
{

    kernel_disable_interrupts();

    void *args[] = {message, file, &line};
    log_message(LOG_TYPE_ERROR, "KERNEL PANIC (%s) at (%s:%d)", args);

    for (;;);

}

