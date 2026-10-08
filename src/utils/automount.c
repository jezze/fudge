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

static unsigned int query(char *field, char *data, unsigned int size)
{

    return system_feed(0, 0, data, size, "mq -query .mounts.%s %s", field, option_getstring("config"));

}

static unsigned int getline(char *data, unsigned int count, unsigned int index, char *out, unsigned int size)
{

    char *line = buffer_tindex(data, count, '\n', index);
    unsigned int length;

    if (!line || line >= data + count)
        return 0;

    length = buffer_findbyte(line, data + count - line, '\n');

    if (length >= size)
        length = size - 1;

    buffer_write(out, size, line, length, 0);

    out[length] = '\0';

    return 1;

}

static void onmain(struct message *message)
{

    char services[1024];
    char indexes[256];
    char partitions[256];
    char names[1024];
    unsigned int nservices = query("service", services, 1024);
    unsigned int nindexes = query("index", indexes, 256);
    unsigned int npartitions = query("partition", partitions, 256);
    unsigned int nnames = query("name", names, 1024);
    char service[64];
    char index[16];
    char partition[16];
    char name[64];
    unsigned int i;

    for (i = 0; getline(services, nservices, i, service, 64) && getline(indexes, nindexes, i, index, 16) && getline(partitions, npartitions, i, partition, 16) && getline(names, nnames, i, name, 64); i++)
        mount(message->source, service, cstring_read_value(index, cstring_length(index), 10), cstring_read_value(partition, cstring_length(partition), 10), name);

}

void init(void)
{

    option_add("config", "initrd:data/config/mount.mq");
    channel_bind(EVENT_MAIN, onmain);

}

