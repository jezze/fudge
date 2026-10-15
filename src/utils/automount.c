#include <fudge.h>
#include <abi.h>
#include <disk.h>

static struct event_blockinfo blockinfo;
static unsigned char gptefi[16] = {0x28, 0x73, 0x2A, 0xC1, 0x1F, 0xF8, 0xD2, 0x11, 0xBA, 0x4B, 0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B};
static unsigned char gptbasic[16] = {0xA2, 0xA0, 0xD0, 0xEB, 0xE5, 0xB9, 0x33, 0x44, 0x87, 0xC0, 0x68, 0xB6, 0xB7, 0x26, 0x99, 0xC7};
static unsigned char gptlinux[16] = {0xAF, 0x3D, 0xC6, 0x0F, 0x83, 0x84, 0x72, 0x47, 0x8E, 0x79, 0x3D, 0x69, 0xD8, 0x47, 0x7D, 0xE4};

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

static void mountmbr(unsigned int source, unsigned int target, struct mbr_partition *partition, char *name, char *service, unsigned int index)
{

    unsigned int offset = ((partition->sectorlba[3] << 24) | (partition->sectorlba[2] << 16) | (partition->sectorlba[1] << 8) | (partition->sectorlba[0])) * blockinfo.blocksize;

    switch (partition->systemid)
    {

    case 0x83:
        mountfat(source, target, offset, name, service, index);
        mountext2(source, target, offset, name, service, index);

        break;

    case 0x01:
    case 0x04:
    case 0x06:
    case 0x0B:
    case 0x0C:
    case 0x0E:
    case 0xEF:
        mountfat(source, target, offset, name, service, index);

        break;

    }

}

static void mountgpt(unsigned int source, unsigned int target, unsigned int ipartition, char *name, char *service, unsigned int index)
{

    struct gpt_header header;
    struct gpt_entry entry;
    unsigned int entryoffset;
    unsigned int offset;

    if (sendblockreadrequest(target, blockinfo.blocksize, blockinfo.blocksize) != blockinfo.blocksize)
        return;

    buffer_copy(&header, (void *)blockinfo.buffer, sizeof (struct gpt_header));

    if (!gpt_validate(&header) || ipartition >= gpt_read32(header.entrycount))
        return;

    entryoffset = gpt_read32(header.entrylba) * blockinfo.blocksize + ipartition * gpt_read32(header.entrysize);

    if (sendblockreadrequest(target, entryoffset - entryoffset % blockinfo.blocksize, blockinfo.blocksize) != blockinfo.blocksize)
        return;

    buffer_copy(&entry, (void *)(blockinfo.buffer + entryoffset % blockinfo.blocksize), sizeof (struct gpt_entry));

    offset = gpt_read32(entry.firstlba) * blockinfo.blocksize;

    if (buffer_match(entry.type, gptlinux, 16) || buffer_match(entry.type, gptefi, 16) || buffer_match(entry.type, gptbasic, 16))
        mountfat(source, target, offset, name, service, index);

    if (buffer_match(entry.type, gptlinux, 16))
        mountext2(source, target, offset, name, service, index);

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

    if (!mbr_validate(&mbr))
        return;

    if (mbr.partition[0].systemid == 0xEE)
        mountgpt(source, target, ipartition, name, service, index);
    else if (ipartition < 4 && mbr.partition[ipartition].systemid)
        mountmbr(source, target, &mbr.partition[ipartition], name, service, index);

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

