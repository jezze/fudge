#include <lib/types.h>
#include <kernel/regs.h>
#include <kernel/isr.h>
#include <kernel/screen.h>
#include <kernel/syscall.h>

static void *syscalls[SYSCALL_ROUTINES_SIZE];

int syscall_write(char *text)
{

    int num;
    int a;

    num = 0;

    __asm__ __volatile__ ("int $0x80");

    return a;

}

void syscall_handler(int num)
{

    screen_puts("Syscall: ");
    screen_puts_hex(num);
    screen_puts("\n");

}

void syscall_init()
{

    syscalls[0] = &screen_puts;
    syscalls[1] = &screen_puts_dec;
    syscalls[2] = &screen_puts_hex;

}

