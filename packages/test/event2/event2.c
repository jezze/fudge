#include <fudge.h>

static unsigned int counter;
static unsigned int ticks;

void explain()
{

    ticks++;

    if (ticks < 10)
        call_idle();

    ticks = 0;
    counter++;

    if (counter & 1)
        call_write(FILE_STDOUT, 0, 4, "Odd\n");
    else
        call_write(FILE_STDOUT, 0, 5, "Even\n");

    if (counter < 10)
        call_idle();

    call_detach(0x20);
    call_exit();

}

void main()
{

    counter = 0;
    ticks = 0;

    call_attach(0x20, explain);
    call_idle();

}

