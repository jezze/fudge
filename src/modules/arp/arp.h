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
    unsigned int (*match)(unsigned int htype, unsigned char hlength, unsigned int ptype, unsigned char plength);
    unsigned char *(*lookup)(void *paddress);
    void (*save)(void *haddress, void *paddress);

};

void *arp_writeheader(void *buffer, unsigned int htype, unsigned char hlength, unsigned int ptype, unsigned char plength, unsigned int operation);
void *arp_writedata(void *buffer, struct arp_header *header, unsigned char *sha, unsigned char *shp, unsigned char *tha, unsigned char *thp);
void arp_registerhook(struct arp_hook *hook);
void arp_unregisterhook(struct arp_hook *hook);
void arp_inithook(struct arp_hook *hook, unsigned int (*match)(unsigned int htype, unsigned char hlength, unsigned int ptype, unsigned char plength), unsigned char *(*lookup)(void *paddress), void (*save)(void *haddress, void *paddress));
