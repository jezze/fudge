#include "event.h"
#include "buffer.h"
#include "file.h"
#include "cstring.h"
#include "message.h"
#include "p9p.h"

unsigned char p9p_read1(void *buffer, unsigned int o)
{

    unsigned char *p = buffer;

    p += o;

    return (p[0] << 0);

}

unsigned short p9p_read2(void *buffer, unsigned int o)
{

    unsigned char *p = buffer;

    p += o;

    return (p[1] << 8 | p[0] << 0);

}

unsigned int p9p_read4(void *buffer, unsigned int o)
{

    unsigned char *p = buffer;

    p += o;

    return (p[3] << 24 | p[2] << 16 | p[1] << 8 | p[0] << 0);

}

unsigned short p9p_readstringlength(void *buffer, unsigned int o)
{

    return p9p_read2(buffer, o);

}

void *p9p_readbuffer(void *buffer, unsigned int o)
{

    char *p = buffer;

    return p + o;

}

char *p9p_readstringdata(void *buffer, unsigned int o)
{

    char *p = buffer;

    return p + o + 2;

}

unsigned int p9p_write1(void *buffer, unsigned int o, unsigned char v)
{

    unsigned char *p = buffer;

    p += o;
    p[0] = v >> 0;

    return 1;

}

unsigned int p9p_write2(void *buffer, unsigned int o, unsigned short v)
{

    unsigned char *p = buffer;

    p += o;
    p[0] = v >> 0;
    p[1] = v >> 8;

    return 2;

}

unsigned int p9p_write4(void *buffer, unsigned int o, unsigned int v)
{

    unsigned char *p = buffer;

    p += o;
    p[0] = v >> 0;
    p[1] = v >> 8;
    p[2] = v >> 16;
    p[3] = v >> 24;

    return 4;

}

