#define UDP_PORTSIZE                    2

struct udp_header
{

    unsigned char sp[UDP_PORTSIZE];
    unsigned char tp[UDP_PORTSIZE];
    unsigned char length[2];
    unsigned char checksum[2];

};

unsigned short udp_checksum(unsigned char sip[IPV4_ADDRSIZE], unsigned char tip[IPV4_ADDRSIZE], unsigned short len, unsigned short *payload);
void udp_initheader(struct udp_header *header, unsigned char *sp, unsigned char *tp, unsigned int count);
