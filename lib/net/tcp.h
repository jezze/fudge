#define TCP_PORTSIZE                    2

struct tcp_header
{

    unsigned char sp[TCP_PORTSIZE];
    unsigned char tp[TCP_PORTSIZE];
    unsigned char seq[4];
    unsigned char ack[4];
    unsigned char flags[2];
    unsigned char window[2];
    unsigned char checksum[2];
    unsigned char urgent[2];

};
