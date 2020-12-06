#define ETHERNET_MTU                    1500
#define ETHERNET_ADDRSIZE               6
#define ETHERNET_TYPE_ARP               0x0806
#define ETHERNET_TYPE_IPV4              0x0800
#define ETHERNET_TYPE_IPV6              0x86DD

struct ethernet_header
{

    unsigned char tha[ETHERNET_ADDRSIZE];
    unsigned char sha[ETHERNET_ADDRSIZE];
    unsigned char type[2];

};

struct ethernet_footer
{

    unsigned char crc[4];

};

unsigned short ethernet_hlen(struct ethernet_header *header);
struct ethernet_header *ethernet_putheader(void *buffer, unsigned short type, unsigned char sha[ETHERNET_ADDRSIZE], unsigned char tha[ETHERNET_ADDRSIZE]);
void ethernet_initheader(struct ethernet_header *header, unsigned short type, unsigned char sha[ETHERNET_ADDRSIZE], unsigned char tha[ETHERNET_ADDRSIZE]);
