#include <fudge.h>

void main(int argc, char *argv[])
{

    for (;;)
    {

        call_load("/test.ko");
        call_execute("/shell", 0, 0);

    }

}

