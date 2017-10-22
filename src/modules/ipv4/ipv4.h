#define IPV4_ADDRSIZE                   4

struct ipv4_header
{

    unsigned char version;
    unsigned char dscp;
    unsigned char length[2];
    unsigned char identification[2];
    unsigned char fragment[2];
    unsigned char ttl;
    unsigned char protocol;
    unsigned char checksum[2];
    unsigned char sip[IPV4_ADDRSIZE];
    unsigned char tip[IPV4_ADDRSIZE];

};

struct ipv4_protocol
{

    struct resource resource;
    struct system_node root;
    struct system_node data;
    struct list datastates;
    unsigned char id;
    void (*notify)(struct ipv4_header *ipv4header, void *buffer, unsigned int count);

};

unsigned int ipv4_calculatechecksum(void *buffer, unsigned int count);
void *ipv4_writehead(void *buffer, unsigned char *sip, unsigned char *tip, unsigned int protocol, unsigned int count);
void ipv4_send(void *buffer, unsigned int count);
void ipv4_registerprotocol(struct ipv4_protocol *protocol);
void ipv4_unregisterprotocol(struct ipv4_protocol *protocol);
void ipv4_initprotocol(struct ipv4_protocol *protocol, char *name, unsigned char id, void (*notify)(struct ipv4_header *ipv4header, void *buffer, unsigned int count));
