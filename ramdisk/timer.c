#include <lib/types.h>
#include <kernel/syscall.h>

void main()
{

    call_send(SYSCALL_SCREEN_PUTS, "Timer: ");
    unsigned int timer = call_send(SYSCALL_PIT_READ, 0);
    call_send(SYSCALL_SCREEN_PUTS_DEC, &timer);
    call_send(SYSCALL_SCREEN_PUTS, "\n");

}

