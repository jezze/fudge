#include <fudge.h>
#include "net.h"
#include "ethernet.h"

unsigned short ethernet_hlen(struct ethernet_header *header)
{

    return sizeof (struct ethernet_header);

}

struct ethernet_header *ethernet_putheader(void *buffer, unsigned short type, unsigned char sha[ETHERNET_ADDRSIZE], unsigned char tha[ETHERNET_ADDRSIZE])
{

    struct ethernet_header *header = buffer;

    ethernet_initheader(header, type, sha, tha);

    return header;

}

void ethernet_initheader(struct ethernet_header *header, unsigned short type, unsigned char sha[ETHERNET_ADDRSIZE], unsigned char tha[ETHERNET_ADDRSIZE])
{

    net_save16(header->type, type);
    buffer_copy(header->sha, sha, ETHERNET_ADDRSIZE);
    buffer_copy(header->tha, tha, ETHERNET_ADDRSIZE);

}

