#define ARP_REQUEST                     1
#define ARP_REPLY                       2

struct arp_header
{

    unsigned char htype[2];
    unsigned char ptype[2];
    unsigned char hlength;
    unsigned char plength;
    unsigned char operation[2];

};

void arp_initheader(struct arp_header *header, unsigned int htype, unsigned char hlength, unsigned int ptype, unsigned char plength, unsigned int operation);
