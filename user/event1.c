#include <fudge.h>

void before()
{

    call_write(FILE_STDOUT, 0, 6, "START\n");
    call_wait();

}

void after()
{

    call_write(FILE_STDOUT, 0, 4, "END\n");
    call_detach(0x90);
    call_detach(0x91);
    call_exit();

}

void main(int argc, char *argv[])
{

    call_attach(0x90, before);
    call_attach(0x91, after);
    call_wait();

}

