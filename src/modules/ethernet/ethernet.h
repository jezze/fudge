#define ETHERNET_MTU                    1500
#define ETHERNET_ADDRSIZE               6

struct ethernet_header
{

    unsigned char tha[6];
    unsigned char sha[6];
    unsigned char type[2];

};

struct ethernet_footer
{

    unsigned char crc[4];

};

struct ethernet_interface
{

    struct resource resource;
    struct system_node root;
    struct system_node ctrl;
    struct system_node data;
    struct system_node addr;
    struct list datastates;
    unsigned int id;
    unsigned char haddress[ETHERNET_ADDRSIZE];
    unsigned int (*send)(void *buffer, unsigned int count);

};

struct ethernet_protocol
{

    struct resource resource;
    struct system_node root;
    struct system_node data;
    struct list datastates;
    unsigned int type;
    void (*notify)(struct ethernet_interface *ethernetinterface, struct ethernet_header *header, void *buffer, unsigned int count);

};

void *ethernet_writehead(void *buffer, unsigned int type, unsigned char *sha, unsigned char *tha);
void ethernet_send(struct ethernet_interface *interface, void *buffer, unsigned int count);
void ethernet_notify(struct ethernet_interface *interface, void *buffer, unsigned int count);
struct ethernet_interface *ethernet_findinterface(void *haddress);
void ethernet_registerinterface(struct ethernet_interface *interface, unsigned int id);
void ethernet_registerprotocol(struct ethernet_protocol *protocol);
void ethernet_unregisterinterface(struct ethernet_interface *interface);
void ethernet_unregisterprotocol(struct ethernet_protocol *protocol);
void ethernet_initinterface(struct ethernet_interface *interface, unsigned int (*send)(void *buffer, unsigned int count));
void ethernet_initprotocol(struct ethernet_protocol *protocol, char *name, unsigned int type, void (*notify)(struct ethernet_interface *ethernetinterface, struct ethernet_header *header, void *buffer, unsigned int count));
