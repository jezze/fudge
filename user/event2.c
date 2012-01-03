#include <fudge.h>

static unsigned int counter;
static unsigned int ticks;

void explain()
{

    ticks++;

    if (ticks < 10)
        call_wait();

    ticks = 0;
    counter++;

    if (counter & 1)
        file_write_format(FILE_STDOUT, "Odd:    %d\n", counter);
    else
        file_write_format(FILE_STDOUT, "Even:   %d\n", counter);

    if (counter < 10)
        call_wait();

    call_detach(0x00);
    call_exit();

}

void main(int argc, char *argv[])
{

    counter = 0;
    ticks = 0;

    call_attach(0x00, explain);
    call_wait();

}

