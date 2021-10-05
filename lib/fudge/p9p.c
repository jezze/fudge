#include "event.h"
#include "buffer.h"
#include "file.h"
#include "ascii.h"
#include "message.h"
#include "p9p.h"

unsigned char p9p_read1(void *buffer)
{

    unsigned char *p = buffer;

    return (p[0] << 0);

}

unsigned short p9p_read2(void *buffer)
{

    unsigned char *p = buffer;

    return (p[1] << 8 | p[0] << 0);

}

unsigned int p9p_read4(void *buffer)
{

    unsigned char *p = buffer;

    return (p[3] << 24 | p[2] << 16 | p[1] << 8 | p[0] << 0);

}

unsigned int p9p_write1(void *buffer, unsigned char v)
{

    unsigned char *p = buffer;

    p[0] = v >> 0;

    return 1;

}

unsigned int p9p_write2(void *buffer, unsigned short v)
{

    unsigned char *p = buffer;

    p[0] = v >> 0;
    p[1] = v >> 8;

    return 2;

}

unsigned int p9p_write4(void *buffer, unsigned int v)
{

    unsigned char *p = buffer;

    p[0] = v >> 0;
    p[1] = v >> 8;
    p[2] = v >> 16;
    p[3] = v >> 24;

    return 4;

}

unsigned int p9p_write8(void *buffer, unsigned int vl, unsigned int vh)
{

    unsigned char *p = buffer;

    p[0] = vl >> 0;
    p[1] = vl >> 8;
    p[2] = vl >> 16;
    p[3] = vl >> 24;
    p[4] = vh >> 0;
    p[5] = vh >> 8;
    p[6] = vh >> 16;
    p[7] = vh >> 24;

    return 8;

}

unsigned int p9p_writebuffer(void *buffer, unsigned int count, void *v)
{

    buffer_copy(buffer, v, count);

    return count;

}

unsigned int p9p_writestring(void *buffer, char *v)
{

    unsigned short count = ascii_length(v);
    unsigned char *x = buffer;
    unsigned int n = 0;

    n += p9p_write2(x + n, count);
    n += p9p_writebuffer(x + n, count, v);

    return n;

}

unsigned int p9p_mk(void *buffer, unsigned int size, unsigned char type, unsigned short tag)
{

    unsigned char *x = buffer;
    unsigned int n = 0;

    n += p9p_write4(x + n, size);
    n += p9p_write1(x + n, type);
    n += p9p_write2(x + n, tag);

    return n;

}

unsigned int p9p_mkrerror(void *buffer, unsigned short tag, char *ename)
{

    unsigned char *x = buffer;
    unsigned int n = sizeof (struct event_p9p);

    n += p9p_writestring(x + n, ename);

    p9p_mk(buffer, n, P9P_RERROR, tag);

    return n;

}

unsigned int p9p_mktattach(void *buffer, unsigned short tag, unsigned int fid, unsigned int afid, char *uname, char *aname)
{

    unsigned char *x = buffer;
    unsigned int n = sizeof (struct event_p9p);

    n += p9p_write4(x + n, fid);
    n += p9p_write4(x + n, afid);
    n += p9p_writestring(x + n, uname);
    n += p9p_writestring(x + n, aname);

    p9p_mk(buffer, n, P9P_TATTACH, tag);

    return n;

}

unsigned int p9p_mkrattach(void *buffer, unsigned short tag, char *qid[13])
{

    unsigned int n = sizeof (struct event_p9p);

    n += 13 /* qid */;

    p9p_mk(buffer, n, P9P_RATTACH, tag);

    return n;

}

unsigned int p9p_mktauth(void *buffer, unsigned short tag, unsigned int afid, char *uname, char *aname)
{

    unsigned char *x = buffer;
    unsigned int n = sizeof (struct event_p9p);

    n += p9p_write4(x + n, afid);
    n += p9p_writestring(x + n, uname);
    n += p9p_writestring(x + n, aname);

    p9p_mk(buffer, n, P9P_TAUTH, tag);

    return n;

}

