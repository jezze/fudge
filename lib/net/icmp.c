#include <fudge.h>
#include "net.h"
#include "ipv4.h"
#include "icmp.h"

unsigned short icmp_calculatechecksum(struct icmp_header *header, unsigned int len)
{

    return net_checksum(net_sum16(header, len, 0));

}

unsigned short icmp_hlen(struct icmp_header *header)
{

    return sizeof (struct icmp_header);

}

struct icmp_header *icmp_putheader(void *buffer, unsigned char type, unsigned char code)
{

    struct icmp_header *header = buffer;

    icmp_initheader(header, type, code);

    return header;

}

void icmp_initheader(struct icmp_header *header, unsigned char type, unsigned char code)
{

    net_save8(header->type, type);
    net_save8(header->code, code);
    net_save16(header->checksum, 0);

}

