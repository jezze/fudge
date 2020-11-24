#define UDP_PORTSIZE                    2

struct udp_header
{

    unsigned char sp[UDP_PORTSIZE];
    unsigned char tp[UDP_PORTSIZE];
    unsigned char length[2];
    unsigned char checksum[2];

};

unsigned short udp_checksum(unsigned short iphdr_saddr, unsigned short iphdr_daddr, unsigned short udphdr_len, unsigned short *payload);
void udp_initheader(struct udp_header *header, unsigned char *sp, unsigned char *tp, unsigned int count);
