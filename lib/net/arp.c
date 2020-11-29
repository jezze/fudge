#include <fudge.h>
#include "arp.h"

unsigned short arp_hlen(struct arp_header *header)
{

    return sizeof (struct arp_header);

}

unsigned short arp_len(struct arp_header *header)
{

    return sizeof (struct arp_header) + header->hlength * 2 + header->plength * 2;

}

void arp_initheader(struct arp_header *header, unsigned short htype, unsigned char hlength, unsigned short ptype, unsigned char plength, unsigned short operation)
{

    header->htype[0] = htype >> 8;
    header->htype[1] = htype;
    header->ptype[0] = ptype >> 8;
    header->ptype[1] = ptype;
    header->hlength = hlength;
    header->plength = plength;
    header->operation[0] = operation >> 8;
    header->operation[1] = operation;

}