unsigned int p9p_write8(void *buffer, unsigned int o, unsigned int vl, unsigned int vh)
{

    unsigned char *p = buffer;

    p += o;
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

unsigned int p9p_writebuffer(void *buffer, unsigned int o, unsigned int count, void *v)
{

    unsigned char *p = buffer;

    p += o;

    buffer_copy(p, v, count);

    return count;

}

unsigned int p9p_writestring(void *buffer, unsigned int o, char *v)
{

    unsigned short count = cstring_length(v);
    unsigned char *p = buffer;
    unsigned int n = 0;

    p += o;
    n += p9p_write2(p, n, count);
    n += p9p_writebuffer(p, n, count, v);

    return n;

}

unsigned int p9p_mk(void *buffer, unsigned int size, unsigned char type, unsigned short tag)
{

    unsigned int n = 0;

    n += p9p_write4(buffer, n, size);
    n += p9p_write1(buffer, n, type);
    n += p9p_write2(buffer, n, tag);

    return n;

}

unsigned int p9p_mkrlerror(void *buffer, unsigned short tag, int ecode)
{

    unsigned int n = sizeof (struct p9p_header);

    n += p9p_write4(buffer, n, ecode);

    p9p_mk(buffer, n, P9P_RLERROR, tag);

    return n;

}

unsigned int p9p_mkrerror(void *buffer, unsigned short tag, char *ename)
{

    unsigned int n = sizeof (struct p9p_header);

    n += p9p_writestring(buffer, n, ename);

    p9p_mk(buffer, n, P9P_RERROR, tag);

    return n;

}

unsigned int p9p_mktattach(void *buffer, unsigned short tag, unsigned int fid, unsigned int afid, char *uname, char *aname)
{

    unsigned int n = sizeof (struct p9p_header);

    n += p9p_write4(buffer, n, fid);
    n += p9p_write4(buffer, n, afid);
    n += p9p_writestring(buffer, n, uname);
    n += p9p_writestring(buffer, n, aname);

    p9p_mk(buffer, n, P9P_TATTACH, tag);

    return n;

}

unsigned int p9p_mktattach_u(void *buffer, unsigned short tag, unsigned int fid, unsigned int afid, char *uname, char *aname, unsigned int nuname)
{

    unsigned int n = sizeof (struct p9p_header);

    n += p9p_write4(buffer, n, fid);
    n += p9p_write4(buffer, n, afid);
    n += p9p_writestring(buffer, n, uname);
    n += p9p_writestring(buffer, n, aname);
    n += p9p_write4(buffer, n, nuname);

    p9p_mk(buffer, n, P9P_TATTACH, tag);

    return n;

}

unsigned int p9p_mkrattach(void *buffer, unsigned short tag, char qid[13])
{

    unsigned int n = sizeof (struct p9p_header);

    n += p9p_writebuffer(buffer, n, 13, qid);

    p9p_mk(buffer, n, P9P_RATTACH, tag);

    return n;

}

unsigned int p9p_mktauth(void *buffer, unsigned short tag, unsigned int afid, char *uname, char *aname)
{

    unsigned int n = sizeof (struct p9p_header);

    n += p9p_write4(buffer, n, afid);
    n += p9p_writestring(buffer, n, uname);
    n += p9p_writestring(buffer, n, aname);

    p9p_mk(buffer, n, P9P_TAUTH, tag);

    return n;

}

unsigned int p9p_mktauth_u(void *buffer, unsigned short tag, unsigned int afid, char *uname, char *aname, unsigned int nuname)
{

    unsigned int n = sizeof (struct p9p_header);

    n += p9p_write4(buffer, n, afid);
    n += p9p_writestring(buffer, n, uname);
    n += p9p_writestring(buffer, n, aname);
    n += p9p_write4(buffer, n, nuname);

    p9p_mk(buffer, n, P9P_TAUTH, tag);

    return n;

}

unsigned int p9p_mkrauth(void *buffer, unsigned short tag, char aqid[13])
{

    unsigned int n = sizeof (struct p9p_header);

    n += p9p_writebuffer(buffer, n, 13, aqid);

    p9p_mk(buffer, n, P9P_RAUTH, tag);

    return n;

}

unsigned int p9p_mktclunk(void *buffer, unsigned short tag, unsigned int fid)
{

    unsigned int n = sizeof (struct p9p_header);

    n += p9p_write4(buffer, n, fid);

    p9p_mk(buffer, n, P9P_TCLUNK, tag);

    return n;

}

unsigned int p9p_mkrclunk(void *buffer, unsigned short tag)
{

    unsigned int n = sizeof (struct p9p_header);

    p9p_mk(buffer, n, P9P_RCLUNK, tag);

    return n;

}

unsigned int p9p_mktflush(void *buffer, unsigned short tag, unsigned short oldtag)
{

    unsigned int n = sizeof (struct p9p_header);

    n += p9p_write2(buffer, n, oldtag);

    p9p_mk(buffer, n, P9P_TFLUSH, tag);

    return n;

}

unsigned int p9p_mkrflush(void *buffer, unsigned short tag)
{

    unsigned int n = sizeof (struct p9p_header);

    p9p_mk(buffer, n, P9P_RFLUSH, tag);

    return n;

}

unsigned int p9p_mktversion(void *buffer, unsigned short tag, unsigned int msize, char *version)
{

    unsigned int n = sizeof (struct p9p_header);

    n += p9p_write4(buffer, n, msize);
    n += p9p_writestring(buffer, n, version);

    p9p_mk(buffer, n, P9P_TVERSION, tag);

    return n;

}

unsigned int p9p_mkrversion(void *buffer, unsigned short tag, unsigned int msize, char *version)
{

    unsigned int n = sizeof (struct p9p_header);

    n += p9p_write4(buffer, n, msize);
    n += p9p_writestring(buffer, n, version);

    p9p_mk(buffer, n, P9P_RVERSION, tag);

    return n;

}

unsigned int p9p_mktwalk(void *buffer, unsigned short tag, unsigned int fid, unsigned int newfid, unsigned short nwname, char **wname)
{

    unsigned int n = sizeof (struct p9p_header);
    unsigned int i;

    n += p9p_write4(buffer, n, fid);
    n += p9p_write4(buffer, n, newfid);
    n += p9p_write2(buffer, n, nwname);

    for (i = 0; i < nwname; i++)
        n += p9p_writestring(buffer, n, wname[i]);

    p9p_mk(buffer, n, P9P_TWALK, tag);

    return n;

}

unsigned int p9p_mkrwalk(void *buffer, unsigned short tag, unsigned short nwqid, char *wqid[13])
{

    unsigned int n = sizeof (struct p9p_header);
    unsigned int i;

    n += p9p_write2(buffer, n, nwqid);

    for (i = 0; i < nwqid; i++)
        n += p9p_writebuffer(buffer, n, 13, wqid[i]);

    p9p_mk(buffer, n, P9P_RWALK, tag);

    return n;

}

unsigned int p9p_mktread(void *buffer, unsigned short tag, unsigned int fid, unsigned int offsetl, unsigned int offseth, unsigned int count)
{

    unsigned int n = sizeof (struct p9p_header);

    n += p9p_write4(buffer, n, fid);
    n += p9p_write8(buffer, n, offsetl, offseth);
    n += p9p_write4(buffer, n, count);

    p9p_mk(buffer, n, P9P_TREAD, tag);

    return n;

}

unsigned int p9p_mkrread(void *buffer, unsigned short tag, unsigned int count, void *data)
{

    unsigned int n = sizeof (struct p9p_header);

    n += p9p_write4(buffer, n, count);
    n += p9p_writebuffer(buffer, n, count, data);

    p9p_mk(buffer, n, P9P_RREAD, tag);

    return n;

}

unsigned int p9p_mktwrite(void *buffer, unsigned short tag, unsigned int fid, unsigned int offsetl, unsigned int offseth, unsigned int count, void *data)
{

    unsigned int n = sizeof (struct p9p_header);

    n += p9p_write4(buffer, n, fid);
    n += p9p_write8(buffer, n, offsetl, offseth);
    n += p9p_write4(buffer, n, count);
    n += p9p_writebuffer(buffer, n, count, data);

    p9p_mk(buffer, n, P9P_TWRITE, tag);

    return n;

}

unsigned int p9p_mkrwrite(void *buffer, unsigned short tag, unsigned int count)
{

    unsigned int n = sizeof (struct p9p_header);

    n += p9p_write4(buffer, n, count);

    p9p_mk(buffer, n, P9P_RWRITE, tag);

    return n;

}

unsigned int p9p_mktopen(void *buffer, unsigned short tag, unsigned int fid, unsigned char mode)
{

    unsigned int n = sizeof (struct p9p_header);

    n += p9p_write4(buffer, n, fid);
    n += p9p_write1(buffer, n, mode);

    p9p_mk(buffer, n, P9P_TOPEN, tag);

    return n;

}

unsigned int p9p_mkropen(void *buffer, unsigned short tag, char qid[13], unsigned int iounit)
{

    unsigned int n = sizeof (struct p9p_header);

    n += p9p_writebuffer(buffer, n, 13, qid);
    n += p9p_write4(buffer, n, iounit);

    p9p_mk(buffer, n, P9P_ROPEN, tag);

    return n;

}

unsigned int p9p_mktcreate(void *buffer, unsigned short tag, unsigned int fid, char *name, unsigned int perm, unsigned char mode)
{

    unsigned int n = sizeof (struct p9p_header);

    n += p9p_write4(buffer, n, fid);
    n += p9p_writestring(buffer, n, name);
    n += p9p_write4(buffer, n, perm);
    n += p9p_write1(buffer, n, mode);

    p9p_mk(buffer, n, P9P_TCREATE, tag);

    return n;

}

unsigned int p9p_mkrcreate(void *buffer, unsigned short tag, char qid[13], unsigned int iounit)
{

    unsigned int n = sizeof (struct p9p_header);

    n += p9p_writebuffer(buffer, n, 13, qid);
    n += p9p_write4(buffer, n, iounit);

    p9p_mk(buffer, n, P9P_RCREATE, tag);

    return n;

}

unsigned int p9p_mktremove(void *buffer, unsigned short tag, unsigned int fid)
{

    unsigned int n = sizeof (struct p9p_header);

    n += p9p_write4(buffer, n, fid);

    p9p_mk(buffer, n, P9P_TREMOVE, tag);

    return n;

}

unsigned int p9p_mkrremove(void *buffer, unsigned short tag)
{

    unsigned int n = sizeof (struct p9p_header);

    p9p_mk(buffer, n, P9P_RREMOVE, tag);

    return n;

}

unsigned int p9p_mktstat(void *buffer, unsigned short tag, unsigned int fid)
{

    unsigned int n = sizeof (struct p9p_header);

    n += p9p_write4(buffer, n, fid);

    p9p_mk(buffer, n, P9P_TSTAT, tag);

    return n;

}

unsigned int p9p_mkrstat(void *buffer, unsigned short tag, void *stat)
{

    unsigned int n = sizeof (struct p9p_header);

    p9p_mk(buffer, n, P9P_RSTAT, tag);

    return n;

}

unsigned int p9p_mktwstat(void *buffer, unsigned short tag, unsigned int fid, void *stat)
{

    unsigned int n = sizeof (struct p9p_header);

    n += p9p_write4(buffer, n, fid);

    p9p_mk(buffer, n, P9P_TWSTAT, tag);

    return n;

}

unsigned int p9p_mkrwstat(void *buffer, unsigned short tag)
{

    unsigned int n = sizeof (struct p9p_header);

    p9p_mk(buffer, n, P9P_RWSTAT, tag);

    return n;

}

unsigned int p9p_mktgetattr(void *buffer, unsigned short tag, unsigned int fid, char mask[8])
{

    unsigned int n = sizeof (struct p9p_header);

    n += p9p_write4(buffer, n, fid);
    n += p9p_writebuffer(buffer, n, 8, mask);

    p9p_mk(buffer, n, P9P_TGETATTR, tag);

    return n;

}

unsigned int p9p_mkrgetattr(void *buffer, unsigned short tag, char valid[8], char qid[13], unsigned int mode, unsigned int uid, unsigned int gid, char nlink[8], char rdev[8], char size[8], char blksize[8], char blocks[8], char atimesec[8], char atimensec[8], char mtimesec[8], char mtimensec[8], char ctimesec[8], char ctimensec[8], char btimesec[8], char btimensec[8], char gen[8], char dataversion[8])
{

    unsigned int n = sizeof (struct p9p_header);

    n += p9p_writebuffer(buffer, n, 8, valid);
    n += p9p_writebuffer(buffer, n, 13, qid);
    n += p9p_write4(buffer, n, mode);
    n += p9p_write4(buffer, n, uid);
    n += p9p_write4(buffer, n, gid);
    n += p9p_writebuffer(buffer, n, 8, nlink);
    n += p9p_writebuffer(buffer, n, 8, rdev);
    n += p9p_writebuffer(buffer, n, 8, size);
    n += p9p_writebuffer(buffer, n, 8, blksize);
    n += p9p_writebuffer(buffer, n, 8, blocks);
    n += p9p_writebuffer(buffer, n, 8, atimesec);
    n += p9p_writebuffer(buffer, n, 8, atimensec);
    n += p9p_writebuffer(buffer, n, 8, mtimesec);
    n += p9p_writebuffer(buffer, n, 8, mtimensec);
    n += p9p_writebuffer(buffer, n, 8, ctimesec);
    n += p9p_writebuffer(buffer, n, 8, ctimensec);
    n += p9p_writebuffer(buffer, n, 8, btimesec);
    n += p9p_writebuffer(buffer, n, 8, btimensec);
    n += p9p_writebuffer(buffer, n, 8, gen);
    n += p9p_writebuffer(buffer, n, 8, dataversion);

    p9p_mk(buffer, n, P9P_RGETATTR, tag);

    return n;

}

