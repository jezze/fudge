#include <fudge.h>
#include "net.h"
#include "ipv4.h"
#include "tcp.h"

unsigned short tcp_checksum(struct tcp_header *header, unsigned char sip[IPV4_ADDRSIZE], unsigned char tip[IPV4_ADDRSIZE], unsigned short len)
{

    unsigned int sum = net_sum16(sip, IPV4_ADDRSIZE, 0);

    sum = net_sum16(tip, IPV4_ADDRSIZE, sum);

    return net_checksum(net_sum16(header, len, sum + 0x0006 + len));

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

