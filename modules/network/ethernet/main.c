#include <module.h>
#include <kernel/resource.h>
#include <system/system.h>
#include <base/base.h>
#include <base/network.h>

static struct base_network_channel channel;

static unsigned int channel_match(struct base_network_interface *interface)
{

    return 1;

}

static void channel_notify(struct base_network_interface *interface)
{

}

void init()
{

    base_network_initchannel(&channel, channel_match, channel_notify);
    base_network_registerchannel(&channel);

}

void destroy()
{

    base_network_unregisterchannel(&channel);

}

