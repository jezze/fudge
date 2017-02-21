#define ARP_PROTOCOL                    0x0806
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

struct arp_hook
{

    struct list_item item;
    unsigned int (*match)(unsigned short htype, unsigned char hlength, unsigned short ptype, unsigned char plength);
    unsigned char *(*lookup)(void *paddress);
    void (*save)(void *haddress, void *paddress);

};

unsigned int arp_writeheader(unsigned short htype, unsigned char hlength, unsigned short ptype, unsigned char plength, unsigned short operation, unsigned char *sha, unsigned char *shp, unsigned char *tha, unsigned char *thp, void *buffer);
void arp_registerhook(struct arp_hook *hook);
void arp_unregisterhook(struct arp_hook *hook);
void arp_inithook(struct arp_hook *hook, unsigned int (*match)(unsigned short htype, unsigned char hlength, unsigned short ptype, unsigned char plength), unsigned char *(*lookup)(void *paddress), void (*save)(void *haddress, void *paddress));
