#define ICMP_ECHOREPLY                  0
#define ICMP_UNREACHABLE                3
#define ICMP_REDIRECT                   5
#define ICMP_ECHOREQUEST                8
#define ICMP_ROUTERADVERTISEMENT        9
#define ICMP_ROUTERSOLICITATION         10
#define ICMP_TIMEEXCEEDED               11

struct icmp_header
{

    unsigned char type[1];
    unsigned char code[1];
    unsigned char checksum[2];

};

unsigned short icmp_calculatechecksum(struct icmp_header *header, unsigned int len);
unsigned short icmp_hlen(struct icmp_header *header);
struct icmp_header *icmp_putheader(void *buffer, unsigned char type, unsigned char code);
void icmp_initheader(struct icmp_header *header, unsigned char type, unsigned char code);
