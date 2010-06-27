#include <lib/types.h>
#include <kernel/syscall.h>

void main()
{

    call_send(SYSCALL_SCREEN_PUTS, "Hello World!\n");

}
