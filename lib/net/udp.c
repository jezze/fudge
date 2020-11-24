#include <fudge.h>
#include "udp.h"

static unsigned short htons(unsigned short v)
{

    unsigned char c1 = v;
    unsigned char c2 = v >> 8;

    return c1 << 8 | c2;

}

unsigned short udp_checksum(unsigned short iphdr_saddr, unsigned short iphdr_daddr, unsigned short udphdr_len, unsigned short *payload)
{

    unsigned int sum = 0;
    unsigned short udpLen = htons(udphdr_len);

    sum += (iphdr_saddr >> 16) & 0xFFFF;
    sum += (iphdr_saddr) & 0xFFFF;
    sum += (iphdr_daddr >> 16) & 0xFFFF;
    sum += (iphdr_daddr) & 0xFFFF;
    sum += htons(0x11);
    sum += htons(udpLen);

    while (udpLen > 1)
    {

        sum += * payload++;
        udpLen -= 2;

    }

    if (udpLen > 0)
        sum += ((*payload)&htons(0xFF00));

    while (sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);

    sum = ~sum;

    return ((unsigned short)sum == 0x0000) ? 0xFFFF : (unsigned short)sum;

}

void udp_initheader(struct udp_header *header, unsigned char *sp, unsigned char *tp, unsigned int count)
{

    header->sp[0] = sp[0];
    header->sp[1] = sp[1];
    header->tp[0] = tp[0];
    header->tp[1] = tp[1];
    header->length[0] = (count + 8) >> 8;
    header->length[1] = (count + 8);
    header->checksum[0] = 0;
    header->checksum[1] = 0;

}

