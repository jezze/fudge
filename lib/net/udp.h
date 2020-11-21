struct udp_header
{

    unsigned char sp[2];
    unsigned char tp[2];
    unsigned char length[2];
    unsigned char checksum[2];

};

void udp_initheader(struct udp_header *header, unsigned char *sp, unsigned char *tp, unsigned int count);
