#include <fudge.h>
#include "ethernet.h"

unsigned short ethernet_hlen(struct ethernet_header *header)
{

    return sizeof (struct ethernet_header);

}

void ethernet_initheader(struct ethernet_header *header, unsigned int type, unsigned char sha[ETHERNET_ADDRSIZE], unsigned char tha[ETHERNET_ADDRSIZE])
{

    header->type[0] = type >> 8;
    header->type[1] = type;

    buffer_copy(header->sha, sha, ETHERNET_ADDRSIZE);
    buffer_copy(header->tha, tha, ETHERNET_ADDRSIZE);

}

