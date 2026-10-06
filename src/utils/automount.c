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

        channel_send(1, target, EVENT_BLOCKREADREQUEST, sizeof (struct event_blockrequest), &request);
        channel_wait(1, target, EVENT_BLOCKREADRESPONSE, sizeof (struct event_blockresponse), &response);

        return response.count;

    }

    return 0;

}

static void startservice(unsigned int source, char *program, char *service, unsigned int offset)
{

    char line[128];

    cstring_write_fmt3(line, 128, 0, "%s -service %s -partoffset %u &\\0", program, service, &offset);
    system_run(source, line);

}

static void mountfat(unsigned int source, unsigned int offset, char *service)
{

    struct fat *fat = (struct fat *)blockinfo.buffer;

    sendblockreadrequest(offset, 512);

    if (fat_validate(fat))
        startservice(source, "initrd:bin/fatsrv", service, offset);

}

static void mountext2(unsigned int source, unsigned int offset, char *service)
{

    struct ext2_superblock *sb = (struct ext2_superblock *)blockinfo.buffer;

    sendblockreadrequest(offset + 1024, 1024);

    if (ext2_validate(sb))
        startservice(source, "initrd:bin/ext2srv", service, offset);

}

static void mountpartition(unsigned int source, struct mbr_partition *partition, char *service)
{

    unsigned int start = (partition->sectorlba[3] << 24) | (partition->sectorlba[2] << 16) | (partition->sectorlba[1] << 8) | (partition->sectorlba[0]);

    switch (partition->systemid)
    {

    case 0x83:
        mountfat(source, start * blockinfo.blocksize, service);
        mountext2(source, start * blockinfo.blocksize, service);

        break;

    case 0xEF:
        mountfat(source, start * blockinfo.blocksize, service);

        break;

    }

}

static void onmain(struct message *message)
{

    unsigned int block = channel_lookup(option_getstring("block-service"));
    char *service[4] = {
        "efi",
        "boot",
        "root",
        "home"
    };

    if (block)
    {

        unsigned int count;
        channel_send(1, block, EVENT_INFO, 0, 0);
        channel_wait(1, block, EVENT_BLOCKINFO, sizeof (struct event_blockinfo), &blockinfo);

        count = sendblockreadrequest(0, blockinfo.blocksize);

        if (count == 512)
        {

            struct mbr mbr;

            buffer_copy(&mbr, (void *)blockinfo.buffer, sizeof (struct mbr));

            if (mbr_validate(&mbr))
            {

                unsigned int i;

                for (i = 0; i < 4; i++)
                {

                    struct mbr_partition *partition = &mbr.partition[i];

                    if (partition->systemid)
                        mountpartition(message->source, partition, service[i]);

                }

            }

        }

    }

}

void init(void)
{

    option_add("block-service", "block");
    channel_bind(EVENT_MAIN, onmain);

}

