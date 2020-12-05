#include <fudge.h>
#include "net.h"
#include "ipv4.h"
#include "icmp.h"

unsigned short icmp_calculatechecksum(struct icmp_header *header, unsigned int len)
{

    unsigned short *payload = (unsigned short *)header;
    unsigned int sum = 0;

    while (len > 1)
    {

        sum += *payload++;

        if (sum & 0x80000000)
            sum = (sum & 0xFFFF) + (sum >> 16);

        len -= 2;

    }

    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);

    return ~sum;

}

unsigned short icmp_hlen(struct icmp_header *header)
{

    return sizeof (struct icmp_header);

}

void icmp_initheader(struct icmp_header *header, unsigned char type, unsigned char code)
{

    header->type = type;
    header->code = code;

    net_save16(header->checksum, 0);

}

