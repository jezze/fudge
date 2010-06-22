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

void syscall_handler2(registers_t *r)
{

    if (r->eax >= SYSCALL_ROUTINES_SIZE)
        return;

    void *location = syscalls[r->eax];

    int ret;

    __asm__ __volatile__ (" \
        push %1; \
        push %2; \
        push %3; \
        push %4; \
        push %5; \
        call *%6; \
        pop %%ebx; \
        pop %%ebx; \
        pop %%ebx; \
        pop %%ebx; \
        pop %%ebx; \
    " : "=a" (ret) : "r" (r->edi), "r" (r->esi), "r" (r->edx), "r" (r->ecx), "r" (r->ebx), "r" (location));

    r->eax = ret;

}

void syscall_handler(registers_t *r)
{

    screen_puts("Syscall: ");
    screen_puts_hex(r->eax);
    screen_puts("\n");

    r->eax = 1;

}

void syscall_init()
{

    syscalls[0] = &screen_puts;
    syscalls[1] = &screen_puts_dec;
    syscalls[2] = &screen_puts_hex;

}

