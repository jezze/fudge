#include <fudge.h>
#include "net.h"
#include "ipv4.h"

unsigned short ipv4_calculatechecksum(struct ipv4_header *header, unsigned int len)
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

unsigned short ipv4_hlen(struct ipv4_header *header)
{

    return (net_load8(header->version) & 0x0F) * 4;

}

unsigned short ipv4_len(struct ipv4_header *header)
{

    return net_load16(header->length);

}

struct ipv4_header *ipv4_putheader(void *buffer, unsigned char sip[IPV4_ADDRSIZE], unsigned char tip[IPV4_ADDRSIZE], unsigned char protocol, unsigned short length)
{

    struct ipv4_header *header = buffer;

    ipv4_initheader(header, sip, tip, protocol, length);

    return header;

}

void ipv4_initheader(struct ipv4_header *header, unsigned char sip[IPV4_ADDRSIZE], unsigned char tip[IPV4_ADDRSIZE], unsigned int protocol, unsigned int count)
{

    unsigned short checksum;

    net_save8(header->version, 0x45);
    net_save8(header->dscp, 0);
    net_save16(header->length, count + 20);
    net_save16(header->identification, 0);
    net_save16(header->fragment, 0);
    net_save8(header->ttl, 64);
    net_save8(header->protocol, protocol);
    net_save16(header->checksum, 0);
    buffer_copy(header->sip, sip, IPV4_ADDRSIZE);
    buffer_copy(header->tip, tip, IPV4_ADDRSIZE);

    checksum = ipv4_calculatechecksum(header, sizeof (struct ipv4_header));

    header->checksum[0] = checksum;
    header->checksum[1] = checksum >> 8;

}

