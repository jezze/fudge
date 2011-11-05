#include <fudge.h>

void before()
{

    file_write_format(FILE_STDOUT, "*** PROGRAM START ***\n");

    call_wait();

}

void after()
{

    file_write_format(FILE_STDOUT, "*** PROGRAM END ***\n");

    call_detach(0x21);
    call_detach(0x22);
    call_exit();

}

void main(int argc, char *argv[])
{

    call_attach(0x21, before);
    call_attach(0x22, after);
    call_wait();

}

