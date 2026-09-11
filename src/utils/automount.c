#include <fudge.h>
#include <abi.h>
#include <disk.h>

static struct event_blockinfo blockinfo;

static void mountpartition(struct mbr_partition *partition)
{

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int block = channel_lookup(option_getstring("block-service"));

    if (block)
    {

        struct event_blockrequest request;
        struct event_blockresponse response;

        channel_send(0, block, EVENT_INFO, 0, 0);
        channel_wait(0, block, EVENT_BLOCKINFO, sizeof (struct event_blockinfo), &blockinfo);

        request.offset = 0;
        request.count = blockinfo.blocksize;

        channel_send(0, block, EVENT_BLOCKREADREQUEST, sizeof (struct event_blockrequest), &request);
        channel_wait(0, block, EVENT_BLOCKREADRESPONSE, sizeof (struct event_blockresponse), &response);

        if (response.count == request.count)
        {

            struct mbr *mbr = (struct mbr *)blockinfo.buffer;

            if (mbr_validate(mbr))
            {

                unsigned int i;

                for (i = 0; i < 4; i++)
                {

                    struct mbr_partition *partition = &mbr->partition[i];

                    if (partition->systemid)
                        mountpartition(partition);

                }

            }

        }

    }

}

void init(void)
{

    option_add("block-service", "block");
    channel_bind(EVENT_MAIN, onmain);

    while (channel_process(0));

}

