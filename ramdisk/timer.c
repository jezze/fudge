#include <lib/types.h>
#include <kernel/syscall.h>

void main()
{

    unsigned int timer;

    call_send(SYSCALL_SCREEN_PUTS, "Timer: ");
    call_send(SYSCALL_PIT_READ, &timer);
    call_send(SYSCALL_SCREEN_PUTS_DEC, &timer);
    call_send(SYSCALL_SCREEN_PUTS, "\n");

}

