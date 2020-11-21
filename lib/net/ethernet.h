#define ETHERNET_MTU                    1500
#define ETHERNET_ADDRSIZE               6

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

void ethernet_initheader(struct ethernet_header *header, unsigned int type, unsigned char sha[ETHERNET_ADDRSIZE], unsigned char tha[ETHERNET_ADDRSIZE]);
