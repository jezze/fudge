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
        file_write(FILE_STDOUT, 0, 4, "Odd\n");
    else
        file_write(FILE_STDOUT, 0, 5, "Even\n");

    if (counter < 10)
        call_wait();

    call_detach(0x20);
    call_exit();

}

void main(int argc, char *argv[])
{

    counter = 0;
    ticks = 0;

    call_attach(0x20, explain);
    call_wait();

}