unsigned int p9p_mkrauth(void *buffer, unsigned short tag, char *aqid[13])
{

    unsigned int n = sizeof (struct event_p9p);

    n += 13 /* aqid */;

    p9p_mk(buffer, n, P9P_RAUTH, tag);

    return n;

}

unsigned int p9p_mktclunk(void *buffer, unsigned short tag, unsigned int fid)
{

    unsigned char *x = buffer;
    unsigned int n = sizeof (struct event_p9p);

    n += p9p_write4(x + n, fid);

    p9p_mk(buffer, n, P9P_TCLUNK, tag);

    return n;

}

unsigned int p9p_mkrclunk(void *buffer, unsigned short tag)
{

    unsigned int n = sizeof (struct event_p9p);

    p9p_mk(buffer, n, P9P_RCLUNK, tag);

    return n;

}

unsigned int p9p_mktflush(void *buffer, unsigned short tag, unsigned short oldtag)
{

    unsigned char *x = buffer;
    unsigned int n = sizeof (struct event_p9p);

    n += p9p_write2(x + n, oldtag);

    p9p_mk(buffer, n, P9P_TFLUSH, tag);

    return n;

}

unsigned int p9p_mkrflush(void *buffer, unsigned short tag)
{

    unsigned int n = sizeof (struct event_p9p);

    p9p_mk(buffer, n, P9P_RFLUSH, tag);

    return n;

}

unsigned int p9p_mktversion(void *buffer, unsigned short tag, unsigned int msize, char *version)
{

    unsigned char *x = buffer;
    unsigned int n = sizeof (struct event_p9p);

    n += p9p_write4(x + n, msize);
    n += p9p_writestring(x + n, version);

    p9p_mk(buffer, n, P9P_TVERSION, tag);

    return n;

}

unsigned int p9p_mkrversion(void *buffer, unsigned short tag, unsigned int msize, char *version)
{

    unsigned char *x = buffer;
    unsigned int n = sizeof (struct event_p9p);

    n += p9p_write4(x + n, msize);
    n += p9p_writestring(x + n, version);

    p9p_mk(buffer, n, P9P_RVERSION, tag);

    return n;

}

unsigned int p9p_mktwalk(void *buffer, unsigned short tag, unsigned int fid, unsigned int newfid, unsigned short nwname, char **wname)
{

    unsigned char *x = buffer;
    unsigned int n = sizeof (struct event_p9p);
    unsigned int i;

    n += p9p_write4(x + n, fid);
    n += p9p_write4(x + n, newfid);
    n += p9p_write2(x + n, nwname);

    for (i = 0; i < nwname; i++)
        n += p9p_writestring(x + n, wname[i]);

    p9p_mk(buffer, n, P9P_TWALK, tag);

    return n;

}

unsigned int p9p_mkrwalk(void *buffer, unsigned short tag, unsigned short nwqid, char *wqid[13])
{

    unsigned char *x = buffer;
    unsigned int n = sizeof (struct event_p9p);
    unsigned int i;

    n += p9p_write2(x + n, nwqid);

    for (i = 0; i < nwqid; i++)
        n += 13; /* wqid */

    p9p_mk(buffer, n, P9P_RWALK, tag);

    return n;

}

unsigned int p9p_mktread(void *buffer, unsigned short tag, unsigned int fid, unsigned int offsetl, unsigned int offseth, unsigned int count)
{

    unsigned char *x = buffer;
    unsigned int n = sizeof (struct event_p9p);

    n += p9p_write4(x + n, fid);
    n += p9p_write8(x + n, offsetl, offseth);
    n += p9p_write4(x + n, count);

    p9p_mk(buffer, n, P9P_TREAD, tag);

    return n;

}

unsigned int p9p_mkrread(void *buffer, unsigned short tag, unsigned int count, void *data)
{

    unsigned char *x = buffer;
    unsigned int n = sizeof (struct event_p9p);

    n += p9p_write4(x + n, count);
    n += p9p_writebuffer(x + n, count, data);

    p9p_mk(buffer, n, P9P_RREAD, tag);

    return n;

}

