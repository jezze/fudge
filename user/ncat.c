#include <fudge.h>

void handle_network_event()
{

    file_write_format(FILE_STDOUT, "Network data received\n");

    call_wait();

}

void main(int argc, char *argv[])
{

    call_attach(0x07, handle_network_event);
    call_wait();

}

