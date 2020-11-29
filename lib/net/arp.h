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

unsigned short arp_hlen(struct arp_header *header);
unsigned short arp_len(struct arp_header *header);
void arp_initheader(struct arp_header *header, unsigned short htype, unsigned char hlength, unsigned short ptype, unsigned char plength, unsigned short operation);
