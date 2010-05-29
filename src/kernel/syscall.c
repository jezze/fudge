#include <types.h>
#include <kernel/regs.h>
#include <kernel/isr.h>
#include <kernel/screen.h>
#include <kernel/syscall.h>

static void *syscalls[3] = {&screen_puts, &screen_puts_hex, &screen_puts_dec};
uint32_t num_syscalls = 3;

int syscall_write(char *text)
{

    int num;
    int a;

    num = 0;

    __asm__ __volatile__ ("int $0x80" : "=a" (a) : "0" (num), "b" ((int)text));

    return a;

}

void syscall_handler(registers_t *r)
{

    if (r->eax >= num_syscalls)
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

void syscall_init()
{

    isr_register_handler(0x80, syscall_handler);

}

