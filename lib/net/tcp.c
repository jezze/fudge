#include <fudge.h>
#include "net.h"
#include "ipv4.h"
#include "tcp.h"

static unsigned short htons(unsigned short v)
{

    unsigned char c1 = v;
    unsigned char c2 = (v >> 8);

    return (c1 << 8) | c2;

}

unsigned short tcp_checksum(struct tcp_header *header, unsigned char sip[IPV4_ADDRSIZE], unsigned char tip[IPV4_ADDRSIZE], unsigned short len)
{

    unsigned short *payload = (unsigned short *)header;
    unsigned int sum = 0;

    sum += ((sip[1] << 8) | sip[0]);
    sum += ((sip[3] << 8) | sip[2]);
    sum += ((tip[1] << 8) | tip[0]);
    sum += ((tip[3] << 8) | tip[2]);
    sum += htons(0x0006);
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

    return ~sum;

}

unsigned short tcp_hlen(struct tcp_header *header)
{

    return (header->flags[0] >> 4) * 4;

}

struct tcp_header *tcp_putheader(void *buffer, unsigned char sp[TCP_PORTSIZE], unsigned char tp[TCP_PORTSIZE], unsigned short flags, unsigned int seq, unsigned int ack, unsigned int window)
{

    struct tcp_header *header = buffer;

    tcp_initheader(header, sp, tp);

    header->flags[0] = (5 << 4);
    header->flags[1] = flags;

    net_save32(header->seq, seq);
    net_save32(header->ack, ack);
    net_save16(header->window, window);

    return header;

}

void tcp_initheader(struct tcp_header *header, unsigned char *sp, unsigned char *tp)
{

    buffer_copy(header->sp, sp, TCP_PORTSIZE);
    buffer_copy(header->tp, tp, TCP_PORTSIZE);
    net_save32(header->seq, 0);
    net_save32(header->ack, 0);
    net_save16(header->flags, 0);
    net_save16(header->window, 0);
    net_save16(header->checksum, 0);
    net_save16(header->urgent, 0);

}

