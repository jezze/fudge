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

static unsigned int sendpoll(struct request *request, unsigned int offset, unsigned int count)
{

    request_init(request, offset, count);
    request_send(request);

    return request_poll(request);

}

static void *getdata(struct request *request)
{

    return ((char *)blocks + request->blockoffset);

}

static unsigned int walk(struct request *request, unsigned int length, char *path)
{

    unsigned int offset = 0;

    while (sendpoll(request, offset, sizeof (struct cpio_header) + 1024))
    {

        struct cpio_header *header = getdata(request);

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

static unsigned int on9perror(void *buffer, struct p9p_event *p9p, char *error)
{

    unsigned int tag = p9p_read4(p9p, P9P_OFFSET_TAG);

    return p9p_mkrerror(buffer, tag, error);

}

static unsigned int on9pversion(void *buffer, struct p9p_event *p9p)
{

    unsigned int tag = p9p_read4(p9p, P9P_OFFSET_TAG);
    unsigned int msize = p9p_read4(p9p, P9P_OFFSET_DATA);

    return p9p_mkrversion(buffer, tag, msize, "9P2000.L");

}

static unsigned int on9pattach(void *buffer, struct p9p_event *p9p)
{

    unsigned int tag = p9p_read4(p9p, P9P_OFFSET_TAG);
    char qid[13];

    p9p_write1(qid, 0, 0);
    p9p_write4(qid, 1, 0);
    p9p_write8(qid, 5, 101010, 0);

    return p9p_mkrattach(buffer, tag, qid);

}

static unsigned int on9pclunk(void *buffer, struct p9p_event *p9p)
{

    unsigned int tag = p9p_read4(p9p, P9P_OFFSET_TAG);

    return p9p_mkrclunk(buffer, tag);

}

static unsigned int on9pwalk(void *buffer, struct p9p_event *p9p)
{

    unsigned int tag = p9p_read4(p9p, P9P_OFFSET_TAG);
    unsigned int fid = p9p_read4(p9p, P9P_OFFSET_DATA);
    unsigned int newfid = p9p_read4(p9p, P9P_OFFSET_DATA);
    struct request *request = request_get(fid);
    struct request *request2 = request_get(newfid);
    unsigned int status;

    file_link(FILE_G5);

    status = walk(request, p9p_read2(p9p, P9P_OFFSET_DATA + 10), (char *)p9p + P9P_OFFSET_DATA + 12);

    file_unlink(FILE_G5);

    if (status == OK)
    {

        if (fid != newfid)
            buffer_copy(request2, request, sizeof (struct request));

        return p9p_mkrwalk(buffer, tag, 0, 0);

    }

    return on9perror(buffer, p9p, "File not found");

}

static unsigned int on9pread(void *buffer, struct p9p_event *p9p)
{

    unsigned int tag = p9p_read4(p9p, P9P_OFFSET_TAG);
    unsigned int fid = p9p_read4(p9p, P9P_OFFSET_DATA);
    struct request *request = request_get(fid);
    unsigned int rc = 0;

    file_link(FILE_G5);

    if (sendpoll(request, request->offset, sizeof (struct cpio_header) + 1024))
    {

        struct cpio_header *header = getdata(request);

        if (cpio_validate(header))
        {

            unsigned int count;

            if ((count = sendpoll(request, request->offset + cpio_filedata(header), cpio_filesize(header))))
            {

                rc = p9p_mkrread(buffer, tag, count, getdata(request));

            }

        }

    }

    file_unlink(FILE_G5);

    return rc;

}

static unsigned int handle(void *reply, struct p9p_event *p9p)
{

    if (p9p_read4(p9p, P9P_OFFSET_SIZE) < P9P_OFFSET_DATA)
        return on9perror(reply, p9p, "Error: Packet is too small\n");

/*
    if (p9p_read4(p9p, P9P_OFFSET_SIZE) > MAXSIZE)
        return on9perror(reply, p9p, "Error: Packet is too big\n");
*/

    switch (p9p_read1(p9p, P9P_OFFSET_TYPE))
    {

    case P9P_TVERSION:
        return on9pversion(reply, p9p);

    case P9P_TATTACH:
        return on9pattach(reply, p9p);

    case P9P_TCLUNK:
        return on9pclunk(reply, p9p);

    case P9P_TWALK:
        return on9pwalk(reply, p9p);

    case P9P_TREAD:
        return on9pread(reply, p9p);

    default:
        channel_sendstring(EVENT_DATA, "Error: Packet has unknown type\n");

        return on9perror(reply, p9p, "Packet has unknown type");

    }

}

static void onp9p(unsigned int source, void *mdata, unsigned int msize)
{

    struct p9p_event *p9p = mdata;
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

            struct p9p_event *p9p = (struct p9p_event *)buffer;
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

