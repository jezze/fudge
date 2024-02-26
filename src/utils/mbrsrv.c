#include <fudge.h>
#include <abi.h>

struct partition
{

    unsigned char boot;
    unsigned char headbase;
    unsigned char sectorbase;
    unsigned char cylinderbase;
    unsigned char systemid;
    unsigned char headlimit;
    unsigned char sectorlimit;
    unsigned char cylinderlimit;
    unsigned char sectorlba[4];
    unsigned char sectortotal[4];

};

struct mbr
{

    unsigned char boot[446];
    struct partition partition[4];
    unsigned char signature[2];

};

static void request_send(unsigned int sector, unsigned int count)
{

    struct event_blockrequest blockrequest;

    blockrequest.sector = sector;
    blockrequest.count = count;

    call_notify(FILE_G5, EVENT_BLOCKREQUEST, sizeof (struct event_blockrequest), &blockrequest);

}

static void request_readblocks(void *buffer, unsigned int count, unsigned int sector, unsigned int nblocks)
{

    unsigned int total = nblocks * 512;
    unsigned int read = 0;
    struct message message;
    char data[MESSAGE_SIZE];

    request_send(sector, nblocks);

    while (channel_poll_any(EVENT_DATA, &message, data))
    {

        read += buffer_write(buffer, count, data, message_datasize(&message), read);

        if (read == total)
            break;

    }

}

static unsigned int isvalid(struct mbr *mbr)
{

    return (mbr->signature[0] == 0x55 && mbr->signature[1] == 0xAA);

}

static void printpartition(struct partition *partition, unsigned int num)
{

    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "Partition %u:\n", &num);

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

        channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "    Boot: 0x%H2c\n", &partition->boot);
        channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "    Id: 0x%H2c\n", &partition->systemid);
        channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "    Start: %u\n", &start);
        channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "    End: %u\n", &end);
        channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "    Sectors: %u\n", &sectors);
        channel_send_fmt3(CHANNEL_DEFAULT, EVENT_DATA, "    Start-C/H/S: %u/%u/%u\n", &cstart, &hstart, &sstart);
        channel_send_fmt3(CHANNEL_DEFAULT, EVENT_DATA, "    End-C/H/S: %u/%u/%u\n", &cend, &hend, &send);

    }

}

static void print(struct mbr *mbr)
{

    unsigned int i;

    channel_send_fmt2(CHANNEL_DEFAULT, EVENT_DATA, "Signature: 0x%H2c%H2c\n", &mbr->signature[0], &mbr->signature[1]);

    for (i = 0; i < 4; i++)
        printpartition(&mbr->partition[i], i);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    if (call_walk_absolute(FILE_G5, option_getstring("volume")))
    {

        unsigned char block[1024];
        struct mbr *mbr = (struct mbr *)block;

        call_link(FILE_G5, 8000);
        request_readblocks(block, 1024, 0, 1);

        if (isvalid(mbr))
            print(mbr);

        call_unlink(FILE_G5);

    }

    else
    {

        channel_send_fmt1(CHANNEL_DEFAULT, EVENT_ERROR, "Volume not found: %s\n", option_getstring("volume"));

    }

}

void init(void)
{

    option_add("volume", "system:block/if.0/data");
    channel_bind(EVENT_MAIN, onmain);

}

