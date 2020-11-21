#define IPV4_ADDRSIZE                   4

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

unsigned int ipv4_calculatechecksum(void *buffer, unsigned int count);
void ipv4_initheader(struct ipv4_header *header, unsigned char sip[IPV4_ADDRSIZE], unsigned char tip[IPV4_ADDRSIZE], unsigned int protocol, unsigned int count);
