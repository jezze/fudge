struct tcp_header
{

    unsigned short sp;
    unsigned short tp;
    unsigned short seq[2];
    unsigned short ack[2];
    unsigned short flags;
    unsigned short window;
    unsigned short checksum;
    unsigned short urgent;

};
