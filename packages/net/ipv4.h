struct ipv4_header
{

    unsigned char version;
    unsigned char ihl;
    unsigned char dscp[2];
    unsigned char length[4];
    unsigned char id[4];
    unsigned char flags;
    unsigned char offset[3];
    unsigned char ttl;
    unsigned char protocol;
    unsigned char checksum[2];
    unsigned char sip[4];
    unsigned char tip[4];

};
