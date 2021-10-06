#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

#define MAXSIZE 1200
#define BLOCKSIZE 512
#define OK 1
#define ERROR 0

struct request
{

    unsigned int offset;
    unsigned int count;
    unsigned int blocksector;
    unsigned int blockoffset;
    unsigned int blockcount;
    unsigned int blockreads;

} requests[256];

static struct socket local;
static struct socket remote;
static struct socket router;
static char blocks[BLOCKSIZE * 4];
static unsigned int version = P9P_VERSION_UNKNOWN;

static struct request *request_get(unsigned int fid)
{

    return &requests[fid];

}

static void request_init(struct request *request, unsigned int offset, unsigned int count)
{

    request->offset = offset;
    request->count = count;
    request->blocksector = offset / BLOCKSIZE;
    request->blockoffset = offset % BLOCKSIZE;
    request->blockcount = ((BLOCKSIZE - 1) + request->blockoffset + request->count) / BLOCKSIZE;
    request->blockreads = 0;

}

static void request_send(struct request *request)
{

    struct {struct message_header header; struct event_blockrequest blockrequest;} message;

    message.blockrequest.sector = request->blocksector;
    message.blockrequest.count = request->blockcount;

    message_initheader(&message.header, EVENT_BLOCKREQUEST, sizeof (struct event_blockrequest));
    file_writeall(FILE_G5, &message, message.header.length);

}

static unsigned int request_poll(struct request *request)
{

    struct message message;

    while (channel_polleventsystem(EVENT_DATA, &message))
    {

        request->blockreads += buffer_write(blocks, BLOCKSIZE * 4, message.data.buffer, message_datasize(&message.header), request->blockreads * BLOCKSIZE) / BLOCKSIZE;

        if (request->blockreads == request->blockcount)
            return request->count;

    }

    return 0;

}

static unsigned int request_sendpoll(struct request *request, unsigned int offset, unsigned int count)
{

    request_init(request, offset, count);
    request_send(request);

    return request_poll(request);

}

static void *request_getdata(struct request *request)
{

    return ((char *)blocks + request->blockoffset);

}

static unsigned int request_walk(struct request *request, unsigned int length, char *path)
{

    unsigned int offset = 0;

    while (request_sendpoll(request, offset, sizeof (struct cpio_header) + 1024))
    {

        struct cpio_header *header = request_getdata(request);

        if (!cpio_validate(header))
            break;

        if (header->namesize == 11)
        {

            if (buffer_match("TRAILER!!!", header + 1, 11))
                break;

        }

        if (header->namesize == length + 1)
        {

            if (buffer_match(path, header + 1, length))
                return OK;

        }

        offset = request->offset + cpio_next(header);

    }

    return ERROR;

}

static unsigned int protocol_error(void *buffer, struct p9p_header *p9p, char *ename, int ecode)
{

    unsigned int tag = p9p_read4(p9p, P9P_OFFSET_TAG);

    if (version == P9P_VERSION_9P2000L)
        return p9p_mkrlerror(buffer, tag, ecode);

    return p9p_mkrerror(buffer, tag, ename);

}

static unsigned int protocol_version(void *buffer, struct p9p_header *p9p)
{

    unsigned int tag = p9p_read4(p9p, P9P_OFFSET_TAG);
    unsigned int msize = p9p_read4(p9p, P9P_OFFSET_DATA);
    unsigned int vlength = p9p_readstringlength(p9p, P9P_OFFSET_DATA + 4);
    char *vdata = p9p_readstringdata(p9p, P9P_OFFSET_DATA + 4);

    if (vlength == 6 && buffer_match(vdata, "9P2000", vlength))
    {

        version = P9P_VERSION_9P2000;

        return p9p_mkrversion(buffer, tag, msize, "9P2000");

    }

    if (vlength == 8 && buffer_match(vdata, "9P2000.u", vlength))
    {

        version = P9P_VERSION_9P2000U;

        return p9p_mkrversion(buffer, tag, msize, "9P2000.u");

    }

    if (vlength == 8 && buffer_match(vdata, "9P2000.L", vlength))
    {

        version = P9P_VERSION_9P2000L;

        return p9p_mkrversion(buffer, tag, msize, "9P2000.L");

    }

    if (vlength == 8 && buffer_match(vdata, "9P2000.F", vlength))
    {

        version = P9P_VERSION_9P2000F;

        return p9p_mkrversion(buffer, tag, msize, "9P2000.F");

    }

    return p9p_mkrerror(buffer, tag, "Protocol not supported");

}

static unsigned int protocol_auth(void *buffer, struct p9p_header *p9p)
{

    return protocol_error(buffer, p9p, "Authentication not required", -1);

}

static unsigned int protocol_attach(void *buffer, struct p9p_header *p9p)
{

    unsigned int tag = p9p_read4(p9p, P9P_OFFSET_TAG);
    unsigned int fid = p9p_read4(p9p, P9P_OFFSET_DATA);
    struct request *request = request_get(fid);
    unsigned int status;

    file_link(FILE_G5);

    status = request_walk(request, 5, "build");

    file_unlink(FILE_G5);

    if (status == OK)
    {

        char qid[13];

        p9p_write1(qid, 0, 0);
        p9p_write4(qid, 1, 0);
        p9p_write8(qid, 5, request->offset, 0);

        return p9p_mkrattach(buffer, tag, qid);

    }

    return protocol_error(buffer, p9p, "Could not find root directory", -1);

}

static unsigned int protocol_clunk(void *buffer, struct p9p_header *p9p)
{

    unsigned int tag = p9p_read4(p9p, P9P_OFFSET_TAG);

    return p9p_mkrclunk(buffer, tag);

}

