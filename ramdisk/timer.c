#include <lib/types.h>
#include <kernel/syscall.h>

void main()
{

    unsigned int timer;

    call_puts("Timer: ");
    call_send(SYSCALL_PIT_READ, &timer);
    call_puts_dec(timer);
    call_puts("\n");

}

