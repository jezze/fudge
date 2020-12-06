#include <fudge.h>
#include "net.h"
#include "ipv4.h"
#include "udp.h"

static unsigned short htons(unsigned short v)
{

    unsigned char c1 = v;
    unsigned char c2 = (v >> 8);

    return (c1 << 8) | c2;

}

unsigned short udp_checksum(struct udp_header *header, unsigned char sip[IPV4_ADDRSIZE], unsigned char tip[IPV4_ADDRSIZE], unsigned short len)
{

    unsigned short *payload = (unsigned short *)header;
    unsigned int sum = 0;
    unsigned short sum2;

    sum += ((sip[1] << 8) | sip[0]);
    sum += ((sip[3] << 8) | sip[2]);
    sum += ((tip[1] << 8) | tip[0]);
    sum += ((tip[3] << 8) | tip[2]);
    sum += htons(0x0011);
    sum += htons(len);

    while (len > 1)
    {

        sum += *payload++;
        len -= 2;

    }

    if (len > 0)
        sum += ((*payload) & htons(0xFF00));

    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);

    sum2 = ~sum;

    return (sum2) ? sum2 : 0xFFFF;

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

