struct ipv4_header
{

    unsigned char version;
    unsigned char dscp;
    unsigned char length[2];
    unsigned char id[2];
    unsigned char flags[2];
    unsigned char ttl;
    unsigned char protocol;
    unsigned char checksum[2];
    unsigned char sip[4];
    unsigned char tip[4];

};
