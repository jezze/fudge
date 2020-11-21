struct tcp_header
{

    unsigned char sp[2];
    unsigned char tp[2];
    unsigned char seq[4];
    unsigned char ack[4];
    unsigned char flags[2];
    unsigned char window[2];
    unsigned char checksum[2];
    unsigned char urgent[2];

};
