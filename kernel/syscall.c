#include <lib/types.h>
#include <kernel/regs.h>
#include <kernel/isr.h>
#include <kernel/screen.h>
#include <kernel/syscall.h>

void syscall_handler(uint32_t num, void *data)
{

    if (num == SYSCALL_STRING_PUTS)
        screen_puts((char *)data);

}

void syscall_init()
{

}

