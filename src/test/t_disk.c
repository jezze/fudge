#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

#define MAXSIZE 1200
#define BLOCKSIZE 512
#define OK 1
#define ERROR 0

struct state
{

    unsigned int offset;
    unsigned int count;
    unsigned int blocksector;
    unsigned int blockoffset;
    unsigned int blockcount;
    unsigned int blockreads;

} states[256];

static struct socket local;
static struct socket remote;
static struct socket router;
static char blockdata[BLOCKSIZE * 4];
static unsigned int version = P9P_VERSION_UNKNOWN;

static struct state *request_getstate(unsigned int fid)
{

    return &states[fid];

}

static void request_init(struct state *state, unsigned int offset, unsigned int count)
{

    state->offset = offset;
    state->count = count;
    state->blocksector = offset / BLOCKSIZE;
    state->blockoffset = offset % BLOCKSIZE;
    state->blockcount = ((BLOCKSIZE - 1) + state->blockoffset + state->count) / BLOCKSIZE;
    state->blockreads = 0;

}

static void request_send(struct state *state)
{

    struct event_blockrequest blockrequest;

    blockrequest.sector = state->blocksector;
    blockrequest.count = state->blockcount;

    call_notify(FILE_G5, EVENT_BLOCKREQUEST, sizeof (struct event_blockrequest), &blockrequest);

}

static unsigned int request_poll(struct state *state)
{

    struct message message;
    char data[MESSAGE_SIZE];

    while (channel_poll_any(EVENT_DATA, &message, data))
    {

        state->blockreads += buffer_write(blockdata, BLOCKSIZE * 4, data, message_datasize(&message), state->blockreads * BLOCKSIZE) / BLOCKSIZE;

        if (state->blockreads == state->blockcount)
            return state->count;

    }

    return 0;

}

static unsigned int request_sendpoll(struct state *state, unsigned int offset, unsigned int count)
{

    request_init(state, offset, count);
    request_send(state);

    return request_poll(state);

}

static void *request_getdata(struct state *state)
{

    return ((char *)blockdata + state->blockoffset);

}

static unsigned int request_walk(struct state *state, unsigned int length, char *path)
{

    unsigned int status = ERROR;
    unsigned int offset = 0;

    call_link(FILE_G5, 8000);

    while (request_sendpoll(state, offset, sizeof (struct cpio_header) + 1024))
    {

        struct cpio_header *header = request_getdata(state);

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
            {

                status = OK;

                break;

            }

        }

        offset = state->offset + cpio_next(header);

    }

    call_unlink(FILE_G5);

    return status;

}

static unsigned int request_read(struct state *state)
{

    unsigned int status = ERROR;

    call_link(FILE_G5, 8001);

    if (request_sendpoll(state, state->offset, sizeof (struct cpio_header) + 1024))
    {

        struct cpio_header *header = request_getdata(state);

        if (cpio_validate(header))
        {

            if (request_sendpoll(state, state->offset + cpio_filedata(header), cpio_filesize(header)))
            {

                status = OK;

            }

        }

    }

    call_unlink(FILE_G5);

    return status;

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

    return protocol_error(buffer, p9p, "Protocol not supported", -1);

}

static unsigned int protocol_auth(void *buffer, struct p9p_header *p9p)
{

    return protocol_error(buffer, p9p, "Authentication not required", -1);

}

static unsigned int protocol_attach(void *buffer, struct p9p_header *p9p)
{

    unsigned int tag = p9p_read4(p9p, P9P_OFFSET_TAG);
    unsigned int fid = p9p_read4(p9p, P9P_OFFSET_DATA);
    struct state temp;

    buffer_copy(&temp, request_getstate(fid), sizeof (struct state));

    if (request_walk(&temp, 5, "build") == OK)
    {

        char qid[13];

        p9p_write1(qid, 0, 0);
        p9p_write4(qid, 1, 0);
        p9p_write8(qid, 5, temp.offset, 0);
        buffer_copy(request_getstate(fid), &temp, sizeof (struct state));

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
    unsigned int newfid = p9p_read4(p9p, P9P_OFFSET_DATA + 4);
    struct state temp;

    buffer_copy(&temp, request_getstate(fid), sizeof (struct state));

    if (request_walk(&temp, p9p_readstringlength(p9p, P9P_OFFSET_DATA + 10), p9p_readstringdata(p9p, P9P_OFFSET_DATA + 10)) == OK)
    {

        buffer_copy(request_getstate(newfid), &temp, sizeof (struct state));

        return p9p_mkrwalk(buffer, tag, 0, 0);

    }

    return protocol_error(buffer, p9p, "File not found", -1);

}

static unsigned int protocol_read(void *buffer, struct p9p_header *p9p)
{

    unsigned int tag = p9p_read4(p9p, P9P_OFFSET_TAG);
    unsigned int fid = p9p_read4(p9p, P9P_OFFSET_DATA);
    struct state *state = request_getstate(fid);

    if (request_read(state) == OK)
        return p9p_mkrread(buffer, tag, state->count, request_getdata(state));

    return protocol_error(buffer, p9p, "Read error", -1);

}

static unsigned int protocol_getattr(void *buffer, struct p9p_header *p9p)
{

    unsigned int tag = p9p_read4(p9p, P9P_OFFSET_TAG);
    unsigned int fid = p9p_read4(p9p, P9P_OFFSET_DATA);
    struct state *state = request_getstate(fid);
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
    p9p_write1(qid, 0, 0);
    p9p_write4(qid, 1, 0);
    p9p_write8(qid, 5, state->offset, 0);

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
        channel_send_fmt0(CHANNEL_DEFAULT, EVENT_ERROR, "Packet has unknown type\n");

        return protocol_error(reply, p9p, "Packet has unknown type", -1);

    }

}

static void onp9p(unsigned int source, void *mdata, unsigned int msize)
{

    struct p9p_header *p9p = mdata;
    char buffer[MESSAGE_SIZE];

    channel_send_buffer(source, EVENT_P9P, handle(buffer, p9p), buffer);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    call_link(FILE_G4, 8002);

    if (call_walk_relative(FILE_L0, FILE_G0, "addr"))
        socket_resolvelocal(FILE_L0, &local);

    if (call_walk_relative(FILE_G1, FILE_G0, "data"))
    {

        char buffer[4096];
        unsigned int count;

        call_link(FILE_G1, 8003);
        socket_resolveremote(FILE_G1, &local, &router);
        socket_listen_tcp(FILE_G1, &local, &remote, 1, &router);

        while ((count = socket_receive(FILE_G1, &local, &remote, 1, &router, buffer, 4096)))
        {

            char reply[MESSAGE_SIZE];

            socket_send_tcp(FILE_G1, &local, &remote, &router, handle(reply, (struct p9p_header *)buffer), reply);

        }

        call_unlink(FILE_G1);

    }

    call_unlink(FILE_G4);

}

void init(void)
{

    call_walk_absolute(FILE_G4, "system:service/fd0");
    call_walk_absolute(FILE_G5, "system:block/if.0/data");
    call_walk_absolute(FILE_G0, "system:ethernet/if.0");
    socket_init(&local);
    socket_bind_ipv4s(&local, "10.0.5.1");
    socket_bind_tcps(&local, "564", 42, 42);
    socket_init(&router);
    socket_bind_ipv4s(&router, "10.0.5.80");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_P9P, onp9p);

}

