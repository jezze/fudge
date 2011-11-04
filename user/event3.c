#include <fudge.h>

void explain()
{

    file_write_format(FILE_STDOUT, "You pressed a key it seems...\n");

    call_wait();

}

void main(int argc, char *argv[])
{

    call_attach(0x07, explain);

    call_wait();

}

