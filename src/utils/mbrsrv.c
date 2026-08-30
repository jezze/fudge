#include <fudge.h>
#include <abi.h>
#include <disk.h>

static unsigned int readblocks(unsigned int block, void *buffer, unsigned int count, unsigned int sector, unsigned int nblocks)
{

    struct event_blockrequest blockrequest;
    unsigned int c = 0;
    unsigned int i;

    blockrequest.sector = sector;
    blockrequest.count = count;

    channel_send(0, block, EVENT_BLOCKREQUEST, sizeof (struct event_blockrequest), &blockrequest);

    for (i = 0; i < nblocks; i++)
    {

        struct message message;

        channel_poll(0, block, EVENT_BLOCKRESPONSE, &message);

        c += buffer_read(buffer, count, message_data(&message, 0), message.length, i * 512);

    }

    return c;

}

static void print(unsigned int source, struct mbr *mbr)
{

    unsigned int i;

    channel_send_fmt2(0, source, EVENT_DATA, "Signature: 0x%H2c%H2c\n", &mbr->signature[0], &mbr->signature[1]);

    for (i = 0; i < 4; i++)
    {

        struct mbr_partition *partition = &mbr->partition[i];

        channel_send_fmt1(0, source, EVENT_DATA, "Partition %u:\n", &i);

        if (partition->systemid)
        {

            unsigned int start = (partition->sectorlba[3] << 24) | (partition->sectorlba[2] << 16) | (partition->sectorlba[1] << 8) | (partition->sectorlba[0]);
            unsigned int sectors = (partition->sectortotal[3] << 24) | (partition->sectortotal[2] << 16) | (partition->sectortotal[1] << 8) | (partition->sectortotal[0]);
            unsigned int end = start + sectors - 1;
            unsigned int cstart = partition->cylinderbase | ((partition->sectorbase & 0xC0) << 8);
            unsigned int cend = partition->cylinderlimit | ((partition->sectorlimit & 0xC0) << 8);
            unsigned int hstart = partition->headbase;
            unsigned int hend = partition->headlimit;
            unsigned int sstart = partition->sectorbase & 0x2F;
            unsigned int send = partition->sectorlimit & 0x2F;

            channel_send_fmt1(0, source, EVENT_DATA, "    Boot: 0x%H2c\n", &partition->boot);
            channel_send_fmt1(0, source, EVENT_DATA, "    Id: 0x%H2c\n", &partition->systemid);
            channel_send_fmt1(0, source, EVENT_DATA, "    Start: %u\n", &start);
            channel_send_fmt1(0, source, EVENT_DATA, "    End: %u\n", &end);
            channel_send_fmt1(0, source, EVENT_DATA, "    Sectors: %u\n", &sectors);
            channel_send_fmt3(0, source, EVENT_DATA, "    Start-C/H/S: %u/%u/%u\n", &cstart, &hstart, &sstart);
            channel_send_fmt3(0, source, EVENT_DATA, "    End-C/H/S: %u/%u/%u\n", &cend, &hend, &send);

        }

    }

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int block = channel_lookup(option_getstring("block-service"));

    if (block)
    {

        unsigned char blockdata[512];
        struct mbr *mbr = (struct mbr *)blockdata;
        unsigned int count;

        channel_send(0, block, EVENT_LINK, 0, 0);

        count = readblocks(block, blockdata, 512, 0, 1);

        if (count == 512 && mbr_validate(mbr))
            print(source, mbr);

        channel_send(0, block, EVENT_UNLINK, 0, 0);

    }

}

void init(void)
{

    option_add("block-service", "block");
    channel_bind(EVENT_MAIN, onmain);

    while (channel_process(0));

}