static unsigned int protocol_walk(void *buffer, struct p9p_header *p9p)
{

    unsigned int tag = p9p_read4(p9p, P9P_OFFSET_TAG);
    unsigned int fid = p9p_read4(p9p, P9P_OFFSET_DATA);
    unsigned int newfid = p9p_read4(p9p, P9P_OFFSET_DATA);
    struct request temp;
    unsigned int status;

    buffer_copy(&temp, request_get(fid), sizeof (struct request));

    file_link(FILE_G5);

    status = request_walk(&temp, p9p_readstringlength(p9p, P9P_OFFSET_DATA + 10), p9p_readstringdata(p9p, P9P_OFFSET_DATA + 10));

    file_unlink(FILE_G5);

    if (status == OK)
    {

        buffer_copy(request_get(newfid), &temp, sizeof (struct request));

        return p9p_mkrwalk(buffer, tag, 0, 0);

    }

    return protocol_error(buffer, p9p, "File not found", -1);

}

static unsigned int protocol_read(void *buffer, struct p9p_header *p9p)
{

    unsigned int tag = p9p_read4(p9p, P9P_OFFSET_TAG);
    unsigned int fid = p9p_read4(p9p, P9P_OFFSET_DATA);
    struct request *request = request_get(fid);
    unsigned int rc = 0;

    file_link(FILE_G5);

    if (request_sendpoll(request, request->offset, sizeof (struct cpio_header) + 1024))
    {

        struct cpio_header *header = request_getdata(request);

        if (cpio_validate(header))
        {

            unsigned int count;

            if ((count = request_sendpoll(request, request->offset + cpio_filedata(header), cpio_filesize(header))))
            {

                rc = p9p_mkrread(buffer, tag, count, request_getdata(request));

            }

        }

    }

    file_unlink(FILE_G5);

    return rc;

}

static unsigned int protocol_getattr(void *buffer, struct p9p_header *p9p)
{

    unsigned int tag = p9p_read4(p9p, P9P_OFFSET_TAG);
    char valid[8];
    char qid[13];
    unsigned int mode = 0;
    unsigned int uid = 0;
    unsigned int gid = 0;
    char nlink[8];
    char rdev[8];
    char size[8];
    char blksize[8];
    char blocks[8];
    char atimesec[8];
    char atimensec[8];
    char mtimesec[8];
    char mtimensec[8];
    char ctimesec[8];
    char ctimensec[8];
    char btimesec[8];
    char btimensec[8];
    char gen[8];
    char dataversion[8];

    buffer_clear(valid, 8);

    return p9p_mkrgetattr(buffer, tag, valid, qid, mode, uid, gid, nlink, rdev, size, blksize, blocks, atimesec, atimensec, mtimesec, mtimensec, ctimesec, ctimensec, btimesec, btimensec, gen, dataversion);

}

static unsigned int handle(void *reply, struct p9p_header *p9p)
{

    if (p9p_read4(p9p, P9P_OFFSET_SIZE) < P9P_OFFSET_DATA)
        return protocol_error(reply, p9p, "Error: Packet is too small\n", -1);

    switch (p9p_read1(p9p, P9P_OFFSET_TYPE))
    {

    case P9P_TVERSION:
        return protocol_version(reply, p9p);

    case P9P_TAUTH:
        return protocol_auth(reply, p9p);

    case P9P_TATTACH:
        return protocol_attach(reply, p9p);

    case P9P_TCLUNK:
        return protocol_clunk(reply, p9p);

    case P9P_TWALK:
        return protocol_walk(reply, p9p);

    case P9P_TREAD:
        return protocol_read(reply, p9p);

    case P9P_TGETATTR:
        return protocol_getattr(reply, p9p);

    default:
        channel_sendstring(EVENT_DATA, "Error: Packet has unknown type\n");

        return protocol_error(reply, p9p, "Packet has unknown type", -1);

    }

}

static void onp9p(unsigned int source, void *mdata, unsigned int msize)
{

    struct p9p_header *p9p = mdata;
    char buffer[MESSAGE_SIZE];

    channel_sendbufferto(source, EVENT_P9P, handle(buffer, p9p), buffer);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    char buffer[BUFFER_SIZE];
    unsigned int count;

    file_link(FILE_G4);

    if (file_walk(FILE_L0, FILE_G0, "addr"))
        socket_resolvelocal(FILE_L0, &local);

    if (file_walk(FILE_G1, FILE_G0, "data"))
    {

        channel_sendstring(EVENT_DATA, "Listening\n");
        file_link(FILE_G1);
        socket_resolveremote(FILE_G1, &local, &router);
        socket_listen_tcp(FILE_G1, &local, &remote, &router);
        channel_sendstring(EVENT_DATA, "Established\n");

        while ((count = socket_receive_tcp(FILE_G1, &local, &remote, &router, buffer, BUFFER_SIZE)))
        {

            struct p9p_header *p9p = (struct p9p_header *)buffer;
            char reply[MESSAGE_SIZE];

            channel_sendstring(EVENT_DATA, "Packet received\n");
            socket_send_tcp(FILE_G1, &local, &remote, &router, handle(reply, p9p), reply);

        }

        file_unlink(FILE_G1);

    }

    file_unlink(FILE_G4);

}

void init(void)
{

    if (!file_walk2(FILE_G4, "system:service/fd0"))
        return;

    if (!file_walk2(FILE_G5, "system:block/if:0/data"))
        return;

    file_walk2(FILE_G0, "system:ethernet/if:0");
    socket_init(&local);
    socket_bind_ipv4s(&local, "10.0.5.1");
    socket_bind_tcps(&local, "564", 42);
    socket_init(&router);
    socket_bind_ipv4s(&router, "10.0.5.80");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_P9P, onp9p);

}

