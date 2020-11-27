#include <fudge.h>
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

void icmp_initheader(struct icmp_header *header, unsigned char type, unsigned char code)
{

    unsigned short checksum;

    header->type = type;
    header->code = code;
    header->checksum[0] = 0;
    header->checksum[1] = 0;

    checksum = icmp_calculatechecksum(header, sizeof (struct icmp_header));

    header->checksum[0] = checksum;
    header->checksum[1] = checksum >> 8;

}

