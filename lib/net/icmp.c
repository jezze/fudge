#include <fudge.h>
#include "ipv4.h"
#include "icmp.h"

void icmp_initheader(struct icmp_header *header, unsigned char type, unsigned char code)
{

    unsigned int checksum;

    header->type = type;
    header->code = code;
    header->checksum[0] = 0;
    header->checksum[1] = 0;

    checksum = ipv4_calculatechecksum(header, sizeof (struct icmp_header));

    header->checksum[0] = checksum;
    header->checksum[1] = checksum >> 8;

}

