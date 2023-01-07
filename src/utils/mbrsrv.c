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

static unsigned char block[1024];

static void request_send(unsigned int sector, unsigned int count)
{

    struct {struct message_header header; struct event_blockrequest blockrequest;} message;

    message.blockrequest.sector = sector;
    message.blockrequest.count = count;

    message_initheader(&message.header, EVENT_BLOCKREQUEST, sizeof (struct event_blockrequest));
    file_writeall(FILE_G5, &message, message.header.length);

}

static unsigned int isvalid(struct mbr *mbr)
{

    return (mbr->signature[0] == 0x55 && mbr->signature[1] == 0xAA);

}

static void printpartition(struct partition *partition, unsigned int num)
{

    struct message message;

    message_init(&message, EVENT_DATA);
    message_putstring(&message, "Partition ");
    message_putvalue(&message, num, 10, 0);
    message_putstring(&message, ":\n");

    if (partition->systemid)
    {

        unsigned int start = (partition->sectorlba[3] << 24) | (partition->sectorlba[2] << 16) | (partition->sectorlba[1] << 8) | (partition->sectorlba[0]);
        unsigned int sectors = (partition->sectortotal[3] << 24) | (partition->sectortotal[2] << 16) | (partition->sectortotal[1] << 8) | (partition->sectortotal[0]);
        unsigned int cstart = partition->cylinderbase | ((partition->sectorbase & 0xC0) << 8);
        unsigned int cend = partition->cylinderlimit | ((partition->sectorlimit & 0xC0) << 8);
        unsigned int hstart = partition->headbase;
        unsigned int hend = partition->headlimit;
        unsigned int sstart = partition->sectorbase & 0x2F;
        unsigned int send = partition->sectorlimit & 0x2F;

        message_putstring(&message, "    Boot: 0x");
        message_putvalue(&message, partition->boot, 16, 2);
        message_putstring(&message, "\n");
        message_putstring(&message, "    Id: 0x");
        message_putvalue(&message, partition->systemid, 16, 2);
        message_putstring(&message, "\n");
        message_putstring(&message, "    Start: ");
        message_putvalue(&message, start, 10, 0);
        message_putstring(&message, "\n");
        message_putstring(&message, "    End: ");
        message_putvalue(&message, start + sectors - 1, 10, 0);
        message_putstring(&message, "\n");
        message_putstring(&message, "    Sectors: ");
        message_putvalue(&message, sectors, 10, 0);
        message_putstring(&message, "\n");
        message_putstring(&message, "    Start-C/H/S: ");
        message_putvalue(&message, cstart, 10, 0);
        message_putstring(&message, "/");
        message_putvalue(&message, hstart, 10, 0);
        message_putstring(&message, "/");
        message_putvalue(&message, sstart, 10, 0);
        message_putstring(&message, "\n");
        message_putstring(&message, "    End-C/H/S: ");
        message_putvalue(&message, cend, 10, 0);
        message_putstring(&message, "/");
        message_putvalue(&message, hend, 10, 0);
        message_putstring(&message, "/");
        message_putvalue(&message, send, 10, 0);
        message_putstring(&message, "\n");

    }

    channel_sendmessage(&message);

}

static void print(struct mbr *mbr)
{

    struct message message;
    unsigned int i;

    message_init(&message, EVENT_DATA);
    message_putstring(&message, "Signature: 0x");
    message_putvalue(&message, mbr->signature[0], 16, 2);
    message_putvalue(&message, mbr->signature[1], 16, 2);
    message_putstring(&message, "\n");
    channel_sendmessage(&message);

    for (i = 0; i < 4; i++)
        printpartition(&mbr->partition[i], i);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct message message;

    file_walk2(FILE_G5, option_getstring("volume"));
    file_link(FILE_G5);
    request_send(0, 1);

    if (channel_kpollevent(EVENT_DATA, &message))
    {

        struct mbr *mbr = (struct mbr *)block;

        buffer_write(block, 1024, message.data.buffer, message_datasize(&message.header), 0);

        if (isvalid(mbr))
            print(mbr);

    }

    file_unlink(FILE_G5);
    channel_close();

}

void init(void)
{

    option_add("volume", "system:block/if:0/data");
    channel_bind(EVENT_MAIN, onmain);

}

