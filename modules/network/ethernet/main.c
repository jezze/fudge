#include <module.h>
#include <kernel/resource.h>
#include <system/system.h>
#include <base/base.h>
#include <base/network.h>

static struct base_network_channel channel;
static struct base_network_channelnode channelnode;

static unsigned int channel_match(struct base_network_interface *interface)
{

    return 1;

}

static void channel_notify(struct base_network_interface *interface)
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

    base_network_initchannel(&channel, channel_match, channel_notify, channel_rdata, channel_wdata);
    base_network_registerchannel(&channel);
    base_network_initchannelnode(&channelnode, &channel);
    base_network_registerchannelnode(&channelnode);

}

void destroy()
{

    base_network_unregisterchannelnode(&channelnode);
    base_network_unregisterchannel(&channel);

}

