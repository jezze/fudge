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

void *arp_writehead(void *buffer, unsigned int htype, unsigned char hlength, unsigned int ptype, unsigned char plength, unsigned int operation, unsigned char *sha, unsigned char *spa, unsigned char *tha, unsigned char *tpa);
void arp_send(void *buffer, unsigned int count);
void arp_registerhook(struct arp_hook *hook);
void arp_unregisterhook(struct arp_hook *hook);
void arp_inithook(struct arp_hook *hook, unsigned int (*match)(unsigned int htype, unsigned char hlength, unsigned int ptype, unsigned char plength), unsigned char *(*lookup)(void *paddress), void (*save)(void *haddress, void *paddress));