unsigned int p9p_mktwrite(void *buffer, unsigned short tag, unsigned int fid, unsigned int offsetl, unsigned int offseth, unsigned int count, void *data)
{

    unsigned char *x = buffer;
    unsigned int n = sizeof (struct event_p9p);

    n += p9p_write4(x + n, fid);
    n += p9p_write8(x + n, offsetl, offseth);
    n += p9p_write4(x + n, count);
    n += p9p_writebuffer(x + n, count, data);

    p9p_mk(buffer, n, P9P_TWRITE, tag);

    return n;

}

unsigned int p9p_mkrwrite(void *buffer, unsigned short tag, unsigned int count)
{

    unsigned char *x = buffer;
    unsigned int n = sizeof (struct event_p9p);

    n += p9p_write4(x + n, count);

    p9p_mk(buffer, n, P9P_RWRITE, tag);

    return n;

}

unsigned int p9p_mktopen(void *buffer, unsigned short tag, unsigned int fid, unsigned char mode)
{

    unsigned char *x = buffer;
    unsigned int n = sizeof (struct event_p9p);

    n += p9p_write4(x + n, fid);
    n += p9p_write1(x + n, mode);

    p9p_mk(buffer, n, P9P_TOPEN, tag);

    return n;

}

unsigned int p9p_mkropen(void *buffer, unsigned short tag, char *qid[13], unsigned int iounit)
{

    unsigned char *x = buffer;
    unsigned int n = sizeof (struct event_p9p);

    n += 13; /* qid */
    n += p9p_write4(x + n, iounit);

    p9p_mk(buffer, n, P9P_ROPEN, tag);

    return n;

}

unsigned int p9p_mktcreate(void *buffer, unsigned short tag, unsigned int fid, char *name, unsigned int perm, unsigned char mode)
{

    unsigned char *x = buffer;
    unsigned int n = sizeof (struct event_p9p);

    n += p9p_write4(x + n, fid);
    n += p9p_writestring(x + n, name);
    n += p9p_write4(x + n, perm);
    n += p9p_write1(x + n, mode);

    p9p_mk(buffer, n, P9P_TCREATE, tag);

    return n;

}

unsigned int p9p_mkrcreate(void *buffer, unsigned short tag, char *qid[13], unsigned int iounit)
{

    unsigned char *x = buffer;
    unsigned int n = sizeof (struct event_p9p);

    n += 13; /* qid */
    n += p9p_write4(x + n, iounit);

    p9p_mk(buffer, n, P9P_RCREATE, tag);

    return n;

}

unsigned int p9p_mktremove(void *buffer, unsigned short tag, unsigned int fid)
{

    unsigned char *x = buffer;
    unsigned int n = sizeof (struct event_p9p);

    n += p9p_write4(x + n, fid);

    p9p_mk(buffer, n, P9P_TREMOVE, tag);

    return n;

}

unsigned int p9p_mkrremove(void *buffer, unsigned short tag)
{

    unsigned int n = sizeof (struct event_p9p);

    p9p_mk(buffer, n, P9P_RREMOVE, tag);

    return n;

}

unsigned int p9p_mktstat(void *buffer, unsigned short tag, unsigned int fid)
{

    unsigned char *x = buffer;
    unsigned int n = sizeof (struct event_p9p);

    n += p9p_write4(x + n, fid);

    p9p_mk(buffer, n, P9P_TSTAT, tag);

    return n;

}

unsigned int p9p_mkrstat(void *buffer, unsigned short tag, void *stat)
{

    unsigned int n = sizeof (struct event_p9p);

    p9p_mk(buffer, n, P9P_RSTAT, tag);

    return n;

}

unsigned int p9p_mktwstat(void *buffer, unsigned short tag, unsigned int fid, void *stat)
{

    unsigned char *x = buffer;
    unsigned int n = sizeof (struct event_p9p);

    n += p9p_write4(x + n, fid);

    p9p_mk(buffer, n, P9P_TWSTAT, tag);

    return n;

}

unsigned int p9p_mkrwstat(void *buffer, unsigned short tag)
{

    unsigned int n = sizeof (struct event_p9p);

    p9p_mk(buffer, n, P9P_RWSTAT, tag);

    return n;

}

