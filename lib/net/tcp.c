#include <fudge.h>
#include "tcp.h"

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

