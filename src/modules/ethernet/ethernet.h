#define ETHERNET_MTU                    1500
#define ETHERNET_ADDRSIZE               6

struct ethernet_interface
{

    struct resource resource;
    struct system_node root;
    struct system_node ctrl;
    struct system_node data;
    struct system_node addr;
    unsigned int id;
    unsigned char haddress[ETHERNET_ADDRSIZE];
    unsigned int (*send)(void *buffer, unsigned int count);

};

struct ethernet_protocol
{

    struct resource resource;
    struct system_node root;
    struct system_node data;
    unsigned short type;
    void (*notify)(struct ethernet_interface *interface, void *buffer, unsigned int count);

};

unsigned int ethernet_writeheader(struct ethernet_protocol *protocol, unsigned char *sha, unsigned char *tha, void *buffer);
void ethernet_notify(struct ethernet_interface *interface, void *buffer, unsigned int count);
void ethernet_registerinterface(struct ethernet_interface *interface, unsigned int id);
void ethernet_registerprotocol(struct ethernet_protocol *protocol);
void ethernet_unregisterinterface(struct ethernet_interface *interface);
void ethernet_unregisterprotocol(struct ethernet_protocol *protocol);
void ethernet_initinterface(struct ethernet_interface *interface, unsigned int (*send)(void *buffer, unsigned int count));
void ethernet_initprotocol(struct ethernet_protocol *protocol, char *name, unsigned short type, void (*notify)(struct ethernet_interface *interface, void *buffer, unsigned int count));
