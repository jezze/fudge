#define TCP_PORTSIZE                    2
#define TCP_FLAGS1_FIN                  0x01
#define TCP_FLAGS1_SYN                  0x02
#define TCP_FLAGS1_RST                  0x04
#define TCP_FLAGS1_PSH                  0x08
#define TCP_FLAGS1_ACK                  0x10
#define TCP_FLAGS1_URG                  0x20
#define TCP_FLAGS1_ECE                  0x40
#define TCP_FLAGS1_CWR                  0x80

struct tcp_header
{

    unsigned char sp[TCP_PORTSIZE];
    unsigned char tp[TCP_PORTSIZE];
    unsigned char seq[4];
    unsigned char ack[4];
    unsigned char flags[2];
    unsigned char window[2];
    unsigned char checksum[2];
    unsigned char urgent[2];

};

unsigned short tcp_checksum(unsigned int iphdr_saddr, unsigned int iphdr_daddr, unsigned short *payload);
void tcp_initheader(struct tcp_header *header, unsigned char *sp, unsigned char *tp);
