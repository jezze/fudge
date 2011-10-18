#include <fudge.h>

void main(int argc, char *argv[])
{

    for (;;)
    {

        call_load("/serial.ko");
        call_load("/pci.ko");
        call_load("/ata.ko");
        call_execute("/shell", 0, 0);

    }

}

