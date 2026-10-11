#include <fudge.h>
#include <abi.h>
#include <disk.h>

static struct event_blockinfo blockinfo;

static void printpartition(unsigned int source, struct mbr_partition *partition)
{

    unsigned int start = (partition->sectorlba[3] << 24) | (partition->sectorlba[2] << 16) | (partition->sectorlba[1] << 8) | (partition->sectorlba[0]);
    unsigned int sectors = (partition->sectortotal[3] << 24) | (partition->sectortotal[2] << 16) | (partition->sectortotal[1] << 8) | (partition->sectortotal[0]);
    unsigned int end = start + sectors - 1;
    unsigned int cstart = partition->cylinderbase | ((partition->sectorbase & 0xC0) << 8);
    unsigned int cend = partition->cylinderlimit | ((partition->sectorlimit & 0xC0) << 8);
    unsigned int hstart = partition->headbase;
    unsigned int hend = partition->headlimit;
    unsigned int sstart = partition->sectorbase & 0x3F;
    unsigned int send = partition->sectorlimit & 0x3F;

    channel_send_fmt(0, source, EVENT_DATA, "    Boot: 0x%H2c\n", &partition->boot);
    channel_send_fmt(0, source, EVENT_DATA, "    Id: 0x%H2c\n", &partition->systemid);
    channel_send_fmt(0, source, EVENT_DATA, "    Start: %u\n", &start);
    channel_send_fmt(0, source, EVENT_DATA, "    End: %u\n", &end);
    channel_send_fmt(0, source, EVENT_DATA, "    Sectors: %u\n", &sectors);
    channel_send_fmt(0, source, EVENT_DATA, "    Start-C/H/S: %u/%u/%u\n", &cstart, &hstart, &sstart);
    channel_send_fmt(0, source, EVENT_DATA, "    End-C/H/S: %u/%u/%u\n", &cend, &hend, &send);

}

static unsigned int readblock(unsigned int block, unsigned int offset)
{

    struct event_blockrequest request;
    struct event_blockresponse response;

    request.offset = offset;
    request.count = blockinfo.blocksize;

    channel_send(1, block, EVENT_BLOCKREADREQUEST, sizeof (struct event_blockrequest), &request);
    channel_wait(1, block, EVENT_BLOCKREADRESPONSE, sizeof (struct event_blockresponse), &response);

    return response.count == request.count;

}

static void printentry(unsigned int source, unsigned int i, struct gpt_entry *entry)
{

    unsigned char *t = entry->type;
    unsigned int start = gpt_read32(entry->firstlba);
    unsigned int end = gpt_read32(entry->lastlba);
    unsigned int sectors = end - start + 1;
    char name[37];
    unsigned int j;

    for (j = 0; j < 36 && entry->name[j * 2]; j++)
        name[j] = (entry->name[j * 2 + 1] || entry->name[j * 2] > 0x7E) ? '?' : entry->name[j * 2];

    name[j] = '\0';

    channel_send_fmt(0, source, EVENT_DATA, "Partition %u:\n", &i);
    channel_send_fmt(0, source, EVENT_DATA, "    Type: %H2c%H2c%H2c%H2c-%H2c%H2c-%H2c%H2c-%H2c%H2c-%H2c%H2c%H2c%H2c%H2c%H2c\n", &t[3], &t[2], &t[1], &t[0], &t[5], &t[4], &t[7], &t[6], &t[8], &t[9], &t[10], &t[11], &t[12], &t[13], &t[14], &t[15]);
    channel_send_fmt(0, source, EVENT_DATA, "    Name: %s\n", name);
    channel_send_fmt(0, source, EVENT_DATA, "    Start: %u\n", &start);
    channel_send_fmt(0, source, EVENT_DATA, "    End: %u\n", &end);
    channel_send_fmt(0, source, EVENT_DATA, "    Sectors: %u\n", &sectors);

}

static void printgpt(unsigned int source, unsigned int block)
{

    struct gpt_header header;
    unsigned int offset;
    unsigned int entrysize;
    unsigned int entrycount;
    unsigned int i;

    if (!readblock(block, blockinfo.blocksize))
        return;

    buffer_copy(&header, (void *)blockinfo.buffer, sizeof (struct gpt_header));

    if (!gpt_validate(&header))
        return;

    offset = gpt_read32(header.entrylba) * blockinfo.blocksize;
    entrysize = gpt_read32(header.entrysize);
    entrycount = gpt_read32(header.entrycount);

    channel_send_fmt(0, source, EVENT_DATA, "Table: GPT\n");

    for (i = 0; i < entrycount; i++)
    {

        unsigned int entryoffset = offset + i * entrysize;
        struct gpt_entry *entry;

        if (entryoffset % blockinfo.blocksize == 0 && !readblock(block, entryoffset))
            return;

        entry = (struct gpt_entry *)(blockinfo.buffer + entryoffset % blockinfo.blocksize);

        if (gpt_isused(entry))
            printentry(source, i, entry);

    }

}

static void printmbr(unsigned int source, struct mbr *mbr)
{

    unsigned int i;

    channel_send_fmt(0, source, EVENT_DATA, "Table: MBR\n");

    for (i = 0; i < 4; i++)
    {

        struct mbr_partition *partition = &mbr->partition[i];

        channel_send_fmt(0, source, EVENT_DATA, "Partition %u:\n", &i);

        if (partition->systemid)
            printpartition(source, partition);

    }

}

static void onmain(struct message *message)
{

    unsigned int block = channel_lookup(option_getstring("block-service"));
    struct mbr mbr;

    if (!block)
        return;

    channel_send(1, block, EVENT_INFO, 0, 0);
    channel_wait(1, block, EVENT_BLOCKINFO, sizeof (struct event_blockinfo), &blockinfo);

    if (!readblock(block, 0))
        return;

    buffer_copy(&mbr, (void *)blockinfo.buffer, sizeof (struct mbr));

    if (!mbr_validate(&mbr))
        return;

    channel_send_fmt(0, message->source, EVENT_DATA, "Signature: 0x%H2c%H2c\n", &mbr.signature[0], &mbr.signature[1]);

    if (mbr.partition[0].systemid == 0xEE)
        printgpt(message->source, block);
    else
        printmbr(message->source, &mbr);

}

void init(void)
{

    option_add("block-service", "block");
    channel_bind(EVENT_MAIN, onmain);

}

