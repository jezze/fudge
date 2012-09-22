#include <fudge.h>

void handle_network_event()
{

    struct arp_header header;
    unsigned int count;

    call_write(FILE_STDOUT, 0, 9, "INCOMING\n");

    call_open(3, 10, "/net/0/arp");
    count = call_read(3, 0, sizeof (struct arp_header), &header);
    call_close(3);

    if (count)
        call_write(FILE_STDOUT, 0, 13, "Received ARP\n");

    call_idle();

}

void main()
{

    call_attach(0x2B, handle_network_event);
    call_write(FILE_STDOUT, 0, 13, "Listening...\n");
    call_idle();

}

