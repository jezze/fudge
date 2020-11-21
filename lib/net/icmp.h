#define ICMP_ECHOREPLY                  0
#define ICMP_UNREACHABLE                3
#define ICMP_REDIRECT                   5
#define ICMP_ECHOREQUEST                8
#define ICMP_ROUTERADVERTISEMENT        9
#define ICMP_ROUTERSOLICITATION         10
#define ICMP_TIMEEXCEEDED               11

struct icmp_header
{

    unsigned char type;
    unsigned char code;
    unsigned char checksum[2];

};

void icmp_initheader(struct icmp_header *header, unsigned char type, unsigned char code);
