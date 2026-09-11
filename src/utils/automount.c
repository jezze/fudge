#include <fudge.h>
#include <abi.h>
#include <disk.h>

static struct event_blockinfo blockinfo;

static unsigned int sendblockreadrequest(unsigned int offset, unsigned int count)
{

    unsigned int target = channel_lookup(option_getstring("block-service"));

    if (target)
    {

        struct event_blockrequest request;
        struct event_blockresponse response;

        request.offset = offset;
        request.count = count;

        channel_send(0, target, EVENT_BLOCKREADREQUEST, sizeof (struct event_blockrequest), &request);
        channel_wait(0, target, EVENT_BLOCKREADRESPONSE, sizeof (struct event_blockresponse), &response);

        return response.count;

    }

    return 0;

}

static void mountext2(unsigned int source, unsigned int offset)
{

    struct ext2_superblock *sb = (struct ext2_superblock *)blockinfo.buffer;

    sendblockreadrequest(offset + 1024, 1024);

    if (ext2_validate(sb))
    {

        unsigned int target = fs_spawn(1, "initrd:bin/ext2srv");

        if (target)
        {

            channel_send_fmt1(1, target, EVENT_OPTION, "service=fd0&partoffset=%u\n", &offset);
            channel_send(1, target, EVENT_MAIN, 0, 0);

        }

    }

}

static void mountpartition(unsigned int source, struct mbr_partition *partition)
{

    unsigned int start = (partition->sectorlba[3] << 24) | (partition->sectorlba[2] << 16) | (partition->sectorlba[1] << 8) | (partition->sectorlba[0]);

    if (partition->systemid == 0x83)
    {

        mountext2(source, start * blockinfo.blocksize);

    }

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int block = channel_lookup(option_getstring("block-service"));

    if (block)
    {

        unsigned int count;
        channel_send(0, block, EVENT_INFO, 0, 0);
        channel_wait(0, block, EVENT_BLOCKINFO, sizeof (struct event_blockinfo), &blockinfo);

        count = sendblockreadrequest(0, blockinfo.blocksize);

        if (count == 512)
        {

            struct mbr *mbr = (struct mbr *)blockinfo.buffer;

            if (mbr_validate(mbr))
            {

                unsigned int i;

                for (i = 0; i < 4; i++)
                {

                    struct mbr_partition *partition = &mbr->partition[i];

                    if (partition->systemid)
                        mountpartition(source, partition);

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

