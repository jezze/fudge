#include "event.h"
#include "file.h"
#include "ascii.h"
#include "message.h"
#include "p9p.h"

unsigned char p9p_read1(unsigned char p[1])
{

    return (p[0] << 0);

}

unsigned short p9p_read2(unsigned char p[2])
{

    return (p[1] << 8 | p[0] << 0);

}

unsigned int p9p_read4(unsigned char p[4])
{

    return (p[3] << 24 | p[2] << 16 | p[1] << 8 | p[0] << 0);

}

void p9p_write1(unsigned char p[1], unsigned char v)
{

    p[0] = v >> 0;

}

void p9p_write2(unsigned char p[2], unsigned short v)
{

    p[0] = v >> 0;
    p[1] = v >> 8;

}

void p9p_write4(unsigned char p[4], unsigned int v)
{

    p[0] = v >> 0;
    p[1] = v >> 8;
    p[2] = v >> 16;
    p[3] = v >> 24;

}

void p9p_write8(unsigned char p[8], unsigned int vl, unsigned int vh)
{

    p[0] = vl >> 0;
    p[1] = vl >> 8;
    p[2] = vl >> 16;
    p[3] = vl >> 24;
    p[4] = vh >> 0;
    p[5] = vh >> 8;
    p[6] = vh >> 16;
    p[7] = vh >> 24;

}

void p9p_mktversion(struct message *message, unsigned short tag, unsigned int msize, char *version)
{

    struct event_p9p header;
    struct p9p_tversion body;
    unsigned short count = ascii_length(version);

    p9p_write4(header.size, sizeof (struct event_p9p) + sizeof (struct p9p_tversion) + count);
    p9p_write1(header.type, P9P_TVERSION);
    p9p_write2(header.tag, tag);
    p9p_write4(body.msize, msize);
    message_init(message, EVENT_P9P);
    message_putbuffer(message, sizeof (struct event_p9p), &header);
    message_putbuffer(message, sizeof (struct p9p_tversion), &body);
    message_putbuffer(message, count, version);

}

void p9p_mkrversion(struct message *message, unsigned short tag, unsigned int msize, char *version)
{

    struct event_p9p header;
    struct p9p_rversion body;
    unsigned short count = ascii_length(version);

    p9p_write4(header.size, sizeof (struct event_p9p) + sizeof (struct p9p_rversion) + count);
    p9p_write1(header.type, P9P_RVERSION);
    p9p_write2(header.tag, tag);
    p9p_write4(body.msize, msize);
    message_init(message, EVENT_P9P);
    message_putbuffer(message, sizeof (struct event_p9p), &header);
    message_putbuffer(message, sizeof (struct p9p_rversion), &body);
    message_putbuffer(message, count, version);

}

void p9p_mktwalk(struct message *message, unsigned short tag, unsigned int fid, unsigned int newfid, char *wname)
{

    struct event_p9p header;
    struct p9p_twalk body;
    unsigned short count = ascii_length(wname);

    p9p_write4(header.size, sizeof (struct event_p9p) + sizeof (struct p9p_twalk) + count);
    p9p_write1(header.type, P9P_TWALK);
    p9p_write2(header.tag, tag);
    p9p_write4(body.fid, fid);
    p9p_write4(body.newfid, newfid);
    p9p_write2(body.nwname, count);
    message_init(message, EVENT_P9P);
    message_putbuffer(message, sizeof (struct event_p9p), &header);
    message_putbuffer(message, sizeof (struct p9p_twalk), &body);
    message_putbuffer(message, count, wname);

}

void p9p_mkrwalk(struct message *message, unsigned short tag, unsigned short nwqid)
{

    struct event_p9p header;
    struct p9p_rwalk body;

    p9p_write4(header.size, sizeof (struct event_p9p) + sizeof (struct p9p_rwalk));
    p9p_write1(header.type, P9P_RWALK);
    p9p_write2(header.tag, tag);
    p9p_write2(body.nwqid, nwqid);
    message_init(message, EVENT_P9P);
    message_putbuffer(message, sizeof (struct event_p9p), &header);
    message_putbuffer(message, sizeof (struct p9p_twalk), &body);

}

void p9p_mktread(struct message *message, unsigned short tag, unsigned int fid, unsigned int offsetl, unsigned int offseth, unsigned int count)
{

    struct event_p9p header;
    struct p9p_tread body;

    p9p_write4(header.size, sizeof (struct event_p9p) + sizeof (struct p9p_tread));
    p9p_write1(header.type, P9P_TREAD);
    p9p_write2(header.tag, tag);
    p9p_write4(body.fid, fid);
    p9p_write8(body.offset, offsetl, offseth);
    p9p_write4(body.count, count);
    message_init(message, EVENT_P9P);
    message_putbuffer(message, sizeof (struct event_p9p), &header);
    message_putbuffer(message, sizeof (struct p9p_tread), &body);

}

void p9p_mkrread(struct message *message, unsigned short tag, unsigned int count, void *buffer)
{

    struct event_p9p header;
    struct p9p_rread body;

    p9p_write4(header.size, sizeof (struct event_p9p) + sizeof (struct p9p_rread) + count);
    p9p_write1(header.type, P9P_RREAD);
    p9p_write2(header.tag, tag);
    p9p_write4(body.count, count);
    message_init(message, EVENT_P9P);
    message_putbuffer(message, sizeof (struct event_p9p), &header);
    message_putbuffer(message, sizeof (struct p9p_rread), &body);
    message_putbuffer(message, count, buffer);

}

void p9p_mktopen(struct message *message, unsigned short tag, unsigned int fid, unsigned char mode)
{

    struct event_p9p header;
    struct p9p_topen body;

    p9p_write4(header.size, sizeof (struct event_p9p) + sizeof (struct p9p_topen));
    p9p_write1(header.type, P9P_TOPEN);
    p9p_write2(header.tag, tag);
    p9p_write4(body.fid, fid);
    p9p_write1(body.mode, mode);
    message_init(message, EVENT_P9P);
    message_putbuffer(message, sizeof (struct event_p9p), &header);
    message_putbuffer(message, sizeof (struct p9p_topen), &body);

}

void p9p_mkropen(struct message *message, unsigned short tag, unsigned int qid, unsigned int iounit)
{

    struct event_p9p header;
    struct p9p_ropen body;

    p9p_write4(header.size, sizeof (struct event_p9p) + sizeof (struct p9p_ropen));
    p9p_write1(header.type, P9P_ROPEN);
    p9p_write2(header.tag, tag);
    /* What about qid here */
    p9p_write4(body.iounit, iounit);
    message_init(message, EVENT_P9P);
    message_putbuffer(message, sizeof (struct event_p9p), &header);
    message_putbuffer(message, sizeof (struct p9p_ropen), &body);

}

