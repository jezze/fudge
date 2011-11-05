#include <fudge.h>

void explain()
{

    file_write_format(FILE_STDOUT, "You executed a program it seems...\n");

    call_detach(0x21);

    call_exit();

}

void main(int argc, char *argv[])
{

    call_attach(0x21, explain);

    call_wait();

}

