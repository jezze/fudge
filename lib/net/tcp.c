#include <fudge.h>
#include "ipv4.h"
#include "tcp.h"

static unsigned short htons(unsigned short v)
{

    unsigned char c1 = v;
    unsigned char c2 = (v >> 8);

    return (c1 << 8) | c2;

}

unsigned short tcp_checksum(unsigned char sip[IPV4_ADDRSIZE], unsigned char tip[IPV4_ADDRSIZE], unsigned short len, unsigned short *payload)
{

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

void tcp_initheader(struct tcp_header *header, unsigned char *sp, unsigned char *tp)
{

    header->sp[0] = sp[0];
    header->sp[1] = sp[1];
    header->tp[0] = tp[0];
    header->tp[1] = tp[1];
    header->seq[0] = 0;
    header->seq[1] = 0;
    header->seq[2] = 0;
    header->seq[3] = 0;
    header->ack[0] = 0;
    header->ack[1] = 0;
    header->ack[2] = 0;
    header->ack[3] = 0;
    header->flags[0] = 0;
    header->flags[1] = 0;
    header->window[0] = 0;
    header->window[1] = 0;
    header->checksum[0] = 0;
    header->checksum[1] = 0;
    header->urgent[0] = 0;
    header->urgent[1] = 0;

}

