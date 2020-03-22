#include <fudge.h>
#include <abi.h>

#define BLOCKSIZE 512
#define STATUS_REQUESTING 1
#define STATUS_COMPLETE 2

struct request
{

    unsigned int offset;
    unsigned int count;
    unsigned int blocksector;
    unsigned int blockoffset;
    unsigned int blockcount;
    unsigned int blockreads;
    void *data;

} requests[8];

static char blocks[BLOCKSIZE * 4];
static unsigned int qp;

static unsigned int request_notifydata(struct request *request, void *data, unsigned int size)
{

    unsigned int count = memory_write(request->data, BLOCKSIZE * 4, data, size, request->blockreads * BLOCKSIZE);

    request->blockreads += count / BLOCKSIZE;

    if (request->blockreads == request->blockcount)
        return STATUS_COMPLETE;
    else
        return STATUS_REQUESTING;

}

static void request_init(struct request *request, unsigned int offset, unsigned int count)
{

    request->offset = offset;
    request->count = count;
    request->blocksector = offset / BLOCKSIZE;
    request->blockoffset = offset % BLOCKSIZE;
    request->blockcount = ((BLOCKSIZE - 1) + request->blockoffset + request->count) / BLOCKSIZE;
    request->blockreads = 0;
    request->data = blocks;

}

static void sendblockrequest(struct channel *channel, unsigned int sector, unsigned int count)
{

    struct event_blockrequest blockrequest;

    blockrequest.sector = sector;
    blockrequest.count = count;

    channel_request(channel, EVENT_BLOCKREQUEST);
    channel_append(channel, sizeof (struct event_blockrequest), &blockrequest);
    channel_write(channel, FILE_G0);

}

static void createrequest(struct channel *channel, struct request *request, unsigned int offset)
{

    request_init(request, offset, sizeof (struct cpio_header) + 1024);
    sendblockrequest(channel, request->blocksector, request->blockcount);

}

static void walk(struct channel *channel, unsigned int source, char *path)
{

    struct request *request = &requests[qp];
    struct ipc_header iheader;
    char data[FUDGE_BSIZE];
    unsigned int offset = 0;
    unsigned int length = ascii_length(path) + 1;

    createrequest(channel, request, offset);

    while (channel_apoll(channel, &iheader, data, EVENT_DATA))
    {

        unsigned int status = request_notifydata(request, data, ipc_datasize(&iheader));

        if (status == STATUS_COMPLETE)
        {

            struct cpio_header *header = (struct cpio_header *)((char *)request->data + request->blockoffset);

            if (cpio_validate(header))
            {

                if (header->namesize == 11)
                {

                    if (memory_match("TRAILER!!!", header + 1, 11))
                        return;

                }

                if (header->namesize == length)
                {

                    if (memory_match(path, header + 1, length))
                    {

                        channel_request(channel, EVENT_DATA);
                        channel_append(channel, header->namesize - 1, header + 1);
                        channel_appendstring(channel, "\n");
                        channel_place(channel, source);

                        return;

                    }

                }

                offset = cpio_next(header, request->offset);

                createrequest(channel, request, offset);

            }

        }

    }

}

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    walk(channel, source, "build/bin/hello");
    channel_close(channel);

}

static void onredirect(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_redirect *redirect = mdata;

    channel_setredirect(channel, redirect->type, redirect->mode, redirect->id, source);

}

static void oninit(struct channel *channel)
{

    if (!file_walk2(FILE_G0, "/system/block/if:0/data"))
        return;

    file_open(FILE_G0);

}

static void onexit(struct channel *channel)
{

    file_close(FILE_G0);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_MAIN, onmain);
    channel_setsignal(&channel, EVENT_REDIRECT, onredirect);
    channel_listen2(&channel, oninit, onexit);

}

