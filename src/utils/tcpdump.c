#include <abi.h>
#include <fudge.h>
#include <common.h>

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

void main(void)
{

    struct tcp_header header;
    unsigned int sp;
    unsigned int tp;
    unsigned int seq;
    unsigned int ack;
    unsigned int dataoff;
    unsigned int flags0;
    unsigned int flags1;
    unsigned int flags2;
    unsigned int windowsize;
    unsigned int checksum;
    unsigned int urgent;

    file_open(FILE_PO);
    file_open(FILE_PI);

    for (;;)
    {

        file_readall(FILE_PI, &header, sizeof (struct tcp_header));

        sp = (header.sp[0] << 8) | header.sp[1];
        tp = (header.tp[0] << 8) | header.tp[1];
        seq = (header.seq[0] << 24) | (header.seq[1] << 16) | (header.seq[2] << 8) | header.seq[3];
        ack = (header.ack[0] << 24) | (header.ack[1] << 16) | (header.ack[2] << 8) | header.ack[3];
        dataoff = header.flags[0] >> 4;
        flags0 = header.flags[0] & 0xf;
        flags1 = header.flags[1] >> 4;
        flags2 = header.flags[1] & 0xf;
        windowsize = (header.window[0] << 8) | header.window[1];
        checksum = (header.checksum[0] << 8) | header.checksum[1];
        urgent = (header.urgent[0] << 8) | header.urgent[1];

        write_keydec("source port", sp);
        write_keydec("target port", tp);
        write_keydec("seq number", seq);
        write_keydec("ack number", ack);
        write_keydec("data offset", dataoff);
        write_keydec("flags0", flags0);
        write_keydec("flags1", flags1);
        write_keydec("flags2", flags2);
        write_keydec("window size", windowsize);
        write_keydec("checksum", checksum);
        write_keydec("urgent", urgent);

        if (dataoff > 5)
        {

            char options[FUDGE_BSIZE];

            file_readall(FILE_PI, options, 4 * (dataoff - 5));

        }

        write_keystring("---", "");

    }

    file_close(FILE_PI);
    file_close(FILE_PO);

}

