#include <fudge.h>
#include "net.h"
#include "ipv4.h"
#include "udp.h"

unsigned short udp_checksum(struct udp_header *header, unsigned char sip[IPV4_ADDRSIZE], unsigned char tip[IPV4_ADDRSIZE], unsigned short len)
{

    unsigned int sum = net_sum16(sip, IPV4_ADDRSIZE, 0);
    unsigned short checksum;

    sum = net_sum16(tip, IPV4_ADDRSIZE, sum);
    checksum = net_checksum(net_sum16(header, len, sum + 0x0011 + len));

    return (checksum) ? checksum : 0xFFFF;

}

unsigned short udp_hlen(struct udp_header *header)
{

    return sizeof (struct udp_header);

}

struct udp_header *udp_putheader(void *buffer, unsigned char sp[UDP_PORTSIZE], unsigned char tp[UDP_PORTSIZE], unsigned int count)
{

    struct udp_header *header = buffer;

    udp_initheader(header, sp, tp, count);

    return header;

}

void udp_initheader(struct udp_header *header, unsigned char *sp, unsigned char *tp, unsigned int count)
{

    buffer_copy(header->sp, sp, UDP_PORTSIZE);
    buffer_copy(header->tp, tp, UDP_PORTSIZE);
    net_save16(header->length, count + 8);
    net_save16(header->checksum, 0);

}

