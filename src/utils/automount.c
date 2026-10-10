#include <fudge.h>
#include <abi.h>
#include <disk.h>

static struct event_blockinfo blockinfo;

static unsigned int sendblockreadrequest(unsigned int target, unsigned int offset, unsigned int count)
{

    struct event_blockrequest request;
    struct event_blockresponse response;

    request.offset = offset;
    request.count = count;

    channel_send(1, target, EVENT_BLOCKREADREQUEST, sizeof (struct event_blockrequest), &request);
    channel_wait(1, target, EVENT_BLOCKREADRESPONSE, sizeof (struct event_blockresponse), &response);

    return response.count;

}

static void mountfat(unsigned int source, unsigned int target, unsigned int offset, char *name, char *service, unsigned int index)
{

    sendblockreadrequest(target, offset, 512);

    if (fat_validate((struct fat *)blockinfo.buffer))
        system_run(source, "initrd:bin/fatsrv -service %s -block-service %s:%u -partoffset %u &", name, service, &index, &offset);

}

static void mountext2(unsigned int source, unsigned int target, unsigned int offset, char *name, char *service, unsigned int index)
{

    sendblockreadrequest(target, offset + 1024, 1024);

    if (ext2_validate((struct ext2_superblock *)blockinfo.buffer))
        system_run(source, "initrd:bin/ext2srv -service %s -block-service %s:%u -partoffset %u &", name, service, &index, &offset);

}

static void mountpartition(unsigned int source, unsigned int target, struct mbr_partition *partition, char *name, char *service, unsigned int index)
{

    unsigned int offset = ((partition->sectorlba[3] << 24) | (partition->sectorlba[2] << 16) | (partition->sectorlba[1] << 8) | (partition->sectorlba[0])) * blockinfo.blocksize;

    switch (partition->systemid)
    {

    case 0x83:
        mountfat(source, target, offset, name, service, index);
        mountext2(source, target, offset, name, service, index);

        break;

    case 0x0B:
    case 0x0C:
    case 0xEF:
        mountfat(source, target, offset, name, service, index);

        break;

    }

}

static void mount(unsigned int source, char *service, unsigned int index, unsigned int ipartition, char *name)
{

    unsigned int target = call_find(cstring_length(service), service, index);
    struct mbr mbr;

    if (!target)
    {

        channel_send_fmt(0, source, EVENT_ERROR, "Service not found: %s:%u\n", service, &index);

        return;

    }

    channel_send(1, target, EVENT_INFO, 0, 0);
    channel_wait(1, target, EVENT_BLOCKINFO, sizeof (struct event_blockinfo), &blockinfo);

    if (sendblockreadrequest(target, 0, blockinfo.blocksize) != 512)
        return;

    buffer_copy(&mbr, (void *)blockinfo.buffer, sizeof (struct mbr));

    if (mbr_validate(&mbr) && ipartition < 4 && mbr.partition[ipartition].systemid)
        mountpartition(source, target, &mbr.partition[ipartition], name, service, index);

}

static void onmain(struct message *message)
{

    mount(message->source, option_getstring("service"), option_getdecimal("index"), option_getdecimal("partition"), option_getstring("name"));

}

void init(void)
{

    option_add("service", "block");
    option_add("index", "0");
    option_add("partition", "0");
    option_add("name", "");
    channel_bind(EVENT_MAIN, onmain);

}

