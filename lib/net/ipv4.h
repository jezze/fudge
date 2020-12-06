#define IPV4_ADDRSIZE                   4
#define IPV4_PROTOCOL_ICMP              0x01
#define IPV4_PROTOCOL_TCP               0x06
#define IPV4_PROTOCOL_UDP               0x11

struct ipv4_header
{

    unsigned char version;
    unsigned char dscp;
    unsigned char length[2];
    unsigned char identification[2];
    unsigned char fragment[2];
    unsigned char ttl;
    unsigned char protocol;
    unsigned char checksum[2];
    unsigned char sip[IPV4_ADDRSIZE];
    unsigned char tip[IPV4_ADDRSIZE];

};

unsigned short ipv4_calculatechecksum(struct ipv4_header *header, unsigned int len);
unsigned short ipv4_hlen(struct ipv4_header *header);
unsigned short ipv4_len(struct ipv4_header *header);
struct ipv4_header *ipv4_putheader(void *buffer, unsigned char sip[IPV4_ADDRSIZE], unsigned char tip[IPV4_ADDRSIZE], unsigned char protocol, unsigned short length);
void ipv4_initheader(struct ipv4_header *header, unsigned char sip[IPV4_ADDRSIZE], unsigned char tip[IPV4_ADDRSIZE], unsigned int protocol, unsigned int count);
