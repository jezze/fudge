#include <fudge.h>
#include "net.h"
#include "arp.h"

unsigned short arp_hlen(struct arp_header *header)
{

    return sizeof (struct arp_header);

}

unsigned short arp_len(struct arp_header *header)
{

    return sizeof (struct arp_header) + net_load8(header->hlength) * 2 + net_load8(header->plength) * 2;

}

struct arp_header *arp_putheader(void *buffer, unsigned short htype, unsigned char hlength, unsigned short ptype, unsigned char plength, unsigned short operation)
{

    struct arp_header *header = buffer;

    arp_initheader(header, htype, hlength, ptype, plength, operation);

    return header;

}

void arp_initheader(struct arp_header *header, unsigned short htype, unsigned char hlength, unsigned short ptype, unsigned char plength, unsigned short operation)
{

    net_save16(header->htype, htype);
    net_save16(header->ptype, ptype);
    net_save8(header->hlength, hlength);
    net_save8(header->plength, plength);
    net_save16(header->operation, operation);

}

