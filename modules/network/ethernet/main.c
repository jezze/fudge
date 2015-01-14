#include <module.h>
#include <net/ethernet.h>
#include <kernel/resource.h>
#include <kernel/scheduler.h>
#include <system/system.h>
#include <base/base.h>
#include <network/network.h>

static struct network_channel channel;
static unsigned char buffer[4096];
static struct buffer cfifo;
static struct scheduler_rendezvous rdata;

static unsigned int channel_match(struct network_interface *interface, void *packet, unsigned int count)
{

    return 1;

}

static void channel_notify(struct network_interface *interface, void *packet, unsigned int count)
{

    struct ethernet_header *header = packet;

    if (header->type[0] == 0x08)
    {

        count = buffer_wcfifo(&cfifo, count, packet);
        scheduler_rendezvous_unsleep(&rdata);

    }

}

static unsigned int channel_rdata(unsigned int offset, unsigned int count, void *buffer)
{

    count = buffer_rcfifo(&cfifo, count, buffer);

    if (!count)
        scheduler_rendezvous_sleep(&rdata);

    return count;

}

static unsigned int channel_wdata(unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

void init()
{

    buffer_init(&cfifo, 1, 4096, buffer);
    network_initchannel(&channel, channel_match, channel_notify, channel_rdata, channel_wdata);
    network_registerchannel(&channel);

}

void destroy()
{

    network_unregisterchannel(&channel);

}

