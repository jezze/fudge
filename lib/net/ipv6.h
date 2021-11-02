#define IPV6_ADDRSIZE                   16

struct ipv6_header
{

    unsigned char label[4];
    unsigned char length[2];
    unsigned char next[1];
    unsigned char hop[1];
    unsigned char sip[IPV6_ADDRSIZE];
    unsigned char tip[IPV6_ADDRSIZE];

};
