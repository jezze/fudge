#include "event.h"
#include "file.h"
#include "ascii.h"
#include "message.h"
#include "p9p.h"

unsigned short p9p_read1(unsigned char p[1])
{

    return (p[0] << 0);

}

unsigned short p9p_read2(unsigned char p[2])
{

    return (p[0] << 8 | p[1] << 0);

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

void p9p_mktwalk(struct message *message, char *wname)
{

    struct event_p9p header;
    struct p9p_twalk body;

    p9p_write4(header.size, 0);
    p9p_write1(header.type, P9P_TWALK);
    p9p_write2(header.tag, 0);
    p9p_write4(body.fid, 0);
    p9p_write4(body.newfid, 0);
    p9p_write2(body.nwname, ascii_length(wname));
    message_init(message, EVENT_P9P);
    message_putbuffer(message, sizeof (struct event_p9p), &header);
    message_putbuffer(message, sizeof (struct p9p_twalk), &body);
    message_putbuffer(message, p9p_read2(body.nwname), wname);

}

void p9p_mkrwalk(struct message *message)
{

    struct event_p9p header;
    struct p9p_rwalk body;

    p9p_write4(header.size, 0);
    p9p_write1(header.type, P9P_RWALK);
    p9p_write2(header.tag, 0);
    p9p_write2(body.nwqid, 0);
    message_init(message, EVENT_P9P);
    message_putbuffer(message, sizeof (struct event_p9p), &header);
    message_putbuffer(message, sizeof (struct p9p_twalk), &body);

}

void p9p_mktread(struct message *message, unsigned int fid, unsigned int offsetl, unsigned int offseth, unsigned int count)
{

    struct event_p9p header;
    struct p9p_tread body;

    p9p_write4(header.size, 0);
    p9p_write1(header.type, P9P_TREAD);
    p9p_write2(header.tag, 0);
    p9p_write4(body.fid, fid);
    p9p_write8(body.offset, offsetl, offseth);
    p9p_write4(body.count, count);
    message_init(message, EVENT_P9P);
    message_putbuffer(message, sizeof (struct event_p9p), &header);
    message_putbuffer(message, sizeof (struct p9p_tread), &body);

}

void p9p_mkrread(struct message *message, unsigned int count, void *buffer)
{

    struct event_p9p header;

    p9p_write4(header.size, 0);
    p9p_write1(header.type, P9P_RREAD);
    p9p_write2(header.tag, 0);
    message_init(message, EVENT_P9P);
    message_putbuffer(message, sizeof (struct event_p9p), &header);
    message_putbuffer(message, count, buffer);

}

