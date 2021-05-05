#define TCP_PORTSIZE                    2
#define TCP_FLAGS1_FIN                  0x01
#define TCP_FLAGS1_SYN                  0x02
#define TCP_FLAGS1_RST                  0x04
#define TCP_FLAGS1_PSH                  0x08
#define TCP_FLAGS1_ACK                  0x10
#define TCP_FLAGS1_URG                  0x20
#define TCP_FLAGS1_ECE                  0x40
#define TCP_FLAGS1_CWR                  0x80
#define TCP_STATE_LISTEN                1
#define TCP_STATE_SYNSENT               2
#define TCP_STATE_SYNRECEIVED           3
#define TCP_STATE_ESTABLISHED           4
#define TCP_STATE_FINWAIT1              5
#define TCP_STATE_FINWAIT2              6
#define TCP_STATE_CLOSEWAIT             7
#define TCP_STATE_CLOSING               8
#define TCP_STATE_LASTACK               9
#define TCP_STATE_TIMEWAIT              10
#define TCP_STATE_CLOSED                11

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

unsigned short tcp_checksum(struct tcp_header *header, unsigned char sip[IPV4_ADDRSIZE], unsigned char tip[IPV4_ADDRSIZE], unsigned short len);
unsigned short tcp_hlen(struct tcp_header *header);
struct tcp_header *tcp_putheader(void *buffer, unsigned char sp[TCP_PORTSIZE], unsigned char tp[TCP_PORTSIZE], unsigned short flags, unsigned int seq, unsigned int ack, unsigned int window);
void tcp_initheader(struct tcp_header *header, unsigned char *sp, unsigned char *tp);
