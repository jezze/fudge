#include <fudge.h>

unsigned int counter;

void explain()
{

    if (counter > 10)
    {

        call_detach(0x06);
        call_exit();

    }

    if (counter & 1)
        file_write_format(FILE_STDOUT, "Odd:    %d\n", counter);
    else
        file_write_format(FILE_STDOUT, "Even:   %d\n", counter);

    counter++;

    call_wait();

}

void main(int argc, char *argv[])
{

    counter = 0;

    call_attach(0x06, explain);

    call_wait();

}

