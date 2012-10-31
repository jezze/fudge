struct ipv4_header
{

    unsigned char version;
    unsigned char ihl;
    unsigned short dscp;
    unsigned int length;
    unsigned int id;
    unsigned char flags;
    unsigned char offset[3];
    unsigned char ttl;
    unsigned char protocol;
    unsigned short checksum;
    unsigned char sip[4];
    unsigned char tip[4];

} __attribute__((packed));
