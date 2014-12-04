#include <module.h>
#include <kernel/resource.h>
#include <system/system.h>
#include <base/base.h>
#include <network/network.h>

static struct network_channel channel;
static struct network_channelnode channelnode;

static unsigned int channel_match(struct network_interface *interface, void *packet, unsigned int count)
{

    return 1;

}

static void channel_notify(struct network_interface *interface, void *packet, unsigned int count)
{

}

static unsigned int channel_rdata(unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int channel_wdata(unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

void init()
{

    network_initchannel(&channel, channel_match, channel_notify, channel_rdata, channel_wdata);
    network_registerchannel(&channel);
    network_initchannelnode(&channelnode, &channel);
    network_registerchannelnode(&channelnode);

}

void destroy()
{

    network_unregisterchannelnode(&channelnode);
    network_unregisterchannel(&channel);

}

