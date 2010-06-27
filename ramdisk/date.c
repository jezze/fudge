#include <lib/types.h>
#include <kernel/syscall.h>

void main()
{

    call_send(SYSCALL_RTC_DATE, 0);

}

