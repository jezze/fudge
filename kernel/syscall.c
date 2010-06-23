#include <lib/types.h>
#include <kernel/regs.h>
#include <kernel/isr.h>
#include <kernel/screen.h>
#include <kernel/syscall.h>

static void *syscalls[SYSCALL_ROUTINES_SIZE];

int syscall_write(char *text)
{

    call_send(6, 0);

}

void syscall_handler(uint32_t num, void *data)
{

    screen_puts("Syscall: ");
    screen_puts_dec(num);
    screen_puts("\n");

}

void syscall_init()
{

    syscalls[0] = &screen_puts;
    syscalls[1] = &screen_puts_dec;
    syscalls[2] = &screen_puts_hex;

}

