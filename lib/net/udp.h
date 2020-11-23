#define UDP_PORTSIZE                    2

struct udp_header
{

    unsigned char sp[UDP_PORTSIZE];
    unsigned char tp[UDP_PORTSIZE];
    unsigned char length[2];
    unsigned char checksum[2];

};

void udp_initheader(struct udp_header *header, unsigned char *sp, unsigned char *tp, unsigned int count);
