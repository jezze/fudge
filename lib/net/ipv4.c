#include <fudge.h>
#include "ipv4.h"

unsigned int ipv4_calculatechecksum(void *buffer, unsigned int count)
{

    unsigned short *ip1 = buffer;
    unsigned int sum = 0;

    while (count > 1)
    {

        sum += *ip1++;

        if (sum & 0x80000000)
            sum = (sum & 0xFFFF) + (sum >> 16);

        count -= 2;

    }

    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);

    return ~sum;

}

void ipv4_initheader(struct ipv4_header *header, unsigned char sip[IPV4_ADDRSIZE], unsigned char tip[IPV4_ADDRSIZE], unsigned int protocol, unsigned int count)
{

    unsigned int checksum;

    header->version = 0x45;
    header->dscp = 0;
    header->length[0] = (count + 20) >> 8;
    header->length[1] = (count + 20);
    header->identification[0] = 0;
    header->identification[1] = 0;
    header->fragment[0] = 0;
    header->fragment[1] = 0;
    header->ttl = 64;
    header->protocol = protocol;
    header->checksum[0] = 0;
    header->checksum[1] = 0;

    buffer_copy(header->sip, sip, IPV4_ADDRSIZE);
    buffer_copy(header->tip, tip, IPV4_ADDRSIZE);

    checksum = ipv4_calculatechecksum(header, sizeof (struct ipv4_header));

    header->checksum[0] = checksum;
    header->checksum[1] = checksum >> 8;

}

