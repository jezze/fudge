#include <kernel/syscall.h>

void main(int argc, char *argv[])
{

    call_send(SYSCALL_RTC_DATE, 0);

}

