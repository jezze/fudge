#include <fudge.h>
#include "udp.h"

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

