#include <fudge.h>
#include "tcp.h"

/*
static unsigned char tcp_checksum(unsigned short iphdr_totlen, unsigned char iphdr_ihl, unsigned short iphdr_saddr, unsigned short iphdr_daddr, unsigned short *payload)
{

    unsigned int sum = 0;
    unsigned short tcpLen = ntohs(iphdr_totlen) - (iphdr_ihl << 2);

    sum += (saddr >> 16) & 0xFFFF;
    sum += (saddr) & 0xFFFF;
    sum += (daddr >> 16) & 0xFFFF;
    sum += (daddr) & 0xFFFF;
    sum += htons(0x06);
    sum += htons(tcpLen);

    while (tcpLen > 1)
    {

        sum += *payload++;
        tcpLen -= 2;

    }

    if (tcpLen > 0)
        sum += ((*payload) & htons(0xFF00));

    while (sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);

    sum = ~sum;

    return (unsigned short)sum;
}
*/

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

