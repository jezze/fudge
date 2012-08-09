#include <fudge.h>

void before()
{

    call_write(FILE_STDOUT, 0, 6, "START\n");
    call_idle();

}

void after()
{

    call_write(FILE_STDOUT, 0, 4, "END\n");
    call_detach(0x85);
    call_detach(0x86);
    call_exit();

}

void main()
{

    call_attach(0x85, before);
    call_attach(0x86, after);
    call_idle();

}

